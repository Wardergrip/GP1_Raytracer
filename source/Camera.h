#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fovMultiplier{ 1.0f };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			/*
			- This function should return the Camera ONB matrix
			- Calculate the right & up vector using the forward camera vector
			- Combine to a matrix(also include origin) and return
			*/

			right = Vector3::Cross(Vector3::UnitY,forward);
			up = Vector3::Cross(forward,right);
			// https://gamedev.net/forums/topic/388559-getting-a-up-vector-from-only-having-a-forward-vector/.

			cameraToWorld =
			{
				right,
				up,
				forward,
				origin
			};
			

			return cameraToWorld;
		}

		void SetFovAngle(float newFovAngle)
		{
			fovAngle = newFovAngle;
			fovMultiplier = tanf(TO_RADIANS * newFovAngle / 2.0f);
		}

		void Update(Timer* pTimer)
		{
			const float fovChangingSpeed{ 25 };
			const float baseMovementSpeed{ 0.5f };
			float movementSpeed{ baseMovementSpeed };
			const float rotationSpeed{ 1/32.f };

			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

#pragma region Shift
			if (pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT])
			{
				movementSpeed *= 4;
			}
#pragma endregion 

#pragma region FovControls
			/*if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				if (fovAngle > 10)
				{
					fovAngle -= fovChangingSpeed * deltaTime;
				}
			}
			else if (pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				if (fovAngle < 170)
				{
					fovAngle += fovChangingSpeed * deltaTime;
				}
			}*/
#pragma endregion 

#pragma region KeyboardOnly Controls
			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * movementSpeed;
			}
			else if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin -= forward * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * movementSpeed;
			}
			else if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin -= right * movementSpeed;
			}
#pragma endregion 

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

#pragma region Mousebased Movement
			// LMB
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				// RMB
				if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					origin += right * (-mouseY * movementSpeed * deltaTime);
				}
				else
				{
					origin += forward * (-mouseY * movementSpeed * deltaTime);
				}
			}
#pragma endregion

#pragma region Rotation
			if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				forward = Matrix::CreateRotationY(mouseX * rotationSpeed).TransformVector(forward);
				forward = Matrix::CreateRotationX(-mouseY * rotationSpeed).TransformVector(forward) ;
			}
#pragma endregion

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
