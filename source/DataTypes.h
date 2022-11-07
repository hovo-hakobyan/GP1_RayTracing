#pragma once
#include <cassert>

#include "Math.h"
#include "vector"


namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}
		{
			centroid = (_v0 + _v1 + _v2) / 3.0f;
		}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
			centroid = (_v0 + _v1 + _v2) / 3.0f;
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};
		Vector3 centroid{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	//Implemented using https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
	struct BVHNode
	{
		Vector3 minAABB, maxAABB;
		uint32_t leftFirst, nrPrimitives;

		//leftfirst dependend on nrPrimitives
			//nrPrimitves = 0 --> NotLeafNode && hasChildren
			// nrPrimitives !=0 --> LeafNode && NoChildren	
		//Not too readable but struct size is 32bytes now
	};

	struct AABB
	{
		Vector3 min{INFINITY,INFINITY,INFINITY }, max{ -INFINITY,-INFINITY,-INFINITY };

		void Grow(Vector3 pos)
		{
			min = Vector3::Min(min, pos);
			max = Vector3::Max(max, pos);
		}

		float Area()
		{
			Vector3 size = max - min;
			return size.x * size.y + size.y * size.z + size.z * size.x;
		}
	};

	struct Bin
	{
		AABB bounds;
		uint32_t nrPrimitives = 0;
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode):
		positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			trCount = static_cast<int>(_indices.size()) / 3;
			//Calculate Normals
			CalculateNormals();
			CalculateCentroids();
			InitBVH();
			BuildBVH();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			trCount = static_cast<int>(_indices.size()) / 3;
			CalculateCentroids();
			InitBVH();
			BuildBVH();
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<Vector3> centroids{};
		std::vector<int> indices{};
		unsigned char materialIndex{};
		uint32_t trCount{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		Vector3 minAABB;
		Vector3 maxAABB;

		Vector3 transformedMinAABB;
		Vector3 transformedMaxAABB;

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};
		std::vector<Vector3> transformedCentroids{};

		std::vector<BVHNode> bvhNodes{};
		uint32_t rootNodeIdx{};
		uint32_t nodesUsed{};

		bool shouldUseBVH = false;

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		void CalculateNormals()
		{

			Vector3 v0{};
			Vector3 v1{};
			Vector3 v2{};

			for (size_t currentTriangle = 0;  currentTriangle < trCount; ++currentTriangle)
			{
				v0 = positions[ indices[currentTriangle * 3]];
				v1 = positions[indices[currentTriangle * 3 + 1]];
				v2 = positions[indices[currentTriangle * 3 + 2]];

				normals.push_back(Vector3::Cross(v1 - v0, v2 - v0).Normalized());
			}
					
		}

		void CalculateCentroids()
		{
			trCount = static_cast<int>(indices.size()) / 3;
			centroids.reserve(normals.size());
			Vector3 v0{};
			Vector3 v1{};
			Vector3 v2{};

			for (size_t currentTriangle = 0; currentTriangle < trCount; ++currentTriangle)
			{
				v0 = positions[indices[currentTriangle * 3]];
				v1 = positions[indices[currentTriangle * 3 + 1]];
				v2 = positions[indices[currentTriangle * 3 + 2]];

				centroids.push_back((v0 + v1 + v2) / 3.f);
			}
		}

		void UpdateTransforms()
		{		
			transformedNormals.clear();
			transformedPositions.clear();
			transformedCentroids.clear();
			transformedNormals.reserve(normals.size());
			transformedCentroids.reserve(centroids.size());
			transformedPositions.reserve(positions.size());

			auto transformMatrix = rotationTransform * translationTransform * scaleTransform;

			for (size_t i = 0; i < positions.size(); i++)
			{
				transformedPositions.emplace_back(transformMatrix.TransformPoint(positions[i]));
				
			}

			for (size_t i = 0; i < centroids.size(); i++)
			{
				transformedCentroids.emplace_back(transformMatrix.TransformPoint(centroids[i]));
			}

			for (size_t i = 0; i < normals.size(); i++)
			{
				transformedNormals.emplace_back(transformMatrix.TransformVector(normals[i]).Normalized());
			}
			
			if (!shouldUseBVH)
			{
				UpdateTransformedAABB(transformMatrix);
			}
			else
			{
				RefitBVH();
			}
		}

		void UpdateAABB(uint32_t nodeIdx)
		{	
			//Single AABB
			if (!shouldUseBVH)
			{
				if (positions.size() > 0)
				{
					minAABB = Vector3{INFINITY,INFINITY,INFINITY};
					maxAABB = Vector3{ -INFINITY,-INFINITY,-INFINITY };

					for (auto& p : positions)
					{
						minAABB = Vector3::Min(p, minAABB);
						maxAABB = Vector3::Max(p, maxAABB);
					}
				}
				return;
			}

			//BVH
			BVHNode& node = bvhNodes[nodeIdx];

			uint32_t start{node.leftFirst *3};
			uint32_t end{ start + node.nrPrimitives*3};

			node.minAABB = Vector3{INFINITY,INFINITY,INFINITY};
			node.maxAABB = Vector3{ -INFINITY,-INFINITY,-INFINITY };

			for (uint32_t i = start; i < end ; ++i)
			{
				node.minAABB = Vector3::Min(transformedPositions[indices[i]], node.minAABB);
				node.maxAABB = Vector3::Max(transformedPositions[indices[i]], node.maxAABB);
			
			}
		}

		void UpdateTransformedAABB(const Matrix& finalTransform)
		{
			//Transform the 8 vertices of the aabb
			//then calculate new min and max
			Vector3 tMinAABB = finalTransform.TransformPoint(minAABB);
			Vector3 tMaxAABB = tMinAABB;

			// (xmax,ymin,zmin)
			Vector3 tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// (xmax,ymin,zmax)
			tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// (xmin,ymin,zmax)
			tAABB = finalTransform.TransformPoint(minAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// (xmin,ymax,zmin)
			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// (xmax,ymax,zmin)
			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// (xmax,ymax,zmax)
			tAABB = finalTransform.TransformPoint(maxAABB);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			// (xmin,ymax,zmax)
			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			transformedMinAABB = tMinAABB;
			transformedMaxAABB = tMaxAABB;

		}
		
		void InitBVH()
		{
			bvhNodes.reserve(trCount * 2 - 1);

			for (size_t i = 0; i < bvhNodes.capacity(); i++)
			{
				bvhNodes.push_back(BVHNode{});
			}
			
		}

		void BuildBVH()
		{
			rootNodeIdx = 0;
			nodesUsed = 1;

			bvhNodes[rootNodeIdx].leftFirst = 0;
			bvhNodes[rootNodeIdx].nrPrimitives = trCount; //IsLeaf

			UpdateAABB(rootNodeIdx);
			Subdivide(rootNodeIdx);
		}

		void Subdivide(uint32_t nodeIdx)
		{
			BVHNode& node = bvhNodes[nodeIdx];
			
			//Split axis using SAH !!!EXPENSIVE TO CALCULATE BUT PERFORMANT
			int axis{};
			float splitPos{};
			float bestSplitCost{ FindBestSplitPlane(node, axis, splitPos) };
				
			//Terminate Recursion
			float costIfNoSplit{ CalculateNodeCost(node) };
			if (bestSplitCost >= costIfNoSplit) // If splitting doesn't give us a cheper cost, then don't split
				return;


			//Sort the primitives (Quicksort)
			int left = node.leftFirst;
			int right = left + node.nrPrimitives - 1;
			SortPrimitives(left, right, axis, splitPos);
			
			int leftCount = left - node.leftFirst; //amountTriangles leftside
			
			if (leftCount == 0 || leftCount == node.nrPrimitives)
				return; // Either nothing on the left side, or everything on the left side

			//Create child nodes
			int leftChildIdx = nodesUsed;
			nodesUsed += 2;
	
			bvhNodes[leftChildIdx].leftFirst = node.leftFirst; // Left side starts where the parent's left side starts
			bvhNodes[leftChildIdx].nrPrimitives = leftCount;
			bvhNodes[leftChildIdx + 1].leftFirst = left; // While loop made sure this is where the right side starts
			bvhNodes[leftChildIdx + 1].nrPrimitives = node.nrPrimitives - leftCount; //remaining primitives belong to the right side

			node.nrPrimitives = 0; //Is not leaf 
			node.leftFirst = leftChildIdx;


			UpdateAABB(leftChildIdx);
			UpdateAABB(leftChildIdx + 1); //rightChild

			Subdivide(leftChildIdx);
			Subdivide(leftChildIdx + 1); //rightChild
		}

		float CalculateNodeCost(BVHNode& node)
		{
			Vector3 parentSize = node.maxAABB - node.minAABB;
			float parentArea = parentSize.x * parentSize.y + parentSize.y * parentSize.z + parentSize.z * parentSize.x;
			return node.nrPrimitives * parentArea;
		}

		void SortPrimitives(int& left, int right, int axis, float splitPos)
		{
			while (left <= right)
			{
				//If the centeroid is on the left side --> OK
				if (transformedCentroids[left][axis] < splitPos)
				{
					left++;
				}
				else
				{
					//Move to the end of the container
					std::swap(transformedCentroids[left], transformedCentroids[right]);
					std::swap(transformedNormals[left], transformedNormals[right]);
					std::swap(normals[left], normals[right]);
					std::swap(centroids[left], centroids[right]);
					for (int i = 0; i < 3; i++)
					{
						std::swap(indices[left * 3 + i], indices[right * 3 + i]);
					}
					--right;
				}
			}
		}

		float FindBestSplitPlane(BVHNode& node, int& axis, float& splitPos)
		{
			const uint32_t binCount = 10;
			float bestCost = INFINITY;

			
		//For every axis
			for (int a = 0; a < 3; ++a)
			{
				float minBound = INFINITY;
				float maxBound = -INFINITY;
				//Makes sure bounding box goes through the centroids
				for (uint32_t i = 0; i < node.nrPrimitives; i++)
				{
					minBound = std::min(transformedCentroids[node.leftFirst + i][a], minBound);
					maxBound = std::max(transformedCentroids[node.leftFirst + i][a], maxBound);
				}

				if (minBound == maxBound)
					continue;

		//Populate the bins
				Bin bin[binCount];
				float scale = binCount / (maxBound - minBound);//Used in idxFormula, prevents repeated division which is costly

				for (uint32_t i = 0; i < node.nrPrimitives; ++i)
				{
					//in which bin does this centroid rest
					int binIdx = std::min(binCount - 1, static_cast<uint32_t>(
						(transformedCentroids[node.leftFirst + i][a] - minBound) * scale));

					//scale that bin based on the primitives inside (makes it as tight as possible)
					++bin[binIdx].nrPrimitives;
					bin[binIdx].bounds.Grow(transformedPositions[indices[(node.leftFirst + i) * 3]]);
					bin[binIdx].bounds.Grow(transformedPositions[indices[(node.leftFirst + i) * 3]]);
					bin[binIdx].bounds.Grow(transformedPositions[indices[(node.leftFirst + i) * 3]]);
				}

		//Gather data for the planes between the bins (possible split location)
				float leftArea[binCount - 1], rightArea[binCount - 1];
				int leftCount[binCount - 1], rightCount[binCount - 1]; //nr primitives each side depending on split plane

				AABB leftBox, rightBox;
				int leftSum = 0, rightSum = 0;

				//We consider every right/left box scenarios and save data to find the best SAH later
				for (uint32_t i = 0; i < binCount -1; i++) //-1 otherwise we don't subdivide the box
				{
					//Nr of primitives (LEFT) of the current leftBOX
					leftSum += bin[i].nrPrimitives;
					leftCount[i] = leftSum;

					//Resize the current box
					leftBox.Grow(bin[i].bounds.min);
					leftBox.Grow(bin[i].bounds.max);

					//Save the current box area for SAH calculations
					leftArea[i] = leftBox.Area();

					//Same thing but for the right side
					rightSum += bin[binCount - 1 - i].nrPrimitives;
					rightCount[binCount - 2 - i] = rightSum;

					rightBox.Grow(bin[binCount - 1 - i].bounds.min);
					rightBox.Grow(bin[binCount - 1 - i].bounds.max);

					rightArea[binCount - 2 - i] = rightBox.Area();


				}
	
		//evaluate every plane's SAH cost based on previously collected data
				scale = (maxBound - minBound) / binCount;
				for (uint32_t i = 0; i< binCount - 1; ++i)
				{
					float planeCost = leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
					if (planeCost < bestCost)
					{
						bestCost = planeCost;
						axis = a;
						splitPos = minBound + scale * (i - 1);
						
					}					
				}
			}
			return bestCost;
		}
		
		void RefitBVH()
		{
			for (int i = nodesUsed - 1; i >= 0; i--)
			{
				BVHNode& node = bvhNodes[i];

				if (node.nrPrimitives !=0)
				{
					UpdateAABB(i);
					continue;
				}
				
				BVHNode& leftChild = bvhNodes[node.leftFirst];
				BVHNode& rightChild = bvhNodes[node.leftFirst + 1];
				node.minAABB = Vector3::Min(leftChild.minAABB, rightChild.minAABB);
				node.maxAABB = Vector3::Max(leftChild.maxAABB, rightChild.maxAABB);
			}
		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};
		Vector3 reciprocalDir{}; // 1/n

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};

#pragma endregion
}