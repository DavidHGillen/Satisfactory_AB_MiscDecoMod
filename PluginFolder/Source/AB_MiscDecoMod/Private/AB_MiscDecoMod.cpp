// Copyright Epic Games, Inc. All Rights Reserved.

#include "AB_MiscDecoMod.h"

#define LOCTEXT_NAMESPACE "FAB_MiscDecoModModule"

void FAB_MiscDecoModModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FAB_MiscDecoModModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAB_MiscDecoModModule, AB_MiscDecoMod)