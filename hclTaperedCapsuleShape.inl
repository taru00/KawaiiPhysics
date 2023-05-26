/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2014 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

	
#include <Cloth/Cloth/SimCloth/hclSimClothInstance.h>

inline const hkVector4& hclTaperedCapsuleShape::getSmall() const
{ 
	return m_small; 
}

inline const hkVector4& hclTaperedCapsuleShape::getBig() const 
{ 
	return m_big; 
}

inline hkReal hclTaperedCapsuleShape::getSmallRadius() const 
{ 
	return m_smallRadius; 
}

inline hkReal hclTaperedCapsuleShape::getBigRadius() const 
{ 
	return m_bigRadius; 
}

inline hkBool32 hclTaperedCapsuleShape::BoundingVolume::containsPoint(const hkVector4& point) const
{
	return m_aabb.containsPoint(point);
}

/*static*/ inline void hclTaperedCapsuleShape::_getClosestPointToSphere( const hkVector4& localPos, hkSimdRealParameter particleRadius, hkSimdRealParameter sphereRadius, const hkVector4& c, hkVector4& pointOnSurface, hkVector4& normal, hkSimdReal& signedDistance )
{
	// Build vector cp from sphere center c to point p
	hkVector4 cp;
	cp.setSub4( localPos, c );

	// Get the distance from sphere center c to point p using the norm of cp and normalize
	hkSimdReal length = cp.normalizeWithLength3();

	// The closest point on the sphere is in the direction of cp scaled by the sphere radius
	pointOnSurface.setAddMul4( c, cp, sphereRadius );

	// The normal is just cp
	normal = cp;

	// The signed distance is the distance from the sphere center c to the point p minus the radius
	signedDistance = length - sphereRadius;
}

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
