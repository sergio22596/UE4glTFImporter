// Fill out your copyright notice in the Description page of Project Settings.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GLTFImportedData.h"
#include "UE4glTFImporter.h"
#include "ImageLoader.h"

#include <iostream>

#include "Runtime/Engine/Classes/EditorFramework/AssetImportData.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Runtime/Engine/Public/ImageUtils.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "AssetRegistryModule.h"

#include "winbase.h"
#undef UpdateResource

#define MATERIAL_PBRMETALLICROUGHNESS   TEXT("/UE4glTFImporter/Materials/M_PBRMetallicRoughnessOrigin.M_PBRMetallicRoughnessOrigin")
#define MATERIAL_PBRSPECULARGLOSSINESS  TEXT("/UE4glTFImporter/Materials/M_PBRSpecularGlossinessOrigin.M_PBRSpecularGlossinessOrigin")
#define MATERIAL_BASIC  TEXT("/UE4glTFImporter/Materials/M_Basic.M_Basic")

#define LOCTEXT_NAMESPACE "FUE4glTFImporterModule"

static FString GetFilePathWithoutGLTF(const std::string &Filename) {

  if (Filename.find_last_of("\\\\") != std::string::npos) {
    return UTF8_TO_TCHAR(Filename.substr(Filename.find_last_of("\\") + 1).c_str());
  }
  return "";
}

static FString RemoveFileFormat(const std::string &Filename) {
  std::string format = Filename;
  std::replace(format.begin(), format.end(), '.', '_');
  std::string Name = format.substr(format.find_last_of("_"));
  int tokenIndex = format.find_last_of("/");
  FString FullName = UTF8_TO_TCHAR(format.c_str());;
  FullName.RemoveAt(FullName.Len() - Name.size(), Name.size());
  FullName.RemoveAt(0, tokenIndex+1);
  //FString FullName = UTF8_TO_TCHAR(Name.c_str());
  return FullName;
}

static uint32 accesorSizeValues(tinygltf::Accessor & accesor) {

  switch (accesor.componentType)
  {
  case TINYGLTF_COMPONENT_TYPE_BYTE:
    return 1;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    return 1;
  case TINYGLTF_COMPONENT_TYPE_SHORT:
    return 2;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    return 2;
  case TINYGLTF_COMPONENT_TYPE_INT:
    return 4;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    return 4;
  case TINYGLTF_COMPONENT_TYPE_FLOAT:
    return 4;
  default:
    return 0;
  }
}

static uint32 accesorNumComponents(tinygltf::Accessor & accesor) {

  switch (accesor.type)
  {
  case TINYGLTF_TYPE_SCALAR:
    return 1;
  case TINYGLTF_TYPE_VEC2:
    return 2;
  case TINYGLTF_TYPE_VEC3:
    return 3;
  case TINYGLTF_TYPE_VEC4:
    return 4;
  case TINYGLTF_TYPE_MAT2:
    return 4;
  case TINYGLTF_TYPE_MAT3:
    return 9;
  case TINYGLTF_TYPE_MAT4:
    return 16;
  default:
    return 0;
  }

}

static uint32 accesorByteStride(tinygltf::Accessor & accesor) {
  return accesorSizeValues(accesor) * accesorNumComponents(accesor);
}

static TextureFilter TranslateFiltering(int i) {
  switch (i) {
  case TINYGLTF_TEXTURE_FILTER_LINEAR:
    return TF_Bilinear;
  case TINYGLTF_TEXTURE_FILTER_NEAREST:
    return TF_Nearest;
  case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
    return TF_Trilinear;
  }
  return TF_Nearest;
}

static TextureAddress TranslateWrapping(int i) {
  switch (i) {
  case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
    return TA_Clamp;
  case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
    return TA_Mirror;
  case TINYGLTF_TEXTURE_WRAP_REPEAT:
    return TA_Wrap;
  }
  return TA_Wrap;
}

template<typename ComponentType>
void AddTriangleIndices(TArray<uint32>& OutTriangleIndices, uint32 index, tinygltf::Buffer &indexBuffer) {
  OutTriangleIndices.Add(*(ComponentType*)&indexBuffer.data.at(index));
}

GLTFImportedData::GLTFImportedData()
  : glTFClass(nullptr)
  , glTFParentClass(nullptr)
  , glTFName("")
  , glTFFlags(RF_NoFlags)
{
}

GLTFImportedData::~GLTFImportedData()
{
}

UStaticMesh* GLTFImportedData::ImportData(FString InFilename, UClass* InClass, UObject* InParent,
  FName InName, EObjectFlags InFlags)
{
  std::string error;
  std::string warn;
  std::string filename = std::string(TCHAR_TO_UTF8(*InFilename));

  tinygltf::TinyGLTF Loader;
  tinygltf::Model Model;

  auto result = false;

  result = Loader.LoadASCIIFromFile(&Model, &error, &warn, filename);
  
  if (!error.empty()) 
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Error: %s"), error.c_str());
#endif
  }

  if (!result)
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Failed to parse glTF"));
#endif
    return false;
  }
  
  TSharedPtr<FImporterOptions> opts = MakeShareable(new FImporterOptions);
  *opts = FImporterOptions::Current;
  TSharedPtr<tinygltf::Model> model_ = MakeShareable(new tinygltf::Model);
  *model_ = Model;

  glTFClass = InClass;
  glTFParentClass = InParent;
  glTFName = InName;
  glTFFlags = InFlags;

  
  FilePath = GetFilePathWithoutGLTF(std::string(TCHAR_TO_UTF8(*InFilename)));

  InFilename.RemoveAt(InFilename.Len() - FilePath.Len(), FilePath.Len());

  FilePath = InFilename;

  return CreateStaticMesh(model_, opts);

}

UStaticMesh* GLTFImportedData::CreateStaticMesh(TSharedPtr<tinygltf::Model>& InModel, const TWeakPtr<FImporterOptions>& InImporterOptions)
{

  TSharedPtr<FImporterOptions> ImporterOptions = InImporterOptions.Pin();

  tinygltf::Scene NewScene;

  if (InModel->defaultScene < 0) {
    NewScene = InModel->scenes.at(0);
  }
  else {
    NewScene = InModel->scenes.at(InModel->defaultScene);
  }

  FString NewName = UTF8_TO_TCHAR(NewScene.name.c_str());
  std::string name;
  if (NewName.Compare("")) {
    name = NewScene.name;
  }
  else {
    name = TCHAR_TO_UTF8(*glTFName.ToString());
  }

  FRawMesh NewRawMesh;

  for (auto& Scene : InModel->scenes){
    for (auto &SceneNodeID : Scene.nodes) {

      //if (NewScene.nodes.size() < 0 || SceneNodeID >= NewScene.nodes.size()) {
      //  checkSlow(0);
      //  continue;
      //}

      if (!GenerateRawMesh(InModel, InModel->nodes.at(SceneNodeID), NewRawMesh, FMatrix::Identity, InImporterOptions)) {
#ifdef SHOW_ERROR_LOGGER
        UE_LOG(ImporterLog, Error, TEXT("Failed to generate Mesh"));
#endif
        checkSlow(0);
      }
    }
  }
  if (ImporterOptions->bImportMeshesTogether) {
    if (!CreatMeshFromRaw(NewRawMesh, name, InImporterOptions)) {
      return false;
    }
  }

  if (UStaticMeshArray.Num() > 0) {
    return UStaticMeshArray[0];
  }

  return nullptr;
}

