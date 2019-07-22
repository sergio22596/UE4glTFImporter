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

#include "UtilityWindow.h"
#include "UE4glTFImporter.h"

#include "MainFrame.h"
#include "IDocumentation.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorStyle.h"

#define LOCTEXT_NAMESPACE "FUE4glTFImporterModule"

SUtilityWindow::EMethodChoice SUtilityWindow::EImportChoice = SUtilityWindow::EMethodChoice_Together;

SUtilityWindow::SUtilityWindow()
  : SCompoundWidget()
  , SImporterOptions(nullptr)
  , WMainWindow(nullptr)
{

}

void SUtilityWindow::Construct(const FArguments& InArgs)
{
  
  WMainWindow = InArgs._WMainWindow;
  SImporterOptions = InArgs._SImporterOptions;
  
  checkf(SImporterOptions.IsValid(), TEXT("Why the argument - glTFImportOptions is null?"));
  

  TArray<TSharedPtr<FString>> Options;
  Options.Add(MakeShareable(new FString("Import the mesh together")));
  Options.Add(MakeShareable(new FString("Import the mesh separated")));
  Options.Add(MakeShareable(new FString("Other")));

  CurrentItem = Options[0];

  EImportChoice = EMethodChoice_Together;

  ChildSlot
    .VAlign(VAlign_Fill)
    .HAlign(HAlign_Fill)
  [
    SNew(SVerticalBox)
    + SVerticalBox::Slot()
      .AutoHeight()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Top)
      .Padding(5)
    [
      SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
      [
        SNew(SGridPanel)
        ///////////// Map Import /////////////
        + SGridPanel::Slot(0, 0)
            .Padding(10)
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Left)
        [
          //SNew(SComboBox<TSharedPtr<FString>>)
          //  .OptionsSource(&Options)
          //  .OnGenerateWidget(this, &SUtilityWindow::MakeWidgetForOption)
          //  .OnSelectionChanged(this, &SUtilityWindow::OnSelectionChanged)
          //  //.OnComboBoxOpening(this, &SUtilityWindow::OnComboMenuOpening)
          //  .InitiallySelectedItem(CurrentItem)
          //  [
          //    SNew(STextBlock)
          //      .Text(this, &SUtilityWindow::GetCurrentItemLabel)
          //  ]
          SNew(SBorder)
          .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
          [
            SNew(SGridPanel)
            ///////////// Map Import /////////////
            + SGridPanel::Slot(0, 0)
                .Padding(10)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
            [
              SNew(SCheckBox)
              .HAlign(HAlign_Center)
              .Padding(10)
              .IsChecked(this, &SUtilityWindow::HandleImportMethod, EMethodChoice_Together)
              .OnCheckStateChanged(this, &SUtilityWindow::HandleChangeImportMethod, EMethodChoice_Together)
            ]
            + SGridPanel::Slot(1, 0)
              .Padding(10)
              .VAlign(VAlign_Center)
              .HAlign(HAlign_Left)
            [
              SNew(STextBlock)
                .Text(LOCTEXT("HandleImportMethodTogether_Text", "Importing Meshes together"))
            ]
            + SGridPanel::Slot(0, 1)
                .Padding(10)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Left)
            [
              SNew(SCheckBox)
              .HAlign(HAlign_Center)
              .Padding(10)
              .IsChecked(this, &SUtilityWindow::HandleImportMethod, EMethodChoice_Separate)
              .OnCheckStateChanged(this, &SUtilityWindow::HandleChangeImportMethod, EMethodChoice_Separate)
            ]
            + SGridPanel::Slot(1, 1)
              .Padding(10)
              .VAlign(VAlign_Center)
              .HAlign(HAlign_Left)
              [
                SNew(STextBlock)
                .Text(LOCTEXT("HandleImportMethodSparated_Text", "Import Meshes separated"))
              ]
          ]
        ]
        + SGridPanel::Slot(1, 0)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleImportMethod_Text", "Importing method"))
        ]
        ///////////// Skeletal Mesh Import /////////////
        + SGridPanel::Slot(0, 1)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bImportWithSkeletalMesh ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleSkeletalMeshImport)
        ]
        + SGridPanel::Slot(1, 1)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleSkeletalMeshImport_Text", "Import with Skeletal Mesh"))
        ]
        ///////////// Animations Import /////////////
        + SGridPanel::Slot(0, 2)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bImportAnimations ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleAnimationsImport)
        ]
        + SGridPanel::Slot(1, 2)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleAnimationsImport_Text", "Import with Animations"))
        ]
        ///////////// Morphs Import /////////////
        + SGridPanel::Slot(0, 3)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bImportMorphs ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleMorphsImport)
        ]
        + SGridPanel::Slot(1, 3)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleMorphsImport_Text", "Import with Target Morphs"))
        ]
        ///////////// Material Import /////////////
        + SGridPanel::Slot(0, 4)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bImportMaterial ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleMaterialImport)
        ]
        + SGridPanel::Slot(1, 4)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleMaterialImport_Text", "Import with Material"))
        ]
      ]
    ]
    + SVerticalBox::Slot()
      .AutoHeight()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Top)
      .Padding(5)
    [
      SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
      [
        SNew(SGridPanel)
        ///////////// Mesh Scale Ratio Import /////////////
        + SGridPanel::Slot(0, 0)
            .Padding(10)
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Left)
        [
          SNew(SSpinBox<float>)
            .Value(SImporterOptions.Pin()->MeshScaleRatio)
            .MinValue(0.0f)
            .MaxValue(100000.0f)
            .OnValueChanged(this, &SUtilityWindow::HandleMeshScaleRatio)
     
        ]
        + SGridPanel::Slot(1, 0)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleMeshScaleRatioImport_Text", "Mesh Scale Ratio"))
        ]
        ///////////// Invert Normals Import /////////////
        + SGridPanel::Slot(0, 1)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bInvertNormals ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleMeshInvertNormals)
        ]
        + SGridPanel::Slot(1, 1)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleInvertNormalsImport_Text", "Import with Normals Inverted"))
        ]
        ///////////// Recompute Normals Import /////////////
        + SGridPanel::Slot(0, 2)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bRecomputeNormals ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleMeshRecomputeNormals)
        ]
        + SGridPanel::Slot(1, 2)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleRecomputeNormalsImport_Text", "Import with Normals Recomputed"))
        ]
        ///////////// Recompute Tangents Import /////////////
        + SGridPanel::Slot(0, 3)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bRecomputeTangents ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleMeshRecomputeTangents)
        ]
        + SGridPanel::Slot(1, 3)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandleRecomputeTangentsImport_Text", "Import with Tangents Recomputed"))
        ]
      ]
    ]
    + SVerticalBox::Slot()
      .AutoHeight()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Top)
      .Padding(5)
    [
      SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
      [
        SNew(SGridPanel)
        ///////////// PBR Import /////////////
        + SGridPanel::Slot(0, 0)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(SCheckBox)
            .HAlign(HAlign_Center)
            .Padding(10)
            .IsChecked(SImporterOptions.Pin()->bUsePBR ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
            .OnCheckStateChanged(this, &SUtilityWindow::HandleMaterialPBR)
        ]
        + SGridPanel::Slot(1, 0)
          .Padding(10)
          .VAlign(VAlign_Center)
          .HAlign(HAlign_Left)
        [
          SNew(STextBlock)
            .Text(LOCTEXT("HandlePBRImport_Text", "Import Material with PBR"))
        ]
      ]
    ]
    + SVerticalBox::Slot()
      .AutoHeight()
      .Padding(5)
    [
      SNew(SBox)
        
    ]
    + SVerticalBox::Slot()
      .AutoHeight()
      .Padding(5)
      .HAlign(HAlign_Right)
    [
      SNew(SUniformGridPanel)
        .SlotPadding(5)
      + SUniformGridPanel::Slot(0,0)
      [
        IDocumentation::Get()->CreateAnchor(FString("http://www.google.com"))
      ]
      + SUniformGridPanel::Slot(1,0)
      [
        SNew(SButton)
          .HAlign(HAlign_Center)
          .ToolTipText(LOCTEXT("UtilityWindow_CancelAction_ToolTip", "Cancel import of glTF"))
          .Text(LOCTEXT("UtilityWindow_CancelAction", "Cancel"))
          .OnClicked(this, &SUtilityWindow::OnCancel)
      ]
      + SUniformGridPanel::Slot(2, 0)
      [
        SNew(SButton)
          .HAlign(HAlign_Center)
          .ToolTipText(LOCTEXT("UtilityWindow_AcceptAction_ToolTip", "Import glTF file"))
          .Text(LOCTEXT("UtilityWindow_AcceptAction", "Import"))
          .IsEnabled(this, &SUtilityWindow::CanImport)
          .OnClicked(this, &SUtilityWindow::OnImport)
      ]
    ]
  ];

}

