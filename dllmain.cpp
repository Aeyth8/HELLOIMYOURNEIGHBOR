#include <Windows.h>
#include <iostream>

#include "Aeyth8/Proxy8/ProxyTypes.h"


// Pre-Alpha			-			[StaticConstructObject_Internal 0x23BCD0] [UGameViewportClient::GetMousePosition 0x5B3A10] [UConsole::GetPrivateStaticClass 0x3AC8C0]
// Alpha 1				-			[StaticConstructObject_Internal 0x2AB010] [UGameViewportClient::GetMousePosition 0x4562C0] [UConsole::GetPrivateStaticClass 0x4F3E40]
// Prototype Fangame	-			[StaticConstructObject_Internal 0x220E90] [UGameViewportClient::GetMousePosition 0x68FC30] [UConsole::GetPrivateStaticClass 0x341190] (https://gamejolt.com/games/HNPrototype/762604) (Version 1.0.4)

extern void CopyPastable();

int __stdcall DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);

	if (ulReasonForCall == DLL_PROCESS_ATTACH)
	{
		if (Proxy::Attach(hModule)) {
			CopyPastable();
		}
	}

	return 1;
}