bool GLTFImportedData::GenerateRawMesh(TSharedPtr<tinygltf::Model>& InModel, 
  tinygltf::Node& InNode, FRawMesh& OutRawMesh, const FMatrix& InFatherMatrix,
  const TWeakPtr<FImporterOptions>& InImporterOptions)
{
  TSharedPtr<FImporterOptions> ImporterOptions = InImporterOptions.Pin();

  FMatrix Matrix = InFatherMatrix;

  if (InNode.matrix.size() == 16) {
    for (uint32 j = 0; j < 4; ++j) {
      for (uint32 i = 0; i < 4; ++i) {
        Matrix.M[j][i] = InNode.matrix.at(i + j * 4);
      }
    }
    Matrix *= InFatherMatrix;
  }

  if (InNode.translation.size() == 3) {
    
    FVector Trans;
    Trans.X = InNode.translation.at(0);
    Trans.Y = InNode.translation.at(1);
    Trans.Z = InNode.translation.at(2);

    Matrix = FTranslationMatrix::Make(Trans) * Matrix;
  }

  if (InNode.rotation.size() == 4) {

    FQuat Rot;
    Rot.X = InNode.rotation.at(0);
    Rot.Y = InNode.rotation.at(1);
    Rot.Z = InNode.rotation.at(2);
    Rot.W = InNode.rotation.at(3);

    Matrix = FQuatRotationMatrix::Make(Rot) * Matrix;
  }

  if (InNode.scale.size() == 3) {

    FVector Scale;
    Scale.X = InNode.scale.at(0);
    Scale.Y = InNode.scale.at(1);
    Scale.Z = InNode.scale.at(2);

    Matrix = FScaleMatrix::Make(Scale) * Matrix;
  }

  if (InNode.mesh > -1) {

    FRawMesh NewRawMesh;

    if (ImporterOptions->bImportMeshesSeparate) {
      OutRawMesh.Empty();
      MaterialIndexArray.Empty();
    }

    for (auto &primitive : InModel->meshes.at(InNode.mesh).primitives) {
      
      NewRawMesh.Empty();
      if (!GenerateMeshFromglTF(InModel, primitive, NewRawMesh, Matrix)) {

#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to import the static mesh data!"));
#endif
        checkSlow(0);
        continue;
      }
      
      if (!MergeRawMesh(NewRawMesh, OutRawMesh)) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to merge both current and new raw mesh!"));
#endif
        checkSlow(0);
        continue;
      }
    }
    if (ImporterOptions->bImportMeshesSeparate) {
      if (!CreatMeshFromRaw(OutRawMesh, InModel->meshes.at(InNode.mesh).name, InImporterOptions)) {
        return false;
      }
    }
  }

  for (auto i : InNode.children) {
    if (!GenerateRawMesh(InModel, InModel->nodes.at(i), OutRawMesh, Matrix, InImporterOptions)) {
      checkSlow(0);
#ifdef SHOW_ERROR_LOGGER
      UE_LOG(ImporterLog, Error, TEXT("Failed to Generate Node children %d"), i);
#endif
      return false;
    }
  }
  

  return true;
}

