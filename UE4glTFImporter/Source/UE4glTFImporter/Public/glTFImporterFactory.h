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
