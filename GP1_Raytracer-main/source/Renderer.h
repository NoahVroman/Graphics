#pragma once

#include <cstdint>
#include "Math.h"
struct SDL_Window;
struct SDL_Surface;
struct Vector3;


namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;


		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;

		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin)const;

		bool SaveBufferToImage() const;

		void ToggleShadows();
		void ToggleLightMode();


	private:
		SDL_Window* m_pWindow{};

		enum class LightMode
		{
			observed,
			radiance,
			bdrf,
			combined

		};

		LightMode m_LightMode{ LightMode::combined };

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		bool m_ShadowEnabled{ true };

		int m_Width{};
		int m_Height{};
	};
}
