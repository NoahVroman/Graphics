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
			//todo W1
			
			Vector3 diffVector = ray.origin - sphere.origin;

			float A = Vector3::Dot(ray.direction, ray.direction);
			float B = Vector3::Dot((2 * ray.direction), diffVector);
			float C = Vector3::Dot(diffVector, diffVector) - (sphere.radius * sphere.radius);

			float D = ((B * B) - (4 * A * C));

			if (D <= 0)
			{
				hitRecord.didHit = false;
				return false;
			}
			
			float t = ((-B - sqrtf(D)) / (2 * A));

			if (t < ray.min)
			{
				t = ((-B + sqrtf(D)) / (2 * A));
			}
			if (t > ray.min && t < ray.max)
			{
				if (t < hitRecord.t)
				{
					hitRecord.didHit = true;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.t = t;
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
			const float t = Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction, plane.normal);

			if (t > ray.min && t < ray.max)
			{
				if (t < hitRecord.t)
				{
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.t = t;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = plane.normal;
					hitRecord.didHit = true;
				}
				return true;
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
			//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

			Vector3 edge1, edge2, CrossRayEdge2, s, CrossSQ;
			float dotEdge1H, f, u, v;
			edge1 = triangle.v1 - triangle.v0;
			edge2 = triangle.v2 - triangle.v0;
			
			CrossRayEdge2 = Vector3::Cross(ray.direction, edge2);
			dotEdge1H = Vector3::Dot(edge1, CrossRayEdge2);


			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (dotEdge1H < 0)
				{
					return false;
				}
				break;
			case TriangleCullMode::BackFaceCulling:
				if (dotEdge1H > 0)
				{
					return false;
				}
				break;
			}

			f = 1.0f / dotEdge1H;
			s = ray.origin - triangle.v0;
			u = f * Vector3::Dot(s,CrossRayEdge2);

			if (u < 0.0f || u > 1.0f)
			{
				return false;
			}
			
			CrossSQ = Vector3::Cross(s, edge1);
			v = f * Vector3::Dot(ray.direction, CrossSQ);

			if (v < 0.0f || u + v > 1.0f)
			{
				return false;
			}

			float t = f * Vector3::Dot(edge2, CrossSQ);

			if (t <= ray.min || t >= ray.max)
			{
				return false;
			}
			else if (t < hitRecord.t)
			{
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = triangle.normal;
				hitRecord.didHit = true;
				return true;
			}
			return false;

		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float inverseXRayDir{ 1.f / ray.direction.x };
			float tmin = (mesh.transformedMinAABB.x - ray.origin.x) * inverseXRayDir;
			float tmax = (mesh.transformedMaxAABB.x - ray.origin.x) * inverseXRayDir;

			if (tmin > tmax) std::swap(tmin, tmax);

			float inverseYRayDir{ 1.f / ray.direction.y };
			float tymin = (mesh.transformedMinAABB.y - ray.origin.y) * inverseYRayDir;
			float tymax = (mesh.transformedMaxAABB.y - ray.origin.y) * inverseYRayDir;

			if (tymin > tymax) std::swap(tymin, tymax);

			if ((tmin > tymax) || (tymin > tmax)) return false;

			if (tymin > tmin) tmin = tymin;
			if (tymax < tmax) tmax = tymax;

			float inverseZRayDir{ 1.f / ray.direction.z };
			float tzmin = (mesh.transformedMinAABB.z - ray.origin.z) * inverseZRayDir;
			float tzmax = (mesh.transformedMaxAABB.z - ray.origin.z) * inverseZRayDir;

			if (tzmin > tzmax) std::swap(tzmin, tzmax);

			if ((tmin > tzmax) || (tzmin > tmax)) return false;

			return true;
		}
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}

			HitRecord currentHitRecord;
			Triangle triangle;
			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				triangle = Triangle{ mesh.transformedPositions[mesh.indices[i]],mesh.transformedPositions[mesh.indices[i + 1]],mesh.transformedPositions[mesh.indices[i + 2]], mesh.transformedNormals[i / 3] };
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;
				if (HitTest_Triangle(triangle, ray, currentHitRecord))
				{
					if (ignoreHitRecord == true)
					{
						return true;	
					}
					if (currentHitRecord.t < hitRecord.t)
					{
						hitRecord = currentHitRecord;
					}
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
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			return light.color * (light.intensity / (light.origin - target).SqrMagnitude());
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