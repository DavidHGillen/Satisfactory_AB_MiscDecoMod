// Copyright Epic Games, Inc. All Rights Reserved.

#include "AB_MiscDecoMod.h"

#include "ABFactoryDeadEndComponent.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FAB_MiscDecoModModule"

void FAB_MiscDecoModModule::StartupModule()
{
	// Hooking
	if (!WITH_EDITOR) {
		AFGBuildableConveyorAttachment* VirtualRef_Attachment = GetMutableDefault<AFGBuildableConveyorAttachment>();
		SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableConveyorAttachment::Dismantle_Implementation, VirtualRef_Attachment, [](auto& Scope, AFGBuildableConveyorAttachment* self) {
		//SUBSCRIBE_UOBJECT_METHOD(AFGBuildableConveyorAttachment, Dismantle_Implementation, [](auto& Scope, AFGBuildableConveyorAttachment* self) {
			UE_LOG(LogTemp, Warning, TEXT("===****===****===****===****===****===****===****===****"));

			TArray< UFGFactoryConnectionComponent* > check;
			//UABFactoryDeadEndComponent* testResult;

			UE_LOG(LogTemp, Warning, TEXT("%d"), self);

			check = UABFactoryDeadEndComponent::GetIOFromAttachment(self);
			UE_LOG(LogTemp, Warning, TEXT("%d"), check.Num());
			for (int i=0, l=check.Num(); i < l; i++) {
				UE_LOG(LogTemp, Warning, TEXT("????"));
				/*testResult = Cast<UABFactoryDeadEndComponent>(check[i]->GetConnection());
				if (testResult != nullptr) {
					UE_LOG(LogTemp, Warning, TEXT("!!!!"));
					check[i]->ClearConnection();
				}*/
			}

			/**
				->ClearConnection();
				TArray< UFGFactoryConnectionComponent* > mInputs;
				TArray< UFGFactoryConnectionComponent* > mOutputs;
			**/
			});
	}
	// End Hooking
}

void FAB_MiscDecoModModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAB_MiscDecoModModule, AB_MiscDecoMod)