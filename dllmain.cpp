
// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "config.h"

#pragma comment (lib, "shlwapi.lib")

#ifdef _M_IX86
#pragma comment (lib, "SKinHook/libMinHook.lib")
#else
#pragma comment (lib, "SKinHook/libMinHook64.lib")
#endif

         config_s config;
volatile LONG     __VP_DLL_Refs = 0UL;

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

using LdrLockLoaderLock_pfn   = NTSTATUS (WINAPI *)(ULONG Flags, ULONG *State, ULONG_PTR *Cookie);
using LdrUnlockLoaderLock_pfn = NTSTATUS (WINAPI *)(ULONG Flags,               ULONG_PTR  Cookie);

extern "C"
NTSTATUS
WINAPI
SK_NtLdr_LockLoaderLock (ULONG Flags, ULONG* State, ULONG_PTR* Cookie)
{
  //// The lock must not be acquired until DllMain (...) returns!
  //if (ReadAcquire (&__VP_DLL_Refs) < 1)
  //  return STATUS_SUCCESS; // No-Op

  static LdrLockLoaderLock_pfn LdrLockLoaderLock =
        (LdrLockLoaderLock_pfn)GetProcAddress (GetModuleHandleW (L"NtDll.dll"),
        "LdrLockLoaderLock");

  if (! LdrLockLoaderLock)
    return ERROR_NOT_FOUND;

  return
    LdrLockLoaderLock (Flags, State, Cookie);
}

extern "C"
NTSTATUS
WINAPI
SK_NtLdr_UnlockLoaderLock (ULONG Flags, ULONG_PTR Cookie)
{
  static LdrUnlockLoaderLock_pfn LdrUnlockLoaderLock =
        (LdrUnlockLoaderLock_pfn)GetProcAddress (GetModuleHandleW (L"NtDll.dll"),
        "LdrUnlockLoaderLock");

  if (! LdrUnlockLoaderLock)
    return ERROR_NOT_FOUND;

  NTSTATUS UnlockLoaderStatus =
    LdrUnlockLoaderLock (Flags, Cookie);

//  // Check for Loader Unlock Failure...
//  if (ReadAcquire (&__VP_DLL_Refs) >= 1 && Cookie != 0)
//  {
//#ifdef DEBUG
//    assert (UnlockLoaderStatus == STATUS_SUCCESS);
//#endif
//  }

  return
    UnlockLoaderStatus;
}

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")

IDXGISwapChain* g_pLastFullscreenSwapChain = nullptr;

typedef HRESULT (STDMETHODCALLTYPE *SetFullscreenState_pfn)(
                                       IDXGISwapChain *This,
                                       BOOL            Fullscreen,
                                       IDXGIOutput    *pTarget);

typedef HRESULT (STDMETHODCALLTYPE *GetFullscreenState_pfn)(
                                       IDXGISwapChain  *This,
                            _Out_opt_  BOOL            *pFullscreen,
                            _Out_opt_  IDXGIOutput    **ppTarget );

typedef HRESULT (STDMETHODCALLTYPE *GetDesc_pfn)(
                                    IDXGISwapChain       *This,
                             _Out_  DXGI_SWAP_CHAIN_DESC *pDesc );

typedef HRESULT (STDMETHODCALLTYPE *GetFullscreenDesc_pfn)(
                                    IDXGISwapChain1                 *This,
                             _Out_  DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc );

typedef HRESULT (STDMETHODCALLTYPE *CreateSwapChain_pfn)(
                                       IDXGIFactory          *This,
                           _In_        IUnknown              *pDevice,
                           _In_  const DXGI_SWAP_CHAIN_DESC  *pDesc,
                          _Out_        IDXGISwapChain       **ppSwapChain);

SetFullscreenState_pfn SetFullscreenState_Original = nullptr;
GetFullscreenState_pfn GetFullscreenState_Original = nullptr;
GetDesc_pfn            GetDesc_Original            = nullptr;
GetFullscreenDesc_pfn  GetFullscreenDesc_Original  = nullptr;
CreateSwapChain_pfn    CreateSwapChain_Original    = nullptr;

