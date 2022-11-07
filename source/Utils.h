#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"



namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
		
			Vector3 raySphere{ sphere.origin - ray.origin };
			Vector3 rayNormalized{ ray.direction.Normalized() };

			float raySphereOnRay{ Vector3::Dot(rayNormalized, raySphere) };
			if ( raySphereOnRay< 0)
			{
				return false;
			}
			// projection of tc on the ray
			Vector3 projectionOnRay{ rayNormalized * raySphereOnRay };

			//Perpendicular distance from the center of the sphere to the ray
			float perpDistanceRaySphere{sqrtf( Vector3::Dot(raySphere,raySphere) - Vector3::Dot(projectionOnRay,projectionOnRay))};
			
			float sphereRadius{ sphere.radius };
			if (perpDistanceRaySphere > sphereRadius)
			{
				return false;
			}
			// distance FROM the point that is perpendicular to the sphere center and is on the ray TO the first intersection point
			float insideSphereSegment{ sqrtf(sphereRadius * sphereRadius - perpDistanceRaySphere * perpDistanceRaySphere) };
			float t{ projectionOnRay.Magnitude() - insideSphereSegment };


			if (t >= ray.min && t <= ray.max)
			{
				if (ignoreHitRecord)
				{
					return true;
				}
				if (t < hitRecord.t)
				{
					hitRecord.t = t;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.didHit = true;
					hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
					hitRecord.normal = hitRecord.origin - sphere.origin;
					hitRecord.normal.Normalize();
				}
				return true;
			}
			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 planeNormal{ plane.normal };
			float t{ Vector3::Dot(plane.origin - ray.origin,planeNormal) / Vector3::Dot(ray.direction,planeNormal) };
		

			if (t > ray.min && t < ray.max )
			{
				if (ignoreHitRecord)
				{
					return true;
				}
				if (t < hitRecord.t)
				{
					
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.origin = ray.origin + ray.direction* hitRecord.t;
					hitRecord.normal = plane.normal;
					hitRecord.normal.Normalize();
					return true;
				}
				

			}
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{	
			Vector3 v0 = triangle.v0;
			Vector3 v1 = triangle.v1;
			Vector3 v2 = triangle.v2;

			Vector3 edge1, edge2, p, s, q;
			float determinant, invA, u, v;

			edge1 = v1 - v0;
			edge2 = v2 - v0;

			p = Vector3::Cross(ray.direction, edge2);
			determinant = Vector3::Dot(p, edge1);

			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (ignoreHitRecord)
				{
					if (determinant < FLT_EPSILON)
						return false;
				}
				else if (determinant > FLT_EPSILON)
					return false;
				
				break;
			case TriangleCullMode::BackFaceCulling:
				if (ignoreHitRecord)
				{
					if (determinant > FLT_EPSILON)
						return false;
				}
				else if (determinant < FLT_EPSILON)
					return false;
				break;
			default:
				if (determinant > -FLT_EPSILON && determinant < FLT_EPSILON) //ray parallel to triangle
					return false;
				break;
			}

			invA = 1.0f / determinant;
			s = ray.origin - v0;
			u = invA * Vector3::Dot(s, p);

			if (u < 0.0f || u > 1.f)
				return false;

			q = Vector3::Cross(s, edge1);
			v = invA * Vector3::Dot(ray.direction, q);

			if (v < 0.0f || u + v >1.0f)
				return false;

			float t = invA * Vector3::Dot(edge2, q);

			if (t < ray.min || t > ray.max)
				return false;

			if (ignoreHitRecord)
				return true;

			if (t < hitRecord.t)
			{
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
				hitRecord.didHit = true;
				hitRecord.normal = triangle.normal;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region SlabTest
		inline bool SlabTest(const Vector3& minAABB,const Vector3& maxAABB, const Ray& ray)
		{
			float tx1 = (minAABB.x - ray.origin.x) * ray.reciprocalDir.x;
			float tx2 = (maxAABB.x - ray.origin.x) * ray.reciprocalDir.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (minAABB.y - ray.origin.y) * ray.reciprocalDir.y;
			float ty2 = (maxAABB.y - ray.origin.y) * ray.reciprocalDir.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (minAABB.z - ray.origin.z) * ray.reciprocalDir.z;
			float tz2 = (maxAABB.z - ray.origin.z) * ray.reciprocalDir.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));
			
			return tmax > 0 && tmax >= tmin;
		}

		 inline void IntersectBVH(const Ray& ray, const TriangleMesh& mesh,const uint32_t nodeIdx,std::vector<int>& indexes)
		{
			const BVHNode& node = mesh.bvhNodes[nodeIdx];

			if (!SlabTest(node.minAABB, node.maxAABB, ray))
				return;

			if (node.nrPrimitives !=0) //Leaf
			{		
				indexes.push_back(nodeIdx);
				return;
			}
			else
			{
				IntersectBVH(ray, mesh, node.leftFirst, indexes);
				IntersectBVH(ray, mesh, node.leftFirst + 1, indexes);
			}
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			Triangle triangle{};
			triangle.cullMode = mesh.cullMode;
			triangle.materialIndex = mesh.materialIndex;
			int nrVertices{ 3 };	

			if (mesh.shouldUseBVH)
			{
				std::vector<int> indexes{};
				IntersectBVH(ray, mesh, mesh.rootNodeIdx, indexes);

				if (indexes.empty())
					return hitRecord.didHit;


				for (size_t i = 0; i < indexes.size(); i++)
				{

					uint32_t start = mesh.bvhNodes[indexes[i]].leftFirst;
					uint32_t end = start + mesh.bvhNodes[indexes[i]].nrPrimitives;

					for (uint32_t currentTriangle = start; currentTriangle < end; ++currentTriangle)
					{
						triangle.v0 = mesh.transformedPositions[mesh.indices[currentTriangle * nrVertices]];
						triangle.v1 = mesh.transformedPositions[mesh.indices[currentTriangle * nrVertices + 1]];
						triangle.v2 = mesh.transformedPositions[mesh.indices[currentTriangle * nrVertices + 2]];
						triangle.normal = mesh.transformedNormals[currentTriangle];
						if (HitTest_Triangle(triangle, ray, hitRecord) && ignoreHitRecord)
							return true;
					}
				}
			}
			else
			{
				if (!SlabTest(mesh.transformedMinAABB, mesh.transformedMaxAABB, ray))
					return false;

				for (uint32_t currentTriangle = 0; currentTriangle < mesh.indices.size() / 3; ++currentTriangle)
				{
					triangle.v0 = mesh.transformedPositions[mesh.indices[currentTriangle * nrVertices]];
					triangle.v1 = mesh.transformedPositions[mesh.indices[currentTriangle * nrVertices + 1]];
					triangle.v2 = mesh.transformedPositions[mesh.indices[currentTriangle * nrVertices + 2]];
					triangle.normal = mesh.transformedNormals[currentTriangle];
					if (HitTest_Triangle(triangle, ray, hitRecord) && ignoreHitRecord)
						return true;
				}
			}
			return hitRecord.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}

		

#pragma endregion


	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			return Vector3{light.origin - origin};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			ColorRGB final{};
			Vector3 lightToTarget{ light.origin - target };
			if (light.type == LightType::Point)
			{
				final = light.color * light.intensity / lightToTarget.SqrMagnitude();
				
			}
			else if (light.type == LightType::Directional)
			{
				final = light.color * light.intensity;
			}
			return final;
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}