/*
 *
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Product and Trade Secret source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2014 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 *
 */

#include <Cloth/Cloth/hclCloth.h>
#include <Cloth/Cloth/SimCloth/hclSimClothInstance.h>
#include <Cloth/Cloth/Collide/Shape/TaperedCapsule/hclTaperedCapsuleShape.h>

#if !defined HK_PLATFORM_SPU

HK_COMPILE_TIME_ASSERT(sizeof(hclTaperedCapsuleShape) < hclShape::MAX_SHAPE_SIZE);

hclTaperedCapsuleShape::hclTaperedCapsuleShape (const hkVector4& start, const hkVector4& end, hkReal startRadius, hkReal endRadius) 
: hclShape(HCL_SHAPE_TYPE_TAPERED_CAPSULE)
{
	if (startRadius > endRadius)
	{
		m_big = start; 
		m_small = end; 
		m_bigRadius = startRadius;
		m_smallRadius = endRadius; 
	}
	else
	{
		m_big = end; 
		m_small = start; 
		m_bigRadius = endRadius;
		m_smallRadius = startRadius; 
	}

	if ( (m_bigRadius - m_smallRadius)/m_bigRadius < 0.02f )
	{
		HK_WARN_ALWAYS(0x49f8d103, "Invalid tapered capsule: radii must differ by more than 2 percent.");
	}

	hkReal r = m_smallRadius;
	hkReal R = m_bigRadius;

	// Precompute various quantities
	m_coneAxis.setSub4(m_big, m_small);
	m_l = m_coneAxis.normalizeWithLength3();
	HK_ASSERT2(0x760ce927, m_l != 0.0, "Cannot create a hclTaperedCapsuleShape of zero length");

	m_lVec.setAll(m_l);

	hkReal H = R * ( m_l * m_l - (R - r) * (R - r) ) / ((R - r) * m_l);
	m_sinTheta = (R - r) / m_l;

	if ( m_sinTheta > 1.0f )
	{
		HK_WARN_ALWAYS(0x540bdaa8, "Invalid tapered capsule: R - r < l.");
	}	

	m_d = r / m_sinTheta;
	m_dVec.setAll(m_d);

	m_coneApex = m_small;
	m_coneApex.subMul4(m_d, m_coneAxis);

	hkReal W = hkMath::sqrt(H * R * m_sinTheta);
	m_tanTheta = W / H;
	m_tanThetaSqr = m_tanTheta * m_tanTheta;
	m_tanThetaVecNeg.setAll(m_tanTheta);
	m_tanThetaVecNeg.setNeg4(m_tanThetaVecNeg);

	m_cosTheta = H / hkMath::sqrt(W*W + H*H);
}

#endif // HK_PLATFORM_SPU


hclTaperedCapsuleShape* hclTaperedCapsuleShape::createTransformedShape( const hkTransform& transform, hclShapeBuffer& buffer ) const
{
	hkVector4 smallCtr;
	smallCtr._setTransformedPos( transform, m_small );

	hkVector4 bigCtr;
	bigCtr._setTransformedPos( transform, m_big );

#if defined (HK_PLATFORM_SPU)
	hclTaperedCapsuleShape* transformedShape = reinterpret_cast<hclTaperedCapsuleShape*> (&buffer);
	transformedShape->m_type = HCL_SHAPE_TYPE_TAPERED_CAPSULE;
 
	// unchanged quantities
	transformedShape->m_smallRadius = m_smallRadius; 
	transformedShape->m_bigRadius = m_bigRadius;
	transformedShape->m_l = m_l;
	transformedShape->m_lVec = m_lVec;
	transformedShape->m_sinTheta = m_sinTheta;
	transformedShape->m_cosTheta = m_cosTheta;
	transformedShape->m_tanTheta = m_tanTheta;
	transformedShape->m_tanThetaSqr = m_tanThetaSqr;
	transformedShape->m_tanThetaVecNeg = m_tanThetaVecNeg;
	transformedShape->m_d = m_d;
	transformedShape->m_dVec = m_dVec;

	// transformed quantities
	transformedShape->m_small = smallCtr; 
	transformedShape->m_big = bigCtr;

	hkVector4 axis;
	axis.setRotatedDir( transform.getRotation(), m_coneAxis );
	transformedShape->m_coneAxis = axis;

	transformedShape->m_coneApex = smallCtr;
	transformedShape->m_coneApex.subMul4(m_d, axis);
#else
	hclTaperedCapsuleShape* transformedShape = new (&buffer ) hclTaperedCapsuleShape( smallCtr, bigCtr, m_smallRadius, m_bigRadius );
#endif

	return transformedShape;
}


