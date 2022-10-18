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
#include <iostream>

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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
#pragma region OriginalGradient
#if 0
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ gradient, gradient, gradient };
#endif
#pragma endregion
			// Cameraspace
			float screenWidth{ static_cast<float>(m_Width) }, screenHeight{ static_cast<float>(m_Height) };
			float aspectRatio{ screenWidth / screenHeight };
			float fov{ tan(((camera.fovAngle * TO_RADIANS))/2.f) };
			Vector3 cameraOrigin{ camera.origin };
			float calculationX = ((2 * ((px + 0.5f) / screenWidth) - 1) * aspectRatio * fov);
			float calculationY = (1 - (2 * (py + 0.5f) / screenHeight)) * fov;

			Vector3 rayDirection{calculationX,calculationY,1};
			rayDirection.Normalize();

			rayDirection = cameraToWorld.TransformVector(rayDirection);

			// RAYCALCS ^
			Ray viewRay(cameraOrigin, rayDirection);

			// Color to write to the color buffer (default is black)
			ColorRGB finalColor{};

			// Hitrecord containing more information about a potential hit
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				for (size_t i{0}; i < lights.size(); ++i)
				{
					Vector3 lightDir = LightUtils::GetDirectionToLight(lights[i],closestHit.origin + (closestHit.normal * 0.001f));
					const float lightrayMagnitude{ lightDir.Normalize() };
					if (m_ShadowsEnabled)
					{
						Ray lightRay{closestHit.origin + (closestHit.normal * 0.001f),lightDir};
						lightRay.max = lightrayMagnitude;
						if (pScene->DoesHit(lightRay))
						{
							//finalColor *= 0.5f;
							continue;
						}
					}

					switch (m_CurrentLightingMode)
					{
					case LightingMode::ObservedArea:
					{
						float observedArea = Vector3::Dot(lightDir, closestHit.normal);
						if (observedArea > 0)
						{
							finalColor += ColorRGB{ 1,1,1 } * observedArea;
						}
					}
						break;
					case LightingMode::Radiance:
						finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin);
						break;
					case LightingMode::BRDF:
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit,lightDir,viewRay.direction);
						break;
					case LightingMode::Combined:
					{
						float observedArea = Vector3::Dot(lightDir, closestHit.normal);
						if (observedArea > 0)
						{
							finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin) * observedArea * materials[closestHit.materialIndex]->Shade(closestHit, lightDir, viewRay.direction);
						}
						
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
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
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
