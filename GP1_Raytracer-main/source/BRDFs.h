#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			return cd * kd / static_cast<float>(M_PI);
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return cd * kd / static_cast<float>(M_PI);
		}

		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			const Vector3 reflect{ Vector3::Reflect(n, l) };
			const float cosA{ Vector3::Dot(reflect, v) };

			if (cosA < 0.0f)
			{
				return colors::Black;
			}

			const auto resultPhong = ks * std::pow(cosA, exp);

			return ColorRGB{ resultPhong };
		}
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			const float cosTheta = 1.0f-Vector3::Dot(v, h);

			return f0 + (1.0f - f0) * (cosTheta * cosTheta * cosTheta * cosTheta * cosTheta);
		}

		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			const float dot{ Vector3::Dot(n, h) };
			const float distribution{ dot * dot * ((roughness * roughness) * (roughness * roughness) - 1) + 1 };

			return  ((roughness * roughness) * (roughness * roughness)) / (PI * distribution * distribution);
		}


		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			const float kDirect{ ((roughness * roughness + 1) * (roughness * roughness + 1)) / 8 };

			const float dot{ Vector3::Dot(n, v) };

			if (dot < 0.0f)
			{
				return 0.0f;
			}

			return dot / (dot * (1 - kDirect) + kDirect);
		}

		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{

			return GeometryFunction_SchlickGGX(n,v,roughness) * GeometryFunction_SchlickGGX(n,l,roughness);
		}

	}
}