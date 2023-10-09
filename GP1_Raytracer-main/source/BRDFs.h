#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			return cd * kd / static_cast<float>(M_PI);
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return cd * kd / static_cast<float>(M_PI);
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			auto reflect = Vector3::Reflect(n, l);
			auto cosA = Vector3::Dot(reflect, v);

			auto resultPhong = ks * std::pow(cosA, exp);

			return { resultPhong,resultPhong,resultPhong };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			auto cosTheta = Vector3::Dot(v, h);

			auto diffrenceBetweenWhite = (ColorRGB{ 1.0f,1.0f,1.0f } - f0);

			auto resultFresnel = f0 + diffrenceBetweenWhite * std::pow(1.0f - cosTheta, 5.0f);

			return resultFresnel ;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			auto cosTheta = Vector3::Dot(n, h);

			auto roughnessSquared = roughness * roughness;

			auto distribution = (roughnessSquared + cosTheta * cosTheta - 1.0f) * (roughnessSquared + cosTheta * cosTheta - 1.0f);

			auto result = roughnessSquared / (static_cast<float>(M_PI) * cosTheta * cosTheta * distribution);

			return result;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			auto normalDotVector = std::max(Vector3::Dot(n, v), 0.0f);
			auto roughnessSquared = roughness * roughness;

			auto distribution = normalDotVector * (1.0f - roughnessSquared) + roughnessSquared;
			auto result = (2.0f * normalDotVector) / distribution;

			return std::min(result, 1.0f);
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{

			return GeometryFunction_SchlickGGX(n,v,roughness)*GeometryFunction_SchlickGGX(n,l,roughness);
		}

	}
}