// Fill out your copyright notice in the Description page of Project Settings.

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
