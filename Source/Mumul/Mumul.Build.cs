// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Mumul : ModuleRules
{
	public Mumul(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			"OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam", "SteamSockets", "UMG",
			"Http", "Json", "JsonUtilities", "WebSockets",
			"Voice",
			"AudioCapture",
			"AudioCaptureCore",
			"AudioMixer"
		});
	}
}
