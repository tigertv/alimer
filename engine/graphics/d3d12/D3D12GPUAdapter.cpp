//
// Copyright (c) 2020 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "D3D12GPUAdapter.h"
#include "D3D12GPUDevice.h"
#include "core/String.h"

namespace alimer
{
    D3D12GPUAdapter::D3D12GPUAdapter(ComPtr<IDXGIAdapter1> adapter)
        : GPUAdapter(BackendType::Direct3D12)
        , _adapter(adapter)
    {
        DXGI_ADAPTER_DESC1 desc;
        ThrowIfFailed(adapter->GetDesc1(&desc));

        _vendorId = desc.VendorId;
        _deviceId = desc.DeviceId;

        std::wstring deviceName(desc.Description);
        _name = alimer::ToUtf8(deviceName);

        // Detect adapter type.
        {
            ComPtr<ID3D12Device> tempDevice;
            ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(tempDevice.GetAddressOf())));
            D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
            ThrowIfFailed(tempDevice->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch)));

            _adapterType = arch.UMA ? GraphicsAdapterType::IntegratedGPU : GraphicsAdapterType::DiscreteGPU;
        }
    }
}
