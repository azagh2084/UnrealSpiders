// Copyright Roberto Ostinelli, 2021. All Rights Reserved.

#include "AnimNode_SPW.h"
#include "SPW.h"
#include "Animation/AnimInstanceProxy.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Curves/CurveFloat.h"
#include "SimpleProceduralWalkInterface.h"
#include "Async/Async.h"

// log
DEFINE_LOG_CATEGORY(LogSimpleProceduralWalk);


FAnimNode_SPW::FAnimNode_SPW() : Super()
, bDebug(false)
, bScaleWithSkeletalMesh(true)
, bDetectFalling(true)
, SkeletalMeshForwardAxis(ESimpleProceduralWalk_MeshForwardAxis::Y)
, BodyBone()
, Legs()
, LegGroups()
, MinDistanceToUnplant(5.f)
, FixFeetTargetsAfterPercent(.5f)
, FeetTipBonesRotationInterpSpeed(15.f)
, StepHeight(20.f)
, StepDistanceForward(50.f)
, StepDistanceRight(30.f)
, StepSequencePercent(1.f)
, StepCurveType(ESimpleProceduralWalk_StepCurveType::ROBOT)
, StepSlopeReductionMultiplier(.75f)
, MinStepDuration(0.15f)
, CustomStepHeightCurve()
, CustomStepDistanceCurve()
, BodyBounceMultiplier(.5f)
, BodySlopeMultiplier(.5f)
, BodyLocationInterpSpeed(10.f)
, BodyZOffset()
, bBodyRotateOnAcceleration(true)
, bBodyRotateOnFeetLocations(true)
, BodyRotationInterpSpeed(2.5f)
, BodyAccelerationRotationMultiplier(.1f)
, BodyFeetLocationsRotationMultiplier(.75f)
, MaxBodyRotation(FRotator(45.f, 0.f, 45.f))
, SolverType(ESimpleProceduralWalk_SolverType::ADVANCED)
, FeetInAirInterSpeed(15.f)
, RadiusCheckMultiplier(1.5f)
, DistanceCheckMultiplier(1.2f)
, bEnableIkSolver(true)
, bStartFromTail(false)
, Precision(1.f)
, MaxIterations(10)
, TraceChannel()
, TraceLength(350.f)
, bTraceComplex(true)
, TraceZOffset(50.f)
{
	CreateDefaultCurves();
}

void FAnimNode_SPW::CreateDefaultCurves()
{
	/** ROBOT */
	// -----\/----- height curve
	FRichCurveKey RHKey0 = FRichCurveKey(0.f, 0.f, 2.8878f, 2.8878f, ERichCurveInterpMode::RCIM_Cubic);
	FRichCurveKey RHKey1 = FRichCurveKey(0.5f, 1.f, 2.8878f, 2.8878f, ERichCurveInterpMode::RCIM_Cubic);
	FRichCurveKey RHKey2 = FRichCurveKey(1.f, 0.f, 2.8878f, 2.8878f, ERichCurveInterpMode::RCIM_Cubic);

	RHKey0.TangentMode = ERichCurveTangentMode::RCTM_Break;
	RHKey1.TangentMode = ERichCurveTangentMode::RCTM_Auto;
	RHKey2.TangentMode = ERichCurveTangentMode::RCTM_Break;

	TArray<FRichCurveKey> RHCurveKeys;
	RHCurveKeys.Add(RHKey0);
	RHCurveKeys.Add(RHKey1);
	RHCurveKeys.Add(RHKey2);
	StepHeightCurveRobot.SetKeys(RHCurveKeys);

	/** ORGANIC */
	// -----\/----- height curveasdasd
	FRichCurveKey OHKey0 = FRichCurveKey(0.f, 0.f, 2.8878f, 2.8878f, ERichCurveInterpMode::RCIM_Cubic);
	FRichCurveKey OHKey1 = FRichCurveKey(0.2f, 1.f, 0.f, 0.f, ERichCurveInterpMode::RCIM_Cubic);
	FRichCurveKey OHKey2 = FRichCurveKey(1.f, 0.f, -2.8878f, -2.8878f, ERichCurveInterpMode::RCIM_Cubic);

	OHKey0.TangentMode = ERichCurveTangentMode::RCTM_Auto;
	OHKey1.TangentMode = ERichCurveTangentMode::RCTM_Auto;
	OHKey2.TangentMode = ERichCurveTangentMode::RCTM_Auto;

	TArray<FRichCurveKey> OHCurveKeys;
	OHCurveKeys.Add(OHKey0);
	OHCurveKeys.Add(OHKey1);
	OHCurveKeys.Add(OHKey2);
	StepHeightCurveOrganic.SetKeys(OHCurveKeys);

	/** COMMON */
	// -----\/----- distsadance curve
	FRichCurveKey DKey0 = FRichCurveKey(0.f, 0.f, 0.f, 0.f, ERichCurveInterpMode::RCIM_Cubic);
	FRichCurveKey DKey1 = FRichCurveKey(1.f, 1.f, 0.f, 0.f, ERichCurveInterpMode::RCIM_Cubic);

	DKey0.TangentMode = ERichCurveTangentMode::RCTM_Auto;
	DKey1.TangentMode = ERichCurveTangentMode::RCTM_Auto;

	TArray<FRichCurveKey> DCurveKeys;
	DCurveKeys.Add(DKey0);
	DCurveKeys.Add(DKey1);

	StepDistanceCurveRobot.SetKeys(DCurveKeys);
	StepDistanceCurveOrganic.SetKeys(DCurveKeys);
}

