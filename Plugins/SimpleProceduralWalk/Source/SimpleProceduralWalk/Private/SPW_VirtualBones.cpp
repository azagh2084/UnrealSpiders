// Copyright Epic Games, Inc. and Roberto Ostinelli, 2021. All Rights Reserved.

#include "AnimNode_SPW.h"
#include "FABRIK.h"


void FAnimNode_SPW::Evaluate_TransformBones(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	if (bIsInitialized)
	{
		const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

		for (int LegIndex = 0; LegIndex < Legs.Num(); LegIndex++)
		{
			FCompactPoseBoneIndex CompactPoseBoneToModify = TipBones[LegIndex].GetCompactPoseIndex(BoneContainer);
			FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
			FTransform ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

			// translation
			FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_WorldSpace);
			NewBoneTM.SetTranslation(LegsData[LegIndex].FootLocation);
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_WorldSpace);
			
			// rotation
			FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, BCS_ComponentSpace);
			const FQuat BoneQuat(LegsData[LegIndex].FootTargetRotation);
			NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, BCS_ComponentSpace);

			// merge
			TArray<FBoneTransform> TempTransforms;
			TempTransforms.Add(FBoneTransform(TipBones[LegIndex].GetCompactPoseIndex(BoneContainer), NewBoneTM));
			Output.Pose.LocalBlendCSBoneTransforms(TempTransforms, 1.f);
		}
	}
}