bool GLTFImportedData::RetrieveDataFromglTF(TSharedPtr<tinygltf::Model>& InModel, 
  tinygltf::Primitive& InPrimitive, TArray<uint32>& OutTriangleIndices, TArray<FVector>& OutPositions, 
  TArray<FVector>& OutNormals, TArray<FVector4>& OutTangents, TArray<FColor>& OutColours,
  TArray<FVector2D> OutTextureCoords[MAX_MESH_TEXTURE_COORDS],
  TArray<FVector4>& OutJoints, TArray<FVector4>& OutWeights)
{

  OutPositions.Empty();
  OutNormals.Empty();
  OutTangents.Empty();
  for (uint32 i = 0; i < MAX_MESH_TEXTURE_COORDS; ++i) {
    OutTextureCoords[i].Empty();
  }
  OutTriangleIndices.Empty();

  OutJoints.Empty();
  OutWeights.Empty();

  if (InPrimitive.indices < 0) {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Mesh without primitives"));
#endif
    return false;
  }

  for (auto &it : InPrimitive.attributes) {

    tinygltf::Accessor &accesor = InModel->accessors[it.second];

    uint32 valueSize_ = accesorSizeValues(accesor);
    uint32 byteStride_ = accesorByteStride(accesor);

    tinygltf::BufferView &bufferView = InModel->bufferViews[accesor.bufferView];
    tinygltf::Buffer &buffer = InModel->buffers[bufferView.buffer];

    std::string ComponentName = "POSITION";
    SetAccesorTypeVec(OutPositions,buffer, bufferView, accesor, it.first, ComponentName);
    
    ComponentName = "NORMAL";
    SetAccesorTypeVec(OutNormals, buffer, bufferView, accesor, it.first, ComponentName);
    
    for (int32 i = 0; i < 1; ++i) {
    
      char temp[50];
      memset(temp, NULL, 50);
      sprintf_s(temp, 256, "TEXCOORD_0\0");
      ComponentName = temp;
    
      SetAccesorTypeVec2D(OutTextureCoords[i], buffer, bufferView, accesor, it.first, ComponentName);
    }
    
    ComponentName = "TANGENT";
    SetAccesorTypeVec4(OutTangents, buffer, bufferView, accesor, it.first, ComponentName);
    
    ComponentName = "COLOR_0";
    SetAccesorTypeColor(OutColours, buffer, bufferView, accesor, it.first, ComponentName);
    
    ComponentName = "JOINTS_0";
    SetAccesorTypeVec4(OutJoints, buffer, bufferView, accesor, it.first, ComponentName);
    
    ComponentName = "WEIGHTS_0";
    SetAccesorTypeVec4(OutWeights, buffer, bufferView, accesor, it.first, ComponentName);
  }

  tinygltf::Accessor &indexAccesor = InModel->accessors[InPrimitive.indices];
  tinygltf::BufferView &indexBufferView = InModel->bufferViews[indexAccesor.bufferView];
  tinygltf::Buffer &indexBuffer = InModel->buffers[indexBufferView.buffer];

  uint32 valueSize_ = accesorSizeValues(indexAccesor);
  uint32 byteStride_ = accesorByteStride(indexAccesor);

  for (uint32 count = 0; count < indexAccesor.count; count++) {
    uint32 index = indexAccesor.byteOffset + indexBufferView.byteOffset + (count * byteStride_);

    switch (indexAccesor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      AddTriangleIndices<int8>(OutTriangleIndices, index, indexBuffer);
      continue;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      AddTriangleIndices<uint8>(OutTriangleIndices, index, indexBuffer);
      continue;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      AddTriangleIndices<int16>(OutTriangleIndices, index, indexBuffer);
      continue;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      AddTriangleIndices<uint16>(OutTriangleIndices, index, indexBuffer);
      continue;
    case TINYGLTF_COMPONENT_TYPE_INT:
      AddTriangleIndices<int32>(OutTriangleIndices, index, indexBuffer);
      continue;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      AddTriangleIndices<uint32>(OutTriangleIndices, index, indexBuffer);
      continue;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      AddTriangleIndices<float>(OutTriangleIndices, index, indexBuffer);
      continue;
    }

  }
    /////////Materials/////////

  if (InPrimitive.material >= 0) {
    
    tinygltf::Material &mat = InModel->materials[InPrimitive.material];

    MaterialIndexArray.Add(InPrimitive.material);

    if (MaterialMap.Contains(UTF8_TO_TCHAR(mat.name.c_str()))) {
      Material DupMaterial;
      DupMaterial = *MaterialMap.Find(UTF8_TO_TCHAR(mat.name.c_str()));
      DupMaterial.id = InPrimitive.material;
      MaterialArray.Add(DupMaterial);
      return true;
    }
    
    std::vector<tinygltf::Image> &image = InModel->images;

    std::vector<tinygltf::Texture> &texture = InModel->textures;

    std::vector<tinygltf::Sampler> &sampler = InModel->samplers;

    Material glTFMaterial;
    glTFMaterial.id = InPrimitive.material;
    if (mat.name.compare("")) {
      std::replace(mat.name.begin(), mat.name.end(), '.', '_');
      glTFMaterial.name = UTF8_TO_TCHAR(mat.name.c_str());
    }
    else {
      glTFMaterial.name = glTFName.ToString();
    }

    if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
      glTFMaterial.Alpha.alphaCutoff = mat.additionalValues["alphaCutoff"].number_value;
    }

    if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
      glTFMaterial.Alpha.alphaMode = UTF8_TO_TCHAR(mat.additionalValues["alphaMode"].string_value.c_str());
    }

    if (mat.additionalValues.find("doubleSided") != mat.additionalValues.end()) {
      glTFMaterial.doubleSided = mat.additionalValues["doubleSided"].bool_value;
    }
    
    if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
      std::vector<double> emissiveFactor = mat.additionalValues["emissiveFactor"].number_array;
      if (emissiveFactor.size() >= 3) {
        glTFMaterial.Emissive.factor.X = emissiveFactor[0];
        glTFMaterial.Emissive.factor.Y = emissiveFactor[1];
        glTFMaterial.Emissive.factor.Z = emissiveFactor[2];
      }
    }

    if (mat.values.find("baseColorFactor") != mat.values.end()) {
      std::vector<double> colorFactor = mat.values["baseColorFactor"].number_array;
      if (colorFactor.size() >= 4) {
        glTFMaterial.BaseColor.factor.X = colorFactor[0];
        glTFMaterial.BaseColor.factor.Y = colorFactor[1];
        glTFMaterial.BaseColor.factor.Z = colorFactor[2];
        glTFMaterial.BaseColor.factor.W = colorFactor[3];
      }
    }

    if (mat.values.find("metallicFactor") != mat.values.end()) {
      glTFMaterial.MetallicRoughness.metallic_factor = mat.values["metallicFactor"].number_value;
    }

    if (mat.values.find("roughnessFactor") != mat.values.end()) {
      glTFMaterial.MetallicRoughness.roughness_factor = mat.values["roughnessFactor"].number_value;
    }

    if (!image.empty()) {

      if (mat.values.find("baseColorTexture") != mat.values.end()) {
        glTFMaterial.BaseColor.texture = (int)mat.values["baseColorTexture"].json_double_value["index"];
        glTFMaterial.BaseColor.name = RemoveFileFormat(image[texture[glTFMaterial.BaseColor.texture].source].uri);
        CreateNewOwnTexture(sampler, image, texture, glTFMaterial.BaseColor.texture);
      }

      if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
        glTFMaterial.MetallicRoughness.texture = (int)mat.values["metallicRoughnessTexture"].json_double_value["index"];
        glTFMaterial.MetallicRoughness.name = RemoveFileFormat(image[texture[glTFMaterial.MetallicRoughness.texture].source].uri);
        CreateNewOwnTexture(sampler, image, texture, glTFMaterial.MetallicRoughness.texture);
      }

      if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
        glTFMaterial.Emissive.texture = (int)mat.additionalValues["emissiveTexture"].json_double_value["index"];
        glTFMaterial.Emissive.name = RemoveFileFormat(image[texture[glTFMaterial.Emissive.texture].source].uri);
        CreateNewOwnTexture(sampler, image, texture, glTFMaterial.Emissive.texture);
      }

      if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
        glTFMaterial.Normal.texture = (int)mat.additionalValues["normalTexture"].json_double_value["index"];
        glTFMaterial.Normal.name = RemoveFileFormat(image[texture[glTFMaterial.Normal.texture].source].uri);
        CreateNewOwnTexture(sampler, image, texture, glTFMaterial.Normal.texture);
      }

      if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
        glTFMaterial.Occlusion.texture = (int)mat.additionalValues["occlusionTexture"].json_double_value["index"];
        glTFMaterial.Occlusion.name = RemoveFileFormat(image[texture[glTFMaterial.Occlusion.texture].source].uri);
        CreateNewOwnTexture(sampler, image, texture, glTFMaterial.Occlusion.texture);
      }
    }

    MaterialArray.Add(glTFMaterial);
    MaterialMap.Add(glTFMaterial.name, glTFMaterial);
  }
  else {
    MaterialMap.Add("", Material());
  }
  

  //// Animations

  std::vector<tinygltf::Animation> &animation = InModel->animations;

  std::vector <tinygltf::Accessor> &accesor = InModel->accessors;


  if (animation.size() > 0 && Animations.Num() != animation.size()) {
    
    for (auto &anim : animation) {
      AnimationKeyFrame anim_;

      for (auto &channel : anim.channels) {

        
        TArray<FVector> translation;
        TArray<FQuat> rotation;
        TArray<FVector> scale;
        TArray<float> timeArray;

        // key frame time
        tinygltf::Accessor &TimeAnimDataAccesor = accesor[anim.samplers[channel.sampler].input];
        tinygltf::BufferView &TimebufferView = InModel->bufferViews[TimeAnimDataAccesor.bufferView];
        tinygltf::Buffer &Timebuffer = InModel->buffers[TimebufferView.buffer];

        uint32 valueSize_ = accesorSizeValues(TimeAnimDataAccesor);
        uint32 byteStride_ = accesorByteStride(TimeAnimDataAccesor);


        for (uint32 count = 0; count < TimeAnimDataAccesor.count; count++) {
          uint32 index = TimeAnimDataAccesor.byteOffset + TimebufferView.byteOffset + (count * byteStride_);
          float time = *(float*)&Timebuffer.data.at(index);
          
          timeArray.Add(time);

          if (!anim_.anim_key_frame_data.Find(time)) {
            anim_.anim_key_frame_data.Add(time, TMap<int32, AnimationFrameData>());
          }
        }

        // key frame value
        tinygltf::Accessor &AnimDataAccesor = accesor[anim.samplers[channel.sampler].output];
        tinygltf::BufferView &bufferView = InModel->bufferViews[AnimDataAccesor.bufferView];
        tinygltf::Buffer &buffer = InModel->buffers[bufferView.buffer];

        valueSize_ = accesorSizeValues(AnimDataAccesor);
        byteStride_ = accesorByteStride(AnimDataAccesor);

        if (!channel.target_path.compare("translation")) {
          for (uint32 count = 0; count < AnimDataAccesor.count; count++) {
            uint32 index = AnimDataAccesor.byteOffset + bufferView.byteOffset + (count * byteStride_);
            translation.Add(FVector(
              *(float*)&buffer.data.at(index),
              *(float*)&buffer.data.at(index + valueSize_),
              *(float*)&buffer.data.at(index + valueSize_ * 2)
            ));
          }
        }
        
        if (!channel.target_path.compare("rotation")) {
          for (uint32 count = 0; count < AnimDataAccesor.count; count++) {
            uint32 index = AnimDataAccesor.byteOffset + bufferView.byteOffset + (count * byteStride_);
            rotation.Add(FQuat(
              *(float*)&buffer.data.at(index),
              *(float*)&buffer.data.at(index + valueSize_),
              *(float*)&buffer.data.at(index + valueSize_ * 2),
              *(float*)&buffer.data.at(index + valueSize_ * 3)
            ));
          }
        }

        if (!channel.target_path.compare("scale")) {
          for (uint32 count = 0; count < AnimDataAccesor.count; count++) {
            uint32 index = AnimDataAccesor.byteOffset + bufferView.byteOffset + (count * byteStride_);
            scale.Add(FVector(
              *(float*)&buffer.data.at(index),
              *(float*)&buffer.data.at(index + valueSize_),
              *(float*)&buffer.data.at(index + valueSize_ * 2)
            ));
          }
        }

        //if (TMap<int32, AnimationFrameData> *anim_frame_data = anim_.anim_key_frame_data.Find(channel.target_node)) {
          
          //TArray<float> FrameTime;
          //TMap<int32, AnimationFrameData> anim_frame_data;
          //
          //anim_.anim_key_frame_data.GenerateKeyArray(FrameTime);
          //anim_.anim_key_frame_data.GenerateValueArray(anim_frame_data);

        for (int i = 0; i < timeArray.Num(); ++i) {
          if (TMap<int32, AnimationFrameData> *frameDataMap = anim_.anim_key_frame_data.Find(timeArray[i])) {
            if (AnimationFrameData *frameData = frameDataMap->Find(channel.target_node)) {
              
              if (translation.Num() > 0) {
                frameData->translation = translation[i];
              }
              if (rotation.Num() > 0) {
                frameData->rotation = rotation[i];
              }
              if (scale.Num() > 0) {
                frameData->scale = scale[i];
              }

            } else {
              AnimationFrameData tempFrameData;

              if (translation.Num() > 0) {
                tempFrameData.translation = translation[i];
              }
              if (rotation.Num() > 0) {
                tempFrameData.rotation = rotation[i];
              }
              if (scale.Num() > 0) {
                tempFrameData.scale = scale[i];
              }

              frameDataMap->Add(channel.target_node, tempFrameData);
              frameDataMap->KeySort([](int32 A, int32 B) { return A < B; });

            }


          }
        }

        //if (AnimationFrameData *frame_data = frame_data_map.Find(channel.target_node)) {
        //
        //  for (int32 index = 0; index < FrameTime.Num(); ++index) {
        //    AnimationFrameData *frame_data = anim_frame_data->Find(FrameTime[index]);
        //    if (translation.Num() > 0) {
        //      frame_data->translation = translation[index];
        //    }
        //    if (rotation.Num() > 0) {
        //      frame_data->rotation = rotation[index];
        //    }
        //    if (scale.Num() > 0) {
        //      frame_data->scale = scale[index];
        //    }
        //  };
        //
        //  anim_frame_data->KeySort([](float A, float B) { return A < B; });
        //  anim_.anim_key_frame_data_.Add(channel.target_node, *anim_frame_data);
        //}

        //} else {
        //
        //  anim_frame_data = new TMap<float, AnimationFrameData>();
        //
        //  TArray<float> FrameTime;
        //  anim_frame_data->GenerateKeyArray(FrameTime);
        //
        //  for (int32 index = 0; index < FrameTime.Num(); ++index) {
        //    AnimationFrameData frame_data;
        //    if (translation.Num() > 0) {
        //      frame_data.translation = translation[index];
        //    }
        //    if (rotation.Num() > 0) {
        //      frame_data.rotation = rotation[index];
        //    }
        //    if (scale.Num() > 0) {
        //      frame_data.scale = scale[index];
        //    }
        //
        //    anim_frame_data->Add(FrameTime[index], frame_data);
        //  }
        //
        //  anim_.anim_key_frame_data_.Add(channel.target_node, *anim_frame_data);
        //  delete anim_frame_data;
        //}

        anim_.anim_key_frame_data.KeySort([](float A, float B) { return A < B; });

      }
      anim_.name = UTF8_TO_TCHAR(anim.name.c_str());
      Animations.Add(anim_);
    }
  }

  return true;
}