void FAnimNode_SPW::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine);
	ComponentPose.GatherDebugData(DebugData);
}

void FAnimNode_SPW::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Entering InitializeBoneReferences."));

	Super::InitializeBoneReferences(RequiredBones);

	// init body bone
	BodyBone.Initialize(RequiredBones);
	UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("Body bone %s initialized."), *BodyBone.BoneName.ToString());

	// bones
	ParentBones.Reset();
	TipBones.Reset();
	EffectorTargets.Reset();

	for (FSimpleProceduralWalk_Leg Leg : Legs)
	{
		if (Leg.ParentBone.Initialize(RequiredBones))
		{
			// CCDIK exclude the parent bone from the solver, so in order to keep a simple UX in selecting the bones,
			// we have to add the parent's parent here.
			// NB: the fact that the parent bone is NOT root is ensured by the validation in the AnimGraphNode.
			const FCompactPoseBoneIndex ParentParentIndex = RequiredBones.GetParentBoneIndex(Leg.ParentBone.GetCompactPoseIndex(RequiredBones));
			FBoneReference ParentParentBone = RequiredBones.GetReferenceSkeleton().GetBoneName(ParentParentIndex.GetInt());

			if (ParentParentBone.Initialize(RequiredBones))
			{
				ParentBones.Emplace(ParentParentBone);
				UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("%s bone's parent initialized."), *Leg.ParentBone.BoneName.ToString());
			}
			else
			{
				UE_LOG(LogSimpleProceduralWalk, Error, TEXT("Could not initialize %s bone's parent."), *Leg.ParentBone.BoneName.ToString());
			}

			// init effector target
			FBoneSocketTarget EffectorTarget = FBoneSocketTarget(ParentParentBone.BoneName);
			EffectorTarget.InitializeBoneReferences(RequiredBones);
			EffectorTargets.Emplace(EffectorTarget);
		}
		else
		{
			UE_LOG(LogSimpleProceduralWalk, Error, TEXT("Could not initialize bone %s."), *Leg.ParentBone.BoneName.ToString());
		}

		if (Leg.TipBone.Initialize(RequiredBones))
		{
			TipBones.Emplace(Leg.TipBone);
			UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("%s bone initialized."), *Leg.TipBone.BoneName.ToString());
		}
		else
		{
			UE_LOG(LogSimpleProceduralWalk, Error, TEXT("Could not initialize bone %s."), *Leg.TipBone.BoneName.ToString());
		}
	}
}

