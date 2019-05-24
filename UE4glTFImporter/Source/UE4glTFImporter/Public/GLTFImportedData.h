// Fill out your copyright notice in the Description page of Project Settings.
#ifndef GLTF_IMPORTED_DATA_H_
#define GLTF_IMPORTED_DATA_H_ 1


#include <vector.h>
#include <unordered_map>
#include <iterator>

#include "../../Extras/tinygltf/tiny_gltf.h"
#include "ImporterOptions.h"

#include <CoreMinimal.h>
#include "RenderingThread.h"
#include "RawMesh.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "Runtime/Engine/Classes/Engine/Texture.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"

#include "Runtime/Engine/Classes/Materials/MaterialExpressionScalarParameter.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionTextureSampleParameter.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionVectorParameter.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

#include "Engine/SkeletalMesh.h"
#include "Editor/UnrealEd/Public/SkelImport.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"


// TODO: Morph targets in FbxSceneImportFactory.cpp line 1994

// retrieved from: https://forums.unrealengine.com/development-discussion/c-gameplay-programming/50376-measure-time-a-function-has-taken

class UtilityTimer {
public:

  UtilityTimer() {
    TickTime = 0;
    TockTime = 0;
    data = FString("");
  }
  ~UtilityTimer() {}

  void tick() {
    TickTime = FPlatformTime::Seconds();
  }

  double tock() {
    TockTime = FPlatformTime::Seconds();
    return TockTime - TickTime;
  }

  void CollectData(FString Data, double value) {
    data += FString::SanitizeFloat(value) + '\t';
  }

  bool SaveStringTextToFile(
    FString SaveText = "",
    bool AllowOverWriting = true,
    FString FileName = FString("test.txt"),
    FString SaveDirectory = FString("C:/Users/b8021686/Desktop")
  ) {

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
    {
      FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

      if (AllowOverWriting || !PlatformFile.FileExists(*AbsoluteFilePath))
      {
        FFileHelper::SaveStringToFile(data, *AbsoluteFilePath);
      }
    }
    return false;
  }

private:
  FString data;
  double TickTime;
  double TockTime;

};

struct glTFNode {
  FString name = "";
  int32 parent = -1;
  int32 children = 0;
};

struct ImportSkeletalMeshArgs {

  ImportSkeletalMeshArgs() :
    InParent(nullptr)
    , NodeArray()
    , Name(NAME_None)
    , Flags(RF_NoFlags)
    , LodIndex(0)
    , bCancelOperation(nullptr)
    , OutData(nullptr)
  {}

  UObject* InParent;
  TArray<glTFNode> NodeArray;
  FSkeletalMeshImportData* OutData;
  FName Name;
  int32 LodIndex;
  EObjectFlags Flags;
  bool* bCancelOperation;

};

struct glTFAnimation {
  
  struct Channel {
    int32 sampler = -1;
    int32 node = -1;
    FString path = "";

    struct Sampler {
      int32 input = -1;
      int32 output = -1;
      FString interpolation = "LINEAR";
    } AnimSampler;
  };

  TArray<Channel> AnimChannel;

};

struct SkinData {
  FString name = "";
  int32 root_node = -1;
  FMatrix inverseBindMatrix;
  TArray<int32> jointHierachy;
};

struct AnimationFrameData {
  FVector translation = FVector::ZeroVector;
  FQuat rotation = FQuat::Identity;
  FVector scale = FVector::ZeroVector;
  FString anim_interpolation = "";
};

struct AnimationKeyFrame {
  FString name = "";
  //float value is the key frame time
  //int32 value is the target node of that frame data
  TMap<float, TMap<int32, AnimationFrameData>> anim_key_frame_data;
};

struct Material {

  FString name = TEXT("");
  int id = -1;
  
  struct {
    int texture = -1;
    FString name = TEXT("");
    FVector4 factor = FVector4(1.0f, 1.0f, 1.0f, 1.0f );
  } BaseColor;

  struct {
    int texture = -1;
    FString name = TEXT("");
    float metallic_factor = 0.0f;
    float roughness_factor = 1.0f;
  } MetallicRoughness;

  struct {
    int texture = -1;
    FString name = TEXT("");
    float factor = 0.0f;
  } Normal;

  struct {
    int texture = -1;
    FString name = TEXT("");
    float factor = 0.0f;
  } Occlusion;

  struct {
    int texture = -1;
    FString name = TEXT("");
    FVector4 factor = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
  } Emissive;

  struct {
    float alphaCutoff = 0.0f;
    FString alphaMode = TEXT("OPAQUE");
  } Alpha;

  bool doubleSided = false;

};

struct Texture {
  FString uri = "";
  FString name = "";
  TArray<uint8> ImageData;

  struct {
    uint16_t width = 1;
    uint16_t height = 1;

    TextureFilter Filter = TF_Default;
    TextureAddress Wrapping[2] = { TA_Wrap, TA_Wrap };

  } Info;

};

class GLTFImportedData
{
public:
	GLTFImportedData();
	~GLTFImportedData();

