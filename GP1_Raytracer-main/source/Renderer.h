#pragma once

#include <cstdint>
#include <vector>
struct SDL_Window;
struct SDL_Surface;


namespace dae
{
	class Scene;
	class Material;
	
	struct Matrix;
	struct Vector3;
	struct Light;

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

		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin, const std::vector<dae::Material*>& materials, const std::vector<dae::Light>& lights)const;

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

		std::vector<uint32_t> m_PixelIndeces{};
		uint32_t m_NrPixels{};
	};
}
