// Copyright Roberto Ostinelli, 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SPW.h"
#include "SPW_CCDIKSolver.h"
#include "Curves/CurveFloat.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_SPW.generated.h"


USTRUCT()
struct SIMPLEPROCEDURALWALK_API FAnimNode_SPW : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

public:	
	// ---------- \/ MAIN ----------
	/** Should draw the debug elements? */
	UPROPERTY(EditAnywhere, Category = "Simple Procedural Walk")
		bool bDebug = false;

	/** Should scale the values based on the skeletal mesh scale? */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Simple Procedural Walk")
		bool bScaleWithSkeletalMesh = false;

	/** Should detect falling pawn and raise events? */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Simple Procedural Walk")
		bool bDetectFalling = true;

	// ---------- \/ Skeletal Control ----------
	/**
	 * The forward axis of the Skeletal Mesh.
	 * With Debug enabled, ensure that the RED axis goes towards the front of your mesh.
	 */
	UPROPERTY(EditAnywhere, Category = "Skeletal Control")
		ESimpleProceduralWalk_MeshForwardAxis SkeletalMeshForwardAxis;

	/**
	 * The bone that defines the center of the body.
	 * This bone should ideally be placed at the center of the body, otherwise unoptimal animation may happen.
	 */
	UPROPERTY(EditAnywhere, Category = "Skeletal Control")
		FBoneReference BodyBone;

	/** Defines the legs to animate. */
	UPROPERTY(EditAnywhere, Category = "Skeletal Control")
		TArray<FSimpleProceduralWalk_Leg> Legs;

	// ---------- \/ Walk Cycle ----------
	/** Defines the leg groups (the legs in a group will unplant at the same time). */
	UPROPERTY(EditAnywhere, Category = "Walk Cycle")
		TArray<FSimpleProceduralWalk_LegGroup> LegGroups;

	/** How far should the foot desired position be from the tip bone before a step is taken. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Walk Cycle", meta = (ClampMin = "0.0"))
		float MinDistanceToUnplant = 0.f;

	/** Do not adjust feet targets if the step is over this percentage. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Walk Cycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float FixFeetTargetsAfterPercent = 0.f;

	/** The foot rotation interpolation speed. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Walk Cycle", meta = (ClampMin = "0.0"))
		float FeetTipBonesRotationInterpSpeed = 0.f;

	// ---------- \/ Step Control ----------
	/** How hight should the step be above the ground. */
	UPROPERTY(EditAnywhere, Category = "Step Control", meta = (PinHiddenByDefault, ClampMin = "0.0"))
		float StepHeight = 0.f;

	/** How far should the step move forward (and backwards) */
	UPROPERTY(EditAnywhere, Category = "Step Control", meta = (PinHiddenByDefault, ClampMin = "0.0"))
		float StepDistanceForward = 0.f;

	/** How far should the step move sideways */
	UPROPERTY(EditAnywhere, Category = "Step Control", meta = (PinHiddenByDefault, ClampMin = "0.0"))
		float StepDistanceRight = 0.f;

	/**
 * Defines at which percentage of a step the next group of legs will unplant.
	 * With a value of 1, a group will wait for the previous group to finish the step before unplanting.
	 * Values between 0-1 will make a group unplant while the previous group is still unplanted.
	 * With a value of 0, all groups will unplant at the same time.
	 */
	UPROPERTY(EditAnywhere, Category = "Step Control", meta = (PinHiddenByDefault, ClampMin = "0.0", ClampMax = "1.0"))
		float StepSequencePercent = 0.f;

	/** Defines the curve steps. */
	UPROPERTY(EditAnywhere, Category = "Step Control")
		ESimpleProceduralWalk_StepCurveType StepCurveType = ESimpleProceduralWalk_StepCurveType::ROBOT;

	/**
	 * How much should the step distance be reduced based on slope inclination:
	 * 0: No reduction.
	 * 1: With a slope of 90 degrees the step is reduced to 0.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Step Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float StepSlopeReductionMultiplier = 0.f;

	/** The minimum step duration (steps should never take less than this amount of time). */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Step Control", meta = (ClampMin = "0.0"))
		float MinStepDuration = 0.f;

	/** The curve that defines the foot height evolution during a step. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Step Control", meta = (PinHiddenByDefault, EditCondition = "StepCurveType == ESimpleProceduralWalk_StepCurveType::CUSTOM"))
		UCurveFloat* CustomStepHeightCurve = nullptr;

	/** The curve that defines the foot distance evolution during a step. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Step Control", meta = (PinHiddenByDefault, EditCondition = "StepCurveType == ESimpleProceduralWalk_StepCurveType::CUSTOM"))
		UCurveFloat* CustomStepDistanceCurve = nullptr;

	// ---------- \/ Body Location ----------
	/** How much should the body bounce up and down while walking (0 disables it). */
	UPROPERTY(EditAnywhere, Category = "Body Location", meta = (PinHiddenByDefault, ClampMin = "0.0"))
		float BodyBounceMultiplier = 0.f;

	/** How much should the body be lowered to the ground while on a slope. */
	UPROPERTY(EditAnywhere, Category = "Body Location", meta = (PinHiddenByDefault, ClampMin = "0.0"))
		float BodySlopeMultiplier = 0.f;

	/** How fast should the body location movement be interpolated. */
	UPROPERTY(EditAnywhere, Category = "Body Location", meta = (PinHiddenByDefault, ClampMin = "0.0"))
		float BodyLocationInterpSpeed = 0.f;

	/** Additional body offset along the Z axis. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Body Location", meta = (PinHiddenByDefault))
		float BodyZOffset = 0.f;

	// ---------- \/ Body Rotation ----------
	/** Should the body rotate based on change of direction? */
	UPROPERTY(EditAnywhere, Category = "Body Rotation", meta = (PinHiddenByDefault))
		bool bBodyRotateOnAcceleration = false;

	/** Should the body rotate based on feet locations? */
	UPROPERTY(EditAnywhere, Category = "Body Rotation", meta = (PinHiddenByDefault))
		bool bBodyRotateOnFeetLocations = false;

	/** How fast should the body rotation movement be interpolated. */
	UPROPERTY(EditAnywhere, Category = "Body Rotation", meta = (PinHiddenByDefault, ClampMin = "0.0", EditCondition = "bBodyRotateOnAcceleration || bBodyRotateOnFeetLocations"))
		float BodyRotationInterpSpeed = false;

	/** How much should the acceleration influence the body rotation. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Body Rotation", meta = (ClampMin = "0.0", EditCondition = "bBodyRotateOnAcceleration"))
		float BodyAccelerationRotationMultiplier = 0.f;

	/** How much should the feet locations influence the body rotation. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Body Rotation", meta = (ClampMin = "0.0", EditCondition = "bBodyRotateOnFeetLocations"))
		float BodyFeetLocationsRotationMultiplier = 0.f;

	/** Maximum body rotation, per axis: Roll (X), Pitch (Y), and Yaw (Z, ignored). */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Body Rotation", meta = (ClampMin = "0.0", EditCondition = "bBodyRotateOnAcceleration || bBodyRotateOnFeetLocations"))
		FRotator MaxBodyRotation = FRotator(0.f);

	// ---------- \/ Solver ----------
	/** The ADVANCED solver type is more accurate to some world scenarios, but it's more expensive. */
	UPROPERTY(EditAnywhere, Category = "Solver")
		ESimpleProceduralWalk_SolverType SolverType;

	/** How quickly should feet interpolate while the pawn is falling. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Solver", meta = (ClampMin = "0.0"))
		float FeetInAirInterSpeed = 0.f;

	/** Specifies the radius within which to check for existing places where to plant feet. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Solver", meta = (ClampMin = "1.0", ClampMax = "3.0", EditCondition = "SolverType == ESimpleProceduralWalk_SolverType::ADVANCED"))
		float RadiusCheckMultiplier = 0.f;

	/**
	 * Specifies when the basic vertical location where to plant the foot should be abandoned and a location within a radius should be searched for instead.
	 * This is directly related to how much the leg can "extend" its Z axis when going from idle to walking.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Solver", meta = (ClampMin = "1.0", ClampMax = "3.0", EditCondition = "SolverType == ESimpleProceduralWalk_SolverType::ADVANCED"))
		float DistanceCheckMultiplier = 0.f;

	// ---------- \/ IK Solver ----------
	/**
	 * Set to true to use Simple Procedural Walk's internal CCDIK.
	 * Set to false if you plan on using Virtual Bones to drive animations, for instance with Control Rig.
	 */
	UPROPERTY(EditAnywhere, Category = "IK Solver", meta = (PinHiddenByDefault))
		bool bEnableIkSolver = true;

	/** Start computations from tail. */
	UPROPERTY(EditAnywhere, Category = "IK Solver", meta = (ClampMin = "0.0", EditCondition = "bEnableIkSolver"))
		bool bStartFromTail = false;

	/** Tolerance for final tip bone location delta. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "IK Solver", meta = (ClampMin = "0.0", EditCondition = "bEnableIkSolver"))
		float Precision = 0.f;

	/** Maximum number of iterations allowed, to control performance. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "IK Solver", meta = (ClampMin = "0", EditCondition = "bEnableIkSolver"))
		int32 MaxIterations = 0;

	// ---------- \/ Trace ----------
	/**
	 * The trace channel.
	 * It is recommended to have a channel dedicated to feet placement so that, for instance, feet are not placed on top of grass or small foliage.
	 */
	UPROPERTY(EditAnywhere, Category = "Trace")
		TEnumAsByte<ETraceTypeQuery> TraceChannel;

	/** The length of the downwards trace. */
	UPROPERTY(EditAnywhere, Category = "Trace", meta = (ClampMin = "0.0"))
		float TraceLength = 0.f;

	/** Should the trace be complex? */
	UPROPERTY(EditAnywhere, Category = "Trace")
		bool bTraceComplex = false;

	/** Trace offset (from the foot Parent Bone). */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Trace")
		float TraceZOffset = 0.f;

