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
			return {kd * cd / PI};
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return {kd*cd / PI };
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
			float theta{ std::max(Vector3::Dot(n,l),0.f) };
			Vector3 r{2.f *theta  * n -l };
			float alpha{ std::max(Vector3::Dot(r,v),0.f) };
	
			float result{ ks * powf(alpha,exp) };
			return ColorRGB{result,result,result};
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
			float factor{ 1 - std::max(Vector3::Dot(h,v),0.f) };
			
			return { f0 + (ColorRGB{1.f,1.f,1.f} - f0) * factor * factor * factor * factor * factor };
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
			float a{ roughness * roughness };
			float dot{std::max( Vector3::Dot(n,h),0.0f) };
			float dotNHsqr{ dot * dot };
			float aSqrMin1{ a * a - 1 };
			float factor{ dotNHsqr * aSqrMin1 + 1 };
			float denominator{ factor * factor };
			return a * a / (PI * denominator);
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
			float a{ roughness * roughness };
			float factor{ a + 1 };
			float k{ (factor * factor) / 8.f };
			float nDotV{ std::max(Vector3::Dot(n,v),0.f) };

			return { nDotV / (nDotV * (1 - k) + k) };
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
			return GeometryFunction_SchlickGGX(n,v,roughness) * GeometryFunction_SchlickGGX(n,l,roughness);
		}

	}
}