template<typename ComponentType>
void GLTFImportedData::RetrieveDataFromBufferVec2D(TArray<FVector2D>& OutComponentArray, uint32 DataCount,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  uint32 valueSize_ = accesorSizeValues(ComponentAccesor);
  uint32 byteStride_ = accesorByteStride(ComponentAccesor);

  if (PrimitiveAttributeName == ComponentName) {
    for (uint32 count = 0; count < DataCount; count++) {
      uint32 index = ComponentAccesor.byteOffset + ComponentBufferView.byteOffset + (count * byteStride_);

      OutComponentArray.Add(
        FVector2D(
          *(ComponentType*)&ComponentBuffer.data.at(index),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_)
        )
      );
    }
  }
}

template<typename ComponentType>
void GLTFImportedData::RetrieveDataFromBufferVec(TArray<FVector>& OutComponentArray, uint32 DataCount,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  uint32 valueSize_ = accesorSizeValues(ComponentAccesor);
  uint32 byteStride_ = accesorByteStride(ComponentAccesor);

  if (PrimitiveAttributeName == ComponentName) {
    for (uint32 count = 0; count < DataCount; count++) {
      uint32 index = ComponentAccesor.byteOffset + ComponentBufferView.byteOffset + (count * byteStride_);

      OutComponentArray.Add(
        FVector(
          *(ComponentType*)&ComponentBuffer.data.at(index),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_ * 2)
        )
      );
    }
  }
}

template<typename ComponentType>
void GLTFImportedData::RetrieveDataFromBufferVec4(TArray<FVector4>& OutComponentArray, uint32 DataCount,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  uint32 valueSize_ = accesorSizeValues(ComponentAccesor);
  uint32 byteStride_ = accesorByteStride(ComponentAccesor);

  if (PrimitiveAttributeName == ComponentName) {
    for (uint32 count = 0; count < DataCount; count++) {
      uint32 index = ComponentAccesor.byteOffset + ComponentBufferView.byteOffset + (count * byteStride_);

      OutComponentArray.Add(
        FVector4(
          *(ComponentType*)&ComponentBuffer.data.at(index),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_ * 2),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_ * 3)
        )
      );
    }
  }
}

template<typename ComponentType>
void GLTFImportedData::RetrieveDataFromBufferColor(TArray<FColor>& OutComponentArray, uint32 DataCount,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView,
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  uint32 valueSize_ = accesorSizeValues(ComponentAccesor);
  uint32 byteStride_ = accesorByteStride(ComponentAccesor);

  if (PrimitiveAttributeName == ComponentName) {
    for (uint32 count = 0; count < DataCount; count++) {
      uint32 index = ComponentAccesor.byteOffset + ComponentBufferView.byteOffset + (count * byteStride_);

      OutComponentArray.Add(
        FColor(
          *(ComponentType*)&ComponentBuffer.data.at(index),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_ * 2),
          *(ComponentType*)&ComponentBuffer.data.at(index + valueSize_ * 3)
        )
      );
    }
  }
}

void GLTFImportedData::SetAccesorTypeVec2D(TArray<FVector2D>& OutComponentArray,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView, 
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  switch (ComponentAccesor.componentType) {
  case TINYGLTF_COMPONENT_TYPE_BYTE:
    RetrieveDataFromBufferVec2D<int8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    RetrieveDataFromBufferVec2D<uint8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_SHORT:
    RetrieveDataFromBufferVec2D<int16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    RetrieveDataFromBufferVec2D<uint16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_INT:
    RetrieveDataFromBufferVec2D<int32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    RetrieveDataFromBufferVec2D<uint32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_FLOAT:
    RetrieveDataFromBufferVec2D<float>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  }
}

void GLTFImportedData::SetAccesorTypeVec(TArray<FVector>& OutComponentArray,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView, 
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  switch (ComponentAccesor.componentType) {
  case TINYGLTF_COMPONENT_TYPE_BYTE:
    RetrieveDataFromBufferVec<int8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    RetrieveDataFromBufferVec<uint8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_SHORT:
    RetrieveDataFromBufferVec<int16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    RetrieveDataFromBufferVec<uint16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_INT:
    RetrieveDataFromBufferVec<int32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    RetrieveDataFromBufferVec<uint32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_FLOAT:
    RetrieveDataFromBufferVec<float>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  }
}

void GLTFImportedData::SetAccesorTypeVec4(TArray<FVector4>& OutComponentArray,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView, 
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  switch (ComponentAccesor.componentType) {
  case TINYGLTF_COMPONENT_TYPE_BYTE:
    RetrieveDataFromBufferVec4<int8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    RetrieveDataFromBufferVec4<uint8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_SHORT:
    RetrieveDataFromBufferVec4<int16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    RetrieveDataFromBufferVec4<uint16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_INT:
    RetrieveDataFromBufferVec4<int32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    RetrieveDataFromBufferVec4<uint32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_FLOAT:
    RetrieveDataFromBufferVec4<float>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  }
}

void GLTFImportedData::SetAccesorTypeColor(TArray<FColor>& OutComponentArray,
  tinygltf::Buffer &ComponentBuffer, tinygltf::BufferView &ComponentBufferView, 
  tinygltf::Accessor &ComponentAccesor, const std::string PrimitiveAttributeName, std::string& ComponentName)
{
  switch (ComponentAccesor.componentType) {
  case TINYGLTF_COMPONENT_TYPE_BYTE:
    RetrieveDataFromBufferColor<int8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    RetrieveDataFromBufferColor<uint8>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_SHORT:
    RetrieveDataFromBufferColor<int16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    RetrieveDataFromBufferColor<uint16>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_INT:
    RetrieveDataFromBufferColor<int32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    RetrieveDataFromBufferColor<uint32>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  case TINYGLTF_COMPONENT_TYPE_FLOAT:
    RetrieveDataFromBufferColor<float>(OutComponentArray, ComponentAccesor.count, ComponentBuffer, ComponentBufferView,
      ComponentAccesor, PrimitiveAttributeName, ComponentName);
    break;
  }
}

