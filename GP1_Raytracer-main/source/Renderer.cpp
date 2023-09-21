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

	const float ar =  static_cast<float>(m_Width) / static_cast<float>(m_Height);
	
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
			rayDirection.y = (1.f - ((2.f * pyc) / m_Height));
			rayDirection.x = ((((2.f * pxc) / m_Width) - 1.f) * ar);
			rayDirection.z = 1;
			rayDirection.Normalize();

			Ray viewRay{{0,0,0},rayDirection };
			ColorRGB finalColor{ };
			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);

			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				//const float scaled_t = closestHit.t / 500.0f;

				//finalColor = { scaled_t,scaled_t ,scaled_t };
				finalColor = materials[closestHit.materialIndex]->Shade();
			}

			//Update Color in Buffer;
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
