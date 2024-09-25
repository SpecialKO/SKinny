#include "pch.h"
#include "config.h"
#include <d3d12.h>

HRESULT
WINAPI
D3D12CreateDevice (
  _In_opt_ IUnknown* pAdapter,
  D3D_FEATURE_LEVEL MinimumFeatureLevel,
  _In_ REFIID riid, // Expected: ID3D12Device
  _COM_Outptr_opt_ void** ppDevice )
{
  static PFN_D3D12_CREATE_DEVICE _D3D12CreateDevice =
        (PFN_D3D12_CREATE_DEVICE)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
              "D3D12CreateDevice");

  return
    _D3D12CreateDevice (pAdapter, MinimumFeatureLevel, riid, ppDevice);
}

HRESULT
WINAPI
D3D12GetDebugInterface (
  _In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug )
{
  static PFN_D3D12_GET_DEBUG_INTERFACE _D3D12GetDebugInterface =
        (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
               "D3D12GetDebugInterface");

  return
    _D3D12GetDebugInterface (riid, ppvDebug);
}

HRESULT
WINAPI
D3D12CreateRootSignatureDeserializer (
  _In_reads_bytes_(SrcDataSizeInBytes) LPCVOID pSrcData,
  _In_ SIZE_T SrcDataSizeInBytes,
  _In_ REFIID pRootSignatureDeserializerInterface,
  _Out_ void** ppRootSignatureDeserializer )
{
  static PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER _D3D12CreateRootSignatureDeserializer =
        (PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
                "D3D12CreateRootSignatureDeserializer");

  return
    _D3D12CreateRootSignatureDeserializer (pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}

HRESULT
WINAPI
D3D12CreateVersionedRootSignatureDeserializer (
  _In_reads_bytes_(SrcDataSizeInBytes) LPCVOID pSrcData,
  _In_ SIZE_T SrcDataSizeInBytes,
  _In_ REFIID pRootSignatureDeserializerInterface,
  _Out_ void** ppRootSignatureDeserializer )
{  
  static PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER _D3D12CreateVersionedRootSignatureDeserializer =
        (PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
                "D3D12CreateVersionedRootSignatureDeserializer");
  return
    _D3D12CreateVersionedRootSignatureDeserializer (pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}

HRESULT
WINAPI
D3D12GetInterface ( _In_ REFCLSID rclsid, _In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug )
{
  static PFN_D3D12_GET_INTERFACE _D3D12GetInterface =
        (PFN_D3D12_GET_INTERFACE)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
                "D3D12GetInterface");
  return
    _D3D12GetInterface (rclsid, riid, ppvDebug);
}

HRESULT
WINAPI
D3D12SerializeRootSignature (
  _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
  _In_ D3D_ROOT_SIGNATURE_VERSION Version,
  _Out_ ID3DBlob** ppBlob,
  _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob )
{
  static PFN_D3D12_SERIALIZE_ROOT_SIGNATURE _D3D12SerializeRootSignature =
        (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
                "D3D12SerializeRootSignature");
  return
    _D3D12SerializeRootSignature (pRootSignature, Version, ppBlob, ppErrorBlob);
}

HRESULT
WINAPI
D3D12SerializeVersionedRootSignature (
  _In_      const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *pRootSignature,
  _Out_     ID3DBlob                                  **ppBlob,
  _Out_opt_ ID3DBlob                                  **ppErrorBlob )
{
  static PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE _D3D12SerializeVersionedRootSignature =
        (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress (LoadLibraryExW (config.wszPathToSystemD3D12, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
                "D3D12SerializeVersionedRootSignature");
  return
    _D3D12SerializeVersionedRootSignature (pRootSignature, ppBlob, ppErrorBlob);
}

// D3D12DeviceRemovedExtendedData       ???
// D3D12EnableExperimentalFeatures      ???