bool GLTFImportedData::GenerateMeshFromglTF(TSharedPtr<tinygltf::Model>& InModel, 
  tinygltf::Primitive& InPrimitive, FRawMesh& OutRawMesh,
  const FMatrix& InFatherMatrix)
{
  TArray<FVector> Positions;
  TArray<FVector> Normals;
  TArray<FVector4> Tangents;
  TArray<FColor> Colours;
  TArray<FVector2D> TextureCoords[MAX_MESH_TEXTURE_COORDS];
  TArray<uint32> TriangleIndices;
  TArray<FVector4> OutJoints;
  TArray<FVector4> OutWeights;

  //tinygltf::Node Node = InModel->nodes.at(InNodeIndex);

  if (!RetrieveDataFromglTF(InModel, InPrimitive, TriangleIndices, Positions, Normals, 
    Tangents, Colours, TextureCoords, OutJoints, OutWeights)) {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Failed To Retrieve glTF data"));
#endif
    return false;
  }

  if (Positions.Num() <= 0) {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Mesh without vertex positions"));
#endif
    return false;
  }
  //TODO: Interleaved vertex doesn't work
  OutRawMesh.Empty();
  
  OutRawMesh.WedgeIndices.Append(TriangleIndices);

  FTransform Trans(InFatherMatrix);
  for (auto &Position : Positions) {
    Position = Trans.TransformPosition(Position);
  }
  
  OutRawMesh.VertexPositions.Append(Positions);

  if (Normals.Num() == Positions.Num()) {
    for (auto &Normal : Normals) {
      Normal = Trans.GetRotation().RotateVector(Normal);
    }
  }
  else {
#ifdef SHOW_WARNING_LOGGER
    UE_LOG(ImporterLog, Warning, TEXT("Mesh without Normals, auto-generated"));
#endif
    Normals.Init(FVector(1.0f, 0.0f, 0.0f), Positions.Num());
  }

  if (Colours.Num() != Positions.Num()) {
    Colours.Init(FColor(1.0f, 1.0f, 1.0f, 1.0f), TriangleIndices.Num());
  }
  OutRawMesh.WedgeColors.Append(Colours);
  
  TArray<FVector> WedgeTangentXs;
  TArray<FVector> WedgeTangentYs;

  if (Tangents.Num() == Positions.Num())
  {
    for (int32 i = 0; i < Tangents.Num(); ++i)
    {
      const FVector4& Tangent = Tangents[i];

      FVector WedgeTangentX(Tangent.X, Tangent.Y, Tangent.Z);
      WedgeTangentX = Trans.GetRotation().RotateVector(WedgeTangentX);
      WedgeTangentXs.Add(WedgeTangentX);

      const FVector& Normal = Normals[i];
      WedgeTangentYs.Add(FVector::CrossProduct(Normal, WedgeTangentX * Tangent.W));
    }
  }
  else
  {
#ifdef SHOW_WARNING_LOGGER
    UE_LOG(ImporterLog, Warning, TEXT("Mesh without Tangents, auto-generated"));
#endif
    WedgeTangentXs.Init(FVector(0.0f, 0.0f, 1.0f), Normals.Num());
    WedgeTangentYs.Init(FVector(0.0f, 1.0f, 0.0f), Normals.Num());
  }

  for (int32 i = 0; i < TriangleIndices.Num(); ++i) {
    OutRawMesh.WedgeTangentX.Add(WedgeTangentXs[TriangleIndices[i]]);
    OutRawMesh.WedgeTangentY.Add(WedgeTangentYs[TriangleIndices[i]]);
    OutRawMesh.WedgeTangentZ.Add(Normals[TriangleIndices[i]]);
  }

  uint32 WedgeIndicesCount = OutRawMesh.WedgeIndices.Num();
  if (TextureCoords[0].Num() <= 0) {
#ifdef SHOW_WARNING_LOGGER
    UE_LOG(ImporterLog, Warning, TEXT("Mesh without TextureCoords, auto-generated"));
#endif
    TextureCoords[0].Init(FVector2D::ZeroVector, WedgeIndicesCount);
  }

  for (uint32 i = 0; i < MAX_MESH_TEXTURE_COORDS; ++i)
  {
    if (TextureCoords[i].Num() <= 0) {
#ifdef SHOW_WARNING_LOGGER
      //UE_LOG(ImporterLog, Warning, TEXT("Mesh info: TEXCOORD_%d empty!"), i);
#endif
      continue;
    }

    OutRawMesh.WedgeTexCoords[i].Append(TextureCoords[i]);
  }

  if (WedgeIndicesCount > 0 && (WedgeIndicesCount % 3) == 0) {

    int32 TriangleCount = OutRawMesh.WedgeIndices.Num() / 3;

    if (OutRawMesh.FaceMaterialIndices.Num() <= 0) {
      OutRawMesh.FaceMaterialIndices.Init(0, TriangleCount);
    }

    if (OutRawMesh.FaceSmoothingMasks.Num() <= 0) {
      OutRawMesh.FaceSmoothingMasks.Init(1, TriangleCount);
    }

    for (uint32 i = 0; i < MAX_MESH_TEXTURE_COORDS; ++i) {

      if (OutRawMesh.WedgeTexCoords[i].Num() <= 0) {
        continue;
      }
      if (OutRawMesh.WedgeTexCoords[i].Num() == WedgeIndicesCount) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("WedgeTexCords Num() equal to wedge index count"));
#endif
      } else if (OutRawMesh.WedgeTexCoords[i].Num() == OutRawMesh.VertexPositions.Num()) {

        TArray<FVector2D> WedgeTexCoords = OutRawMesh.WedgeTexCoords[i];
        OutRawMesh.WedgeTexCoords[i].SetNumUninitialized(WedgeIndicesCount);

        for (int32 j = 0; j < OutRawMesh.WedgeIndices.Num(); ++j) {
          OutRawMesh.WedgeTexCoords[i][j] = WedgeTexCoords[OutRawMesh.WedgeIndices[j] % WedgeTexCoords.Num()];
        }
      }
      else {
        OutRawMesh.WedgeTexCoords[i].SetNumZeroed(WedgeIndicesCount);
      }
    }

  }

  return IsMeshValidOrFixeable(OutRawMesh);

}

bool GLTFImportedData::ProcessStaticMesh(UStaticMesh* InOutStaticMesh, FRawMesh& InRawMesh,
  const TWeakPtr<FImporterOptions>& InImporterOptions)
{
  TSharedPtr<FImporterOptions> ImporterOptions = InImporterOptions.Pin();

  checkSlow(InOutStaticMesh);
  if (!InOutStaticMesh) return false;

  InOutStaticMesh->SourceModels.Empty();

  new(InOutStaticMesh->SourceModels) FStaticMeshSourceModel();
  FStaticMeshSourceModel& SourceModel = InOutStaticMesh->SourceModels[0];


  InOutStaticMesh->LightingGuid = FGuid::NewGuid();
  InOutStaticMesh->LightMapResolution = 64;
  InOutStaticMesh->LightMapCoordinateIndex = 1;
  
  //SourceModel.RawMeshBulkData->LoadRawMesh(InRawMesh);

  if (InRawMesh.IsValidOrFixable()) {

    SwapYZ(InRawMesh.VertexPositions);

    SwapYZ(InRawMesh.WedgeTangentX);
    SwapYZ(InRawMesh.WedgeTangentY);
    SwapYZ(InRawMesh.WedgeTangentZ);

    if (ImporterOptions->bInvertNormals) {
      for (auto &Normal : InRawMesh.WedgeTangentZ) {
        Normal *= -1.0f;
      }
    }

    for (auto &Position : InRawMesh.VertexPositions) {
      Position *= ImporterOptions->MeshScaleRatio;
    }

    SourceModel.BuildSettings.bRecomputeNormals = ImporterOptions->bRecomputeNormals;
    SourceModel.BuildSettings.bRecomputeTangents = ImporterOptions->bRecomputeTangents;
    SourceModel.BuildSettings.bRemoveDegenerates = false;
    SourceModel.BuildSettings.bBuildAdjacencyBuffer = false;
    SourceModel.BuildSettings.bUseFullPrecisionUVs = false;
    SourceModel.BuildSettings.bGenerateLightmapUVs = false;
    SourceModel.BuildSettings.bUseMikkTSpace = true;
    SourceModel.RawMeshBulkData->SaveRawMesh(InRawMesh);

    TArray<FText> BuildErrors;
    InOutStaticMesh->Build(false, &BuildErrors);

    if (BuildErrors.Num() > 0) {

#ifdef SHOW_ERROR_LOGGER
      UE_LOG(ImporterLog, Warning, TEXT("Failed to build the static mesh!"));
      for (auto& Errors : BuildErrors) {

        UE_LOG(ImporterLog, Warning, TEXT("%s"), TCHAR_TO_UTF8(*Errors.ToString()));
      }
#endif

    }
  }

  return true;
}

bool GLTFImportedData::CreatMeshFromRaw(FRawMesh& InRawMesh, std::string& InMeshName,
  const TWeakPtr<FImporterOptions>& InImporterOptions)
{
  FString StaticMeshName;

  std::replace(InMeshName.begin(), InMeshName.end(), '.', '_');

  StaticMeshName = FString::Printf(TEXT("SM_%s"), UTF8_TO_TCHAR(InMeshName.c_str()));

  FString SMPackageName = FPackageName::GetLongPackagePath(glTFParentClass->GetPathName()) / StaticMeshName;
  
  UPackage* AssetPackage = FindPackage(nullptr, *SMPackageName);
  if (!AssetPackage)
  {
  /*UPackage* */AssetPackage = CreatePackage(nullptr, *SMPackageName);
  }

  if (!AssetPackage) return nullptr;

  AssetPackage->FullyLoad();

  UStaticMesh* StaticMesh = NewObject<UStaticMesh>(AssetPackage, FName(*StaticMeshName), glTFFlags);


  ProcessStaticMesh(StaticMesh, InRawMesh, InImporterOptions);
  //StaticMesh->SourceModels.AddDefaulted();
  //FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
  // Assign Material

  AssignMaterials(InRawMesh, StaticMesh, InImporterOptions);

 // bool bMeshUsesEmptyMaterial = MaterialIndexArray.Contains(INDEX_NONE);

  FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels[0];
  FMeshBuildSettings& Settings = SourceModel.BuildSettings;

  SourceModel.RawMeshBulkData->SaveRawMesh(InRawMesh);

  StaticMesh->PostEditChange();
  
  AssetPackage->SetDirtyFlag(true);
  FAssetRegistryModule::AssetCreated(StaticMesh);
  UStaticMeshArray.Add(StaticMesh);

  return true;

}