bool FAnimNode_SPW::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("IsValidToEvaluate"));

	if (BodyBone.BoneIndex != INDEX_NONE)
	{
		if (!BodyBone.IsValidToEvaluate(RequiredBones))
		{
			UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: %s is not valid"), *BodyBone.BoneName.ToString());
			return false;
		}
	}

	for (int BoneIndex = 0; BoneIndex < ParentBones.Num(); BoneIndex++)
	{
		if (BoneIndex < ParentBones.Num())
		{
			if (!ParentBones[BoneIndex].IsValidToEvaluate(RequiredBones))
			{
				UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: parent bone %s is not valid"), *ParentBones[BoneIndex].BoneName.ToString());
				return false;
			}
		}

		if (BoneIndex < TipBones.Num())
		{
			if (!TipBones[BoneIndex].IsValidToEvaluate(RequiredBones))
			{
				UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: tip bone %s is not valid"), *TipBones[BoneIndex].BoneName.ToString());
				return false;
			}

			if (VirtualBoneNames.Find(TipBones[BoneIndex].BoneName) == INDEX_NONE)
			{
				/* --> not a virtual bone */

				if (!RequiredBones.BoneIsChildOf(TipBones[BoneIndex].BoneIndex, ParentBones[BoneIndex].BoneIndex))
				{
					UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: tip bone %s is not child of parent bone %s")
						, *TipBones[BoneIndex].BoneName.ToString()
						, *ParentBones[BoneIndex].BoneName.ToString());
					return false;
				}
			}
		}
	}

	if (!IsValid(SkeletalMeshComponent))
	{
		UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: SkeletalMeshComponent is not valid."));
		return false;
	}

	if (!IsValid(SkeletalMeshComponent->SkeletalMesh))
	{
		UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: SkeletalMesh is not valid."));
		return false;
	}

	if (Precision <= 0)
	{
		UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("IsValidToEvaluate: Precision is not valid."));
		return false;
	}

	if (bHasErrors)
	{
		return false;
	}

	UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("IsValidToEvaluate is true."));
	return true;
}

void FAnimNode_SPW::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Entering Initialize_AnyThread."));

	Super::Initialize_AnyThread(Context);

	// set common
	SkeletalMeshComponent = Context.AnimInstanceProxy->GetSkelMeshComponent();
	WorldContext = SkeletalMeshComponent->GetWorld();

	// virtual bones array
	VirtualBoneNames.Reset();
	for (FVirtualBone VirtualBone : SkeletalMeshComponent->SkeletalMesh->Skeleton->GetVirtualBones())
	{
		VirtualBoneNames.Add(VirtualBone.VirtualBoneName);
	}

	// owner
	AActor* SkeletalMeshOwner = SkeletalMeshComponent->GetOwner();

	// set is playing & is preview
	bIsEditorAnimPreview = WorldContext->IsEditorWorld() && WorldContext->IsPreviewWorld();
	bIsPlaying = WorldContext->IsGameWorld();

	UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("Is playing: %d, is in editor: %d"), bIsPlaying, bIsEditorAnimPreview);

	if (bIsPlaying)
	{
		// get pawn & character
		OwnerPawn = Cast<APawn>(SkeletalMeshOwner);

		// set class
		bIsPawnClass = !IsValid(Cast<ACharacter>(SkeletalMeshOwner));
		UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("bIsPawnClass: %d"), bIsPawnClass);

		if (!IsValid(OwnerPawn))
		{
			bHasErrors = true;
			UE_LOG(LogSimpleProceduralWalk, Error, TEXT("Owner actor %s must be a Pawn / Character."), *UKismetSystemLibrary::GetDisplayName(SkeletalMeshOwner));
		}

		if (Legs.Num() == 0)
		{
			bHasErrors = true;
			UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("No legs have been specified, so animation is disabled."));
		}

		if (LegGroups.Num() == 0)
		{
			bHasErrors = true;
			UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("No leg groups have been specified, so animation is disabled."));
		}

		// check & init
		if (Legs.Num() > 0 && LegGroups.Num() > 0)
		{
			UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Initializing computations."));
			Initialize_Computations(Context);
			UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Initializing CCDIK."));
			Initialize_CCDIK();
			// reset after inactive period
			bForceReset = true;
		}
		else
		{
			// flag
			bHasErrors = true;
			UE_LOG(LogSimpleProceduralWalk, Warning, TEXT("No legs or groups have been specified, so animation is disabled."));
		}
	}
}

