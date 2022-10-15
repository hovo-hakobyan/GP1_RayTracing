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
	float aspectRatio{ static_cast<float>(m_Width) / m_Height };
	float fovRadians{tanf( TO_RADIANS * (camera.fovAngle /2)) };
	

	for (int px{}; px < m_Width; ++px)
	{
		float cx{static_cast<float>( (2 * (px + 0.5) - m_Width) / m_Width) * aspectRatio * fovRadians };

		for (int py{}; py < m_Height; ++py)
		{
			float cy{ static_cast<float>( (m_Height - 2 * (py + 0.5)) / m_Height ) * fovRadians};

			Vector3 rayDir{ cx * Vector3::UnitX + cy * Vector3::UnitY + Vector3::UnitZ };
			//we shoot rays from the camera positioin, not from the world origin
			rayDir = camera.cameraToWorld.TransformVector(rayDir);
			rayDir.Normalize();

			Ray viewRay{ camera.origin,rayDir};
			ColorRGB finalColor{};
			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);
			
			if (closestHit.didHit)
			{
				//finalColor = materials[closestHit.materialIndex]->Shade();
				float offset{ 0.0001f };
				Ray lightRay{};
				lightRay.origin = closestHit.origin + closestHit.normal * (offset *2);

				for (const Light& currentLight: lights)
				{
					Vector3 dirToLight{ LightUtils::GetDirectionToLight(currentLight,lightRay.origin) };
					lightRay.direction = dirToLight.Normalized();
					lightRay.min = offset;
					lightRay.max = dirToLight.Magnitude();

					float lambertCosine{ Vector3::Dot(closestHit.normal, lightRay.direction) };
					bool doesHit{ pScene->DoesHit(lightRay) };

					switch (m_CurrentLightingMode)
					{
					case dae::Renderer::LightingMode::ObservedArea:
						if (lambertCosine > 0.f)
						{
							if (m_ShadowsEnabled)
							{
								if (doesHit)
									break;
							}
							finalColor += ColorRGB{ lambertCosine,lambertCosine,lambertCosine };
						}
						break;
					case dae::Renderer::LightingMode::Radiance:
						if (m_ShadowsEnabled)
						{
							if (doesHit)
								break;
						}
						finalColor += LightUtils::GetRadiance(currentLight, closestHit.origin);
						break;
					case dae::Renderer::LightingMode::BRDF:
						
						break;
					case dae::Renderer::LightingMode::Combined:
						break;
					}
					
					//finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, -viewRay.direction);
					if (!pScene->DoesHit(lightRay))
					{
						
						if (lambertCosine > 0.f)
						{
							finalColor += LightUtils::GetRadiance(currentLight, closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit,lightRay.direction,-viewRay.direction) * lambertCosine;
							
						}
						continue;
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

void dae::Renderer::CycleLightingMode()
{

}
