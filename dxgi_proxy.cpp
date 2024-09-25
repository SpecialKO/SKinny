#include "pch.h"
#include "config.h"
#include <dxgi.h>

HRESULT
WINAPI
CreateDXGIFactory2 (UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory)
{
  using  CreateDXGIFactory2_pfn = HRESULT (WINAPI *)(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);
  static CreateDXGIFactory2_pfn
        _CreateDXGIFactory2 =
        (CreateDXGIFactory2_pfn)GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
        "CreateDXGIFactory2");

  return
    _CreateDXGIFactory2 (Flags, riid, ppFactory);
}

HRESULT
WINAPI
CreateDXGIFactory (REFIID riid, _COM_Outptr_ void **ppFactory)
{
  using  CreateDXGIFactory_pfn = HRESULT (WINAPI *)(REFIID riid, _COM_Outptr_ void **ppFactory);
  static CreateDXGIFactory_pfn
        _CreateDXGIFactory =
        (CreateDXGIFactory_pfn)GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
        "CreateDXGIFactory");

  // As-per Microsoft's docs, do not mix and match factory versions...
  //   just use Factory2 for everything!
  return
    CreateDXGIFactory2 (0x0, riid, ppFactory);
  //return
  //  _CreateDXGIFactory (riid, ppFactory);
}

HRESULT
WINAPI
CreateDXGIFactory1 (REFIID riid, _COM_Outptr_ void **ppFactory)
{
  using  CreateDXGIFactory1_pfn = HRESULT (WINAPI *)(REFIID riid, _COM_Outptr_ void **ppFactory);
  static CreateDXGIFactory1_pfn
        _CreateDXGIFactory1 =
        (CreateDXGIFactory1_pfn)GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
        "CreateDXGIFactory1");

  // As-per Microsoft's docs, do not mix and match factory versions...
  //   just use Factory2 for everything!
  return
    CreateDXGIFactory2 (0x0, riid, ppFactory);

  //return
  //  _CreateDXGIFactory1 (riid, ppFactory);
}

HRESULT
WINAPI
DXGIGetDebugInterface1 (UINT Flags, REFIID riid, _COM_Outptr_ void **pDebug)
{
  using  DXGIGetDebugInterface1_pfn = HRESULT (WINAPI *)(UINT Flags, REFIID riid, _COM_Outptr_ void **pDebug);
  static DXGIGetDebugInterface1_pfn
        _DXGIGetDebugInterface1 =
        (DXGIGetDebugInterface1_pfn)GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
        "DXGIGetDebugInterface1");

  return
    _DXGIGetDebugInterface1 (Flags, riid, pDebug);
}

HRESULT
WINAPI
DXGIDeclareAdapterRemovalSupport (void)
{
  using  DXGIDeclareAdapterRemovalSupport_pfn = HRESULT (WINAPI *)(void);
  static DXGIDeclareAdapterRemovalSupport_pfn
        _DXGIDeclareAdapterRemovalSupport =
        (DXGIDeclareAdapterRemovalSupport_pfn)GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
        "DXGIDeclareAdapterRemovalSupport");

  return
    _DXGIDeclareAdapterRemovalSupport ();
}

HRESULT
WINAPI
DXGIDisableVBlankVirtualization (void)
{
  using  DXGIDisableVBlankVirtualization_pfn = HRESULT (WINAPI *)(void);
  static DXGIDisableVBlankVirtualization_pfn
        _DXGIDisableVBlankVirtualization =
        (DXGIDisableVBlankVirtualization_pfn)GetProcAddress (LoadLibraryExW (config.wszPathToSystemDXGI, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32),
        "DXGIDisableVBlankVirtualization");

  return
    _DXGIDisableVBlankVirtualization ();
}