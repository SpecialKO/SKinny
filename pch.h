// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_SUCCESS      ((NTSTATUS)0x00000000L)

#include <Shlwapi.h>
#include <atlbase.h>
#include <cassert>
#include <cstdint>
#include <utility>
#include <string>

#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>
#include <d3d11.h>
#include <d3d12.h>

#include "xinput_defs.h"
#include "SKinHook/MinHook.h"
#include "streamline.h"

using CreateFile2_pfn =
  HANDLE (WINAPI *)(LPCWSTR,DWORD,DWORD,DWORD,
                      LPCREATEFILE2_EXTENDED_PARAMETERS);

using CreateFileW_pfn =
  HANDLE (WINAPI *)(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,
                      DWORD,DWORD,HANDLE);

using CreateFileA_pfn =
  HANDLE (WINAPI *)(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,
                      DWORD,DWORD,HANDLE);

// Queues a hook rather than enabling it immediately.
MH_STATUS
__stdcall
SK_CreateDLLHook2 ( const wchar_t  *pwszModule, const char  *pszProcName,
                          void     *pDetour,          void **ppOriginal,
                          void    **ppFuncAddr
                                      = nullptr );

#endif //PCH_H
