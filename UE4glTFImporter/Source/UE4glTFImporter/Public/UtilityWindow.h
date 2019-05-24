// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#undef PlaySound

#include "SlateBasics.h"
#include "ImporterOptions.h"

class UE4GLTFIMPORTER_API SUtilityWindow : public SCompoundWidget
{

public:

  enum EMethodChoice
  {
    EMethodChoice_Together = 0,
    EMethodChoice_Separate,
  };

  SUtilityWindow();


  SLATE_BEGIN_ARGS(SUtilityWindow) 
    : _SImporterOptions(nullptr)
    , _WMainWindow(nullptr) 
  {}

    SLATE_ARGUMENT(TSharedPtr<struct FImporterOptions>, SImporterOptions)
    SLATE_ARGUMENT(TSharedPtr<SWindow>, WMainWindow)

  SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

  static bool Open(const FString& InFilePathInOS, const FString& InFilePathInEngine);

  static bool bImport;

  static EMethodChoice EImportChoice;

protected:

  TSharedPtr<FString> CurrentItem;
  TSharedPtr<SComboBox<TSharedPtr<FString> > > SelectorComboBox;

  bool CanImport() const;

  FReply OnImport();
  FReply OnCancel();

  ECheckBoxState HandleImportMethod(EMethodChoice Choice) const;
  void HandleChangeImportMethod(ECheckBoxState InState, EMethodChoice Choice);

  void HandleSkeletalMeshImport(ECheckBoxState InState);
  void HandleAnimationsImport(ECheckBoxState InState);
  void HandleMorphsImport(ECheckBoxState InState);
  void HandleMaterialImport(ECheckBoxState InState);

  void HandleMeshScaleRatio(float InValue);
  void HandleMeshInvertNormals(ECheckBoxState InState);
  void HandleMeshRecomputeNormals(ECheckBoxState InState);
  void HandleMeshRecomputeTangents(ECheckBoxState InState);

  void HandleMaterialPBR(ECheckBoxState InState);

  TSharedRef<SWidget> MakeWidgetForOption(TSharedPtr<FString>  InOption);
  void OnSelectionChanged(TSharedPtr<FString>  NewValue, ESelectInfo::Type);
  FText GetCurrentItemLabel() const;

  TWeakPtr<struct FImporterOptions> SImporterOptions;
	TWeakPtr<SWindow> WMainWindow;

};