MH_STATUS
__stdcall
SK_CreateFuncHook ( const wchar_t  *pwszFuncName,
                          void     *pTarget,
                          void     *pDetour,
                          void    **ppOriginal )
{ 
  MH_STATUS status =
    MH_CreateHook ( pTarget,
                      pDetour,
                        ppOriginal );

  if (status != MH_OK && status != MH_ERROR_ALREADY_CREATED)
  {
    //SK_LOG_MINHOOK ( status,
    //                   L"Failed to Install Hook for '%s' "
    //                   L"[Address: %04ph]! ",
    //                     pwszFuncName, pTarget );
  }

  else if (status == MH_ERROR_ALREADY_CREATED)
  {
    if (MH_OK == (status = MH_RemoveHook (pTarget)))
    {
      //dll_log->Log ( L"[HookEngine] Removing Corrupted Hook for '%s'... software "
      //               L"is probably going to explode!", pwszFuncName );
      //
      return SK_CreateFuncHook (pwszFuncName, pTarget, pDetour, ppOriginal);
    } else
    {
      //SK_LOG_MINHOOK ( status,
      //                   L"Failed to Uninstall Hook for '%s' [Address: %04ph]!",
      //                     pwszFuncName, pTarget );
    }
  }

  return status;
}

MH_STATUS
__stdcall
SK_CreateVFTableHook2 ( const wchar_t  *pwszFuncName,
                              void    **ppVFTable,
                              DWORD     dwOffset,
                              void     *pDetour,
                              void    **ppOriginal )
{
  MH_STATUS status =
    MH_ERROR_NOT_EXECUTABLE;

  if (ppVFTable != nullptr)
  {
    status =
      SK_CreateFuncHook (
        pwszFuncName,
          ppVFTable [dwOffset],
            pDetour,
              ppOriginal );

    if (status == MH_OK)
    {
      //SK_ValidateVFTableAddress (pwszFuncName, *ppVFTable, ppVFTable [dwOffset]);

      status =
        MH_QueueEnableHook (ppVFTable [dwOffset]);
    }
  }

  if (status != MH_OK)
  {
    //SK_LOG_MINHOOK ( status,
    //                   L"Failed to Install Hook for '%s' [VFTable Index: %lu]! ",
    //                     pwszFuncName, dwOffset );
  }

  return status;
}

MH_STATUS
__stdcall
SK_CreateDLLHook2 ( const wchar_t  *pwszModule, const char  *pszProcName,
                          void     *pDetour,          void **ppOriginal,
                          void    **ppFuncAddr )
{
  HMODULE hMod = nullptr;

  if (! GetModuleHandleExW (GET_MODULE_HANDLE_EX_FLAG_PIN, pwszModule, &hMod))
  {
    // In the future, establish queuing capabilities, for now, just pull the DLL in.
    //
    //  >> Pass the library load through the original (now hooked) function so that
    //       anything else that hooks this DLL on-load does not miss its initial load.
    //
    hMod =
      LoadLibraryW (pwszModule);

    if (hMod != 0)
      GetModuleHandleExW ( GET_MODULE_HANDLE_EX_FLAG_PIN |
                           GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (wchar_t *)hMod, &hMod );
  }

  void      *pFuncAddr = nullptr;
  MH_STATUS  status    = MH_OK;

  if (hMod == nullptr)
    status = MH_ERROR_MODULE_NOT_FOUND;

  else
  {
    pFuncAddr =
      GetProcAddress (hMod, pszProcName);

    //// Do not warn about kernel functions being in the wrong place
    //if (! StrStrIW (pwszModule, L"kernel"))
    //{
    //  SK_Module_IsProcAddrLocal (
    //    hMod, pszProcName, (FARPROC)pFuncAddr
    //  );
    //}

    status =
      MH_CreateHook ( pFuncAddr,
                        pDetour,
                          ppOriginal );
  }


  if (status != MH_OK)
  {
    if (status == MH_ERROR_ALREADY_CREATED)
    {
      if (ppOriginal == nullptr)
      {
        SH_Introspect ( pFuncAddr,
                          SH_TRAMPOLINE,
                            ppOriginal );

        //SK_LOG_MINHOOK ( status,
        //                   L"WARNING: Hook Already Exists for '%hs' in '%s'!",
        //                     szProcName,
        //                       wszModName );

        return status;
      }

      else if (MH_OK == (status = MH_RemoveHook (pFuncAddr)))
      {
        //dll_log->Log ( L"[HookEngine] Removing Corrupted Hook for '%hs'... software "
        //               L"is probably going to explode!", szProcName );

        return SK_CreateDLLHook2 (pwszModule, pszProcName, pDetour, ppOriginal, ppFuncAddr);
      }

      else
      {
        //SK_LOG_MINHOOK ( status,
        //                   L"Failed to Uninstall Hook for '%hs' "
        //                   L"[Address: %04ph]! ",
        //                     szProcName,
        //                       pFuncAddr );
      }
    }

    //SK_LOG_MINHOOK ( status,
    //                   L"Failed to Install Hook for '%hs' in '%s'!",
    //                     szProcName,
    //                       wszModName );

    if (ppFuncAddr != nullptr)
       *ppFuncAddr  = nullptr;
  }

  else
  {
    if (ppFuncAddr != nullptr)
       *ppFuncAddr  = pFuncAddr;

    MH_QueueEnableHook (pFuncAddr);
  }

  return status;
}

