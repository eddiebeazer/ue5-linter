// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "MarketplaceNamingConvention.h"


UMarketplaceNamingConvention::UMarketplaceNamingConvention(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#define SCRIPT_PATH(ScriptPath)	TSoftClassPtr<UObject>(FSoftObjectPath(TEXT("/Script/" #ScriptPath)))
#define ADD_PREFIX(ClassName, Prefix)	ClassNamingConventions.Push(FLinterNamingConventionInfo(TSoftClassPtr<UObject>(FSoftObjectPath(TEXT("/Script/Engine." #ClassName))), Prefix));
#define ADD_PREFIX_SUFFIX(ClassName, Prefix, Suffix)	ClassNamingConventions.Push(FLinterNamingConventionInfo(TSoftClassPtr<UObject>(FSoftObjectPath(TEXT("/Script/Engine." #ClassName))), Prefix, Suffix));

	// Animation
	ADD_PREFIX(AimOffsetBlendSpace, "AO_");
	ADD_PREFIX(AimOffsetBlendSpace1D, "AO_");
	ADD_PREFIX(AnimBlueprint, "ABP_");
	ADD_PREFIX(AnimComposite, "AC_");
	ADD_PREFIX(AnimMontage, "AM_");
	ADD_PREFIX(AnimSequence, "A_");
	ADD_PREFIX(BlendSpace, "BS_");
	ADD_PREFIX(BlendSpace1D, "BS_");
	ADD_PREFIX(MorphTarget, "MT_");
	ADD_PREFIX(Rig, "Rig_");
	ADD_PREFIX(SkeletalMesh, "SK_");
	ADD_PREFIX(Skeleton, "SKEL_");


	// Artificial Intelligence
	ADD_PREFIX(AIController, "AIC_");
	ADD_PREFIX(BehaviorTree, "BT_");
	ADD_PREFIX(BlackboardData, "BB_");
	ADD_PREFIX(BTDecorator, "BTDecorator_");
	ADD_PREFIX(BTService, "BTService_");
	ADD_PREFIX(BTTaskNode, "BTTask_");

	// Blueprints
	ADD_PREFIX(Blueprint, "BP_");
	ADD_PREFIX(BlueprintFunctionLibrary, "BPFL_");
	ADD_PREFIX(Interface, "BPI_");
	ClassNamingConventions.Push(FLinterNamingConventionInfo(SCRIPT_PATH("IntroTutorials.EditorTutorial"), "TBP_"));
	ADD_PREFIX(UserDefinedEnum, "E");
	ADD_PREFIX(UserDefinedStruct, "F");

	// Materials
	ADD_PREFIX(Material, "M_");
	ADD_PREFIX(Material, "MA_");
	ADD_PREFIX(Material, "MAT_");
	ADD_PREFIX(MaterialFunction, "MF_");
	ADD_PREFIX(MaterialInstance, "MI_");
	ADD_PREFIX(MaterialInstanceConstant, "MI_");
	ADD_PREFIX(MaterialParameterCollection, "MPC_");
	ADD_PREFIX(SubsurfaceProfile, "SP_");

	// Textures
	ADD_PREFIX(Texture2D, "T_");
	ADD_PREFIX(TextureCube, "TC_");
	ADD_PREFIX(TextureRenderTarget2D, "RT_");
	ADD_PREFIX(TextureRenderTargetCube, "RTC_");
	ADD_PREFIX(TextureLightProfile, "TLP_");

	// Media
	ClassNamingConventions.Push(FLinterNamingConventionInfo(SCRIPT_PATH("MediaAssets.MediaTexture"), "MT_"));
	ClassNamingConventions.Push(FLinterNamingConventionInfo(SCRIPT_PATH("MediaAssets.MediaPlayer"), "MP_"));

	// Miscellaneous
	ADD_PREFIX(VectorFieldAnimated, "VFA_");
	ADD_PREFIX(CameraAnim, "CA_");
	ADD_PREFIX(CurveLinearColor, "Curve_");
	ADD_PREFIX(CurveTable, "Curve_");
	ADD_PREFIX(DataTable, "DT_");
	ADD_PREFIX(CurveFloat, "Curve_");
	ADD_PREFIX(ForceFeedbackEffect, "FFE_");
	ADD_PREFIX(MatineeAnimInterface, "Matinee_");
	ADD_PREFIX(ObjectLibrary, "OL_");
	ADD_PREFIX(VectorFieldStatic, "VF_");
	ADD_PREFIX(TouchInterface, "TI_");
	ADD_PREFIX(CurveVector, "Curve_");
	ADD_PREFIX(StaticMesh, "SM_");
	ADD_PREFIX(StaticMesh, "S_");

	// Paper 2D

	// Physics
	ADD_PREFIX(PhysicalMaterial, "PM_");
	ADD_PREFIX(PhysicsAsset, "PHYS_");

	// Sounds
	ADD_PREFIX(DialogueVoice, "DV_");
	ADD_PREFIX(DialogueWave, "DW_");
	ADD_PREFIX(ReverbEffect, "Reverb_");
	ADD_PREFIX(SoundAttenuation, "ATT_");
	ADD_PREFIX(SoundClass, "");
	ADD_PREFIX(SoundConcurrency, "_SC");
	ADD_PREFIX_SUFFIX(SoundCue, "A_", "_Cue");
	ADD_PREFIX(SoundMix, "Mix_");
	ADD_PREFIX(SoundWave, "A_");

	// User Interface
	ADD_PREFIX(Font, "Font_");
	ADD_PREFIX(SlateBrushAsset, "Brush_");
	ADD_PREFIX(SlateWidgetStyleAsset, "Style_");
	ADD_PREFIX(WidgetBlueprint, "WBP_");

	// Effects
	ADD_PREFIX(ParticleSystem, "PS_");

#undef ADD_PREFIX

	SortConventions();
}

