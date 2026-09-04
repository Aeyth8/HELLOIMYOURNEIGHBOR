#include "../Hooks/Hooks.hpp"
#include "BytePatcher.h"

using namespace A8CL;



namespace OFF
{
	OFFSET StaticConstructObject("StaticConstructObject_Internal", 0x0);
	OFFSET GetMousePosition("UGameViewportClient::GetMousePosition", 0x0);
	OFFSET UConsoleStaticClass("UConsole::GetPrivateStaticClass", 0x0);

	constexpr unsigned VIEWPORT_CONSOLE = 0x38;
	constexpr ull	   SCO_HEADER		= 0x565355182444894C;

	constexpr ull SCO_Offsets[]{0x23BCD0, 0x2AB010, 0x220E90};
	constexpr ull GMP_Offsets[]{0x5B3A10, 0x4562C0, 0x68FC30};
	constexpr ull CSC_Offsets[]{0x3AC8C0, 0x4F3E40, 0x341190};
	constexpr ull ADV_Offsets[]{0xF13960, 0x1050D30, 0};				// AllowDebugViewmodes

	#define ARRAY_SIZE(Array) sizeof(Array) / sizeof(Array[0])
	static_assert(ARRAY_SIZE(SCO_Offsets) == ARRAY_SIZE(GMP_Offsets) && ARRAY_SIZE(SCO_Offsets) == ARRAY_SIZE(CSC_Offsets));
	constexpr char OffsetArraySize = ARRAY_SIZE(SCO_Offsets);

}
namespace Struct
{
	struct UGameViewportClient
	{ 
		unsigned char	BytePadding[OFF::VIEWPORT_CONSOLE];
		void*			UConsole;
	};

	struct FName
	{ 
		int ComparisonIndex; 
		int Number; 
	};
}
namespace Decl
{
	// Taken from Unreal Engine 4.20.2
	//using T_GetPrivateStaticClass = void* (__fastcall*)();
	//using T_StaticConstructObject = void*(__fastcall*)(void* InClass, void* InOuter, Struct::FName InName, unsigned InFlags, unsigned InternalSetFlags, void* InTemplate, bool bCopyTransientsFromClassDefaults, void** InInstanceGraph, bool bAssumeTemplateIsArchetype);

	// Taken from Unreal Engine 4.8
	using T_GetPrivateStaticClass = void* (__fastcall*)(const wchar_t*);
	using T_StaticConstructObject = void*(__fastcall*)(void* InClass, void* InOuter, Struct::FName InName, unsigned SetFlags, void* Template, char bCopyTransientsFromClassDefaults, void* InstanceGraph);
}
namespace Var
{
	Struct::UGameViewportClient*	GameViewportClient{0};
	void*							ConsoleStaticClass{0};
}
namespace Hook
{
	char GetMousePosition(Struct::UGameViewportClient* This, void* a2)
	{
		Var::GameViewportClient = This;
		return OFF::GetMousePosition.VerifyFC<char(__thiscall*)(void*, void*)>()(This, a2);
	}
}

void WaitingToMakeConsoleThread()
{
	while (!Var::GameViewportClient) Sleep(100);
	
	//Var::ConsoleStaticClass = OFF::UConsoleStaticClass.VerifyFC<Decl::T_GetPrivateStaticClass>()(L"/Script/Engine");
	while (!(Var::ConsoleStaticClass = OFF::UConsoleStaticClass.VerifyFC<Decl::T_GetPrivateStaticClass>()(L"/Script/Engine"))) Sleep(100);
	
	ull* UConsole = (ull*)OFF::StaticConstructObject.VerifyFC<Decl::T_StaticConstructObject>()(Var::ConsoleStaticClass, Var::GameViewportClient, {}, 0, 0, 0, 0);
	if (UConsole)
	{
		__assume(Var::GameViewportClient != nullptr); // SHUTUP!
		Var::GameViewportClient->UConsole = UConsole;
	}
}

extern void CopyPastable()
{
	const ull GBA = OFFSET::GetImageBase();
	
	bool bCompatible{false};
	for (char i{0}; i < OFF::OffsetArraySize; ++i)
	{
		ull VersionCheck = *(ull*)(GBA + OFF::SCO_Offsets[i]);
		if (VersionCheck == OFF::SCO_HEADER)
		{
			OFF::StaticConstructObject.Offset = OFF::SCO_Offsets[i];
			OFF::GetMousePosition.Offset = OFF::GMP_Offsets[i];
			OFF::UConsoleStaticClass.Offset = OFF::CSC_Offsets[i];

			BytePatcher::ReplaceBytes(GBA + OFF::ADV_Offsets[i], {0xB0, 0x01, 0xC3, 0x90, 0x90, 0x90});

			bCompatible = true;
			break;
		}
	}

	if (!bCompatible)
	{
		MessageBoxA(0, "This tool was only designed to work on Hello Neighbor Pre-Alpha and Hello Neighbor Alpha 1.\nThis tool will be ejected and do nothing.", "Incompatible Game!", MB_OK);
		return;
	}
	
	/*
		565355182444894C

		constexpr byte Frick[8]{0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x53, 0x56};
		LogA("bytes to qword", HexToString(*(qword*)Frick));
	
	*/
	
	if (Hooks::Init()) Hooks::CreateAndEnableHook(OFF::GetMousePosition, Hook::GetMousePosition);

	void* Thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WaitingToMakeConsoleThread, 0, 0, 0);
	if (Thread != nullptr) CloseHandle(Thread);
}