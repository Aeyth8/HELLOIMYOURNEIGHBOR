#include "../Hooks/Hooks.hpp"

using namespace A8CL;



namespace OFF
{
	OFFSET StaticConstructObject("StaticConstructObject_Internal", 0x23BCD0);
	OFFSET GetMousePosition("UGameViewportClient::GetMousePosition", 0x5B3A10);
	OFFSET UConsoleStaticClass("UConsole::GetPrivateStaticClass", 0x3AC8C0);

	constexpr unsigned VIEWPORT_CONSOLE = 0x38;
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
	using ull = unsigned long long;

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
	const char* FileName{nullptr};
	char FileNameBuffer[260]{0};
	GetModuleFileNameA(GetModuleHandleA(0), FileNameBuffer, 260);
	FileName = strrchr(FileNameBuffer, '\\') + 1;
	
	
	if (strcmp(FileName, "HelloNeighborReborn-Win64-Shipping.exe") == 0)
	{
		*((ull*)&OFF::StaticConstructObject.Offset) = 0x2AB010;
		*((ull*)&OFF::GetMousePosition.Offset) = 0x4562C0;
		*((ull*)&OFF::UConsoleStaticClass.Offset) = 0x4F3E40;
	}
	else if (strcmp(FileName, "HelloNeighbour-Win64-Shipping.exe") != 0)
	{
		MessageBoxA(0, "This tool was only designed to work on Hello Neighbor Pre-Alpha and Hello Neighbor Alpha 1.\nThis tool will be ejected and do nothing.", FileName, MB_OK);
	}

	if (Hooks::Init()) Hooks::CreateAndEnableHook(OFF::GetMousePosition, Hook::GetMousePosition);

	void* Thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WaitingToMakeConsoleThread, 0, 0, 0);
	if (Thread != nullptr) CloseHandle(Thread);
}