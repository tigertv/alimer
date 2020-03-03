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

#pragma once

#include "Graphics/GraphicsDevice.h"
#include "D3D12Backend.h"
#include <D3D12MemAlloc.h>

namespace Alimer
{
    class D3D12CommandQueue;

    /// Direct3D12 graphics backend.
    class ALIMER_API D3D12GraphicsDevice final : public GraphicsDevice
    {
    public:
        static bool IsAvailable();

        /// Constructor.
        D3D12GraphicsDevice(const GraphicsDeviceDescriptor* descriptor);
        /// Destructor.
        ~D3D12GraphicsDevice() override;

        void Destroy();
        void WaitIdle() override;
        bool BeginFrame() override;
        void EndFrame() override;

        SwapChain* CreateSwapChainCore(void* nativeHandle, const SwapChainDescriptor* descriptor) override;

        IDXGIFactory4*          GetDXGIFactory() const { return dxgiFactory.Get(); }
        ID3D12Device*           GetD3DDevice() const { return d3dDevice; }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return d3dFeatureLevel; }
        bool                    IsTearingSupported() const { return isTearingSupported; }

        D3D12CommandQueue* GetGraphicsQueue(void) { return graphicsQueue; }
        D3D12CommandQueue* GetComputeQueue(void) { return computeQueue; }
        D3D12CommandQueue* GetCopyQueue(void) { return copyQueue; }

        D3D12CommandQueue* GetQueue(CommandQueueType queueType = CommandQueueType::Graphics) const;
        ID3D12CommandQueue* GetD3DCommandQueue(CommandQueueType queueType = CommandQueueType::Graphics) const;

    private:
        static constexpr D3D_FEATURE_LEVEL d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;

        static bool GetAdapter(ComPtr<IDXGIFactory4> factory4, IDXGIAdapter1** ppAdapter);
        void CreateDeviceResources();
        void InitCapabilities(IDXGIAdapter1* adapter);

        UINT dxgiFactoryFlags = 0;
        ComPtr<IDXGIFactory4> dxgiFactory;
        bool isTearingSupported = false;
        ID3D12Device* d3dDevice = nullptr;
        D3D_FEATURE_LEVEL d3dFeatureLevel = D3D_FEATURE_LEVEL_9_1;
        D3D12MA::Allocator* allocator = nullptr;

        D3D12CommandQueue* graphicsQueue;
        D3D12CommandQueue* computeQueue;
        D3D12CommandQueue* copyQueue;
    };
}
