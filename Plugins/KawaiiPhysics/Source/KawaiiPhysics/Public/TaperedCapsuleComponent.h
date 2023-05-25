#pragma once

#include "Components/CapsuleComponent.h"
#include "TaperedCapsuleComponent.generated.h"

class FPrimitiveSceneProxy;

/** 
 * A taperedcapsule generally used for simple collision. Bounds are rendered as lines in the editor.
 */
UCLASS(ClassGroup = "Collision", editinlinenew, hidecategories = (Object, LOD, Lighting, TextureStreaming), meta = (DisplayName = "TaperedCapsule Collision", BlueprintSpawnableComponent))
class KAWAIIPHYSICS_API UTaperedCapsuleComponent : public UCapsuleComponent
{
	GENERATED_BODY()
public:
	UTaperedCapsuleComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UObject Interface	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject Interface

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	/** 
	 *	Taper, Adjust sphere radius reduction amount
	 *	This value is clamped between Min to Max values
	 *  If Taper is 1.0, it has the same shape as capsule
	 *  Collision cost of taperedcapsule is slightly more expensive than capsule.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape, meta = (ClampMin = "0.01", ClampMax = "100"))
	float Taper;
	
};
