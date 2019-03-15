
#include "ImageLoader.h"
#include "UE4glTFImporter.h"

#include "IImageWrapper.h"
#include "RenderUtils.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "FUE4glTFImporterModule"

static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

UImageLoader* UImageLoader::LoadImageFromDiskAsync(UObject* Outer, const FString& ImagePath)
{
  UImageLoader* ImageLoader = NewObject<UImageLoader>();
  ImageLoader->LoadImageAsync(Outer, ImagePath);
  return ImageLoader;
}

TFuture<UTexture2D*> UImageLoader::LoadImageFromDiskAsync(UObject* Outer, const FString& ImagePath, TFunction<void()> CompletionCallback)
{

  return Async<UTexture2D*>(EAsyncExecution::ThreadPool, [=]() { return LoadImageFromDisk(Outer, ImagePath); }, CompletionCallback);

}

UTexture2D* UImageLoader::LoadImageFromDisk(UObject* Outer, const FString& ImagePath)
{
  if (!FPaths::FileExists(ImagePath)) {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("File not found: %s"), *ImagePath);
#endif
    return nullptr;
  }

  TArray<uint8> FileData;
  if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Failed to load file: %s"), *ImagePath);
#endif
    return nullptr;
  }

  // Detect the image type using the ImageWrapper module
  EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
  if (ImageFormat == EImageFormat::Invalid)
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Unrecognized image type format: %s"), *ImagePath);
#endif
    return nullptr;
  }

  // Create an image wrapper for the detected image format
  TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
  if (!ImageWrapper.IsValid())
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Failed to create image wrapper for file: %s"), *ImagePath);
#endif
    return nullptr;
  }

  // Decompress the image data
  const TArray<uint8>* RawData = nullptr;
  ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num());
  ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData);
  if (RawData == nullptr)
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Error, TEXT("Failed to decompress image file: %s"), *ImagePath);
#endif
    return nullptr;
  }

  // Create the texture and upload the uncompressed image data
  FString TextureBaseName = TEXT("T_") + FPaths::GetBaseFilename(ImagePath);
  return CreateTexture(Outer, *RawData, ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), EPixelFormat::PF_B8G8R8A8, FName(*TextureBaseName));
}

void UImageLoader::LoadImageAsync(UObject* Outer, const FString& ImagePath)
{
  Future = LoadImageFromDiskAsync(Outer, ImagePath, [this]() {
    if (Future.IsValid()) {
      AsyncTask(ENamedThreads::GameThread, [this]() {LoadCompleted.Broadcast(Future.Get()); });
    }
  });
}

UTexture2D* UImageLoader::CreateTexture(UObject* Outer, const TArray<uint8>& PixelData, int32 InSizeX, int32 InSizeY, EPixelFormat InFormat /*= EPixelFormat::PF_B8G8R8A8*/, FName BaseName /*= NAME_None*/)
{
  if (InSizeX <= 0 || InSizeY <= 0 ||
    (InSizeX % GPixelFormats[InFormat].BlockSizeX) != 0 ||
    (InSizeY % GPixelFormats[InFormat].BlockSizeY) != 0)
  {
#ifdef SHOW_ERROR_LOGGER
    UE_LOG(ImporterLog, Warning, TEXT("Invalid parameters specified for UImageLoader::CreateTexture()"));
#endif
    return nullptr;
  }

  // Most important difference with UTexture2D::CreateTransient: we provide the new texture with a name and an owner
  FName TextureName = MakeUniqueObjectName(Outer, UTexture2D::StaticClass(), BaseName);
  UTexture2D* NewTexture = NewObject<UTexture2D>(Outer, TextureName, RF_Transient);

  NewTexture->PlatformData = new FTexturePlatformData();
  NewTexture->PlatformData->SizeX = InSizeX;
  NewTexture->PlatformData->SizeY = InSizeY;
  NewTexture->PlatformData->PixelFormat = InFormat;

  // Allocate first mipmap and upload the pixel data
  int32 NumBlocksX = InSizeX / GPixelFormats[InFormat].BlockSizeX;
  int32 NumBlocksY = InSizeY / GPixelFormats[InFormat].BlockSizeY;
  FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
  Mip->SizeX = InSizeX;
  Mip->SizeY = InSizeY;
  Mip->BulkData.Lock(LOCK_READ_WRITE);
  void* TextureData = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InFormat].BlockBytes);
  FMemory::Memcpy(TextureData, PixelData.GetData(), PixelData.Num());
  Mip->BulkData.Unlock();

  NewTexture->UpdateResource();
  return NewTexture;
}

#undef LOCTEXT_NAMESPACE
