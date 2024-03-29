// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include <iostream>

wchar_t* char_to_wchar(const char* text) {
	wchar_t* wtext = new wchar_t[strlen(text) + 1];
	mbstowcs_s(NULL, wtext, strlen(text) + 1, text, strlen(text));
	return wtext;
}

DWORD WINAPI MessageBoxThread(LPVOID lpParam) {
	MessageBox(NULL, char_to_wchar("Hello world!"), char_to_wchar("Hello World!"), NULL);
	//std::cout << "===== hello =======" << std::endl;
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, MessageBoxThread, NULL, NULL, NULL);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

