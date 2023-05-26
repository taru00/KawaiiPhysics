/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2014 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */


#ifndef HCL_SHAPE_TYPE_TAPERED_CAPSULE__H
#define HCL_SHAPE_TYPE_TAPERED_CAPSULE__H

#include <Cloth/Cloth/Collide/Shape/hclShape.h>
#include <Common/Base/Math/Vector/hkVector4Util.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

extern const class hkClass hclTaperedCapsuleShapeClass;

	/// A cloth collision primitive (shape) representing a tapered capsule.
class hclTaperedCapsuleShape : public hclShape
{
	public:

		//+vtable(1)
		//+version(2)
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE);
		HK_DECLARE_REFLECTION();

			/// Constructor. A tapered capsule is defined as the convex hull of two spheres with different radii.
		hclTaperedCapsuleShape (const hkVector4& start, const hkVector4& end, hkReal startRadius, hkReal endRadius);
	
			/// Retrieve the center of the small sphere.
		inline const hkVector4& getSmall() const;

			/// Retrieve the center of the big sphere.
		inline const hkVector4& getBig() const;

			/// Retrieve the small radius of the tapered capsule.
		inline hkReal getSmallRadius() const;

			/// Retrieve the big radius of the tapered capsule.
		inline hkReal getBigRadius() const;
		
		//
		// METHODS EXPECTED IN ANY hclShape
		//

			/// (From hclShape) Get the closest point on surface to localPos.
		void getClosestPoint( const hkVector4& localPos, hkSimdRealParameter particleRadius, const hkUint16 particleIndex, hkVector4& pointOnSurface, hkVector4& normal, hkSimdReal& signedDistance ) const;

			/// (From hclShape) - as above, but computing four points simultaneously.
		void getClosestPoint4( const hkVector4 localPos[ 4 ], const hkVector4& particleRadii, const hkUint16 particleIndices[ 4 ], hkVector4 pointOnSurface[ 4 ], hkVector4 normal[ 4 ], hkVector4& signedDistance ) const;

			/// (From hclShape) - Create a temporary shape in world space.
		hclTaperedCapsuleShape* createTransformedShape( const hkTransform& transform, hclShapeBuffer& buffer ) const;

		HCL_SHAPE_FORWARD_VIRTUAL_METHODS();

			/// Tapered capsules use an AABB bounding volume
		struct BoundingVolume
		{
			BoundingVolume (const hclTaperedCapsuleShape& shape, hkSimdRealParameter expansionRadius, const hkTransform& worldFromCollidable);
			HK_INLINE hkBool32 containsPoint(const hkVector4& point) const;
			hkAabb m_aabb;
		};


	protected:

		//
		// DATA MEMBERS
		//

		hkVector4 m_small;
		hkVector4 m_big;
		hkVector4 m_coneApex; // Cone apex in local space
		hkVector4 m_coneAxis; // Normalized vector from cone apex to base in local space.
		hkVector4 m_lVec;
		hkVector4 m_dVec;
		hkVector4 m_tanThetaVecNeg;

		hkReal m_smallRadius;
		hkReal m_bigRadius;

		// Precomputed for efficiency		
		hkReal m_l;           // Distance between spheres
		hkReal m_d;
		hkReal m_cosTheta;
		hkReal m_sinTheta;
		hkReal m_tanTheta;
		hkReal m_tanThetaSqr;
		
	private:

		//
		// INTERNAL METHODS
		//

		static HK_INLINE void _getClosestPointToSphere( const hkVector4& localPos, hkSimdRealParameter particleRadius, hkSimdRealParameter sphereRadius, const hkVector4& c, hkVector4& pointOnSurface, hkVector4& normal, hkSimdReal& signedDistance );

			// Get the distance to the surface from localPos. The normal is simple to compute anyway at the samae time
			// The closet point contact is == localpos + (dir*dist) 
		hkSimdReal _getDistanceToSurface(const hkVector4& localPos, hkVector4& dir, hkVector4& normal) const;

	public:

		hclTaperedCapsuleShape( hkFinishLoadedObjectFlag flag ) : hclShape(flag) { m_type = HCL_SHAPE_TYPE_TAPERED_CAPSULE; }

};

#include <Cloth/Cloth/Collide/Shape/TaperedCapsule/hclTaperedCapsuleShape.inl>

#endif // HCL_SHAPE_TYPE_TAPERED_CAPSULE__H

/*
 * Havok SDK - Base file, BUILD(#20141201)
 * 
 * Confidential Information of Havok.  (C) Copyright 1999-2014
 * Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
 * Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
 * rights, and intellectual property rights in the Havok software remain in
 * Havok and/or its suppliers.
 * 
 * Use of this software for evaluation purposes is subject to and indicates
 * acceptance of the End User licence Agreement for this product. A copy of
 * the license is included with this software and is also available from salesteam@havok.com.
 * 
 */