hclTaperedCapsuleShape::BoundingVolume::BoundingVolume(const hclTaperedCapsuleShape& shape, hkSimdRealParameter expansionRadius, const hkTransform& worldFromCollidable)
{
	hkAabb worldAabb;
	{
		hkVector4 smallCtr;
		smallCtr._setTransformedPos( worldFromCollidable, shape.m_small );

		hkVector4 bigCtr;
		bigCtr._setTransformedPos( worldFromCollidable, shape.m_big );

		// Compute world AABB
		hkAabb smallAabb, bigAabb;
		smallAabb.setEmpty();
		bigAabb.setEmpty();
		smallAabb.includePoint(smallCtr);
		bigAabb.includePoint(bigCtr);
		smallAabb.expandBy(shape.m_smallRadius);
		bigAabb.expandBy(shape.m_bigRadius);

		worldAabb.setEmpty();
		worldAabb.includeAabb(smallAabb);
		worldAabb.includeAabb(bigAabb);
	}

	worldAabb.expandBy(expansionRadius);
	m_aabb = worldAabb;
}


void hclTaperedCapsuleShape::getClosestPoint( const hkVector4& P, hkSimdRealParameter particleRadius, const hkUint16 particleIndex, hkVector4& pointOnSurface, hkVector4& normal, hkSimdReal& signedDistance ) const
{
	const hkVector4& V = m_coneApex;
	const hkVector4& lhat = m_coneAxis;

	hkSimdReal z;
	hkVector4 tmp;
	tmp.setSub4(P, V);
	z = tmp.dot3(lhat);

	hkVector4 xperp;
	xperp.setCross(tmp, lhat);
	xperp.setCross(lhat, xperp);

	hkSimdReal xperpmag = xperp.normalizeWithLength3();
	hkSimdReal coneband = hkSimdReal(m_d) - hkSimdReal(m_tanTheta) * xperpmag;

	if ( z <= coneband )
	{
		// Closest point is on small sphere
		_getClosestPointToSphere(P, particleRadius, m_smallRadius, m_small, pointOnSurface, normal, signedDistance);
	}
	else if ( z >= hkSimdReal(m_l) + coneband)
	{
		// Closest point is on big sphere
		_getClosestPointToSphere(P, particleRadius, m_bigRadius, m_big, pointOnSurface, normal, signedDistance);
	}
	else
	{
		// Closest point is on cone
		hkSimdReal sd = -( hkSimdReal(m_tanTheta) * z - xperpmag ) * hkSimdReal(m_cosTheta);
		signedDistance = sd;

		hkVector4 N;
		N.setMul4(m_cosTheta, xperp);
		N.subMul4(m_sinTheta, m_coneAxis);
		N.normalize3();

		pointOnSurface.setAddMul4(P, N, -sd);
		normal = N;
	}
}


