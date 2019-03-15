// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Factories/Factory.h"

#include "glTFImporterFactory.generated.h"

UCLASS(hidecategories = Object)
class UglTFImporterFactory : public UFactory
{
  GENERATED_UCLASS_BODY()

public:
  UglTFImporterFactory();

  //************************************
  // Method:    DoesSupportClass
  // FullName:  UglTFImporterFactory::DoesSupportClass
  // Access:    virtual public 
  // Returns:   bool
  // Qualifier:
  // Parameter: UClass * InClass
  //************************************
  virtual bool DoesSupportClass(UClass* InClass) override;

  //************************************
  // Method:    ResolveSupportedClass
  // FullName:  UglTFImporterFactory::ResolveSupportedClass
  // Access:    virtual public 
  // Returns:   UClass*
  // Qualifier:
  //************************************
  virtual UClass* ResolveSupportedClass() override;

  //************************************
  // Method:    FactoryCanImport
  // FullName:  UglTFImporterFactory::FactoryCanImport
  // Access:    virtual public 
  // Returns:   bool
  // Qualifier:
  // Parameter: const FString & InFilename
  //************************************
  virtual bool FactoryCanImport(const FString& InFilename) override;

  //************************************
  // Method:    FactoryCreateText
  // FullName:  UglTFImporterFactory::FactoryCreateText
  // Access:    virtual public 
  // Returns:   UObject*
  // Qualifier:
  // Parameter: UClass * InClass
  // Parameter: UObject * InParent
  // Parameter: FName InName
  // Parameter: EObjectFlags InFlags
  // Parameter: UObject * InContext
  // Parameter: const TCHAR * InType
  // Parameter: const TCHAR * & InBuffer
  // Parameter: const TCHAR * InBufferEnd
  // Parameter: FFeedbackContext * InWarn
  //************************************
  virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, 
    EObjectFlags InFlags, UObject* InContext, const TCHAR* InType, const TCHAR*& InBuffer, 
    const TCHAR* InBufferEnd, FFeedbackContext* InWarn);

protected:
  
  //************************************
  // Method:    FactoryCreate
  // FullName:  UglTFImporterFactory::FactoryCreate
  // Access:    virtual protected 
  // Returns:   UObject*
  // Qualifier:
  // Parameter: UClass * InClass
  // Parameter: UObject * InParent
  // Parameter: FName InName
  // Parameter: EObjectFlags InFlags
  // Parameter: UObject * InContext
  // Parameter: const TCHAR * InType
  // Parameter: FFeedbackContext * InWarn
  // Parameter: const FString & InglTFJson
  // Parameter: TSharedPtr<class FglTFBuffers> InglTFBuffers
  //************************************
  virtual UObject* FactoryCreate(UClass* InClass, UObject* InParent, FName InName, 
    EObjectFlags InFlags, UObject* InContext, const TCHAR* InType, FFeedbackContext* InWarn, 
    const FString& InglTFJson, TSharedPtr<class FglTFBuffers> InglTFBuffers = nullptr);


private:
  UClass* ImportClass;

};
