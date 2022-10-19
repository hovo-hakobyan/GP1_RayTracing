#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

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

		LightingMode m_CurrentLightingMode{ LightingMode::ObservedArea };
		bool m_ShadowsEnabled{ true };

		SDL_Window* m_pWindow{};
		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};
		int m_Width{};
		int m_Height{};
	};
}
