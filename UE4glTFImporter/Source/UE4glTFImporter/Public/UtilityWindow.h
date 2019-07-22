/* -----------------------------------------------------------------------------
MIT License

Copyright (c) 2018-2019 Sergio Almajano (sergio22596)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
----------------------------------------------------------------------------- */

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