#define DXGI_VIRTUAL_HOOK(_Base,_Index,_Name,_Override,_Original,_Type) {     \
  void** _vftable = *(void***)*(_Base);                                       \
                                                                              \
  if ((_Original) == nullptr) {                                               \
    SK_CreateVFTableHook2 ( L##_Name,                                         \
                              _vftable,                                       \
                                (_Index),                                     \
                                  (_Override),                                \
                                    (LPVOID *)&(_Original));                  \
  }                                                                           \
}

__declspec (noinline)
HRESULT
STDMETHODCALLTYPE
DXGIFactory_CreateSwapChain_Override (
              IDXGIFactory          *This,
  _In_        IUnknown              *pDevice,
  _In_  const DXGI_SWAP_CHAIN_DESC  *pDesc,
  _Out_       IDXGISwapChain       **ppSwapChain )
{
  DXGI_SWAP_CHAIN_DESC swapDesc = (pDesc != nullptr) ? *pDesc : DXGI_SWAP_CHAIN_DESC {};

  if (pDesc != nullptr)
  {
    if (pDesc->SwapEffect == DXGI_SWAP_EFFECT_DISCARD)
      swapDesc.SwapEffect =  DXGI_SWAP_EFFECT_FLIP_DISCARD;
    if (pDesc->SwapEffect == DXGI_SWAP_EFFECT_SEQUENTIAL)
      swapDesc.SwapEffect =  DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  }

  if (pDesc != nullptr)
  {
    HRESULT hr =
      CreateSwapChain_Original (This, pDevice, &swapDesc, ppSwapChain);

    if (SUCCEEDED (hr))
    {
      if (ppSwapChain != nullptr)
      {
        if (g_pLastFullscreenSwapChain == nullptr)
            g_pLastFullscreenSwapChain = *ppSwapChain;
      }

      return hr;
    }
  }

  return
    CreateSwapChain_Original (This, pDevice, pDesc, ppSwapChain);
}

#include <concurrent_unordered_map.h>
concurrency::concurrent_unordered_map <IDXGISwapChain*, BOOL> fullscreen_state;

HRESULT
STDMETHODCALLTYPE
DXGISwap_SetFullscreenState_Override ( IDXGISwapChain *This,
                                       BOOL            Fullscreen,
                                       IDXGIOutput    *pTarget )
{
  fullscreen_state [This] = Fullscreen;

  g_pLastFullscreenSwapChain = This;

  SetFullscreenState_Original (This, FALSE, nullptr);

  return S_OK;
}

