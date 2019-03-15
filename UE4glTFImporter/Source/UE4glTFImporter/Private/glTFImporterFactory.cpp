// Fill out your copyright notice in the Description page of Project Settings.

#include "glTFImporterFactory.h"

#include "UtilityWindow.h"
#include "UE4glTFImporter.h"
#include "GLTFImportedData.h"

#include "Misc/Paths.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"

#define LOCTEXT_NAMESPACE "FUE4glTFImporterModule"

UglTFImporterFactory::UglTFImporterFactory()
  : Super()
  , ImportClass(nullptr)
{

}

UglTFImporterFactory::UglTFImporterFactory(const FObjectInitializer& InObjectInitializer)
  : Super(InObjectInitializer)
  , ImportClass(nullptr)
{
  

  if (Formats.Num() > 0) {
    Formats.Empty();
  }

  Formats.Add(TEXT("gltf;gltf 2.0"));

  bCreateNew = false;
  bText = true;
  bEditorImport = true;

}

bool UglTFImporterFactory::DoesSupportClass(UClass* InClass)
{
  return (InClass == UStaticMesh::StaticClass()
    || InClass == USkeletalMesh::StaticClass());
}

UClass* UglTFImporterFactory::ResolveSupportedClass()
{
  if (ImportClass == nullptr) 
  {
    ImportClass = UStaticMesh::StaticClass();
  }
    
  return ImportClass;

}

bool UglTFImporterFactory::FactoryCanImport(const FString& InFilename)
{
  return FPaths::GetExtension(InFilename).Equals(TEXT("gltf"), ESearchCase::IgnoreCase);
}

UObject* UglTFImporterFactory::FactoryCreateText(UClass* InClass, UObject* InParent, 
  FName InName, EObjectFlags InFlags, UObject* InContext, const TCHAR* InType, 
  const TCHAR*& InBuffer, const TCHAR* InBufferEnd, FFeedbackContext* InWarn)
{

  int64 BufferSize = InBufferEnd - InBuffer;
  
  FString glTFJson;
  glTFJson.Append(InBuffer, BufferSize);
  return FactoryCreate(InClass, InParent, InName, InFlags, InContext, InType, InWarn, glTFJson);
}

UObject* UglTFImporterFactory::FactoryCreate(UClass* InClass, UObject* InParent, 
  FName InName, EObjectFlags InFlags, UObject* InContext, const TCHAR* InType, 
  FFeedbackContext* InWarn, const FString& InglTFJson, TSharedPtr<class FglTFBuffers> InglTFBuffers /*= nullptr*/)
{
  const FString &FilePathInOS = GetCurrentFilename();

  if (!FPaths::GetBaseFilename(FilePathInOS).Equals(InName.ToString())) {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Filename: %s and name: %s not equal"), *FilePathInOS, *InName.ToString());
#endif
    return nullptr;
  }

  if (!SUtilityWindow::Open(FilePathInOS, InParent->GetPathName())) 
  {
    return nullptr;
  }

  GLTFImportedData importer;
  UStaticMesh *aux = importer.ImportData(FilePathInOS, InClass, InParent, InName, InFlags);
  return aux;

}

#undef LOCTEXT_NAMESPACE
