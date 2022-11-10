#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		if (GeometryUtils::SlabTest(m_aabbCircles.min,m_aabbCircles.max,ray))
		{
			for (size_t i = 0; i < m_SphereGeometries.size(); i++)
			{
				GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray, closestHit);
			}
		}
		
		for (size_t i = 0; i < m_PlaneGeometries.size(); i++)
		{
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray, closestHit);
		}

		if (GeometryUtils::SlabTest(m_aabbTriangles.min,m_aabbTriangles.max,ray))
		{
			for (size_t i = 0; i < m_TriangleMeshGeometries.size(); i++)
			{
				GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray, closestHit);

			}
		}
		
	}

	bool Scene::DoesHit(const Ray& ray) const
	{

		if (GeometryUtils::SlabTest(m_aabbCircles.min, m_aabbCircles.max, ray))
		{
			for (size_t i = 0; i < m_SphereGeometries.size(); i++)
			{
				if (GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray))
					return true;
			}
		}

		for (size_t i = 0; i < m_PlaneGeometries.size(); i++)
		{
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray))
				return true;
		}


		if (GeometryUtils::SlabTest(m_aabbTriangles.min, m_aabbTriangles.max, ray))
		{
			for (size_t i = 0; i < m_TriangleMeshGeometries.size(); i++)
			{
				if (GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray))
					return true;
			}
		}
		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
				//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion

