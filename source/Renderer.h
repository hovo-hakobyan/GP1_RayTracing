#pragma once

#include <cstdint>
#include <vector>
struct SDL_Window;
struct SDL_Surface;



namespace dae
{
	class Scene;
	struct Camera;
	struct Light;
	class Material;

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

		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const;

		bool SaveBufferToImage() const;

		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }
		void CycleLightingMode();

	private:
		enum class LightingMode
		{
			ObservedArea,	//Lambert Cosine Law
			Radiance,		//Incident Radiance
			BRDF,			//Scattering of the light
			Combined		
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };

		SDL_Window* m_pWindow{};
		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};
		int m_Width{};
		int m_Height{};
	};
}