bool GLTFImportedData::MergeRawMesh(FRawMesh& InRawMesh, FRawMesh& OutRawMesh)
{

  if (InRawMesh.WedgeIndices.Num() <= 0 || InRawMesh.WedgeIndices.Num() % 3) {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Failed to merge meshes"));
#endif
    return false;
  }
  OutRawMesh.FaceMaterialIndices.Append(InRawMesh.FaceMaterialIndices);
  OutRawMesh.FaceSmoothingMasks.Append(InRawMesh.FaceSmoothingMasks);

  int32 StartIndex = OutRawMesh.VertexPositions.Num();

  for (int32 WedgeIndex : InRawMesh.WedgeIndices) {
    OutRawMesh.WedgeIndices.Add(WedgeIndex + StartIndex);
  }

  OutRawMesh.VertexPositions.Append(InRawMesh.VertexPositions);

  OutRawMesh.WedgeTangentX.Append(InRawMesh.WedgeTangentX);
  OutRawMesh.WedgeTangentY.Append(InRawMesh.WedgeTangentY);
  OutRawMesh.WedgeTangentZ.Append(InRawMesh.WedgeTangentZ);

  for (int32 i = 0; i < MAX_MESH_TEXTURE_COORDS; ++i) {
    OutRawMesh.WedgeTexCoords[i].Append(InRawMesh.WedgeTexCoords[i]);
  }

  OutRawMesh.WedgeColors.Append(InRawMesh.WedgeColors);

  return true;

}

USkeletalMesh* GLTFImportedData::CreateSketalMesh(ImportSkeletalMeshArgs &InImportSkeletalMeshArgs)
{
  return nullptr;
}

USkeletalMesh* GLTFImportedData::ImportSkeletalMesh(USkeletalMesh* InMesh)
{
  USkeletalMesh* NewSkeletalMesh = NULL;
  


  return nullptr;
}

bool GLTFImportedData::IsMeshValidOrFixeable(FRawMesh& InRawMesh)
{
  int32 NumVertices = InRawMesh.VertexPositions.Num();
  int32 NumWedges = InRawMesh.WedgeIndices.Num();
  int32 NumFaces = NumWedges / 3;
  int32 NumTexCoords = InRawMesh.WedgeTexCoords[0].Num();
  int32 NumFaceSmoothingMasks = InRawMesh.FaceSmoothingMasks.Num();
  int32 NumFaceMaterialIndices = InRawMesh.FaceMaterialIndices.Num();

  bool bValidOrFixable = NumVertices > 0
    && NumWedges > 0
    && NumFaces > 0
    && (NumWedges / 3) == NumFaces
    && NumFaceMaterialIndices == NumFaces
    && NumFaceSmoothingMasks == NumFaces
    && (InRawMesh.WedgeColors.Num() == NumWedges || InRawMesh.WedgeColors.Num() == 0)
    // All meshes must have a valid texture coordinate.
    && NumTexCoords == NumWedges;

  for (int32 TexCoordIndex = 1; TexCoordIndex < MAX_MESH_TEXTURE_COORDS; ++TexCoordIndex)
  {
    bValidOrFixable = bValidOrFixable && 
      (InRawMesh.WedgeTexCoords[TexCoordIndex].Num() == NumWedges || InRawMesh.WedgeTexCoords[TexCoordIndex].Num() == 0);
  }

  int32 WedgeIndex = 0;
  while (bValidOrFixable && WedgeIndex < NumWedges)
  {
    bValidOrFixable = bValidOrFixable && (InRawMesh.WedgeIndices[WedgeIndex] < (uint32)NumVertices);
    WedgeIndex++;
  }

  return bValidOrFixable;
}

void GLTFImportedData::SwapYZ(FVector &InOutVector)
{
  float temp = InOutVector.Y;
  InOutVector.Y = InOutVector.Z;
  InOutVector.Z = temp;
}

void GLTFImportedData::SwapYZ(TArray<FVector> &InOutVectorArray)
{
  for (auto &i : InOutVectorArray) {
    SwapYZ(i);
  }
}

void GLTFImportedData::AssignMaterials(FRawMesh& InRawMesh, UStaticMesh* InMesh, 
  const TWeakPtr<FImporterOptions>& InImporterOptions)
{

  TSharedPtr<FImporterOptions> ImporterOptions = InImporterOptions.Pin();

  static UMaterial* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

  TMap<int32, int32> MaterialIndexToSlot;
  MaterialIndexToSlot.Reserve(MaterialIndexArray.Num());

  MaterialIndexArray.Sort();
  InMesh->SectionInfoMap.Clear();
  //int i = 0;        
  //FMeshSectionInfoMap NewMap;

  for (int32 MaterialIndex : MaterialIndexArray) {

    UMaterialInterface* NewMaterial = nullptr;
    int32 MeshSlot;
    FName MatName;
    if (ImporterOptions->bImportMaterial) {
      for (auto &mat : MaterialArray) {
        if (mat.id == MaterialIndex && !NewMaterial) {
          NewMaterial = CreateMaterial(InImporterOptions, mat);
          MatName = (*(NewMaterial->GetName()));
          MeshSlot = InMesh->StaticMaterials.Emplace(NewMaterial, MatName, MatName);
        }
      }
    }

    if (!NewMaterial) {
      NewMaterial = DefaultMaterial;
      MeshSlot = InMesh->StaticMaterials.Add(NewMaterial);
    }

    //FMeshSectionInfo Info = InMesh->SectionInfoMap.Get(0, i);
    //int32 Index = InMesh->StaticMaterials.Add(NewMaterial);
    //Info.MaterialIndex = Index;
    //NewMap.Set(0, i, Info);


    
    MaterialIndexToSlot.Add(MaterialIndex, MeshSlot);
    InMesh->StaticMaterials[MeshSlot].MaterialSlotName = MatName;
    InMesh->SectionInfoMap.Set(0, MeshSlot, FMeshSectionInfo(MeshSlot));
    
    //i++;
  }

  //InMesh->SectionInfoMap.Clear();
  //InMesh->SectionInfoMap.CopyFrom(NewMap);

  for (int32& Index : InRawMesh.FaceMaterialIndices) {
    if (MaterialIndexToSlot.Contains(Index)) {
      Index = MaterialIndexToSlot[Index];
    }
  }

  if (InMesh->AssetImportData)
  {
    InMesh->AssetImportData->Update(ImporterOptions->FilePathInOS);
  }

  InMesh->PostEditChange();
  InMesh->MarkPackageDirty();
}