HRESULT
STDMETHODCALLTYPE
DXGISwap_GetFullscreenState_Override ( IDXGISwapChain  *This,
                            _Out_opt_  BOOL            *pFullscreen,
                            _Out_opt_  IDXGIOutput    **ppTarget )
{
  g_pLastFullscreenSwapChain = This;

  if (fullscreen_state.count (This))
  {
    if (pFullscreen != nullptr)
    {
      HRESULT hr =
        GetFullscreenState_Original (This, pFullscreen, ppTarget);

      *pFullscreen = fullscreen_state [This];

      return hr;
    }
  }

  return
    GetFullscreenState_Original (This, pFullscreen, ppTarget);
}

HRESULT
STDMETHODCALLTYPE
DXGISwap_GetDesc_Override ( IDXGISwapChain       *This,
                     _Out_  DXGI_SWAP_CHAIN_DESC *pDesc )
{
  g_pLastFullscreenSwapChain = This;

  HRESULT hr =
    GetDesc_Original (This, pDesc);

  if (pDesc != nullptr)
  {
    if (fullscreen_state.count (This))
      pDesc->Windowed = !fullscreen_state [This];

    return S_OK;
  }

  return hr;
}

HRESULT
STDMETHODCALLTYPE
DXGISwap_GetFullscreenDesc_Override ( IDXGISwapChain1                 *This,
                               _Out_  DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc )
{
  g_pLastFullscreenSwapChain = This;

  HRESULT hr =
    GetFullscreenDesc_Original (This, pDesc);

  if (SUCCEEDED (hr) && pDesc != nullptr)
  {
    if (fullscreen_state.count (This))
      pDesc->Windowed = !fullscreen_state [This];
  }

  return hr;
}

typedef unsigned short NvU16;
typedef unsigned long  NvU32; /* 0 to 4294967295 */

typedef enum _NvAPI_Status
{
  NVAPI_OK                  =  0, //!< Success. Request is completed.
  NVAPI_ERROR               = -1, //!< Generic error
  NVAPI_LIBRARY_NOT_FOUND   = -2, //!< NVAPI support library cannot be loaded.
  NVAPI_NO_IMPLEMENTATION   = -3, //!< not implemented in current driver installation
  NVAPI_API_NOT_INITIALIZED = -4, //!< NvAPI_Initialize has not been called (successfully)
  NVAPI_INVALID_ARGUMENT    = -5
} NvAPI_Status;

typedef enum _NV_DYNAMIC_RANGE
{
  NV_DYNAMIC_RANGE_VESA = 0x0,
  NV_DYNAMIC_RANGE_CEA  = 0x1,
  
  NV_DYNAMIC_RANGE_AUTO = 0xFF
} NV_DYNAMIC_RANGE;

typedef enum _NV_BPC
{
  NV_BPC_DEFAULT = 0,
  NV_BPC_6       = 1,
  NV_BPC_8       = 2,
  NV_BPC_10      = 3,
  NV_BPC_12      = 4,
  NV_BPC_16      = 5,
} NV_BPC;

//!  See Table 14 of CEA-861E.  Not all of this is supported by the GPU.
typedef enum
{
  NV_COLOR_FORMAT_RGB      = 0,
  NV_COLOR_FORMAT_YUV422,
  NV_COLOR_FORMAT_YUV444,
  NV_COLOR_FORMAT_YUV420,
  
  NV_COLOR_FORMAT_DEFAULT = 0xFE,
  NV_COLOR_FORMAT_AUTO    = 0xFF
} NV_COLOR_FORMAT;

typedef enum
{
  NV_HDR_CMD_GET = 0, //!< Get current HDR output configuration
  NV_HDR_CMD_SET = 1  //!< Set HDR output configuration
} NV_HDR_CMD;

