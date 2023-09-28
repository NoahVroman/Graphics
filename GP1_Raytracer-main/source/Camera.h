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

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
			SetFOV(fovAngle);
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float halfFovTan{ 0.0f };


		void SetFOV(float newfov) 
		{
			fovAngle = newfov;
			halfFovTan = tanf(fovAngle * 0.5f);

		}


		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			Matrix translationMatrix = Matrix::CreateTranslation(origin);

			Matrix rotationMatrix = Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);

			cameraToWorld = rotationMatrix * translationMatrix;

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			int8_t xDirection = pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A];
			int8_t zDirection = pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S];


			constexpr float speed{ 10.0f };
			constexpr float rotationSpeed{ 0.1f };

			Vector3 localForward = cameraToWorld.TransformVector(forward);
			localForward.Normalize();

			Vector3 localRight = Vector3::Cross(up, localForward);

			origin += localForward * zDirection;
			origin += localRight * xDirection;

			if (SDL_BUTTON(mouseState) == SDL_BUTTON_LEFT)
			{
				totalPitch += mouseY * rotationSpeed * pTimer->GetElapsed();				
				totalYaw += mouseX * rotationSpeed * pTimer->GetElapsed();

			}


			if (xDirection != 0 || zDirection != 0 || (SDL_BUTTON(mouseState) == SDL_BUTTON_LEFT && (mouseX > 0.f || mouseY > 0.0f)))
			{
				CalculateCameraToWorld();
			}


		}
	};
}
