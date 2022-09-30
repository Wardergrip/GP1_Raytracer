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
			/*Plane testPlane{ {0,-50,0},{0,1,0},0 };
			GeometryUtils::HitTest_Plane(testPlane, viewRay, closestHit);*/
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				for (size_t i{0}; i < lights.size(); ++i)
				{
					Vector3 lightDir = LightUtils::GetDirectionToLight(lights[i],closestHit.origin + (closestHit.normal * 0.1f));
					const float lightrayMagnitude{ lightDir.Normalize() };
					Ray lightRay{lights[i].origin,lightDir};
					if (lightrayMagnitude > lightRay.max)
					{
						
					}
					else if (pScene->DoesHit(lightRay))
					{
						finalColor *= 0.5f;
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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