typedef enum
{
  // Official production-ready HDR modes
  NV_HDR_MODE_OFF                 = 0,  //!< Turn off HDR
  NV_HDR_MODE_UHDA                = 2,  //!< Source: CCCS [a.k.a FP16 scRGB, linear, sRGB primaries, [-65504,0, 65504] range, RGB(1,1,1) = 80nits]  Output : UHDA HDR [a.k.a HDR10, RGB/YCC 10/12bpc ST2084(PQ) EOTF RGB(1,1,1) = 10000 nits, Rec2020 color primaries, ST2086 static HDR metadata]. This is the only supported production HDR mode.

  // Experimental
  NV_HDR_MODE_UHDA_PASSTHROUGH    = 5,  //!< Experimental mode only, not for production! Source: HDR10 RGB 10bpc Output: HDR10 RGB 10 bpc - signal UHDA HDR mode (PQ + Rec2020) to the sink but send source pixel values unmodified (no PQ or Rec2020 conversions) - assumes source is already in HDR10 format.
  NV_HDR_MODE_DOLBY_VISION        = 7,  //!< Experimental mode only, not for production! Source: RGB8 Dolby Vision encoded (12 bpc YCbCr422 packed into RGB8) Output: Dolby Vision encoded : Application is to encoded frames in DV format and embed DV dynamic metadata as described in Dolby Vision specification.

  // Unsupported/obsolete HDR modes
  NV_HDR_MODE_EDR                 = 3,  //!< Do not use! Internal test mode only, to be removed. Source: CCCS (a.k.a FP16 scRGB) Output : EDR (Extended Dynamic Range) - HDR content is tonemapped and gamut mapped to output on regular SDR display set to max luminance ( ~300 nits ).
  NV_HDR_MODE_SDR                 = 4,  //!< Do not use! Internal test mode only, to be removed. Source: any Output: SDR (Standard Dynamic Range), we continuously send SDR EOTF InfoFrame signaling, HDMI compliance testing.
  NV_HDR_MODE_UHDA_NB             = 6,  //!< Do not use! Internal test mode only, to be removed. Source: CCCS (a.k.a FP16 scRGB) Output : notebook HDR
  NV_HDR_MODE_UHDBD               = 2   //!< Do not use! Obsolete, to be removed. NV_HDR_MODE_UHDBD == NV_HDR_MODE_UHDA, reflects obsolete pre-UHDA naming convention.
} NV_HDR_MODE;

typedef enum
{
  NV_STATIC_METADATA_TYPE_1 = 0 //!< Tells the type of structure used to define the Static Metadata Descriptor block.
}NV_STATIC_METADATA_DESCRIPTOR_ID;

typedef struct _NV_HDR_COLOR_DATA_V1
{
  NvU32                            version;                         //!< Version of this structure
  NV_HDR_CMD                       cmd;                             //!< Command get/set
  NV_HDR_MODE                      hdrMode;                         //!< HDR mode
  NV_STATIC_METADATA_DESCRIPTOR_ID static_metadata_descriptor_id;   //!< Static Metadata Descriptor Id (0 for static metadata type 1)

  struct                                                            //!< Static Metadata Descriptor Type 1, CEA-861.3, SMPTE ST2086
  {
    NvU16                          displayPrimary_x0;               //!< x coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                          displayPrimary_y0;               //!< y coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                          displayPrimary_x1;               //!< x coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                          displayPrimary_y1;               //!< y coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                          displayPrimary_x2;               //!< x coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                          displayPrimary_y2;               //!< y coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                          displayWhitePoint_x;             //!< x coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                          displayWhitePoint_y;             //!< y coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                          max_display_mastering_luminance; //!< Maximum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
    NvU16                          min_display_mastering_luminance; //!< Minimum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 6.55350] cd/m^2)

    NvU16                          max_content_light_level;         //!< Maximum Content Light level (MaxCLL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
    NvU16                          max_frame_average_light_level;   //!< Maximum Frame-Average Light Level (MaxFALL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
  } mastering_display_data;
} NV_HDR_COLOR_DATA_V1;

