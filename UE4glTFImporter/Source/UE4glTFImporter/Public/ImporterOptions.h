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

#include "ImporterOptions.generated.h"


USTRUCT()
struct UE4GLTFIMPORTER_API FImporterOptions
{
  GENERATED_USTRUCT_BODY()

	FImporterOptions();

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  FString FilePathInOS;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  FString FilePathInEngine;

  // Import options
  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportAsMap;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportMeshesTogether;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportMeshesSeparate;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportMeshesSeparate_and_Together;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportWithSkeletalMesh;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportAnimations;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportMorphs;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bImportMaterial;

  // Mesh options
  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  float MeshScaleRatio;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bInvertNormals;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bRecomputeNormals;

  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bRecomputeTangents;

  // Material options
  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bUsePBR;


  UPROPERTY(EditAnywhere, Category = UE4glTFImporter)
  bool bAcceptedImport;

  static const FImporterOptions Default;
  static FImporterOptions Current;


};
