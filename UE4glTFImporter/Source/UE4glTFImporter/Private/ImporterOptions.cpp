// Fill out your copyright notice in the Description page of Project Settings.

#include "ImporterOptions.h"

FImporterOptions::FImporterOptions()
  : FilePathInOS(TEXT(""))
  , FilePathInEngine(TEXT(""))
  , bImportAsMap(false)
  , bImportMeshesTogether(true)
  , bImportMeshesSeparate(false)
  , bImportMeshesSeparate_and_Together(false)
  , bImportWithSkeletalMesh(false)
  , bImportAnimations(true)
  , bImportMorphs(true)
  , bImportMaterial(true)
  , MeshScaleRatio(1.0f)
  , bInvertNormals(false)
  , bRecomputeNormals(false)
  , bRecomputeTangents(false)
  , bUsePBR(false)
  , bAcceptedImport(false)
{
}

const FImporterOptions FImporterOptions::Default;
FImporterOptions FImporterOptions::Current;