UMaterial* GLTFImportedData::CreateMaterial(const TWeakPtr<FImporterOptions>& InImporterOptions, 
  Material& InMaterial)
{
  if (!glTFParentClass) return nullptr;

  if (UMaterialMap.Contains(InMaterial.name)) {
    UMaterial* DupMaterial = nullptr;
    DupMaterial = *UMaterialMap.Find(InMaterial.name);

    return DupMaterial;
  }

  TSharedPtr<FImporterOptions> ImporterOptions = InImporterOptions.Pin();

  UMaterial* NewglTFMaterial = nullptr;

  if (ImporterOptions->bUsePBR) {
    NewglTFMaterial = LoadObject<UMaterial>(nullptr, MATERIAL_PBRMETALLICROUGHNESS);
  }
  else {
    NewglTFMaterial = LoadObject<UMaterial>(nullptr, MATERIAL_BASIC);
  }

  if (!NewglTFMaterial) return nullptr;

  FString MaterialName;

  MaterialName = FString::Printf(TEXT("M_%s"), *InMaterial.name);

  FString MaterialPackageName = FPackageName::GetLongPackagePath(glTFParentClass->GetPathName()) / MaterialName;
  UPackage* MaterialPackage = FindPackage(nullptr, *MaterialPackageName);
  if (!MaterialPackage)
  {
  /*UPackage* */MaterialPackage = CreatePackage(nullptr, *MaterialPackageName);
  }

  if (!MaterialPackage) return nullptr;

  MaterialPackage->FullyLoad();

  UMaterial* FinalMaterial = Cast<UMaterial>(StaticDuplicateObject(NewglTFMaterial, MaterialPackage, *MaterialName, glTFFlags, NewglTFMaterial->GetClass()));

  if (!FinalMaterial) return nullptr;

  // Scalar Parameters
  
  TMap<FName, FGuid> ScalarParameter_FNameToGuid;

  TArray<FName> ParameterNameArray;
  TArray<FGuid> ParameterGuidArray;

  TArray<FMaterialParameterInfo> ParameterInfoArray;

  FinalMaterial->GetAllScalarParameterInfo(ParameterInfoArray, ParameterGuidArray);

  for (const FMaterialParameterInfo& ParameterInfo : ParameterInfoArray) {
    ParameterNameArray.Add(ParameterInfo.Name);
  }

  if (ParameterNameArray.Num() == ParameterGuidArray.Num()) {
    for (int32 i = 0; i < ParameterNameArray.Num(); ++i) {
      ScalarParameter_FNameToGuid.FindOrAdd(ParameterNameArray[i]) = ParameterGuidArray[i];
    }
  }

  // Vector Parameters
  TMap<FName, FGuid> VectorParameter_FNameToGuid;

  ParameterNameArray.Empty();
  ParameterGuidArray.Empty();

  ParameterInfoArray.Empty();

  FinalMaterial->GetAllVectorParameterInfo(ParameterInfoArray, ParameterGuidArray);

  for (const FMaterialParameterInfo& ParameterInfo : ParameterInfoArray) {
    ParameterNameArray.Add(ParameterInfo.Name);
  }

  if (ParameterNameArray.Num() == ParameterGuidArray.Num()) {
    for (int32 i = 0; i < ParameterNameArray.Num(); ++i) {
      VectorParameter_FNameToGuid.FindOrAdd(ParameterNameArray[i]) = ParameterGuidArray[i];
    }
  }

  // Texture Parameters
  TMap<FName, FGuid> TextureParameter_FNameToGuid;

  ParameterNameArray.Empty();
  ParameterGuidArray.Empty();

  ParameterInfoArray.Empty();

  FinalMaterial->GetAllTextureParameterInfo(ParameterInfoArray, ParameterGuidArray);

  for (const FMaterialParameterInfo& ParameterInfo : ParameterInfoArray) {
    ParameterNameArray.Add(ParameterInfo.Name);
  }

  if (ParameterNameArray.Num() == ParameterGuidArray.Num()) {
    for (int32 i = 0; i < ParameterNameArray.Num(); ++i) {
      TextureParameter_FNameToGuid.FindOrAdd(ParameterNameArray[i]) = ParameterGuidArray[i];
    }
  }

  // Setting Material Parameters
  // ------------------> here

  if (ScalarParameter_FNameToGuid.Contains(TEXT("alphaCutoff"))) {
    if (UMaterialExpressionScalarParameter* ScalarParameter = FindPropertyByGuid<UMaterialExpressionScalarParameter>(FinalMaterial, ScalarParameter_FNameToGuid[TEXT("alphaCutoff")])) {
      ScalarParameter->DefaultValue = InMaterial.Alpha.alphaCutoff;
    }
  }

  // Normal
  if (InMaterial.Normal.texture != -1 &&
    TextureParameter_FNameToGuid.Contains(TEXT("normalTexture"))) {
    if (UMaterialExpressionTextureSampleParameter* SampleParameter = FindPropertyByGuid<UMaterialExpressionTextureSampleParameter>(FinalMaterial, TextureParameter_FNameToGuid[TEXT("normalTexture")])) {
      if (!ConstructMaterialParameter(InImporterOptions, SampleParameter, InMaterial.Normal.name, *TextureMap.Find(InMaterial.Normal.name), true)) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to construct normal texture '%s'"), *InMaterial.Normal.name);
#endif
      }
    }
  }

  // Emissive
  if(InMaterial.Emissive.texture != -1 &&
    TextureParameter_FNameToGuid.Contains(TEXT("emissiveTexture"))){
    if (UMaterialExpressionTextureSampleParameter* SampleParameter = FindPropertyByGuid<UMaterialExpressionTextureSampleParameter>(FinalMaterial, TextureParameter_FNameToGuid[TEXT("emissiveTexture")])) {
      if (!ConstructMaterialParameter(InImporterOptions, SampleParameter, InMaterial.Emissive.name, *TextureMap.Find(InMaterial.Emissive.name))) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to construct emmisive texture '%s'"), *InMaterial.Emissive.name);
#endif
      }
    }
  }

  //Base Color
  if (InMaterial.BaseColor.texture != -1 &&
    TextureParameter_FNameToGuid.Contains(TEXT("baseColorTexture"))) {
    if (UMaterialExpressionTextureSampleParameter* SampleParameter = FindPropertyByGuid<UMaterialExpressionTextureSampleParameter>(FinalMaterial, TextureParameter_FNameToGuid[TEXT("baseColorTexture")])) {
      if (!ConstructMaterialParameter(InImporterOptions, SampleParameter, InMaterial.BaseColor.name, *TextureMap.Find(InMaterial.BaseColor.name))) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to construct base color texture '%s'"), *InMaterial.BaseColor.name);
#endif
      }
    }
  }

  //Metallic Roughness 
  if (InMaterial.MetallicRoughness.texture != -1 &&
    TextureParameter_FNameToGuid.Contains(TEXT("metallicRoughnessTexture"))) {
    if (UMaterialExpressionTextureSampleParameter* SampleParameter = FindPropertyByGuid<UMaterialExpressionTextureSampleParameter>(FinalMaterial, TextureParameter_FNameToGuid[TEXT("metallicRoughnessTexture")])) {
      if (!ConstructMaterialParameter(InImporterOptions, SampleParameter, InMaterial.MetallicRoughness.name, *TextureMap.Find(InMaterial.MetallicRoughness.name))) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to construct Metallic Roughness texture '%s'"), *InMaterial.MetallicRoughness.name);
#endif
      }
    }
  }

  //Occlusion
  if (InMaterial.Occlusion.texture != -1 &&
    TextureParameter_FNameToGuid.Contains(TEXT("occlusionTexture"))) {
    if (UMaterialExpressionTextureSampleParameter* SampleParameter = FindPropertyByGuid<UMaterialExpressionTextureSampleParameter>(FinalMaterial, TextureParameter_FNameToGuid[TEXT("occlusionTexture")])) {
      if (!ConstructMaterialParameter(InImporterOptions, SampleParameter, InMaterial.Occlusion.name, *TextureMap.Find(InMaterial.Occlusion.name))) {
#ifdef SHOW_WARNING_LOGGER
        UE_LOG(ImporterLog, Warning, TEXT("Failed to construct Occlusion texture '%s'"), *InMaterial.Occlusion.name);
#endif
      }
    }
  }

  // Metallic & Roughness factor

  if (ScalarParameter_FNameToGuid.Contains(TEXT("roughnessFactor")))
  {
    if (UMaterialExpressionScalarParameter* ScalarParameter = FindPropertyByGuid<UMaterialExpressionScalarParameter>(FinalMaterial, ScalarParameter_FNameToGuid[TEXT("roughnessFactor")]))
    {
      ScalarParameter->DefaultValue = InMaterial.MetallicRoughness.roughness_factor;
    }
  }
  if (ScalarParameter_FNameToGuid.Contains(TEXT("metallicFactor")))
  {
    if (UMaterialExpressionScalarParameter* ScalarParameter = FindPropertyByGuid<UMaterialExpressionScalarParameter>(FinalMaterial, ScalarParameter_FNameToGuid[TEXT("metallicFactor")]))
    {
      ScalarParameter->DefaultValue = InMaterial.MetallicRoughness.metallic_factor;
    }
  }

  //Base Color factor
  if (VectorParameter_FNameToGuid.Contains(TEXT("baseColorFactor")))
  {
    if (UMaterialExpressionVectorParameter* VectorParameter = FindPropertyByGuid<UMaterialExpressionVectorParameter>(FinalMaterial, VectorParameter_FNameToGuid[TEXT("baseColorFactor")]))
    {
      VectorParameter->DefaultValue.R = InMaterial.BaseColor.factor[0];
      VectorParameter->DefaultValue.G = InMaterial.BaseColor.factor[1];
      VectorParameter->DefaultValue.B = InMaterial.BaseColor.factor[2];
      VectorParameter->DefaultValue.A = InMaterial.BaseColor.factor[3];
    }
  }

  //Emissive factor
  if (VectorParameter_FNameToGuid.Contains(TEXT("emissiveFactor")))
  {
    if (UMaterialExpressionVectorParameter* VectorParameter = FindPropertyByGuid<UMaterialExpressionVectorParameter>(FinalMaterial, VectorParameter_FNameToGuid[TEXT("emissiveFactor")]))
    {
      VectorParameter->DefaultValue.R = InMaterial.Emissive.factor[0];
      VectorParameter->DefaultValue.G = InMaterial.Emissive.factor[1];
      VectorParameter->DefaultValue.B = InMaterial.Emissive.factor[2];
      VectorParameter->DefaultValue.A = InMaterial.Emissive.factor[3];
    }
  }

  // Blend Mode Setup

  if (InMaterial.Alpha.alphaMode.Equals(TEXT("OPAQUE"), ESearchCase::IgnoreCase)) {
    FinalMaterial->BlendMode = BLEND_Opaque;
  }
  if (InMaterial.Alpha.alphaMode.Equals(TEXT("MASK"), ESearchCase::IgnoreCase)) {
    FinalMaterial->BlendMode = BLEND_Masked;
  }
  if (InMaterial.Alpha.alphaMode.Equals(TEXT("BLEND"), ESearchCase::IgnoreCase)) {
    FinalMaterial->BlendMode = BLEND_Translucent;
  }

  FinalMaterial->TwoSided = InMaterial.doubleSided;
  
  FinalMaterial->PostEditChange();
  FinalMaterial->MarkPackageDirty();
  
  UMaterialMap.Add(InMaterial.name, FinalMaterial);

  FAssetRegistryModule::AssetCreated(FinalMaterial);

  return FinalMaterial;

}

