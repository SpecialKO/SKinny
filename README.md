## SKinny is a set of fixes derived from Special K that do not require Special K<br>

## StreamlineCondom
 + Reconfigures NVIDIA Streamline to output debug information if developers have turned it off
 + Reconfigures NVIDIA Streamline to use DXGI Factory Proxies instead of vftable hooks to improve DLSS3 Frame Generation compatibility

    > *Place `SKinny.dll` in the same directory as `sl.interposer.dll` and name it either `dxgi.dll` or `d3d12.dll`.*

<br>

## TinyFakeFullscreen
 + Disables Fullscreen Exclusive in D3D11 and D3D12 games and replaces it with Borderless Fullscreen
 + Translates NVAPI HDR to DXGI to enable native HDR in many older HDR games without using Fullscreen Exclusive

   > *Place `SKinny.dll` in the game's directory and name it `xinput1_4.dll`, `dxgi.dll` or `d3d12.dll`.*
   
*TinyFakeFullscreen can also be loaded by an ASI loader if named `SKFakeFullscreen.asi`.*
