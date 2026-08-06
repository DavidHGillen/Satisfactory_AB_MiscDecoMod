// Copyright Epic Games, Inc. All Rights Reserved.

#include "AB_MiscDecoMod.h"

#include "ABFactoryDeadEndComponent.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FAB_MiscDecoModModule"

void FAB_MiscDecoModModule::StartupModule()
{
	// Hooking
	if (!WITH_EDITOR) {
		IFGDismantleInterface* VirtualRef_Attachment = GetMutableDefault<AFGBuildableConveyorAttachment>();
		SUBSCRIBE_METHOD_VIRTUAL(IFGDismantleInterface::Dismantle_Implementation, VirtualRef_Attachment, [](auto& Scope, IFGDismantleInterface* self) {
			//UE_LOG(LogTemp, Warning, TEXT("===****===****===****===****===****===****===****===****"));

			AFGBuildableConveyorAttachment* selfSrc = Cast<AFGBuildableConveyorAttachment>(self);
			TArray< UFGFactoryConnectionComponent* > check = UABFactoryDeadEndComponent::GetIOFromAttachment(selfSrc);

			for (int i=0, l=check.Num(); i < l; i++) {
				UABFactoryDeadEndComponent* testResult = Cast<UABFactoryDeadEndComponent>(check[i]->GetConnection());
				if (testResult != nullptr) {
					check[i]->ClearConnection();
				}
			}
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