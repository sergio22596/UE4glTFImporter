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
