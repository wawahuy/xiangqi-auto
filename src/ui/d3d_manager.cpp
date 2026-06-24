#include "d3d_manager.h"

namespace UI {

ID3D11Device* D3DManager::s_device = nullptr;
ID3D11DeviceContext* D3DManager::s_context = nullptr;
int D3DManager::s_referenceCount = 0;

bool D3DManager::Initialize()
{
    if (s_referenceCount == 0)
    {
        UINT createDeviceFlags = 0;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        
        HRESULT res = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &s_device,
            &featureLevel,
            &s_context
        );

        // Fallback to WARP software driver if hardware creation fails
        if (res == DXGI_ERROR_UNSUPPORTED)
        {
            res = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP,
                nullptr,
                createDeviceFlags,
                featureLevelArray,
                2,
                D3D11_SDK_VERSION,
                &s_device,
                &featureLevel,
                &s_context
            );
        }

        if (res != S_OK)
        {
            return false;
        }
    }

    s_referenceCount++;
    return true;
}

void D3DManager::Shutdown()
{
    if (s_referenceCount > 0)
    {
        s_referenceCount--;
        if (s_referenceCount == 0)
        {
            if (s_context)
            {
                s_context->Release();
                s_context = nullptr;
            }
            if (s_device)
            {
                s_device->Release();
                s_device = nullptr;
            }
        }
    }
}

} // namespace UI
