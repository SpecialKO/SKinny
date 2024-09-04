
// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "config.h"
#include <cassert>
#include <string>

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


#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>
#include <d3d11.h>

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")

IDXGISwapChain* g_pLastFullscreenSwapChain = nullptr;

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
  //if (   ReadAcquire (&__SK_DLL_Ending  ) ||
  //    (! ReadAcquire (&__SK_DLL_Attached))  )
  //{
  //  return MH_ERROR_DISABLED;
  //}

  //SK_RunOnce (SK_MinHook_Init ());

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

CreateSwapChain_pfn CreateSwapChain_Original = nullptr;

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

SetFullscreenState_pfn SetFullscreenState_Original = nullptr;
GetFullscreenState_pfn GetFullscreenState_Original = nullptr;
GetDesc_pfn            GetDesc_Original            = nullptr;
GetFullscreenDesc_pfn  GetFullscreenDesc_Original  = nullptr;

#include <concurrent_unordered_map.h>
concurrency::concurrent_unordered_map <IDXGISwapChain*, BOOL> fullscreen_state;

HRESULT
STDMETHODCALLTYPE
DXGISwap_SetFullscreenState_Override ( IDXGISwapChain *This,
                                       BOOL            Fullscreen,
                                       IDXGIOutput    *pTarget )
{
  fullscreen_state [This] = Fullscreen;

  //if (Fullscreen)
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

namespace sl
{
enum class Result
{
  eOk,
  eErrorIO,
  eErrorDriverOutOfDate,
  eErrorOSOutOfDate,
  eErrorOSDisabledHWS,
  eErrorDeviceNotCreated,
  eErrorNoSupportedAdapterFound,
  eErrorAdapterNotSupported,
  eErrorNoPlugins,
  eErrorVulkanAPI,
  eErrorDXGIAPI,
  eErrorD3DAPI,
  eErrorNRDAPI,
  eErrorNVAPI,
  eErrorReflexAPI,
  eErrorNGXFailed,
  eErrorJSONParsing,
  eErrorMissingProxy,
  eErrorMissingResourceState,
  eErrorInvalidIntegration,
  eErrorMissingInputParameter,
  eErrorNotInitialized,
  eErrorComputeFailed,
  eErrorInitNotCalled,
  eErrorExceptionHandler,
  eErrorInvalidParameter,
  eErrorMissingConstants,
  eErrorDuplicatedConstants,
  eErrorMissingOrInvalidAPI,
  eErrorCommonConstantsMissing,
  eErrorUnsupportedInterface,
  eErrorFeatureMissing,
  eErrorFeatureNotSupported,
  eErrorFeatureMissingHooks,
  eErrorFeatureFailedToLoad,
  eErrorFeatureWrongPriority,
  eErrorFeatureMissingDependency,
  eErrorFeatureManagerInvalidState,
  eErrorInvalidState,
  eWarnOutOfVRAM
};
}

typedef unsigned short   NvU16;
typedef unsigned long    NvU32; /* 0 to 4294967295                         */

typedef enum _NvAPI_Status
{
    NVAPI_OK                                    =  0,      //!< Success. Request is completed.
    NVAPI_ERROR                                 = -1,      //!< Generic error
    NVAPI_LIBRARY_NOT_FOUND                     = -2,      //!< NVAPI support library cannot be loaded.
    NVAPI_NO_IMPLEMENTATION                     = -3,      //!< not implemented in current driver installation
    NVAPI_API_NOT_INITIALIZED                   = -4,      //!< NvAPI_Initialize has not been called (successfully)
    NVAPI_INVALID_ARGUMENT                      = -5,      //!< The argument/parameter value is not valid or NULL.
    NVAPI_NVIDIA_DEVICE_NOT_FOUND               = -6,      //!< No NVIDIA display driver, or NVIDIA GPU driving a display, was found.
    NVAPI_END_ENUMERATION                       = -7,      //!< No more items to enumerate
    NVAPI_INVALID_HANDLE                        = -8,      //!< Invalid handle
    NVAPI_INCOMPATIBLE_STRUCT_VERSION           = -9,      //!< An argument's structure version is not supported
    NVAPI_HANDLE_INVALIDATED                    = -10,     //!< The handle is no longer valid (likely due to GPU or display re-configuration)
    NVAPI_OPENGL_CONTEXT_NOT_CURRENT            = -11,     //!< No NVIDIA OpenGL context is current (but needs to be)
    NVAPI_INVALID_POINTER                       = -14,     //!< An invalid pointer, usually NULL, was passed as a parameter
    NVAPI_NO_GL_EXPERT                          = -12,     //!< OpenGL Expert is not supported by the current drivers
    NVAPI_INSTRUMENTATION_DISABLED              = -13,     //!< OpenGL Expert is supported, but driver instrumentation is currently disabled
    NVAPI_NO_GL_NSIGHT                          = -15,     //!< OpenGL does not support Nsight

    NVAPI_EXPECTED_LOGICAL_GPU_HANDLE           = -100,    //!< Expected a logical GPU handle for one or more parameters
    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE          = -101,    //!< Expected a physical GPU handle for one or more parameters
    NVAPI_EXPECTED_DISPLAY_HANDLE               = -102,    //!< Expected an NV display handle for one or more parameters
    NVAPI_INVALID_COMBINATION                   = -103,    //!< The combination of parameters is not valid. 
    NVAPI_NOT_SUPPORTED                         = -104,    //!< Requested feature is not supported in the selected GPU
    NVAPI_PORTID_NOT_FOUND                      = -105,    //!< No port ID was found for the I2C transaction
    NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE    = -106,    //!< Expected an unattached display handle as one of the input parameters.
    NVAPI_INVALID_PERF_LEVEL                    = -107,    //!< Invalid perf level 
    NVAPI_DEVICE_BUSY                           = -108,    //!< Device is busy; request not fulfilled
    NVAPI_NV_PERSIST_FILE_NOT_FOUND             = -109,    //!< NV persist file is not found
    NVAPI_PERSIST_DATA_NOT_FOUND                = -110,    //!< NV persist data is not found
    NVAPI_EXPECTED_TV_DISPLAY                   = -111,    //!< Expected a TV output display
    NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR     = -112,    //!< Expected a TV output on the D Connector - HDTV_EIAJ4120.
    NVAPI_NO_ACTIVE_SLI_TOPOLOGY                = -113,    //!< SLI is not active on this device.
    NVAPI_SLI_RENDERING_MODE_NOTALLOWED         = -114,    //!< Setup of SLI rendering mode is not possible right now.
    NVAPI_EXPECTED_DIGITAL_FLAT_PANEL           = -115,    //!< Expected a digital flat panel.
    NVAPI_ARGUMENT_EXCEED_MAX_SIZE              = -116,    //!< Argument exceeds the expected size.
    NVAPI_DEVICE_SWITCHING_NOT_ALLOWED          = -117,    //!< Inhibit is ON due to one of the flags in NV_GPU_DISPLAY_CHANGE_INHIBIT or SLI active.
    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED          = -118,    //!< Testing of clocks is not supported.
    NVAPI_UNKNOWN_UNDERSCAN_CONFIG              = -119,    //!< The specified underscan config is from an unknown source (e.g. INF)
    NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO        = -120,    //!< Timeout while reconfiguring GPUs
    NVAPI_DATA_NOT_FOUND                        = -121,    //!< Requested data was not found
    NVAPI_EXPECTED_ANALOG_DISPLAY               = -122,    //!< Expected an analog display
    NVAPI_NO_VIDLINK                            = -123,    //!< No SLI video bridge is present
    NVAPI_REQUIRES_REBOOT                       = -124,    //!< NVAPI requires a reboot for the settings to take effect
    NVAPI_INVALID_HYBRID_MODE                   = -125,    //!< The function is not supported with the current Hybrid mode.
    NVAPI_MIXED_TARGET_TYPES                    = -126,    //!< The target types are not all the same
    NVAPI_SYSWOW64_NOT_SUPPORTED                = -127,    //!< The function is not supported from 32-bit on a 64-bit system.
    NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,    //!< There is no implicit GPU topology active. Use NVAPI_SetHybridMode to change topology.
    NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,      //!< Prompt the user to close all non-migratable applications.    
    NVAPI_OUT_OF_MEMORY                         = -130,    //!< Could not allocate sufficient memory to complete the call.
    NVAPI_WAS_STILL_DRAWING                     = -131,    //!< The previous operation that is transferring information to or from this surface is incomplete.
    NVAPI_FILE_NOT_FOUND                        = -132,    //!< The file was not found.
    NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS         = -133,    //!< There are too many unique instances of a particular type of state object.
    NVAPI_INVALID_CALL                          = -134,    //!< The method call is invalid. For example, a method's parameter may not be a valid pointer.
    NVAPI_D3D10_1_LIBRARY_NOT_FOUND             = -135,    //!< d3d10_1.dll cannot be loaded.
    NVAPI_FUNCTION_NOT_FOUND                    = -136,    //!< Couldn't find the function in the loaded DLL.
    NVAPI_INVALID_USER_PRIVILEGE                = -137,    //!< The application will require Administrator privileges to access this API.
	                                                       //!< The application can be elevated to a higher permission level by selecting "Run as Administrator".
    NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE   = -138,    //!< The handle corresponds to GDIPrimary.
    NVAPI_EXPECTED_COMPUTE_GPU_HANDLE           = -139,    //!< Setting Physx GPU requires that the GPU is compute-capable.
    NVAPI_STEREO_NOT_INITIALIZED                = -140,    //!< The Stereo part of NVAPI failed to initialize completely. Check if the stereo driver is installed.
    NVAPI_STEREO_REGISTRY_ACCESS_FAILED         = -141,    //!< Access to stereo-related registry keys or values has failed.
    NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142, //!< The given registry profile type is not supported.
    NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED   = -143,    //!< The given registry value is not supported.
    NVAPI_STEREO_NOT_ENABLED                    = -144,    //!< Stereo is not enabled and the function needed it to execute completely.
    NVAPI_STEREO_NOT_TURNED_ON                  = -145,    //!< Stereo is not turned on and the function needed it to execute completely.
    NVAPI_STEREO_INVALID_DEVICE_INTERFACE       = -146,    //!< Invalid device interface.
    NVAPI_STEREO_PARAMETER_OUT_OF_RANGE         = -147,    //!< Separation percentage or JPEG image capture quality is out of [0-100] range.
    NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148, //!< The given frustum adjust mode is not supported.
    NVAPI_TOPO_NOT_POSSIBLE                     = -149,    //!< The mosaic topology is not possible given the current state of the hardware.
    NVAPI_MODE_CHANGE_FAILED                    = -150,    //!< An attempt to do a display resolution mode change has failed.        
    NVAPI_D3D11_LIBRARY_NOT_FOUND               = -151,    //!< d3d11.dll/d3d11_beta.dll cannot be loaded.
    NVAPI_INVALID_ADDRESS                       = -152,    //!< Address is outside of valid range.
    NVAPI_STRING_TOO_SMALL                      = -153,    //!< The pre-allocated string is too small to hold the result.
    NVAPI_MATCHING_DEVICE_NOT_FOUND             = -154,    //!< The input does not match any of the available devices.
    NVAPI_DRIVER_RUNNING                        = -155,    //!< Driver is running.
    NVAPI_DRIVER_NOTRUNNING                     = -156,    //!< Driver is not running.
    NVAPI_ERROR_DRIVER_RELOAD_REQUIRED          = -157,    //!< A driver reload is required to apply these settings.
    NVAPI_SET_NOT_ALLOWED                       = -158,    //!< Intended setting is not allowed.
    NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED    = -159,    //!< Information can't be returned due to "advanced display topology".
    NVAPI_SETTING_NOT_FOUND                     = -160,    //!< Setting is not found.
    NVAPI_SETTING_SIZE_TOO_LARGE                = -161,    //!< Setting size is too large.
    NVAPI_TOO_MANY_SETTINGS_IN_PROFILE          = -162,    //!< There are too many settings for a profile. 
    NVAPI_PROFILE_NOT_FOUND                     = -163,    //!< Profile is not found.
    NVAPI_PROFILE_NAME_IN_USE                   = -164,    //!< Profile name is duplicated.
    NVAPI_PROFILE_NAME_EMPTY                    = -165,    //!< Profile name is empty.
    NVAPI_EXECUTABLE_NOT_FOUND                  = -166,    //!< Application not found in the Profile.
    NVAPI_EXECUTABLE_ALREADY_IN_USE             = -167,    //!< Application already exists in the other profile.
    NVAPI_DATATYPE_MISMATCH                     = -168,    //!< Data Type mismatch 
    NVAPI_PROFILE_REMOVED                       = -169,    //!< The profile passed as parameter has been removed and is no longer valid.
    NVAPI_UNREGISTERED_RESOURCE                 = -170,    //!< An unregistered resource was passed as a parameter. 
    NVAPI_ID_OUT_OF_RANGE                       = -171,    //!< The DisplayId corresponds to a display which is not within the normal outputId range.
    NVAPI_DISPLAYCONFIG_VALIDATION_FAILED       = -172,    //!< Display topology is not valid so the driver cannot do a mode set on this configuration.
    NVAPI_DPMST_CHANGED                         = -173,    //!< Display Port Multi-Stream topology has been changed.
    NVAPI_INSUFFICIENT_BUFFER                   = -174,    //!< Input buffer is insufficient to hold the contents.    
    NVAPI_ACCESS_DENIED                         = -175,    //!< No access to the caller.
    NVAPI_MOSAIC_NOT_ACTIVE                     = -176,    //!< The requested action cannot be performed without Mosaic being enabled.
    NVAPI_SHARE_RESOURCE_RELOCATED              = -177,    //!< The surface is relocated away from video memory.
    NVAPI_REQUEST_USER_TO_DISABLE_DWM           = -178,    //!< The user should disable DWM before calling NvAPI.
    NVAPI_D3D_DEVICE_LOST                       = -179,    //!< D3D device status is D3DERR_DEVICELOST or D3DERR_DEVICENOTRESET - the user has to reset the device.
    NVAPI_INVALID_CONFIGURATION                 = -180,    //!< The requested action cannot be performed in the current state.
    NVAPI_STEREO_HANDSHAKE_NOT_DONE             = -181,    //!< Call failed as stereo handshake not completed.
    NVAPI_EXECUTABLE_PATH_IS_AMBIGUOUS          = -182,    //!< The path provided was too short to determine the correct NVDRS_APPLICATION
    NVAPI_DEFAULT_STEREO_PROFILE_IS_NOT_DEFINED = -183,    //!< Default stereo profile is not currently defined
    NVAPI_DEFAULT_STEREO_PROFILE_DOES_NOT_EXIST = -184,    //!< Default stereo profile does not exist
    NVAPI_CLUSTER_ALREADY_EXISTS                = -185,    //!< A cluster is already defined with the given configuration.
    NVAPI_DPMST_DISPLAY_ID_EXPECTED             = -186,    //!< The input display id is not that of a multi stream enabled connector or a display device in a multi stream topology 
    NVAPI_INVALID_DISPLAY_ID                    = -187,    //!< The input display id is not valid or the monitor associated to it does not support the current operation
    NVAPI_STREAM_IS_OUT_OF_SYNC                 = -188,    //!< While playing secure audio stream, stream goes out of sync
    NVAPI_INCOMPATIBLE_AUDIO_DRIVER             = -189,    //!< Older audio driver version than required
    NVAPI_VALUE_ALREADY_SET                     = -190,    //!< Value already set, setting again not allowed.
    NVAPI_TIMEOUT                               = -191,    //!< Requested operation timed out 
    NVAPI_GPU_WORKSTATION_FEATURE_INCOMPLETE    = -192,    //!< The requested workstation feature set has incomplete driver internal allocation resources
    NVAPI_STEREO_INIT_ACTIVATION_NOT_DONE       = -193,    //!< Call failed because InitActivation was not called.
    NVAPI_SYNC_NOT_ACTIVE                       = -194,    //!< The requested action cannot be performed without Sync being enabled.    
    NVAPI_SYNC_MASTER_NOT_FOUND                 = -195,    //!< The requested action cannot be performed without Sync Master being enabled.
    NVAPI_INVALID_SYNC_TOPOLOGY                 = -196,    //!< Invalid displays passed in the NV_GSYNC_DISPLAY pointer.
    NVAPI_ECID_SIGN_ALGO_UNSUPPORTED            = -197,    //!< The specified signing algorithm is not supported. Either an incorrect value was entered or the current installed driver/hardware does not support the input value.
    NVAPI_ECID_KEY_VERIFICATION_FAILED          = -198,    //!< The encrypted public key verification has failed.
    NVAPI_FIRMWARE_OUT_OF_DATE                  = -199,    //!< The device's firmware is out of date.
    NVAPI_FIRMWARE_REVISION_NOT_SUPPORTED       = -200,    //!< The device's firmware is not supported.
    NVAPI_LICENSE_CALLER_AUTHENTICATION_FAILED  = -201,    //!< The caller is not authorized to modify the License.
    NVAPI_D3D_DEVICE_NOT_REGISTERED             = -202,    //!< The user tried to use a deferred context without registering the device first  
    NVAPI_RESOURCE_NOT_ACQUIRED                 = -203,    //!< Head or SourceId was not reserved for the VR Display before doing the Modeset or the dedicated display.
    NVAPI_TIMING_NOT_SUPPORTED                  = -204,    //!< Provided timing is not supported.
    NVAPI_HDCP_ENCRYPTION_FAILED                = -205,    //!< HDCP Encryption Failed for the device. Would be applicable when the device is HDCP Capable.
    NVAPI_PCLK_LIMITATION_FAILED                = -206,    //!< Provided mode is over sink device pclk limitation.
    NVAPI_NO_CONNECTOR_FOUND                    = -207,    //!< No connector on GPU found. 
    NVAPI_HDCP_DISABLED                         = -208,    //!< When a non-HDCP capable HMD is connected, we would inform user by this code.
    NVAPI_API_IN_USE                            = -209,    //!< Atleast an API is still being called
    NVAPI_NVIDIA_DISPLAY_NOT_FOUND              = -210,    //!< No display found on Nvidia GPU(s).
    NVAPI_PRIV_SEC_VIOLATION                    = -211,    //!< Priv security violation, improper access to a secured register.
    NVAPI_INCORRECT_VENDOR                      = -212,    //!< NVAPI cannot be called by this vendor
    NVAPI_DISPLAY_IN_USE                        = -213,    //!< DirectMode Display is already in use
    NVAPI_UNSUPPORTED_CONFIG_NON_HDCP_HMD       = -214,    //!< The Config is having Non-NVidia GPU with Non-HDCP HMD connected
    NVAPI_MAX_DISPLAY_LIMIT_REACHED             = -215,    //!< GPU's Max Display Limit has Reached
    NVAPI_INVALID_DIRECT_MODE_DISPLAY           = -216,    //!< DirectMode not Enabled on the Display
    NVAPI_GPU_IN_DEBUG_MODE                     = -217,    //!< GPU is in debug mode, OC is NOT allowed.
    NVAPI_D3D_CONTEXT_NOT_FOUND                 = -218,    //!< No NvAPI context was found for this D3D object
    NVAPI_STEREO_VERSION_MISMATCH               = -219,    //!< there is version mismatch between stereo driver and dx driver
    NVAPI_GPU_NOT_POWERED                       = -220,    //!< GPU is not powered and so the request cannot be completed.
    NVAPI_ERROR_DRIVER_RELOAD_IN_PROGRESS       = -221,    //!< The display driver update in progress.
    NVAPI_WAIT_FOR_HW_RESOURCE                  = -222,    //!< Wait for HW resources allocation
    NVAPI_REQUIRE_FURTHER_HDCP_ACTION           = -223,    //!< operation requires further HDCP action
    NVAPI_DISPLAY_MUX_TRANSITION_FAILED         = -224,    //!< Dynamic Mux transition failure
    NVAPI_INVALID_DSC_VERSION                   = -225,    //!< Invalid DSC version
    NVAPI_INVALID_DSC_SLICECOUNT                = -226,    //!< Invalid DSC slice count
    NVAPI_INVALID_DSC_OUTPUT_BPP                = -227,    //!< Invalid DSC output BPP
    NVAPI_FAILED_TO_LOAD_FROM_DRIVER_STORE      = -228,    //!< There was an error while loading nvapi.dll from the driver store.
    NVAPI_NO_VULKAN                             = -229,    //!< OpenGL does not export Vulkan fake extensions
    NVAPI_REQUEST_PENDING                       = -230,    //!< A request for NvTOPPs telemetry CData has already been made and is pending a response.
    NVAPI_RESOURCE_IN_USE                       = -231,    //!< Operation cannot be performed because the resource is in use.
    NVAPI_INVALID_IMAGE                         = -232,    //!< Device kernel image is invalid
    NVAPI_INVALID_PTX                           = -233,    //!< PTX JIT compilation failed
    NVAPI_NVLINK_UNCORRECTABLE                  = -234,    //!< Uncorrectable NVLink error was detected during the execution
    NVAPI_JIT_COMPILER_NOT_FOUND                = -235,    //!< PTX JIT compiler library was not found.
    NVAPI_INVALID_SOURCE                        = -236,    //!< Device kernel source is invalid.
    NVAPI_ILLEGAL_INSTRUCTION                   = -237,    //!< While executing a kernel, the device encountered an illegal instruction.
    NVAPI_INVALID_PC                            = -238,    //!< While executing a kernel, the device program counter wrapped its address space
    NVAPI_LAUNCH_FAILED                         = -239,    //!< An exception occurred on the device while executing a kernel
    NVAPI_NOT_PERMITTED                         = -240,    //!< Attempted operation is not permitted.
    NVAPI_CALLBACK_ALREADY_REGISTERED           = -241,    //!< The callback function has already been registered.
    NVAPI_CALLBACK_NOT_FOUND                    = -242,    //!< The callback function is not found or not registered.
} NvAPI_Status;

typedef enum _NV_DYNAMIC_RANGE
{
    NV_DYNAMIC_RANGE_VESA     = 0x0,
    NV_DYNAMIC_RANGE_CEA      = 0x1,

    NV_DYNAMIC_RANGE_AUTO     = 0xFF
} NV_DYNAMIC_RANGE;

typedef enum _NV_BPC
{
    NV_BPC_DEFAULT         = 0,
    NV_BPC_6               = 1,
    NV_BPC_8               = 2,
    NV_BPC_10              = 3,
    NV_BPC_12              = 4,
    NV_BPC_16              = 5,
} NV_BPC;

//!  See Table 14 of CEA-861E.  Not all of this is supported by the GPU.
typedef enum
{
    NV_COLOR_FORMAT_RGB             = 0,
    NV_COLOR_FORMAT_YUV422,
    NV_COLOR_FORMAT_YUV444,
    NV_COLOR_FORMAT_YUV420,

    NV_COLOR_FORMAT_DEFAULT         = 0xFE,
    NV_COLOR_FORMAT_AUTO            = 0xFF
} NV_COLOR_FORMAT;

typedef enum
{
    NV_HDR_CMD_GET = 0,                             //!< Get current HDR output configuration
    NV_HDR_CMD_SET = 1                              //!< Set HDR output configuration
} NV_HDR_CMD;

typedef enum
{
    // Official production-ready HDR modes
    NV_HDR_MODE_OFF                 = 0,            //!< Turn off HDR
    NV_HDR_MODE_UHDA                = 2,            //!< Source: CCCS [a.k.a FP16 scRGB, linear, sRGB primaries, [-65504,0, 65504] range, RGB(1,1,1) = 80nits]  Output : UHDA HDR [a.k.a HDR10, RGB/YCC 10/12bpc ST2084(PQ) EOTF RGB(1,1,1) = 10000 nits, Rec2020 color primaries, ST2086 static HDR metadata]. This is the only supported production HDR mode.

    // Experimental
    NV_HDR_MODE_UHDA_PASSTHROUGH    = 5,            //!< Experimental mode only, not for production! Source: HDR10 RGB 10bpc Output: HDR10 RGB 10 bpc - signal UHDA HDR mode (PQ + Rec2020) to the sink but send source pixel values unmodified (no PQ or Rec2020 conversions) - assumes source is already in HDR10 format.
    NV_HDR_MODE_DOLBY_VISION        = 7,            //!< Experimental mode only, not for production! Source: RGB8 Dolby Vision encoded (12 bpc YCbCr422 packed into RGB8) Output: Dolby Vision encoded : Application is to encoded frames in DV format and embed DV dynamic metadata as described in Dolby Vision specification.

    // Unsupported/obsolete HDR modes
    NV_HDR_MODE_EDR                 = 3,            //!< Do not use! Internal test mode only, to be removed. Source: CCCS (a.k.a FP16 scRGB) Output : EDR (Extended Dynamic Range) - HDR content is tonemapped and gamut mapped to output on regular SDR display set to max luminance ( ~300 nits ).
    NV_HDR_MODE_SDR                 = 4,            //!< Do not use! Internal test mode only, to be removed. Source: any Output: SDR (Standard Dynamic Range), we continuously send SDR EOTF InfoFrame signaling, HDMI compliance testing.
    NV_HDR_MODE_UHDA_NB             = 6,            //!< Do not use! Internal test mode only, to be removed. Source: CCCS (a.k.a FP16 scRGB) Output : notebook HDR
    NV_HDR_MODE_UHDBD               = 2             //!< Do not use! Obsolete, to be removed. NV_HDR_MODE_UHDBD == NV_HDR_MODE_UHDA, reflects obsolete pre-UHDA naming convention.

} NV_HDR_MODE;

typedef enum
{
    NV_STATIC_METADATA_TYPE_1 = 0                   //!< Tells the type of structure used to define the Static Metadata Descriptor block.
}NV_STATIC_METADATA_DESCRIPTOR_ID;

typedef struct _NV_HDR_COLOR_DATA_V1
{
    NvU32                             version;                                 //!< Version of this structure
    NV_HDR_CMD                        cmd;                                     //!< Command get/set
    NV_HDR_MODE                       hdrMode;                                 //!< HDR mode
    NV_STATIC_METADATA_DESCRIPTOR_ID  static_metadata_descriptor_id;           //!< Static Metadata Descriptor Id (0 for static metadata type 1)

    struct                                                                    //!< Static Metadata Descriptor Type 1, CEA-861.3, SMPTE ST2086
    {
        NvU16    displayPrimary_x0;                                           //!< x coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayPrimary_y0;                                           //!< y coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    displayPrimary_x1;                                           //!< x coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayPrimary_y1;                                           //!< y coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    displayPrimary_x2;                                           //!< x coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayPrimary_y2;                                           //!< y coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    displayWhitePoint_x;                                         //!< x coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayWhitePoint_y;                                         //!< y coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    max_display_mastering_luminance;                             //!< Maximum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
        NvU16    min_display_mastering_luminance;                             //!< Minimum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 6.55350] cd/m^2)

        NvU16    max_content_light_level;                                     //!< Maximum Content Light level (MaxCLL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
        NvU16    max_frame_average_light_level;                               //!< Maximum Frame-Average Light Level (MaxFALL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
    } mastering_display_data;
} NV_HDR_COLOR_DATA_V1;

typedef struct _NV_HDR_COLOR_DATA_V2
{
    NvU32                             version;                                 //!< Version of this structure
    NV_HDR_CMD                        cmd;                                     //!< Command get/set
    NV_HDR_MODE                       hdrMode;                                 //!< HDR mode
    NV_STATIC_METADATA_DESCRIPTOR_ID  static_metadata_descriptor_id;           //!< Static Metadata Descriptor Id (0 for static metadata type 1)

    struct                                                                    //!< Static Metadata Descriptor Type 1, CEA-861.3, SMPTE ST2086
    {
        NvU16    displayPrimary_x0;                                           //!< x coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayPrimary_y0;                                           //!< y coordinate of color primary 0 (e.g. Red) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    displayPrimary_x1;                                           //!< x coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayPrimary_y1;                                           //!< y coordinate of color primary 1 (e.g. Green) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    displayPrimary_x2;                                           //!< x coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayPrimary_y2;                                           //!< y coordinate of color primary 2 (e.g. Blue) of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    displayWhitePoint_x;                                         //!< x coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])
        NvU16    displayWhitePoint_y;                                         //!< y coordinate of white point of mastering display ([0x0000-0xC350] = [0.0 - 1.0])

        NvU16    max_display_mastering_luminance;                             //!< Maximum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
        NvU16    min_display_mastering_luminance;                             //!< Minimum display mastering luminance ([0x0001-0xFFFF] = [1.0 - 6.55350] cd/m^2)

        NvU16    max_content_light_level;                                     //!< Maximum Content Light level (MaxCLL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
        NvU16    max_frame_average_light_level;                               //!< Maximum Frame-Average Light Level (MaxFALL) ([0x0001-0xFFFF] = [1.0 - 65535.0] cd/m^2)
    } mastering_display_data;

    NV_COLOR_FORMAT          hdrColorFormat;                                     //!< Optional, One of NV_COLOR_FORMAT enum values, if set it will apply requested color format for HDR session
    NV_DYNAMIC_RANGE         hdrDynamicRange;                                    //!< Optional, One of NV_DYNAMIC_RANGE enum values, if set it will apply requested dynamic range for HDR session
    NV_BPC                   hdrBpc;                                             //!< Optional, One of NV_BPC enum values, if set it will apply requested color depth
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
  if (MH_OK == MH_Initialize ())
  {
    CoInitializeEx (nullptr, COINIT_MULTITHREADED);

    using PFun_slUpgradeInterface   = sl::Result(                      void** baseInterface);
   static PFun_slUpgradeInterface * slUpgradeInterface =
         (PFun_slUpgradeInterface *) GetProcAddress (GetModuleHandleW (L"sl.interposer.dll"), "slUpgradeInterface");

    using PFun_slGetNativeInterface = sl::Result(void* proxyInterface, void** baseInterface);
   static PFun_slGetNativeInterface * slGetNativeInterface =
         (PFun_slGetNativeInterface *) GetProcAddress (GetModuleHandleW (L"sl.interposer.dll"), "slGetNativeInterface");

    CComPtr <IDXGIFactory>                         pFactory;
    CreateDXGIFactory (IID_IDXGIFactory, (void **)&pFactory.p);

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

    HWND hWnd =
      CreateWindow (
        L"static", L"HDR10", WS_VISIBLE | WS_POPUP        |
            WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN |
            WS_CLIPSIBLINGS, 0, 0,
                               2, 2,
                                0, 0, 0, 0
      );

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

      MH_ApplyQueued ();
    }
  }

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

      config.dwFillTheSwamp = 0x0;

#ifdef _M_IX86
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_4,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_3,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_2,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_1,   MAX_PATH);
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput9_1_0, MAX_PATH);
#else
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_4,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_3,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_2,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_1,   MAX_PATH);
      GetSystemDirectoryW      (config.wszPathToSystemXInput9_1_0, MAX_PATH);
#endif
      PathAppendW              (config.wszPathToSystemXInput1_4,   L"XInput1_4.dll");
      PathAppendW              (config.wszPathToSystemXInput1_3,   L"XInput1_3.dll");
      PathAppendW              (config.wszPathToSystemXInput1_2,   L"XInput1_2.dll");
      PathAppendW              (config.wszPathToSystemXInput1_1,   L"XInput1_1.dll");
      PathAppendW              (config.wszPathToSystemXInput9_1_0, L"XInput9_1_0.dll");

      HANDLE hThread =
          CreateThread ( nullptr, 0x0, FakeFullscreen_InitThread,
                         nullptr, 0x0, nullptr );

      WaitForSingleObject (hThread, 1500);

      CloseHandle (hThread);

      GetModuleHandleExW (GET_MODULE_HANDLE_EX_FLAG_PIN |
                          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)DllMain, &hModule);

      DisableThreadLibraryCalls (hModule);
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