template<typename TMaterialParameter>
TMaterialParameter* GLTFImportedData::FindPropertyByGuid(UMaterial* InMaterial, const FGuid& InGuid)
{
  if (!InMaterial || !InGuid.IsValid()) return nullptr;

  for (auto MatParam : InMaterial->Expressions) {
    if (!MatParam || !UMaterial::IsParameter(MatParam)) continue;
    TMaterialParameter* TempParameter = Cast<TMaterialParameter>(MatParam);
    if (!TempParameter || TempParameter->ExpressionGUID != InGuid) continue;
    return TempParameter;
  }
  return nullptr;
}

bool GLTFImportedData::ConstructMaterialParameter(const TWeakPtr<FImporterOptions>& InglTFImportOptions, 
  UMaterialExpressionTextureSampleParameter* InSampleParameter, 
  const FString& InParameterName, const Texture &InTexture, bool InIsNormal /*= false*/)
{
  FString TextureName = FString::Printf(TEXT("T_%s_%s"), *glTFName.ToString(), *InParameterName);

  UTexture* Texture = nullptr;

  Texture = CreateTexture(InglTFImportOptions, InTexture, InIsNormal);

  if (Texture) {
    InSampleParameter->Texture = Texture;
  }

  //InSampleParameter->ConstCoordinate = 

  return (Texture != nullptr);
}

UTexture* GLTFImportedData::CreateTexture(const TWeakPtr<FImporterOptions>& InglTFImportOptions, 
  const Texture &InTexture, bool InIsNormal /*= false*/)
{
  FString TextureName;

  TextureName = FString::Printf(TEXT("T_%s"), *InTexture.name);

  FString PackageName = FPackageName::GetLongPackagePath(glTFParentClass->GetPathName()) / TextureName;

  UPackage* TexturePackage = FindPackage(nullptr, *PackageName);
  if (!TexturePackage) {
  /*UPackage* */TexturePackage = CreatePackage(nullptr, *PackageName);
  }
  if (!TexturePackage) return nullptr;
  TexturePackage->FullyLoad();
  
  //auto image = UImageLoader::LoadImageFromDiskAsync(TexturePackage, *(FilePath + InTexture.uri));
  //
  //FScriptDelegate Del;
  //Del.BindUFunction(nullptr, "RetrieveImage");
  //image->OnLoadCompleted().Add(Del);

  UTexture2D* FinalTexture = nullptr;
  //TArray<FColor> SrcData;
  //FinalTexture = FImageUtils::CreateTexture2D(InTexture.Info.width, InTexture.Info.height, SrcData, TexturePackage, *InTexture.name, glTFFlags);
  IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

  TSharedPtr<IImageWrapper> ImageWrappers[7] = {
    ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
    ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
    ImageWrapperModule.CreateImageWrapper(EImageFormat::GrayscaleJPEG),
    ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
    ImageWrapperModule.CreateImageWrapper(EImageFormat::ICO),
    ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR),
    ImageWrapperModule.CreateImageWrapper(EImageFormat::ICNS)
  };

  for (auto ImageWrapper : ImageWrappers) {
    if (!ImageWrapper.IsValid()) {
      continue;
    }

    if (!ImageWrapper->SetCompressed(InTexture.ImageData.GetData(), InTexture.ImageData.Num())) {
      continue;
    }

    ETextureSourceFormat TextureFormat = TSF_Invalid;

    int32 Width = ImageWrapper->GetWidth();
    int32 Height = ImageWrapper->GetHeight();
    int32 BitDepth = ImageWrapper->GetBitDepth();

    ERGBFormat ImageFormat = ImageWrapper->GetFormat();

    //TextureFormat = TSF_RGBA16;

    if (ImageFormat == ERGBFormat::Gray)
    {
      if (BitDepth <= 8)
      {
        TextureFormat = TSF_G8;
        ImageFormat = ERGBFormat::Gray;
        BitDepth = 8;
      }
      else if (BitDepth == 16)
      {

        TextureFormat = TSF_RGBA16;
        ImageFormat = ERGBFormat::RGBA;
        BitDepth = 16;
      }
    }
    else if (ImageFormat == ERGBFormat::RGBA || ImageFormat == ERGBFormat::BGRA)
    {
      if (BitDepth <= 8)
      {
        TextureFormat = TSF_BGRA8;
        ImageFormat = ERGBFormat::BGRA;
        BitDepth = 8;
      }
      else if (BitDepth == 16)
      {
        TextureFormat = TSF_RGBA16;
        ImageFormat = ERGBFormat::RGBA;
        BitDepth = 16;
      }
    }

    if (TextureFormat == TSF_Invalid)
    {
#ifdef SHOW_ERROR_LOGGER
      UE_LOG(ImporterLog, Error, TEXT("Unsuported Image Format"));
#endif
    }

    FinalTexture = NewObject<UTexture2D>(TexturePackage, UTexture2D::StaticClass(), *TextureName, glTFFlags);
    
    if (!FinalTexture) { 
      break; 
    }

    if (FMath::IsPowerOfTwo(Width) && FMath::IsPowerOfTwo(Height)) {
      FinalTexture->Source.Init2DWithMipChain(Width, Height, TextureFormat);
    } else {
      FinalTexture->Source.Init(Width, Height, 1, 1, TextureFormat);
      FinalTexture->MipGenSettings = TMGS_NoMipmaps;
    }
    // TODO> check compression settings for all the textures
    FinalTexture->SRGB = !InIsNormal;
    FinalTexture->CompressionSettings = !InIsNormal ? TC_Default : TC_Normalmap;

    const TArray<uint8>* RawData = nullptr;
    if (ImageWrapper->GetRaw(ImageFormat, BitDepth, RawData)) {
      uint8* MipData = FinalTexture->Source.LockMip(0);
      FMemory::Memcpy(MipData, RawData->GetData(), RawData->Num());
      FinalTexture->Source.UnlockMip(0);
    }
    break;   

  }

  if (FinalTexture) {
    FinalTexture->Filter = InTexture.Info.Filter;
    FinalTexture->AddressX = InTexture.Info.Wrapping[0];
    FinalTexture->AddressY = InTexture.Info.Wrapping[1];
  }
  if (FinalTexture)
  {
    FinalTexture->UpdateResource();
    if (!InTexture.uri.IsEmpty())
    {
      FString ImageFilePath = FilePath + InTexture.uri;

      FinalTexture->AssetImportData->Update(*ImageFilePath);
    }

    FinalTexture->PostEditChange();
    FinalTexture->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(FinalTexture);
  }

  return FinalTexture;

}

void GLTFImportedData::CreateNewOwnTexture(std::vector<tinygltf::Sampler>& InSamplerVec,
  std::vector<tinygltf::Image>& InImageVec, std::vector<tinygltf::Texture>& InTexVec, 
  int MaterialValue)
{
  FString TexName = RemoveFileFormat(InImageVec[InTexVec[MaterialValue].source].uri);
  if (!TextureMap.Contains(TexName)) {

    Texture glTFTexture;
    glTFTexture.uri = UTF8_TO_TCHAR(InImageVec[InTexVec[MaterialValue].source].uri.c_str());

    if (!InImageVec[InTexVec[MaterialValue].source].name.compare("")) {

      glTFTexture.name = RemoveFileFormat(InImageVec[InTexVec[MaterialValue].source].uri);

    } else {
      std::replace(InImageVec[InTexVec[MaterialValue].source].name.begin(), InImageVec[InTexVec[MaterialValue].source].name.end(), '.', '_');
      glTFTexture.name = UTF8_TO_TCHAR(InImageVec[InTexVec[MaterialValue].source].name.c_str());
    }
    glTFTexture.Info.width = InImageVec[InTexVec[MaterialValue].source].width;
    glTFTexture.Info.height = InImageVec[InTexVec[MaterialValue].source].height;

    if (InTexVec[MaterialValue].sampler >= 0) {
      glTFTexture.Info.Filter = TranslateFiltering(InSamplerVec[InTexVec[MaterialValue].sampler].magFilter);
      glTFTexture.Info.Wrapping[0] = TranslateWrapping(InSamplerVec[InTexVec[MaterialValue].sampler].wrapR);
      glTFTexture.Info.Wrapping[1] = TranslateWrapping(InSamplerVec[InTexVec[MaterialValue].sampler].wrapS);
    }

    if (!FFileHelper::LoadFileToArray(glTFTexture.ImageData, *(FilePath + glTFTexture.uri))) {
#ifdef SHOW_ERROR_LOGGER
      UE_LOG(ImporterLog, Error, TEXT("Failed to load file: %s "), *glTFTexture.uri);
#endif
    }

    TextureMap.Add(glTFTexture.name, glTFTexture);    
  }
}

void GLTFImportedData::RetrieveImage(UTexture2D* tex)
{
  tex_ = tex;
}

#undef LOCTEXT_NAMESPACE