void FAnimNode_SPW::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("Entering EvaluateSkeletalControl_AnyThread."));

	Super::EvaluateSkeletalControl_AnyThread(Output, OutBoneTransforms);

	// set common
	SkeletalMeshComponent = Output.AnimInstanceProxy->GetSkelMeshComponent();
	WorldContext = SkeletalMeshComponent->GetWorld();

	if (bIsPlaying)
	{
		// body
		Evaluate_BodySolver(Output);

		if (bEnableIkSolver)
		{
			// IK
			Evaluate_CCDIKSolver(Output);
		}
		else
		{
			// virtual bones
			Evaluate_TransformBones(Output, OutBoneTransforms);
		}
	}
}

void FAnimNode_SPW::UpdateInternal(const FAnimationUpdateContext& Context)
{
	UE_LOG(LogSimpleProceduralWalk, VeryVerbose, TEXT("Entering UpdateInternal."));

	Super::UpdateInternal(Context);

	WorldDeltaSeconds = Context.GetDeltaTime();

	// set common
	SkeletalMeshComponent = Context.AnimInstanceProxy->GetSkelMeshComponent();
	WorldContext = SkeletalMeshComponent->GetWorld();

	if (bIsPlaying)
	{
		// falling events
		if (bIsInitialized)
		{
			if (bForceReset)
			{
				// reset feet targets & locations
				ResetFeetTargetsAndLocations();
				bForceReset = false;
			}

			if (bDetectFalling)
			{
				if (IsFalling())
				{
					/* -> falling */
					if (!bIsFalling)
					{
						/* -> triggered once after starting to fall */
						UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Pawn started falling."));
						// reset feet targets & locations
						ResetFeetTargetsAndLocations();
						// track falling state
						bIsFalling = true;
					}
				}
				else
				{
					/* -> not falling */
					if (bIsFalling)
					{
						/* -> triggered once after landing on ground */
						UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Pawn landed."));
						// reset falling state
						bIsFalling = false;
						// reset feet targets & locations
						ResetFeetTargetsAndLocations();
						// interface
						CallLandedInterfaces();
					}
				}
			}
		}

		// compute procedurals
		Evaluate_Computations();
	}
	else if (bIsEditorAnimPreview)
	{
		EditorDebugShow(SkeletalMeshComponent->GetOwner());
	}
}

void FAnimNode_SPW::CallLandedInterfaces()
{
	UE_LOG(LogSimpleProceduralWalk, Verbose, TEXT("Calling OnLanded interfaces."));
	// pawn
	if (OwnerPawn->GetClass()->ImplementsInterface(USimpleProceduralWalkInterface::StaticClass()))
	{
		CallLandedInterface(OwnerPawn);
	}
	// anim instance
	if (SkeletalMeshComponent->GetAnimInstance()->GetClass()->ImplementsInterface(USimpleProceduralWalkInterface::StaticClass()))
	{
		CallLandedInterface(SkeletalMeshComponent->GetAnimInstance());
	}
}

void FAnimNode_SPW::CallLandedInterface(UObject* InterfaceOwner)
{
	FVector Location = OwnerPawn->GetActorLocation();

	AsyncTask(ENamedThreads::GameThread, [=]() {
		ISimpleProceduralWalkInterface::Execute_OnPawnLanded(InterfaceOwner, Location);
	});
}

#if WITH_EDITOR
void FAnimNode_SPW::CCDIK_ResizeRotationLimitPerJoints(int32 LegIndex, int32 NewSize)
{
	if (NewSize == 0)
	{
		Legs[LegIndex].RotationLimitPerJoints.Reset();
	}
	else if (Legs[LegIndex].RotationLimitPerJoints.Num() != NewSize)
	{
		int32 StartIndex = Legs[LegIndex].RotationLimitPerJoints.Num();
		Legs[LegIndex].RotationLimitPerJoints.SetNum(NewSize);
		for (int32 Index = StartIndex; Index < Legs[LegIndex].RotationLimitPerJoints.Num(); ++Index)
		{
			Legs[LegIndex].RotationLimitPerJoints[Index] = 30.f;
		}
	}
}
#endif 
