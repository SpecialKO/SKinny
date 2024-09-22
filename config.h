#pragma once

#include <windef.h>

struct config_s {
  wchar_t wszPathToSystemXInput1_4   [MAX_PATH] = L"";
  wchar_t wszPathToSystemXInput1_3   [MAX_PATH] = L"";
  wchar_t wszPathToSystemXInput1_2   [MAX_PATH] = L"";
  wchar_t wszPathToSystemXInput1_1   [MAX_PATH] = L"";
  wchar_t wszPathToSystemXInput9_1_0 [MAX_PATH] = L"";
  wchar_t wszPathToSystemDXGI        [MAX_PATH] = L"";
  wchar_t wszPathToSystemD3D12       [MAX_PATH] = L"";
};

extern config_s config;
extern volatile LONG __VP_DLL_Refs;