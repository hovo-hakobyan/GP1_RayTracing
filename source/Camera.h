#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <iostream>
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

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			Matrix rot{ Matrix::CreateRotation(Vector3(totalPitch, totalYaw,0)) };
			//Calculates the current forward vector by applying the rotation to it
			forward = rot.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			right = Vector3::Cross(Vector3::UnitY, forward);
			right.Normalize();
			up = Vector3::Cross(forward, right);
			up.Normalize();
		

			return {right,up,forward,origin};
		}
		
		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float moveSpeed{ 10.f };
			const float rotSpeed{ 4.f };
			bool hasRotated{};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin.z += moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin.z -= moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin.x -= moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin.x += moveSpeed * deltaTime;
			}



			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			
			if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) && mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				if (mouseY > 0)
				{
					origin.y -= moveSpeed * deltaTime;
				}
				if (mouseY < 0)
				{
					origin.y += moveSpeed * deltaTime;
				}
				if (mouseX > 0)
				{
					origin.x += moveSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					origin.x -= moveSpeed * deltaTime;
				}
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				if (mouseY > 0)
				{
					origin.z -= moveSpeed * deltaTime;
				}
				if (mouseY < 0)
				{
					origin.z += moveSpeed * deltaTime;
				}
				if (mouseX < 0)
				{
					totalYaw -= rotSpeed * deltaTime;
					hasRotated = true;
					
				}
				if (mouseX > 0)
				{
					totalYaw += rotSpeed * deltaTime;
					hasRotated = true;
				}
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				if (mouseX < 0)
				{
					totalYaw -= rotSpeed * deltaTime;
					hasRotated = true;
				}
				if (mouseX > 0)
				{
					totalYaw += rotSpeed * deltaTime;
					hasRotated = true;
				}
				if (mouseY > 0)
				{
					totalPitch -= rotSpeed * deltaTime;
					hasRotated = true;
				}
				if (mouseY < 0)
				{
					totalPitch += rotSpeed * deltaTime;
					hasRotated = true;
				}
			}

			if (hasRotated)
				cameraToWorld = CalculateCameraToWorld();
			
		}
	};
}