bool SUtilityWindow::Open(const FString& InFilePathInOS, const FString& InFilePathInEngine)
{
  TSharedPtr<FImporterOptions> ImporterOptions = MakeShareable(new FImporterOptions());
  *ImporterOptions = FImporterOptions::Current;

  ImporterOptions->FilePathInOS = InFilePathInOS;
  ImporterOptions->FilePathInEngine = InFilePathInEngine;

  TSharedPtr<SWindow> SParentWindow;

  if (FModuleManager::Get().IsModuleLoaded("MainFrame")) 
  {
    IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
    SParentWindow = MainFrame.GetParentWindow();
  }

  TSharedRef<SWindow> SChildWindow = SNew(SWindow)
    .Title(LOCTEXT("WindowTitle", "glTF Importer"))
    .SizingRule(ESizingRule::Autosized);

  TSharedPtr<SUtilityWindow> ImporterWindow;

  SChildWindow->SetContent
  (
    SAssignNew(ImporterWindow, SUtilityWindow)
      .SImporterOptions(ImporterOptions)
      .WMainWindow(SChildWindow)
  );

  FSlateApplication::Get().AddModalWindow(SChildWindow, SParentWindow);

  if (!FImporterOptions::Current.bAcceptedImport) {
#ifdef SHOW_INFO_LOGGER
    UE_LOG(ImporterLog, Log, TEXT("Import re Cancelled"));
#endif
    return false;
  }
  
  switch (EImportChoice)
  {
  case EMethodChoice_Together:
    ImporterOptions->bImportMeshesSeparate = false;
    ImporterOptions->bImportMeshesTogether = true;
    break;
  case EMethodChoice_Separate:
    ImporterOptions->bImportMeshesTogether = false;
    ImporterOptions->bImportMeshesSeparate = true;
    break;
  }


  FImporterOptions::Current = *ImporterOptions;

  return (FImporterOptions::Current.bAcceptedImport == true);

}

