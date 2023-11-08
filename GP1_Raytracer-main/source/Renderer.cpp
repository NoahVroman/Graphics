//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <execution>

#define PARALLEL_EXECUTION

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	const float aspectRatio = m_Width / static_cast<float>(m_Height);

	const float fovAngle = camera.fovAngle * TO_RADIANS;
	const float fov = tan(fovAngle / 2.f);

	
#ifdef PARALLEL_EXECUTION

	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);
	for (size_t index{}; index < amountOfPixels; ++index) pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin);
	});





#else
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			float pxc = px + 0.5f;
			float pyc = py + 0.5f;

			Vector3 rayDirection;
			rayDirection.x = ((((2.f * pxc) / m_Width) - 1.f) * ar * camera.halfFovTan);
			rayDirection.y = (1.f - ((2.f * pyc) / m_Height))* camera.halfFovTan;
			rayDirection.z = 1;
			rayDirection.Normalize();

			rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();

			Ray viewRay{camera.origin,rayDirection };
			ColorRGB finalColor{ };
			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);

			rayDirection = -rayDirection;
			if (closestHit.didHit)
			{
				

				for (auto& light : lights)
				{
					Vector3 LightRayDirection= LightUtils::GetDirectionToLight(light, closestHit.origin);
					LightRayDirection.Normalize();
					Vector3 lightDirection = light.origin - closestHit.origin;
					Vector3 lightRayOrigin = closestHit.origin + closestHit.normal * 0.0000001f;

					Ray lightRay{ lightRayOrigin,lightDirection.Normalized()};
					lightRay.max = lightDirection.Magnitude();

					if (m_ShadowEnabled && pScene->DoesHit(lightRay))
					{
						continue;
					}
					

					auto dot = Vector3::Dot(closestHit.normal, LightRayDirection);
					if (dot >= 0)
					{
						finalColor += LightUtils::GetRadiance(light, closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit,LightRayDirection,rayDirection) * dot;
					}

				}

			}

			//Update Color in Buffer;
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}


#endif 




	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	auto& lights = pScene->GetLights();
	auto& materials{ pScene->GetMaterials() };

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	float rx{ px + 0.5f }, ry{ py + 0.5f };
	float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
	float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };
	

	Vector3 rayDirection;
	rayDirection.x = cx;
	rayDirection.y = cy;
	rayDirection.z = 1;
	rayDirection.Normalize();

	rayDirection = cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	Ray viewRay{cameraOrigin,rayDirection };
	ColorRGB finalColor{ };
	HitRecord closestHit{};

	pScene->GetClosestHit(viewRay, closestHit);

	rayDirection = -rayDirection;
	if (closestHit.didHit)
	{

		for (int index = 0; index < lights.size(); ++index)
		{
			Vector3 LightRayDirection = LightUtils::GetDirectionToLight(lights[index], closestHit.origin);
		    viewRay.max = LightRayDirection.Normalize();
			viewRay.origin = closestHit.origin + closestHit.normal * 0.0000001f;
			viewRay.direction = LightRayDirection;

			if (m_ShadowEnabled && pScene->DoesHit(viewRay))
			{
					continue;
			}

			switch (m_LightMode)
			{
			case dae::Renderer::LightMode::observed:
			{
				auto dot = std::max(Vector3::Dot(closestHit.normal, LightRayDirection),0.0f);

				if (dot < 0)
				{
					finalColor = colors::Black;
				}

				finalColor += ColorRGB{ dot,dot,dot };

			}
				break;
			case dae::Renderer::LightMode::radiance:
			{
				finalColor += LightUtils::GetRadiance(lights[index], closestHit.origin);

			}
				break;
			case dae::Renderer::LightMode::bdrf:
			{
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, LightRayDirection, rayDirection);

			}
				break;
			case dae::Renderer::LightMode::combined:
			{
				auto dot = std::max(Vector3::Dot(closestHit.normal, LightRayDirection), 0.0f);

				if (dot < 0)
				{
					finalColor = colors::Black;
				}

				finalColor += LightUtils::GetRadiance(lights[index], closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, LightRayDirection, rayDirection) * dot;

			}
			
				break;
			}

		}

	}

	//Update Color in Buffer;
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::ToggleShadows()
{
	m_ShadowEnabled = !m_ShadowEnabled;
}

void dae::Renderer::ToggleLightMode()
{
	m_LightMode = static_cast<LightMode>((static_cast<int>(m_LightMode) + 1) % 4);

}