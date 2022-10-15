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
			return { (kd * cd) / PI};
			//todo: W3
			return {};
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return { (kd * cd) / PI };
			//todo: W3
			return {};
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
			Vector3 reflect = l.Reflect(l, n);
			reflect.Normalize();
			float alfa = Vector3::Dot(reflect, v);
			float PSR{};
			if (alfa > 0)
			{
				PSR = ks * (powf(alfa, exp));
			}
			return {PSR,PSR,PSR};
			//todo: W3
			return {};
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
			return { f0 + ((ColorRGB{1,1,1} - f0) * powf((1 - Vector3::Dot(h, v)), 5)) };
			//todo: W3
			return {};
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
			const float alfa = roughness * roughness;
			const float alfaSqrd = alfa * alfa;
			const float nhSqrd = (Vector3::Dot(n, h) * Vector3::Dot(n, h));
			const float NormalDis = alfaSqrd / (PI * ((nhSqrd * (alfaSqrd - 1) + 1) * (nhSqrd * (alfaSqrd - 1) + 1)));
			return NormalDis;
			//todo: W3
			return {};
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
			float k = ((roughness * roughness + 1) * (roughness * roughness + 1)) / 8.0f;
			float geometryFunc = Vector3::Dot(n, v) / (Vector3::Dot(n, v) * (1 - k) + k);
			return geometryFunc;
			//todo: W3
			return {};
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
			//todo: W3
			return {};
		}

	}
}