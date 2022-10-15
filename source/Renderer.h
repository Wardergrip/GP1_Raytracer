#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;
	class Timer;

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
		void Update(dae::Timer* pTimer);
		bool SaveBufferToImage() const;

		void CycleLightingMode();
		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }

	private:

		enum class LightingMode
		{
			ObservedArea, // Lambert Cosine
			Radiance,	  // Incident Radiance
			BRDF,		  // Scattering of the light
			Combined	  // ObservedArea * Radiance * BRDF
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };

		bool m_F2Held{ false };
		bool m_F3Held{ false };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
	};
}