  UStaticMesh* ImportData(FString InFilename, UClass* InClass, UObject* InParent,
    FName InName, EObjectFlags InFlags);

  UStaticMesh* CreateStaticMesh(TSharedPtr<tinygltf::Model>& InModel, 
    const TWeakPtr<FImporterOptions>& InImporterOptions);

  bool GenerateRawMesh(TSharedPtr<tinygltf::Model>& InModel, tinygltf::Node& InNode,
    FRawMesh& OutRawMesh, const FMatrix& InFatherMatrix, 
    const TWeakPtr<FImporterOptions>& InImporterOptions);

  //// GLTF
  bool RetrieveDataFromglTF(TSharedPtr<tinygltf::Model>& InModel,
    tinygltf::Primitive& InPrimitive, TArray<uint32>& OutTriangleIndices, 
    TArray<FVector>& OutPositions, TArray<FVector>& OutNormals,
    TArray<FVector4>& OutTangents, TArray<FColor>& OutColours,
    TArray<FVector2D> OutTextureCoords[MAX_MESH_TEXTURE_COORDS],
    TArray<FVector4>& OutJoints, TArray<FVector4>& OutWeights);
  
  template<typename ComponentType>
  void RetrieveDataFromBufferVec2D(TArray<FVector2D>& OutComponentArray, uint32 DataCount,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  template<typename ComponentType>
  void RetrieveDataFromBufferVec(TArray<FVector>& OutComponentArray, uint32 DataCount,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  template<typename ComponentType>
  void RetrieveDataFromBufferVec4(TArray<FVector4>& OutComponentArray, uint32 DataCount,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  template<typename ComponentType>
  void RetrieveDataFromBufferColor(TArray<FColor>& OutComponentArray, uint32 DataCount,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  void SetAccesorTypeVec2D(TArray<FVector2D>& OutComponentArray,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  void SetAccesorTypeVec(TArray<FVector>& OutComponentArray,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  void SetAccesorTypeVec4(TArray<FVector4>& OutComponentArray,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  void SetAccesorTypeColor(TArray<FColor>& OutComponentArray,
    tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
    tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName,
    std::string& ComponentName);

  /////////////

  //// STATIC MESH
  bool GenerateMeshFromglTF(TSharedPtr<tinygltf::Model>& InModel, 
    tinygltf::Primitive& InPrimitive, FRawMesh& OutRawMesh, 
    const FMatrix& InFatherMatrix);

  bool ProcessStaticMesh(UStaticMesh* InOutStaticMesh, FRawMesh& InRawMesh,
    const TWeakPtr<FImporterOptions>& InImporterOptions);

  bool CreatMeshFromRaw(FRawMesh& InRawMesh, std::string& InMeshName,
    const TWeakPtr<FImporterOptions>& InImporterOptions);

  bool MergeRawMesh(FRawMesh& InRawMesh, FRawMesh& OutRawMesh);
  /////////////////////

  //// UTILITIES
  bool IsMeshValidOrFixeable(FRawMesh& InRawMesh);

  void SwapYZ(FVector &InOutVector);
  
  void SwapYZ(TArray<FVector> &InOutVectorArray);
  ////////////////////

  //// MATERIALS
  void AssignMaterials(FRawMesh& InRawMesh, UStaticMesh* InMesh, 
    const TWeakPtr<FImporterOptions>& InImporterOptions);

  UMaterial* CreateMaterial(const TWeakPtr<FImporterOptions>& InImporterOptions,
    Material& InMaterial);

  template<typename MaterialProperty>
  MaterialProperty* FindPropertyByGuid(UMaterial* InMaterial, const FGuid& InGuid);

  bool ConstructMaterialParameter(const TWeakPtr<FImporterOptions>& InglTFImportOptions,
    UMaterialExpressionTextureSampleParameter* InSampleParameter, 
    const FString& InParameterName, const Texture &InTexture, bool InIsNormal = false);
  /////////////////

  //// TEXTURES
  UTexture* CreateTexture(const TWeakPtr<FImporterOptions>& InglTFImportOptions,
    const Texture &InTexture, bool InIsNormal = false);

  void CreateNewOwnTexture(
    std::vector<tinygltf::Sampler>& InSamplerVec,
    std::vector<tinygltf::Image>& InImageVec,
    std::vector<tinygltf::Texture>& InTexVec, int MaterialValue
  );
  //////////////////

protected:

  UClass* glTFClass;
  UObject* glTFParentClass;
  FName glTFName;
  EObjectFlags glTFFlags;

  FString FilePath;
  
  TMap<FString, Material> MaterialMap;
  TArray<Material> MaterialArray;
  TArray<int32> MaterialIndexArray;
  TMap<FString, UMaterial*> UMaterialMap;
  TMap<FString, Texture> TextureMap;
  TArray<UStaticMesh*> UStaticMeshArray;

  TArray<AnimationKeyFrame> Animations;
  TArray<SkinData> Skins;

  UtilityTimer timer;

};

#endif