typedef struct _NV_HDR_COLOR_DATA_V2
{
  NvU32                             version;                         //!< Version of this structure
  NV_HDR_CMD                        cmd;                             //!< Command get/set
  NV_HDR_MODE                       hdrMode;                         //!< HDR mode
  NV_STATIC_METADATA_DESCRIPTOR_ID  static_metadata_descriptor_id;   //!< Static Metadata Descriptor Id (0 for static metadata type 1)

  struct                                                             //!< Static Metadata Descriptor Type 1, CEA-861.3, SMPTE ST2086
  {
    NvU16                           displayPrimary_x0;               //!< x coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                           displayPrimary_y0;               //!< y coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                           displayPrimary_x1;               //!< x coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                           displayPrimary_y1;               //!< y coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                           displayPrimary_x2;               //!< x coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                           displayPrimary_y2;               //!< y coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                           displayWhitePoint_x;             //!< x coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
    NvU16                           displayWhitePoint_y;             //!< y coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

    NvU16                           max_display_mastering_luminance; //!< Maximum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
    NvU16                           min_display_mastering_luminance; //!< Minimum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 6.55350] cd/m^2)

    NvU16                           max_content_light_level;         //!< Maximum Content Light level (MaxCLL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
    NvU16                           max_frame_average_light_level;   //!< Maximum Frame-Average Light Level (MaxFALL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
  } mastering_display_data;

  NV_COLOR_FORMAT                   hdrColorFormat;                  //!< Optional, One of NV_COLOR_FORMAT enum values, if set it will apply requested color format for HDR session
  NV_DYNAMIC_RANGE                  hdrDynamicRange;                 //!< Optional, One of NV_DYNAMIC_RANGE enum values, if set it will apply requested dynamic range for HDR session
  NV_BPC                            hdrBpc;                          //!< Optional, One of NV_BPC enum values, if set it will apply requested color depth
                                                                     //!< Dolby Vision mode: DV supports specific combinations of colorformat, dynamic range and bpc. Please refer Dolby Vision specification.
                                                                     //!<                    If invalid or no combination is passed driver will force default combination of RGB format + full range + 8bpc.
                                                                     //!< HDR mode: These fields are ignored in hdr mode
} NV_HDR_COLOR_DATA_V2;

#define NV_HDR_COLOR_DATA_VER1  MAKE_NVAPI_VERSION(NV_HDR_COLOR_DATA_V1, 1)
#define NV_HDR_COLOR_DATA_VER2  MAKE_NVAPI_VERSION(NV_HDR_COLOR_DATA_V2, 2)

#ifndef NV_HDR_COLOR_DATA_VER
#define NV_HDR_COLOR_DATA_VER   NV_HDR_COLOR_DATA_VER2
typedef NV_HDR_COLOR_DATA_V2    NV_HDR_COLOR_DATA;
#endif

using NvAPI_Disp_HdrColorControl_pfn      = NvAPI_Status (__cdecl *)(NvU32, NV_HDR_COLOR_DATA*);
      NvAPI_Disp_HdrColorControl_pfn
      NvAPI_Disp_HdrColorControl_Original = nullptr;

NvAPI_Status
__cdecl
NvAPI_Disp_HdrColorControl_Override ( NvU32              displayId,
                                      NV_HDR_COLOR_DATA *pHdrColorData )
{
  NvAPI_Status ret = NVAPI_OK;

  static NV_HDR_MODE mode = NV_HDR_MODE_OFF;

  if (pHdrColorData != nullptr && pHdrColorData->cmd == NV_HDR_CMD_SET)
  {
    mode = pHdrColorData->hdrMode;

    __try {
      CComQIPtr <IDXGISwapChain3>
          pSwapChain3 (g_pLastFullscreenSwapChain);
      if (pSwapChain3.p != nullptr)
      {
        pSwapChain3->SetColorSpace1 (
          pHdrColorData->hdrMode == NV_HDR_MODE_UHDA_PASSTHROUGH ? DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 :
          pHdrColorData->hdrMode == NV_HDR_MODE_UHDA             ? DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709    :
                                                                   DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 );
      }
    }

    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
  }

  else if (pHdrColorData != nullptr && pHdrColorData->cmd == NV_HDR_CMD_GET)
  {
    ret =
      NvAPI_Disp_HdrColorControl_Original (displayId, pHdrColorData);

    if (ret == NVAPI_OK)
      pHdrColorData->hdrMode = mode;
  }

  return ret;
}

