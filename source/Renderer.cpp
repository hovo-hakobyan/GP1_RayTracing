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
#include <future> //async
#include <ppl.h> //parallel_for
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
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	camera.CalculateCameraToWorld();

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float aspectRatio{ static_cast<float>(m_Width) / m_Height };
	float fovRadians{tanf( TO_RADIANS * (camera.fovAngle /2)) };
	
	const uint32_t numPixels = m_Width * m_Height;
	
#if defined(ASYNC)
	//Async Logic
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_features{};
	const uint32_t numPixelsPerTask = numPixels / numCores;
	uint32_t numUnassignedPixels = numPixels % numCores;
	uint32_t currPixelIndex = 0;

	for (uint32_t coreId{ 0 }; coreId < numCores; ++coreId)
	{
		//If there are unissigned tasks we want to distrubute those to the cores
		uint32_t taskSize = numPixelsPerTask;
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_features.push_back(std::async(std::launch::async, [=, this]
			{
				//Render all pixels for this task (currentPixelIndex > currPixIndes + taskSize)
				const uint32_t pixelIndexEnd = currPixelIndex + taskSize;
				for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, fovRadians, aspectRatio, camera, lights, materials);
				}

			}));

		currPixelIndex += taskSize;
	}

#elif defined(PARALLEL_FOR)
	//Parallel-For Logic

	concurrency::parallel_for(0u, numPixels, [=, this](int i) {
		RenderPixel(pScene, i, fovRadians, aspectRatio, camera, lights, materials);
		});
#else
	//Synchronous Logic (no threading)
	for (uint32_t i = 0; i < numPixels; ++i)
	{
		RenderPixel(pScene, i, fovRadians, aspectRatio, camera, lights, materials);
	}
#endif
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	float rx = px + 0.5f;
	float ry = py + 0.5f;

	float cx = (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov;
	float cy = (1 - (2 * (ry / float(m_Height)))) * fov;

	ColorRGB finalColor{};
	HitRecord closestHit{};

	Vector3 rayDir{ cx * Vector3::UnitX + cy * Vector3::UnitY + Vector3::UnitZ };
	//we shoot rays from the camera positioin, not from the world origin
	rayDir = camera.cameraToWorld.TransformVector(rayDir);
	rayDir.Normalize();

	Ray viewRay{ camera.origin,rayDir,{1.f/rayDir.x, 1.f / rayDir.y, 1.f / rayDir.z} };

	finalColor = dae::colors::Black;

	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		float offset{ 0.0001f };
		Ray lightRay{};
		lightRay.origin = closestHit.origin + closestHit.normal * (offset * 2);

		for (const Light& currentLight : lights)
		{
			Vector3 dirToLight{ LightUtils::GetDirectionToLight(currentLight,lightRay.origin) };
			lightRay.direction = dirToLight.Normalized();
			lightRay.min = offset;
			lightRay.max = dirToLight.Magnitude();
			lightRay.reciprocalDir = { 1.f / lightRay.direction.x,1.f / lightRay.direction.y,1.f / lightRay.direction.z };

			float lambertCosine{ Vector3::Dot(closestHit.normal, lightRay.direction) };
			bool skipCalculations{ false };

			if (m_ShadowsEnabled)
			{
				skipCalculations = pScene->DoesHit(lightRay);
			}

			if (!skipCalculations)
			{
				switch (m_CurrentLightingMode)
				{
				case dae::Renderer::LightingMode::ObservedArea:
					if (lambertCosine > 0.f)
					{
						finalColor += ColorRGB{ lambertCosine,lambertCosine,lambertCosine };
					}
					break;
				case dae::Renderer::LightingMode::Radiance:
					finalColor += LightUtils::GetRadiance(currentLight, closestHit.origin);
					break;
				case dae::Renderer::LightingMode::BRDF:
					finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, -viewRay.direction);
					break;
				case dae::Renderer::LightingMode::Combined:
					if (lambertCosine > 0.f)
					{
						finalColor += LightUtils::GetRadiance(currentLight, closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, -viewRay.direction) * lambertCosine;
					}
					break;
				}
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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	if (m_CurrentLightingMode == LightingMode::Combined)
	{
		m_CurrentLightingMode = LightingMode::ObservedArea;
	}
	else
	{
		m_CurrentLightingMode = static_cast<LightingMode>(static_cast<int>(m_CurrentLightingMode) + 1);
	}
}
