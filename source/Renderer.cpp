//External includes
#include "SDL.h"
#include "SDL_surface.h"

#include <thread>
#include <future> // Async Stuff
#include <ppl.h> // Parallel Stuff

#include "Timer.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <iostream>

using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = m_Width / static_cast<float>(m_Height);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	camera.CalculateCameraToWorld();

	// The number of pixels that are going to be shown
	const unsigned int nrPixels{ static_cast<unsigned int>(m_Width * m_Height) };

#if defined(ASYNC)
	// Async Logic
	const unsigned int nrCores{ std::thread::hardware_concurrency() };
	std::vector<std::future<void>> asyncFutures{};

	const unsigned int nrPixelsPerTask{ nrPixels / nrCores };
	unsigned int nrUnassignedPixels{ nrPixels % nrCores };
	unsigned int curPixelIdx{};

	for (unsigned int coreIdx{}; coreIdx < nrCores; ++coreIdx)
	{
		unsigned int taskSize{ nrPixelsPerTask };
		if (nrUnassignedPixels > 0)
		{
			++taskSize;
			--nrUnassignedPixels;
		}

		asyncFutures.push_back(
			std::async(std::launch::async, [=, this]
				{
					const unsigned int endPixelIdx{ curPixelIdx + taskSize };
					for (unsigned int pixelIdx{ curPixelIdx }; pixelIdx < endPixelIdx; ++pixelIdx)
					{
						RenderPixel(pScene, pixelIdx, camera, lights, materials);
					}
				})
		);

		curPixelIdx += taskSize;
	}

	for (const std::future<void>& f : asyncFutures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	// Parallel For Logic
	concurrency::parallel_for(0u, nrPixels,
		[=, this](int i)
		{
			RenderPixel(pScene, i, camera, lights, materials);
		});
#else
	// Synchronous Logic
	for (unsigned int i{}; i < nrPixels; ++i)
	{
		RenderPixel(pScene, i, camera, lights, materials);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, unsigned int pixelIndex, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	// Calculate the row and column from pixelIndex
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	// Calculate the raster cordinates in camera space
	const float cx{ ((2.0f * (px + 0.5f) / m_Width - 1.0f) * m_AspectRatio) * camera.fovMultiplier };
	const float cy{ (1.0f - 2.0f * (py + 0.5f) / m_Height) * camera.fovMultiplier };

	// Calculate the direction from the camera to the raster
	Vector3 rayDirection{ cx,cy,1 };
	rayDirection.Normalize();

	rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	// RAYCALCS ^
	Ray viewRay(camera.origin, rayDirection);

	// Color to write to the color buffer (default is black)
	ColorRGB finalColor{};

	// Hitrecord containing more information about a potential hit
	HitRecord closestHit{};
	// Get the closest object that interesected with the ray
	pScene->GetClosestHit(viewRay, closestHit);

	// If we hit anything 
	if (closestHit.didHit)
	{
		// Go over all Lights
		for (size_t i{ 0 }; i < lights.size(); ++i)
		{
			// Get a ray from the point we hit, to the light and add a small offset
			Vector3 lightDir = LightUtils::GetDirectionToLight(lights[i], closestHit.origin + (closestHit.normal * 0.001f));
			const float lightrayMagnitude{ lightDir.Normalize() };
			if (m_ShadowsEnabled)
			{
				Ray lightRay{ closestHit.origin + (closestHit.normal * 0.001f),lightDir };
				lightRay.max = lightrayMagnitude;
				// If we hit something in the scene from the point we hit towards the light
				// it means there is an  object obstructing the ray
				// this means we are at a shadow
				if (pScene->DoesHit(lightRay))
				{
					continue;
				}
			}

			float observedArea = Vector3::DotClamp(lightDir, closestHit.normal);
			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				if (observedArea > 0)
				{
					finalColor += ColorRGB{ 1,1,1 } * observedArea;
				}
				break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin);
				break;
			case LightingMode::BRDF:
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDir, viewRay.direction);
				break;
			case LightingMode::Combined:
				if (observedArea > 0)
				{
					finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * observedArea * materials[closestHit.materialIndex]->Shade(closestHit, lightDir, viewRay.direction);
				}
				break;
			}
		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

void dae::Renderer::Update(dae::Timer* pTimer)
{
	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	if (pKeyboardState[SDL_SCANCODE_F2])
	{
		if (!m_F2Held) ToggleShadows();
		m_F2Held = true;
	}
	else m_F2Held = false;
	if (pKeyboardState[SDL_SCANCODE_F3])
	{
		if (!m_F3Held) CycleLightingMode();
		m_F3Held = true;
	}
	else m_F3Held = false;
	if (pKeyboardState[SDL_SCANCODE_F6])
	{
		if (!m_F6Held) pTimer->StartBenchmark();
		m_F6Held = true;
	}
	else m_F6Held = false;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	m_CurrentLightingMode = static_cast<LightingMode>((static_cast<int>(m_CurrentLightingMode) + 1) % 4);
	std::cout << "Current Mode: ";
	switch (m_CurrentLightingMode)
	{
	case LightingMode::BRDF:
		std::cout << "BRDF";
		break;
	case LightingMode::ObservedArea:
		std::cout << "ObservedArea";
		break;
	case LightingMode::Radiance:
		std::cout << "Radiance";
		break;
	case LightingMode::Combined:
		std::cout << "Combined";
		break;
	}
	std::cout << '\n';
}