DWORD
WINAPI
FakeFullscreen_InitThread (LPVOID)
{
  CoInitializeEx (nullptr, COINIT_MULTITHREADED);

   using PFun_slUpgradeInterface   = sl::Result(                      void** baseInterface);
  static PFun_slUpgradeInterface * slUpgradeInterface =
        (PFun_slUpgradeInterface *) GetProcAddress (GetModuleHandleW (L"sl.interposer.dll"), "slUpgradeInterface");

   using PFun_slGetNativeInterface = sl::Result(void* proxyInterface, void** baseInterface);
  static PFun_slGetNativeInterface * slGetNativeInterface =
        (PFun_slGetNativeInterface *) GetProcAddress (GetModuleHandleW (L"sl.interposer.dll"), "slGetNativeInterface");

  HWND hWnd =
    CreateWindow (
      L"static", L"HDR10", WS_VISIBLE | WS_POPUP        |
          WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN |
          WS_CLIPSIBLINGS, 0, 0,
                             2, 2,
                              0, 0, 0, 0
    );

  // Special K's DLL might be injected after creating the window above...
  //if (! GetModuleHandleW (L"SpecialK64.dll"))
  {
    using   CreateDXGIFactory2_pfn = HRESULT (WINAPI *)(UINT,REFIID,void**);
    static  CreateDXGIFactory2_pfn
           _CreateDXGIFactory2 =
           (CreateDXGIFactory2_pfn) GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
           "CreateDXGIFactory2");

    CComPtr <IDXGIFactory>                                pFactory;
    _CreateDXGIFactory2 (0x0, IID_IDXGIFactory, (void **)&pFactory.p);

    CComPtr <IDXGIFactory> pFactoryNative;

    if (slUpgradeInterface != nullptr)
        slUpgradeInterface ((void **)&pFactory.p);

    if (slGetNativeInterface != nullptr)
        slGetNativeInterface (pFactory, (void **)&pFactoryNative.p);

    if (pFactoryNative.p != nullptr)
        pFactory = pFactoryNative;

    // ... wtf?
    if (! pFactory)
      return 0;

    DXGI_SWAP_CHAIN_DESC
      swapDesc                   = { };
      swapDesc.BufferDesc.Width  = 2;
      swapDesc.BufferDesc.Height = 2;
      swapDesc.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
      swapDesc.BufferCount       = 3;
      swapDesc.Windowed          = TRUE;
      swapDesc.OutputWindow      = hWnd;
      swapDesc.SampleDesc        = { 1, 0 };
      swapDesc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
      swapDesc.BufferUsage       = DXGI_USAGE_BACK_BUFFER;

    CComPtr <IDXGISwapChain> pSwapChain;

    DXGI_VIRTUAL_HOOK ( &pFactory,     10,
                 "IDXGIFactory::CreateSwapChain",
                  DXGIFactory_CreateSwapChain_Override,
                              CreateSwapChain_Original,
                              CreateSwapChain_pfn );


    if ( SUCCEEDED (
           D3D11CreateDeviceAndSwapChain (
              nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0x0, nullptr,
                0, D3D11_SDK_VERSION, &swapDesc, &pSwapChain.p,
                  nullptr, nullptr, nullptr ) )
       )
    {
      CComQIPtr <IDXGISwapChain>
          pSwapChain4 (pSwapChain);
      CComPtr   <IDXGISwapChain>
          pSwapChainNative;

      if (slUpgradeInterface != nullptr)
          slUpgradeInterface ((void **)&pSwapChain4.p);

      if (slGetNativeInterface != nullptr)
          slGetNativeInterface (pSwapChain4.p, (void **)&pSwapChainNative.p);

      if (pSwapChainNative.p != nullptr)
          pSwapChain4 = pSwapChainNative.p;

      DXGI_VIRTUAL_HOOK ( &pSwapChain4.p, 10, "IDXGISwapChain::SetFullscreenState",
                                                      DXGISwap_SetFullscreenState_Override,
                                                               SetFullscreenState_Original,
                                                               SetFullscreenState_pfn );

      DXGI_VIRTUAL_HOOK ( &pSwapChain4.p, 11, "IDXGISwapChain::GetFullscreenState",
                                                      DXGISwap_GetFullscreenState_Override,
                                                               GetFullscreenState_Original,
                                                               GetFullscreenState_pfn );

      DXGI_VIRTUAL_HOOK ( &pSwapChain4.p, 12, "IDXGISwapChain::GetDesc",
                                                      DXGISwap_GetDesc_Override,
                                                               GetDesc_Original,
                                                               GetDesc_pfn );

      DXGI_VIRTUAL_HOOK ( &pSwapChain4.p, 19, "IDXGISwapChain::GetFullscreenDesc",
                                                      DXGISwap_GetFullscreenDesc_Override,
                                                               GetFullscreenDesc_Original,
                                                               GetFullscreenDesc_pfn );

      typedef void* (*NvAPI_QueryInterface_pfn)(unsigned int offset);
      typedef NvAPI_Status(__cdecl *NvAPI_RestartDisplayDriver_pfn)(void);
      NvAPI_QueryInterface_pfn          NvAPI_QueryInterface       =
        (NvAPI_QueryInterface_pfn)GetProcAddress (LoadLibraryW (L"nvapi64.dll"), "nvapi_QueryInterface");

      if (NvAPI_QueryInterface != nullptr)
      {
        SK_CreateFuncHook ( L"NvAPI_Disp_HdrColorControl",
                              NvAPI_QueryInterface (891134500),
                              NvAPI_Disp_HdrColorControl_Override,
                  (void **) (&NvAPI_Disp_HdrColorControl_Original) );

        MH_QueueEnableHook (NvAPI_QueryInterface (891134500));
      }
    }

    MH_ApplyQueued ();
  }

  DestroyWindow (hWnd);

  return 0;
}