public:
	// Constructor
	FAnimNode_SPW();

	// FAnimNode_Base interface
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;

	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;

	// from graph node: resize rotation limit array based on set up
	void CCDIK_ResizeRotationLimitPerJoints(int32 LegIndex, int32 NewSize);

private:
	// internals
	bool bHasErrors = false;
	bool bIsPlaying = false;
	bool bIsEditorAnimPreview = false;
	bool bIsInitialized = false;
	bool bForceReset = false;
	float WorldDeltaSeconds = 0.f;

	// References
	UPROPERTY()
	UWorld* WorldContext = nullptr;
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	UPROPERTY()
	TArray<FName> VirtualBoneNames;

	// pawn
	bool bIsFalling = false;
	float Speed = 0.f;
	float ForwardPercent = 0.f;
	float RightPercent = 0.f;
	float PreviousSpeed = 0.f;
	float PreviousForwardPercent = 0.f;
	float PreviousRightPercent = 0.f;
	float ForwardAcceleration = 0.f;
	float RightAcceleration = 0.f;
	FRotator PreviousRotation = FRotator(0.f);
	float YawDelta = 0.f;
	float CurrentStepLength = 0.f;
	float CurrentStepDuration = 0.f;

	// pawn data
	UPROPERTY()
	APawn* OwnerPawn = nullptr;
	bool bIsPawnClass;
	float OwnerHalfHeight;
	FVector MeshScale;
	float MeshAverageScale;

	// legs
	UPROPERTY()
	TArray<FSimpleProceduralWalk_LegData> LegsData;

	// groups
	int32 CurrentGroupIndex = 0;
	UPROPERTY()
	TArray<FSimpleProceduralWalk_LegGroupData> GroupsData;

	// body
	FRotator CurrentBodyRelRotation = FRotator(0.f);
	FVector CurrentBodyRelLocation = FVector(0.f);
	float ReduceSlopeMultiplierPitch = 1.f;
	float ReduceSlopeMultiplierRoll = 1.f;

	// IK
	UPROPERTY()
	TArray<FBoneSocketTarget> EffectorTargets;
	UPROPERTY()
	TArray<FBoneReference> ParentBones;
	UPROPERTY()
	TArray<FBoneReference> TipBones;
	UPROPERTY()
	TArray<FSimpleProceduralWalk_RotationLimitsPerJoint> FeetRotationLimitsPerJoints;

	// step
	void CreateDefaultCurves();
	FRichCurve StepHeightCurveRobot;
	FRichCurve StepDistanceCurveRobot;
	FRichCurve StepHeightCurveOrganic;
	FRichCurve StepDistanceCurveOrganic;

	// ---------- \/ computations ----------
	void Initialize_Computations(const FAnimationInitializeContext& Context);
	void Evaluate_Computations();
	void UpdatePawnVariables();
	void SetSupportCompDeltas();
	// walk
	void SetFeetTargetLocations();
	void SetFootTargetLocation(int32 LegIndex);
	void SetCurrentGroupUnplanted();
	void ComputeFeet();
	void SetGroupsPlanted();
	void ResetFeetTargetsAndLocations();
	// body
	void ComputeBodyTransform();
	void ComputeBodyRotation(FVector AverageFeetTargetsForward
		, FVector AverageFeetTargetsBackwards
		, FVector AverageFeetTargetsRight
		, FVector AverageFeetTargetsLeft);
	void ComputeBodyLocation(FVector AverageFeetTargetsForward
		, FVector AverageFeetTargetsBackwards
		, FVector AverageFeetTargetsRight
		, FVector AverageFeetTargetsLeft);
	void GetAverageFeetTargets(FVector* AverageFeetTargetsForward
		, FVector* AverageFeetTargetsBackwards
		, FVector* AverageFeetTargetsRight
		, FVector* AverageFeetTargetsLeft);

	// ix
	void CallStepInterfaces(int32 GroupIndex, bool bIsDown);
	void CallStepInterface(UObject* InterfaceOwner, int32 GroupIndex, bool bIsDown);
	void CallLandedInterfaces();
	void CallLandedInterface(UObject* InterfaceOwner);

	// helpers
	void SetSupportComponentData(int32 LegIndex, FVector RefLocation);
	float GetReductionSlopeMultiplier();
	bool IsLegUnplanted(int32 LegIndex);
	float GetLegStepPercent(int32 LegIndex);
	void SetNextCurrentGroupIndex();
	float GetStepHeightValue(float Time);
	float GetStepDistanceValue(float Time);

	// additional movement
	bool IsFalling();

	// scale
	FVector GetScaledLegOffset(FSimpleProceduralWalk_Leg Leg);
	float GetScaledStepHeight();
	float GetScaledStepDistanceForward();
	float GetScaledStepDistanceRight();
	float GetScaledBodyZOffset();
	float GetScaledTraceLength();
	float GetScaledTraceZOffset();
	float GetScaledMinStepDuration();
	float GetAdaptedMinDistanceToUnplant(int32 LegIndex);

	// debug
	void DebugShow();
	void EditorDebugShow(AActor* SkeletalMeshOwner);

	// BODY
	void Evaluate_BodySolver(FComponentSpacePoseContext& Output);

	// solver
	float RadiusCheck;

	// CCDIK
	void Initialize_CCDIK();
	void Evaluate_CCDIKSolver(FComponentSpacePoseContext& Output);
	FTransform CCDIK_GetTargetTransform(const FTransform& InComponentTransform
		, FCSPose<FCompactPose>& MeshBases
		, FBoneSocketTarget& InTarget
		, const FVector& InOffset);

	bool SolveCCDIK(TArray<FSPW_CCDIKChainLink>& InOutChain
		, const FVector& TargetPosition
		, bool bEnableRotationLimit
		, const TArray<float>& RotationLimitPerJoints);

	void Evaluate_TransformBones(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms);
};
