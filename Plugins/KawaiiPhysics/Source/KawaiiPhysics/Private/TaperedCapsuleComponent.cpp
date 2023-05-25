#include "TaperedCapsuleComponent.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"

UTaperedCapsuleComponent::UTaperedCapsuleComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	Taper(1.0f)
{

}

#if WITH_EDITOR
void UTaperedCapsuleComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FPrimitiveSceneProxy* UTaperedCapsuleComponent::CreateSceneProxy()
{
	/** Represents a UCapsuleComponent to the scene manager. */
	class FTaperedCapsuleSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FTaperedCapsuleSceneProxy(const UTaperedCapsuleComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, CapsuleRadius(InComponent->CapsuleRadius)
			, CapsuleHalfHeight(InComponent->CapsuleHalfHeight)
			, ShapeColor(InComponent->ShapeColor)
			, Taper(InComponent->Taper)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_GetDynamicMeshElements_DrawDynamicElements);

			const FMatrix& LocalToWorld = GetLocalToWorld();
			const int32 CapsuleSides = FMath::Clamp<int32>(CapsuleRadius / 4.f, 16, 64);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					const FLinearColor DrawCapsuleColor = GetViewSelectionColor(ShapeColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);										
					const float taperedRadius = CapsuleRadius * Taper;
					const float scaledRadius = CapsuleRadius * LocalToWorld.GetScaleVector().X;
					const float scaledTaperedRadius = scaledRadius * Taper;
					const float coneHeight = CapsuleHalfHeight - (scaledRadius + scaledTaperedRadius) / 2;
					DrawWireChoppedCone(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), DrawCapsuleColor, CapsuleRadius, taperedRadius, coneHeight, CapsuleSides, SDPG_World);

					FVector SphereOrigin = LocalToWorld.GetOrigin();
					SphereOrigin -= LocalToWorld.GetScaledAxis(EAxis::Z) * (coneHeight);
					DrawWireSphere(PDI, SphereOrigin, DrawCapsuleColor, scaledRadius, CapsuleSides, SDPG_World);

					FVector TaperedSphereOrigin = LocalToWorld.GetOrigin();
					TaperedSphereOrigin += LocalToWorld.GetScaledAxis(EAxis::Z) * coneHeight;
					DrawWireSphere(PDI, TaperedSphereOrigin, DrawCapsuleColor, scaledTaperedRadius, CapsuleSides, SDPG_World);

					//GetLocalToWorld().ToQuat().Rotator(),
					//DrawSphere(PDI, TaperedSphereOrigin, FRotator::ZeroRotator, FVector(TaperedRadius), CapsuleSides, CapsuleSides, GEngine->ArrowMaterial->GetRenderProxy(true), SDPG_World, true);
					//DrawWireSphere(PDI, PDI, LocalToWorld.GetOrigin(), TopRadius, CapsuleSides, SDPG_World);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const uint32	bDrawOnlyIfSelected : 1;
		const float		CapsuleRadius;
		const float		CapsuleHalfHeight;
		const FColor	ShapeColor;
		const float		Taper = 1.2f;
	};

	return new FTaperedCapsuleSceneProxy(this);
}