#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 90.f;

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane({ -5.f,0.f,0.f }, { 1.f,0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f,0.f,0.f }, { -1.f,0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0,0.f,0.f }, { 0.f,1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0,10.f,0.f }, { 0.f,-1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0,0.f,10.f }, { 0.f,0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f,1.f,0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f,1.f,0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f,1.f,0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f,3.f,0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f,3.f,0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f,3.f,0.f }, .75f, matId_Solid_Blue);

		//Light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion
	void Scene_W3_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f,1.f,-5.f };
		m_Camera.fovAngle = 45.f;

		const unsigned char matId_Solid_Red = AddMaterial(new Material_Lambert(colors::Red,1.f));
		const unsigned char matLambertPhong_Blue = AddMaterial(new Material_LambertPhong(colors::Blue,1.f,1.f,60.f));
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));

		AddSphere({ -.75f, 1.f,.0f }, 1.f, matId_Solid_Red);
		AddSphere({ .75f, 1.f,.0f }, 1.f, matLambertPhong_Blue);

		AddPlane({ 0.f,0.f,0.f }, { 0.f,1.f,0.f }, matId_Solid_Yellow);

		AddPointLight({ 0.f,5.f,5.f }, 25.f, colors::White);
		AddPointLight({ 0.f,2.5f,-5.f }, 25.f, colors::White);
	}
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({.972f,.960f,.915f},1.f,1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f,.960f,.915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f,.960f,.915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({.75f,.75f,.75f},.0f,1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f,.75f,.75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f,.75f,.75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f,.57f,.57f }, 1.f));

		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f,0.f,0.f }, Vector3{ -1.f,0.f,0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f,0.f,0.f }, Vector3{ 1.f,0.f,0.f }, matLambert_GrayBlue); //LEFT

		//Temp Phong
		//const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f));
		//const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f));
		//const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f));

		//AddSphere(Vector3{ -1.75f,1.f,0.f }, .75f, matLambertPhong1);
		//AddSphere(Vector3{ 0.f,1.f,0.f }, .75f, matLambertPhong2);
		//AddSphere(Vector3{ 1.75f,1.f,0.f }, .75f, matLambertPhong3);


		AddSphere(Vector3{ -1.75f,1.f,0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f,1.f,0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f,1.f,0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f,3.f,0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f,3.f,0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f,3.f,0.f }, .75f, matCT_GraySmoothPlastic);

		AddPointLight(Vector3{ 0.f,5.f,5.f }, 50.f, ColorRGB{ 1.f,.61f,.45f }); 
		AddPointLight(Vector3{ -2.5f,5.f,-5.f }, 70.f, ColorRGB{ 1.f,.8f,.45f }); 
		AddPointLight(Vector3{ 2.5f,2.5f,-5.f }, 50.f, ColorRGB{ .34f,.47f,.68f });
	}

	void Scene_W4_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f,.57f,.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f,0.f,0.f }, Vector3{ -1.f,0.f,0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f,0.f,0.f }, Vector3{ 1.f,0.f,0.f }, matLambert_GrayBlue); //LEFT

		pMesh = AddTriangleMesh(dae::TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/simple_cube.obj",pMesh->positions,pMesh->normals,pMesh->indices);



		pMesh->shouldUseBVH = false;
		pMesh->Scale({ .7f,.7f,.7f });
		pMesh->Translate({ .0f,1.f,0.f });
		//pMesh->CalculateNormals(); //calculated inside ParseOBJ
		pMesh->CalculateCentroids();

		pMesh->UpdateAABB(-1);
		pMesh->UpdateTransforms();


		AddPointLight(Vector3{ 0.f,5.f,5.f }, 50.f, ColorRGB{ 1.f,.61f,.45f });
		AddPointLight(Vector3{ -2.5f,5.f,-5.f }, 70.f, ColorRGB{ 1.f,.8f,.45f });
		AddPointLight(Vector3{ 2.5f,2.5f,-5.f }, 50.f, ColorRGB{ .34f,.47f,.68f });
	}
	void Scene_W4_TestScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		//pMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
		//pMesh->UpdateTransforms();

	}
	
	void Scene_W4_ReferenceScene::Initialize()
	{
		sceneName = "Reference Scene";
		m_Camera.origin = { 0.f, 3.0f, -9.0f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.0f, 1.0f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.0f, 0.6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ 0.972f, 0.960f, 0.915f }, 1.0f, 0.1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.0f, 0.6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ 0.75f, 0.75f, 0.75f }, 0.0f, 0.1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.0f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Plane
		AddPlane(Vector3{ 0.0f, 0.0f, 10.0f }, Vector3{ 0.0f, 0.0f, -1.0f }, matLambert_GrayBlue);; //Back
		AddPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f }, matLambert_GrayBlue);; //Bottom
		AddPlane(Vector3{ 0.0f, 10.0f, 0.0f }, Vector3{ 0.0f, -1.0f, 0.0f }, matLambert_GrayBlue);; //Top
		AddPlane(Vector3{ 5.0f, 0.f, 0.0f }, Vector3{ -1.0f, 0.0f, 0.0f }, matLambert_GrayBlue);; //Right
		AddPlane(Vector3{ -5.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 0.0f, 0.0f }, matLambert_GrayBlue);; //Left

		//Spheres
		AddSphere(Vector3{ -1.75f, 1.0f, 0.0f }, 0.75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.0f, 1.0f, 0.0f }, 0.75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.0f, 0.0f }, 0.75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f, 3.0f, 0.0f }, 0.75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.0f, 3.0f, 0.0f }, 0.75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.0f, 0.0f }, 0.75f, matCT_GraySmoothPlastic);

		const Triangle baseTriangle = { Vector3(-0.75f, 1.5f, 0.0f), Vector3(0.75f, 0.0f, 0.0f), Vector3(-0.75f, 0.0f, 0.0f) };

		m_pMeshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_pMeshes[0]->AppendTriangle(baseTriangle, true);
		m_pMeshes[0]->shouldUseBVH = false;
		m_pMeshes[0]->Translate({ -1.75f, 4.5f, 0.0f });
		m_pMeshes[0]->CalculateCentroids();

		m_pMeshes[0]->UpdateAABB(-1);
		m_pMeshes[0]->UpdateTransforms();
		/*m_pMeshes[0]->InitBVH();
		m_pMeshes[0]->BuildBVH();*/

		m_pMeshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_pMeshes[1]->AppendTriangle(baseTriangle, true);
		m_pMeshes[1]->shouldUseBVH = false;
		m_pMeshes[1]->Translate({ 0.0f, 4.5f, 0.0f });
		m_pMeshes[1]->CalculateCentroids();
		
		m_pMeshes[1]->UpdateAABB(-1);
		m_pMeshes[1]->UpdateTransforms();
		/*m_pMeshes[1]->InitBVH();
		m_pMeshes[1]->BuildBVH();*/


		m_pMeshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_pMeshes[2]->AppendTriangle(baseTriangle, true);
		m_pMeshes[2]->shouldUseBVH = false;
		m_pMeshes[2]->Translate({ 1.75f, 4.5f, 0.0f });
		m_pMeshes[2]->CalculateCentroids();
		
		m_pMeshes[2]->UpdateAABB(-1);
		m_pMeshes[2]->UpdateTransforms();
		/*m_pMeshes[2]->InitBVH();
		m_pMeshes[2]->BuildBVH();*/

		
		//Light
		AddPointLight(Vector3{ 0.0f, 5.0f, 5.0f }, 50.f, ColorRGB{ 1.0f, 0.61f, 0.45f }); // Backlight
		AddPointLight(Vector3{ -2.5f, 5.0f, -5.0f }, 70.f, ColorRGB{ 1.0f, 0.8f, 0.45f }); // Frontlight left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.0f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });
	}
	void Scene_W4_ReferenceScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle = (cosf(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		for (const auto m : m_pMeshes)
		{
			m->RotateY(yawAngle);
			m->UpdateTransforms();
			
			for (uint32_t i = 0; i < m->trCount; i++)
			{
				m_aabbTriangles.Grow(m->transformedPositions[m->indices[i * 3]]);
				m_aabbTriangles.Grow(m->transformedPositions[m->indices[i * 3 + 1]]);
				m_aabbTriangles.Grow(m->transformedPositions[m->indices[i * 3 + 2]]);
			}

		}

		for (const auto& s : m_SphereGeometries)
		{
			m_aabbCircles.Grow(Vector3{s.origin.x - s.radius, s.origin.y - s.radius,s.origin.z - s.radius});
			m_aabbCircles.Grow(Vector3{ s.origin.x + s.radius, s.origin.y + s.radius,s.origin.z + s.radius });
		}


	}
	
	void Scene_W4_BunnyScene::Initialize()
	{
		sceneName = "Bunny Scene";
		m_Camera.origin = { 0.f, 3.0f, -9.0f };
		m_Camera.fovAngle = 45.f;

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ 0.49f, 0.57f, 0.57f }, 1.0f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Plane
		AddPlane(Vector3{ 0.0f, 0.0f, 10.0f }, Vector3{ 0.0f, 0.0f, -1.0f }, matLambert_GrayBlue);; //Back
		AddPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f }, matLambert_GrayBlue);; //Bottom
		AddPlane(Vector3{ 0.0f, 10.0f, 0.0f }, Vector3{ 0.0f, -1.0f, 0.0f }, matLambert_GrayBlue);; //Top
		AddPlane(Vector3{ 5.0f, 0.f, 0.0f }, Vector3{ -1.0f, 0.0f, 0.0f }, matLambert_GrayBlue);; //Right
		AddPlane(Vector3{ -5.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 0.0f, 0.0f }, matLambert_GrayBlue);; //Left


		m_pBunny = AddTriangleMesh(dae::TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj", m_pBunny->positions, m_pBunny->normals, m_pBunny->indices);
		//m_pBunny->CalculateNormals();	
		m_pBunny->shouldUseBVH = true;
		m_pBunny->Scale({ 2.f,2.f,2.f });
	
		m_pBunny->CalculateCentroids();
		
		m_pBunny->UpdateTransforms();
	
		m_pBunny->InitBVH();
		m_pBunny->BuildBVH();

		

		//Light
		AddPointLight(Vector3{ 0.0f, 5.0f, 5.0f }, 50.f, ColorRGB{ 1.0f, 0.61f, 0.45f }); // Backlight
		AddPointLight(Vector3{ -2.5f, 5.0f, -5.0f }, 70.f, ColorRGB{ 1.0f, 0.8f, 0.45f }); // Frontlight left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.0f }, 50.f, ColorRGB{ 0.34f, 0.47f, 0.68f });
	}
	void Scene_W4_BunnyScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle = (cosf(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		m_pBunny->RotateY(yawAngle);
		m_pBunny->UpdateTransforms();

	}
	
}