void hclTaperedCapsuleShape::getClosestPoint4( const hkVector4 P[ 4 ],const hkVector4& particleRadii, const hkUint16 particleIndices[ 4 ], 
											   hkVector4 pointOnSurface[ 4 ], hkVector4 normal[ 4 ], hkVector4& signedDistance ) const
{	
	hkVector4 PV0, PV1, PV2, PV3;
	PV0.setSub4(P[0], m_coneApex);
	PV1.setSub4(P[1], m_coneApex);
	PV2.setSub4(P[2], m_coneApex);
	PV3.setSub4(P[3], m_coneApex);

	hkVector4 zVec;
	hkVector4Util::dot3_4vs4( PV0, m_coneAxis, PV1, m_coneAxis, PV2, m_coneAxis, PV3, m_coneAxis, zVec );

	hkVector4 xperp[4];
	xperp[0].setCross(PV0, m_coneAxis);
	xperp[1].setCross(PV1, m_coneAxis);
	xperp[2].setCross(PV2, m_coneAxis);
	xperp[3].setCross(PV3, m_coneAxis);
	xperp[0].setCross(m_coneAxis, xperp[0]);
	xperp[1].setCross(m_coneAxis, xperp[1]);
	xperp[2].setCross(m_coneAxis, xperp[2]);
	xperp[3].setCross(m_coneAxis, xperp[3]);

	hkVector4 xperpmag;
	hclShape::_batchNormalizeWithLength3( xperp[0], xperp[1], xperp[2], xperp[3], xperpmag );

	hkVector4 coneBand;
	coneBand.setAddMul4(m_dVec, m_tanThetaVecNeg, xperpmag);

	hkVector4 coneBandMax;
	coneBandMax.setAdd4(coneBand, m_lVec);

	hkVector4 sdCone = xperpmag;
	sdCone.subMul4(m_tanTheta, zVec);
	sdCone.mul4(m_cosTheta);

	hkVector4Comparison compMinBand = zVec.compareLessThan4( coneBand );
	hkVector4Comparison compMaxBand = zVec.compareGreaterThan4( coneBandMax );

	hkVector4 normalCone, tmp;
	hkSimdReal sd0, sd1, sd2, sd3;

	// Particle 0
	if ( compMinBand.anyIsSet(hkVector4ComparisonMask::MASK_X) )	   
	{
		_getClosestPointToSphere( P[0], particleRadii(0), m_smallRadius, m_small, pointOnSurface[0], normal[0], sd0 );
	}
	else if ( compMaxBand.anyIsSet(hkVector4ComparisonMask::MASK_X) )  
	{
		_getClosestPointToSphere( P[0], particleRadii(0), m_bigRadius, m_big, pointOnSurface[0], normal[0], sd0 );
	}
	else															
	{
		sd0 = sdCone.getSimdAt(0);

		normalCone.setMul4(m_cosTheta, xperp[0]);
		normalCone.subMul4(m_sinTheta, m_coneAxis);
		normalCone.fastNormalize3();
		normal[0] = normalCone;

		tmp = P[0];
		tmp.subMul4(sd0, normalCone);
		pointOnSurface[0] = tmp;
	}
	
	// Particle 1
	if ( compMinBand.anyIsSet(hkVector4ComparisonMask::MASK_Y) )	   
	{
		_getClosestPointToSphere( P[1], particleRadii(1), m_smallRadius, m_small, pointOnSurface[1], normal[1], sd1 );
	}
	else if ( compMaxBand.anyIsSet(hkVector4ComparisonMask::MASK_Y) )  
	{
		_getClosestPointToSphere( P[1], particleRadii(1), m_bigRadius, m_big, pointOnSurface[1], normal[1], sd1 );
	}
	else															
	{
		sd1 = sdCone.getSimdAt(1);

		normalCone.setMul4(m_cosTheta, xperp[1]);
		normalCone.subMul4(m_sinTheta, m_coneAxis);
		normalCone.fastNormalize3();
		normal[1] = normalCone;

		tmp = P[1];
		tmp.subMul4(sd1, normalCone);
		pointOnSurface[1] = tmp;
	}
	
	// Particle 2
	if ( compMinBand.anyIsSet(hkVector4ComparisonMask::MASK_Z) )	   
	{
		_getClosestPointToSphere( P[2], particleRadii(2), m_smallRadius, m_small, pointOnSurface[2], normal[2], sd2 );
	}
	else if ( compMaxBand.anyIsSet(hkVector4ComparisonMask::MASK_Z) )  
	{
		_getClosestPointToSphere( P[2], particleRadii(2), m_bigRadius, m_big, pointOnSurface[2], normal[2], sd2 );
	}
	else															
	{
		sd2 = sdCone.getSimdAt(2);

		normalCone.setMul4(m_cosTheta, xperp[2]);
		normalCone.subMul4(m_sinTheta, m_coneAxis);
		normalCone.fastNormalize3();
		normal[2] = normalCone;

		tmp = P[2];
		tmp.subMul4(sd2, normalCone);
		pointOnSurface[2] = tmp;
	}
	
	// Particle 3
	if ( compMinBand.anyIsSet(hkVector4ComparisonMask::MASK_W) )	   
	{
		_getClosestPointToSphere( P[3], particleRadii(3), m_smallRadius, m_small, pointOnSurface[3], normal[3], sd3 );
	}
	else if ( compMaxBand.anyIsSet(hkVector4ComparisonMask::MASK_W) )  
	{
		_getClosestPointToSphere( P[3], particleRadii(3), m_bigRadius, m_big, pointOnSurface[3], normal[3], sd3 );
	}
	else															
	{
		sd3 = sdCone.getSimdAt(3);

		normalCone.setMul4(m_cosTheta, xperp[3]);
		normalCone.subMul4(m_sinTheta, m_coneAxis);
		normalCone.fastNormalize3();
		normal[3] = normalCone;

		tmp = P[3];
		tmp.subMul4(sd3, normalCone);
		pointOnSurface[3] = tmp;
	}
	
	signedDistance.set(sd0, sd1, sd2, sd3);
}

/*
 * Havok SDK - Product file, BUILD(#20141201)
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
