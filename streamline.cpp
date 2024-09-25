#include "pch.h"
#include "streamline.h"

using  slInit_pfn = sl::Result (*)(const sl::Preferences &pref, uint64_t sdkVersion);
static slInit_pfn
       slInit_Original = nullptr;

sl::Result
slInit_Detour (const sl::Preferences &pref, uint64_t sdkVersion = sl::kSDKVersion)
{
  //SK_LOGi0 (L"[!] slInit (pref.structVersion=%d, sdkVersion=%d)",
  //                        pref.structVersion,    sdkVersion);

  // For versions of Streamline using a compatible preferences struct,
  //   start overriding stuff for compatibility.
  if (pref.structVersion == sl::kStructVersion1)
  {
    sl::Preferences pref_copy = pref;

    //
    // Always print Streamline debug output to Special K's game_output.log,
    //   disable any log redirection the game would ordinarily do on its own.
    //
    pref_copy.logMessageCallback = nullptr;
#ifdef _DEBUG
    pref_copy.logLevel           = sl::LogLevel::eVerbose;
#else
    pref_copy.logLevel           = sl::LogLevel::eDefault;
#endif

    pref_copy.flags |=
      sl::PreferenceFlags::eUseDXGIFactoryProxy;

    return
      slInit_Original (pref_copy, sdkVersion);
  }

  //SK_LOGi0 (
  //  L"WARNING: Game is using a version of Streamline too new for Special K!"
  //);

  return
    slInit_Original (pref, sdkVersion);
}

#include <tuple>

using  slIsFeatureSupported_pfn = sl::Result (*)(sl::Feature feature, const sl::AdapterInfo& adapterInfo);
static slIsFeatureSupported_pfn
       slIsFeatureSupported_Original = nullptr;

sl::Result
slIsFeatureSupported_Detour (sl::Feature feature, const sl::AdapterInfo& adapterInfo)
{
  std::ignore = feature;
  std::ignore = adapterInfo;

  return sl::Result::eOk;
}

bool
SK_COMPAT_CheckStreamlineSupport (void)
{
  static bool once = false;

  if (! std::exchange (once, true))
  {
    if (LoadLibraryW (L"sl.interposer.dll"))
    {
      SK_CreateDLLHook2 (      L"sl.interposer.dll",
                                "slInit",
                                 slInit_Detour,
                      (void **)(&slInit_Original) );

      SK_CreateDLLHook2 (      L"sl.interposer.dll",
                                "slIsFeatureSupported",
                                 slIsFeatureSupported_Detour,
                      (void **)(&slIsFeatureSupported_Original) );

      MH_ApplyQueued ();
    }
  }

  return once;
}

// It is never necessary to call this, it can be implemented using QueryInterface
using slGetNativeInterface_pfn = sl::Result (*)(void* proxyInterface, void** baseInterface);
// This, on the other hand, requires an import from sl.interposer.dll
using slUpgradeInterface_pfn   = sl::Result (*)(                      void** baseInterface);

struct DECLSPEC_UUID ("ADEC44E2-61F0-45C3-AD9F-1B37379284FF")
  IStreamlineBaseInterface : IUnknown { };

sl::Result
SK_slGetNativeInterface (void *proxyInterface, void **baseInterface)
{
  if (proxyInterface == nullptr || baseInterface == nullptr)
    return sl::Result::eErrorMissingInputParameter;

  IUnknown* pUnk =
    static_cast <IUnknown *> (proxyInterface);

  if (FAILED (pUnk->QueryInterface (__uuidof (IStreamlineBaseInterface), baseInterface)))
    return sl::Result::eErrorUnsupportedInterface;

  return sl::Result::eOk;
}

sl::Result
SK_slUpgradeInterface (void **baseInterface)
{
  if (baseInterface == nullptr)
    return sl::Result::eErrorMissingInputParameter;

  IUnknown* pUnkInterface = *(IUnknown **)baseInterface;
  void*     pProxy        = nullptr;

  if (SUCCEEDED (pUnkInterface->QueryInterface (__uuidof (IStreamlineBaseInterface), &pProxy)))
  {
    // The passed interface already has a proxy, do nothing...
    static_cast <IUnknown *> (pProxy)->Release ();

    return sl::Result::eOk;
  }

  // If slInit (...) has not been called yet, sl.common.dll will not be present 
  if (! GetModuleHandleW (L"sl.common.dll"))
    return sl::Result::eErrorInitNotCalled;

  auto slUpgradeInterface =
      (slUpgradeInterface_pfn)GetProcAddress (GetModuleHandleW (L"sl.interposer.dll"),
      "slUpgradeInterface");

  if (slUpgradeInterface != nullptr)
  {
    auto result =
      slUpgradeInterface (baseInterface);

    if (result == sl::Result::eOk)
    {
      static HMODULE
          hModPinnedInterposer  = nullptr,   hModPinnedCommon  = nullptr;
      if (hModPinnedInterposer == nullptr || hModPinnedCommon == nullptr)
      {
        // Once we have done this, there's no going back...
        //   we MUST pin the interposer DLL permanently.
        const BOOL bPinnedSL =
           (nullptr != hModPinnedInterposer || GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_PIN, L"sl.interposer.dll", &hModPinnedInterposer))
        && (nullptr != hModPinnedCommon     || GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_PIN,     L"sl.common.dll", &hModPinnedCommon    ));

        //if (! bPinnedSL)
        //  SK_LOGi0 (L"Streamline Integration Has Invalid State!");
      }
    }

    return result;
  }

  return sl::Result::eErrorNotInitialized;
}