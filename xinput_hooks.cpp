#include "pch.h"

#include "config.h"

using XInputGetDSoundAudioDeviceGuids_pfn = DWORD (WINAPI *)(DWORD,GUID*,GUID*);
using XInputGetAudioDeviceIds_pfn         = DWORD (WINAPI *)(DWORD,LPWSTR,UINT*,LPWSTR,UINT*);

DWORD
WINAPI
XInputGetState (DWORD dwUserIndex, XINPUT_STATE *pState)
{
  DWORD dwState;

  static XInputGetState_pfn _XInputGetState =
        (XInputGetState_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ), "XInputGetState" );

  dwState =
    _XInputGetState (dwUserIndex, pState);

  if (dwState != ERROR_SUCCESS)
    return ERROR_SUCCESS;

  return dwState;
}

DWORD
WINAPI
XInputGetStateEx (DWORD dwUserIndex, XINPUT_STATE_EX *pState)
{
  static XInputGetStateEx_pfn _XInputGetStateEx =
        (XInputGetStateEx_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                           XINPUT_GETSTATEEX_ORDINAL );

  return
    _XInputGetStateEx (dwUserIndex, pState);
}

DWORD
WINAPI
XInputSetState (
  _In_    DWORD             dwUserIndex,
  _Inout_ XINPUT_VIBRATION *pVibration )
{
  static XInputSetState_pfn _XInputSetState =
        (XInputSetState_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                            "XInputSetState" );

  DWORD dwState =
    _XInputSetState (dwUserIndex, pVibration);

  if (dwState != ERROR_SUCCESS)
    return ERROR_SUCCESS;

  return dwState;
}

DWORD
WINAPI
XInputGetCapabilities (
  _In_  DWORD                dwUserIndex,
  _In_  DWORD                dwFlags,
  _Out_ XINPUT_CAPABILITIES *pCapabilities )
{
  static XInputGetCapabilities_pfn _XInputGetCapabilities =
        (XInputGetCapabilities_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                   "XInputGetCapabilities" );

  return
    _XInputGetCapabilities (dwUserIndex, dwFlags, pCapabilities);
}

DWORD
WINAPI
XInputGetCapabilitiesEx (
  _In_  DWORD                   dwReserved,
  _In_  DWORD                   dwUserIndex,
  _In_  DWORD                   dwFlags,
  _Out_ XINPUT_CAPABILITIES_EX *pCapabilitiesEx )
{
  static XInputGetCapabilitiesEx_pfn _XInputGetCapabilitiesEx =
        (XInputGetCapabilitiesEx_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                           XINPUT_GETCAPABILITIES_EX_ORDINAL );

  return
    _XInputGetCapabilitiesEx (dwReserved, dwUserIndex, dwFlags, pCapabilitiesEx);
}

DWORD
WINAPI
XInputGetBatteryInformation (
  _In_  DWORD                       dwUserIndex,
  _In_  BYTE                        devType,
  _Out_ XINPUT_BATTERY_INFORMATION *pBatteryInformation )
{
  static XInputGetBatteryInformation_pfn _XInputGetBatteryInformation =
        (XInputGetBatteryInformation_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
            nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 | 
                     LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                            "XInputGetBatteryInformation" );

  return
    _XInputGetBatteryInformation (dwUserIndex, devType, pBatteryInformation);
}

DWORD
WINAPI
XInputGetKeystroke (
  DWORD             dwUserIndex,
  DWORD             dwReserved,
  PXINPUT_KEYSTROKE pKeystroke )
{
  static XInputGetKeystroke_pfn _XInputGetKeystroke =
        (XInputGetKeystroke_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
            nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                     LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                            "XInputGetKeystroke" );

  return
    _XInputGetKeystroke (dwUserIndex, dwReserved, pKeystroke);
}

DWORD
WINAPI
XInputGetAudioDeviceIds (
  DWORD  dwUserIndex,
  LPWSTR pRenderDeviceId,
  UINT   *pRenderCount,
  LPWSTR pCaptureDeviceId,
  UINT   *pCaptureCount )
{
  static XInputGetAudioDeviceIds_pfn _XInputGetAudioDeviceIds =
        (XInputGetAudioDeviceIds_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                                "XInputGetAudioDeviceIds");

  return
    _XInputGetAudioDeviceIds (dwUserIndex, pRenderDeviceId, pRenderCount, pCaptureDeviceId, pCaptureCount);
}

DWORD
WINAPI
XInputGetDSoundAudioDeviceGuids (
  DWORD dwUserIndex,
  GUID  *pDSoundRenderGuid,
  GUID  *pDSoundCaptureGuid )
{
  static XInputGetDSoundAudioDeviceGuids_pfn _XInputGetDSoundAudioDeviceGuids =
        (XInputGetDSoundAudioDeviceGuids_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                            "XInputGetDSoundAudioDeviceGuids" );

  return
    _XInputGetDSoundAudioDeviceGuids (dwUserIndex, pDSoundRenderGuid, pDSoundCaptureGuid);
}

DWORD
WINAPI
XInputPowerOff (
  _In_ DWORD dwUserIndex )
{
  static XInputPowerOff_pfn _XInputPowerOff =
        (XInputPowerOff_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ), XINPUT_POWEROFF_ORDINAL );

  return
    _XInputPowerOff (dwUserIndex);
}

void
WINAPI
XInputEnable (
  _In_ BOOL enable )
{
  static XInputEnable_pfn _XInputEnable =
        (XInputEnable_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ), "XInputEnable" );

  return
    _XInputEnable (enable);
}