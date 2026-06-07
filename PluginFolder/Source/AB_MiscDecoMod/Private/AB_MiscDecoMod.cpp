// Copyright Epic Games, Inc. All Rights Reserved.

#include "AB_MiscDecoMod.h"

#define LOCTEXT_NAMESPACE "FAB_MiscDecoModModule"

void FAB_MiscDecoModModule::StartupModule()
{
	// Hooking
	//#if !WITH_EDITOR

	//#endif
	// End Hooking
}

void FAB_MiscDecoModModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAB_MiscDecoModModule, AB_MiscDecoMod)