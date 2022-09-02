// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <detours.h>
#include <stdio.h>
#include <sstream>

HMODULE GameAssembly;

UINT_PTR OverwriteAddr;

//make template
typedef void(__thiscall* t_awake)(void*);
//inline Awake = reinterpret_cast<t_awake>((UINT_PTR)GameAssembly + (UINT_PTR)0x3BC380);

void SetOverwriteFunc(void* _this)
{
    //MessageBox(0, std::to_string((UINT_PTR)_this).c_str(), "Test", MB_OK);
    t_awake OriginalAwake = (t_awake)(OverwriteAddr);

    //go back to original code
    OriginalAwake(_this);
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            GameAssembly = GetModuleHandleA("GameAssembly.dll");

            //MessageBox(0, std::to_string((UINT_PTR)GameAssembly).c_str(), "Test", MB_OK);

            DetourRestoreAfterWith();

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            //RVA value == second value there
            OverwriteAddr = (UINT_PTR)((UINT_PTR)GameAssembly + (UINT_PTR)0x3BC380);

            DetourAttach(&(PVOID&)OverwriteAddr, &SetOverwriteFunc);
            DetourTransactionCommit();
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            DetourDetach(&(PVOID&)OverwriteAddr, &SetOverwriteFunc);
            DetourTransactionCommit();
            break;
        }
    }
    return TRUE;
}