BOOL
APIENTRY
DllMain ( HMODULE hModule,
          DWORD   ul_reason_for_call,
          LPVOID  lpReserved )
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
      InterlockedIncrement (&__VP_DLL_Refs);

      config = { };

#ifdef _M_IX86
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_4,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_3,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_2,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_1,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput9_1_0, MAX_PATH);

      GetSystemWow64DirectoryW (config.wszPathToSystemDXGI,        MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemD3D12,       MAX_PATH);
#else
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_4,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_3,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_2,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_1,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput9_1_0, MAX_PATH);

      GetSystemDirectoryW      (config.wszPathToSystemDXGI,        MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemD3D12,       MAX_PATH);
#endif
      PathAppendW              (config.wszPathToSystemXInput1_4,   L"XInput1_4.dll");
      PathAppendW              (config.wszPathToSystemXInput1_3,   L"XInput1_3.dll");
      PathAppendW              (config.wszPathToSystemXInput1_2,   L"XInput1_2.dll");
      PathAppendW              (config.wszPathToSystemXInput1_1,   L"XInput1_1.dll");
      PathAppendW              (config.wszPathToSystemXInput9_1_0, L"XInput9_1_0.dll");

      PathAppendW              (config.wszPathToSystemDXGI,        L"dxgi.dll");
      PathAppendW              (config.wszPathToSystemD3D12,       L"d3d12.dll");

      if (MH_OK == MH_Initialize ())
      {
        SK_COMPAT_CheckStreamlineSupport ();

        if (! GetModuleHandleW (L"SpecialK64.dll"))
        {
          HANDLE hThread =
              CreateThread ( nullptr, 0x0, FakeFullscreen_InitThread,
                             nullptr, 0x0, nullptr );

          WaitForSingleObject (hThread, 1500);

          CloseHandle (hThread);

          GetModuleHandleExW (GET_MODULE_HANDLE_EX_FLAG_PIN |
                              GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)DllMain, &hModule);

          DisableThreadLibraryCalls (hModule);
        }
      }
    } break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;

    case DLL_PROCESS_DETACH:
      InterlockedDecrement (&__VP_DLL_Refs);

      MH_Uninitialize ();
      break;
  }

  return TRUE;
}