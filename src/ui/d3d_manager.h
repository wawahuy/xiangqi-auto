#pragma once
#include <d3d11.h>

namespace UI {

class D3DManager {
private:
    static ID3D11Device* s_device;
    static ID3D11DeviceContext* s_context;
    static int s_referenceCount;

public:
    // Initializes the global D3D11 device and context if not already created.
    static bool Initialize();

    // Releases a reference and shuts down the global device/context when reference count reaches 0.
    static void Shutdown();

    // Retrieves the global D3D11 device.
    static ID3D11Device* GetDevice() { return s_device; }

    // Retrieves the global D3D11 device context.
    static ID3D11DeviceContext* GetContext() { return s_context; }
};

} // namespace UI