bool SUtilityWindow::CanImport() const
{
  return true;
}

FReply SUtilityWindow::OnImport()
{
  if (WMainWindow.IsValid())
  {
    WMainWindow.Pin()->RequestDestroyWindow();
  }

  SImporterOptions.Pin()->bAcceptedImport = true;
  FImporterOptions::Current = *SImporterOptions.Pin();

#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Import accepted"));
#endif
  return FReply::Unhandled();
}

FReply SUtilityWindow::OnCancel()
{

  if (WMainWindow.IsValid())
  {
    WMainWindow.Pin()->RequestDestroyWindow();
  }
  SImporterOptions.Pin()->bAcceptedImport = false;

#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Import cancelled"));
#endif

  return FReply::Handled();

}

ECheckBoxState SUtilityWindow::HandleImportMethod(EMethodChoice Choice) const
{
  return (EImportChoice == Choice)
    ? ECheckBoxState::Checked
    : ECheckBoxState::Unchecked;
}

void SUtilityWindow::HandleChangeImportMethod(ECheckBoxState InState, EMethodChoice ChangedChoice)
{
  if ((InState == ECheckBoxState::Checked)) {
    EImportChoice = ChangedChoice;
  }
}

void SUtilityWindow::HandleSkeletalMeshImport(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bImportWithSkeletalMesh = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Skeletal Mesh Value Changed"));
#endif
}

void SUtilityWindow::HandleAnimationsImport(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bImportAnimations = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Animations Value Changed"));
#endif
}

void SUtilityWindow::HandleMorphsImport(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bImportMorphs = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Morphs Value Changed"));
#endif
}

void SUtilityWindow::HandleMaterialImport(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bImportMaterial = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Material Value Changed"));
#endif
}

void SUtilityWindow::HandleMeshScaleRatio(float InValue)
{
  SImporterOptions.Pin()->MeshScaleRatio = InValue;
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Mesh scale Value Changed"));
#endif
}

void SUtilityWindow::HandleMeshInvertNormals(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bInvertNormals = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Inv Norm Value Changed"));
#endif
}

void SUtilityWindow::HandleMeshRecomputeNormals(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bRecomputeNormals = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Rec Norm Value Changed"));
#endif
}

void SUtilityWindow::HandleMeshRecomputeTangents(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bRecomputeTangents = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("Rec Tang Value Changed"));
#endif
}

void SUtilityWindow::HandleMaterialPBR(ECheckBoxState InState)
{
  SImporterOptions.Pin()->bUsePBR = (InState == ECheckBoxState::Checked);
#ifdef SHOW_INFO_LOGGER
  UE_LOG(ImporterLog, Log, TEXT("PBR Value Changed"));
#endif
}

FText SUtilityWindow::GetCurrentItemLabel() const {
  if (CurrentItem.IsValid())
  {
    return FText::FromString(*CurrentItem);
  }

  return LOCTEXT("InvalidComboEntryText", "<<Invalid option>>");
}

void SUtilityWindow::OnSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type)
{
  if (NewValue.IsValid()) {
    CurrentItem = NewValue;
  }

  if (CurrentItem.IsValid()) {
    if (!CurrentItem->Compare("Import the mesh together")) {
      SImporterOptions.Pin()->bImportMeshesSeparate = true;
      SImporterOptions.Pin()->bImportMeshesTogether = false;
    }
    else {
      SImporterOptions.Pin()->bImportMeshesSeparate = false;
      SImporterOptions.Pin()->bImportMeshesTogether = true;
    }
  }
}

TSharedRef<SWidget> SUtilityWindow::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
  return SNew(STextBlock).Text(FText::FromString(*InOption));
}

#undef LOCTEXT_NAMESPACE
