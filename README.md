Disables Fullscreen Exclusive and translates NVAPI HDR to DXGI to allow games that need Fullscreen Exclusive for HDR to work in borderless window mode.

Move `XInput1_4.dll` to your game's directory; all Unreal Engine games use XInput1_3, others may use XInput9_1_0 or XInput1_4

The DLL can also be loaded using a standard asi loader if you name it `TinyFakeFullscreen.asi`
