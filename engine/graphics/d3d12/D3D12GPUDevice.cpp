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

#include "D3D12Backend.h"
#include "graphics/GPUDevice.h"
#include "D3D12MemAlloc.h"
#include "core/String.h"

namespace alimer
{
    static constexpr D3D_FEATURE_LEVEL s_minFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    static struct {
        UINT dxgiFactoryFlags = 0;
        IDXGIFactory4* dxgiFactory = nullptr;
        bool isTearingSupported = false;

        uint32_t deviceCount = 0;
    } d3d12;

    struct GPUDeviceApiData {
        ID3D12Device* d3dDevice = nullptr;
        D3D12MA::Allocator* allocator = nullptr;
        /// Current supported feature level.
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        /// Root signature version
        D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    };

    IDXGIFactory4* GetDXGIFactory()
    {
        return d3d12.dxgiFactory;
    }

    bool IsDXGITearingSupported()
    {
        return d3d12.isTearingSupported;
    }

    static bool CreateFactory(bool validation)
    {
#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        //
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        if (validation)
        {
            ID3D12Debug* debugController;

            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                ID3D12Debug1* debugController1;
                if (SUCCEEDED(debugController->QueryInterface(&debugController1)))
                {
                    const bool gpuValidation = false;
                    if (gpuValidation)
                    {
                        debugController1->SetEnableGPUBasedValidation(TRUE);
                        debugController1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                    }
                    else
                    {
                        debugController1->SetEnableGPUBasedValidation(FALSE);
                    }
                    debugController1->Release();
                }
            }
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }

            SAFE_RELEASE(debugController);

            IDXGIInfoQueue* dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
            {
                d3d12.dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
                {
                    80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                };
                DXGI_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
                dxgiInfoQueue->Release();
            }
        }
#endif
        ThrowIfFailed(CreateDXGIFactory2(d3d12.dxgiFactoryFlags, IID_PPV_ARGS(&d3d12.dxgiFactory)));

        BOOL allowTearing = FALSE;
        IDXGIFactory5* dxgiFactory5 = nullptr;
        HRESULT hr = d3d12.dxgiFactory->QueryInterface(&dxgiFactory5);
        if (SUCCEEDED(hr))
        {
            hr = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        if (FAILED(hr) || !allowTearing)
        {
            d3d12.isTearingSupported = false;
#ifdef _DEBUG
            OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
        }
        else
        {
            d3d12.isTearingSupported = true;
        }
        SAFE_RELEASE(dxgiFactory5);

        return true;
    }

    bool GPUDevice::ApiInit()
    {
        GPUDeviceApiData* pData = new GPUDeviceApiData;
        apiData = pData;

        if (d3d12.deviceCount == 0)
        {
            if (!CreateFactory(desc.validation))
            {
                return false;
            }
        }

        // Find adapter to use.
        IDXGIAdapter1* adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        IDXGIFactory6* factory6;
        HRESULT hr = d3d12.dxgiFactory->QueryInterface(&factory6);
        if (SUCCEEDED(hr))
        {
            DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
            if (desc.powerPreference == GPUPowerPreference::LowPower)
            {
                gpuPreference = DXGI_GPU_PREFERENCE_MINIMUM_POWER;
            }

            for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(i, gpuPreference, IID_PPV_ARGS(&adapter)); i++)
            {
                DXGI_ADAPTER_DESC1 desc;
                ThrowIfFailed(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    adapter->Release();

                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, s_minFeatureLevel, _uuidof(ID3D12Device), nullptr)))
                {
#ifdef _DEBUG
                    wchar_t buff[256] = {};
                    swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", i, desc.VendorId, desc.DeviceId, desc.Description);
                    OutputDebugStringW(buff);
#endif
                    break;
                }
            }
        }
        SAFE_RELEASE(factory6);
#endif

        if (!adapter)
        {
            for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != d3d12.dxgiFactory->EnumAdapters1(i, &adapter); i++)
            {
                DXGI_ADAPTER_DESC1 desc;
                ThrowIfFailed(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    adapter->Release();
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, s_minFeatureLevel, _uuidof(ID3D12Device), nullptr)))
                {
#ifdef _DEBUG
                    wchar_t buff[256] = {};
                    swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", i, desc.VendorId, desc.DeviceId, desc.Description);
                    OutputDebugStringW(buff);
#endif
                    break;
                }
            }
        }

#if !defined(NDEBUG)
        if (!adapter)
        {
            // Try WARP12 instead
            if (FAILED(d3d12.dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter))))
            {
                ALIMER_LOGERROR("WARP12 not available. Enable the 'Graphics Tools' optional feature");
                return false;
            }

            OutputDebugStringA("Direct3D Adapter - WARP12\n");
        }
#endif

        if (!adapter)
        {
            ALIMER_LOGERROR("No Direct3D 12 device found");
            return false;
        }

        // Create the DX12 API device object.
        ThrowIfFailed(D3D12CreateDevice(adapter, s_minFeatureLevel, IID_PPV_ARGS(&pData->d3dDevice)));

#ifndef NDEBUG
        if (desc.validation)
        {
            // Configure debug device (if active).
            ID3D12InfoQueue* d3dInfoQueue;
            if (SUCCEEDED(pData->d3dDevice->QueryInterface(&d3dInfoQueue)))
            {
#ifdef _DEBUG
                d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
                D3D12_MESSAGE_ID hide[] =
                {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
                    D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES
                };
                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                d3dInfoQueue->AddStorageFilterEntries(&filter);
                d3dInfoQueue->Release();
            }
        }
#endif
        // Create memory allocator
        {
            D3D12MA::ALLOCATOR_DESC desc = {};
            desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
            desc.pDevice = pData->d3dDevice;
            desc.pAdapter = adapter;

            ThrowIfFailed(D3D12MA::CreateAllocator(&desc, &pData->allocator));

            switch (pData->allocator->GetD3D12Options().ResourceHeapTier)
            {
            case D3D12_RESOURCE_HEAP_TIER_1:
                ALIMER_LOGDEBUG("ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_1");
                break;
            case D3D12_RESOURCE_HEAP_TIER_2:
                ALIMER_LOGDEBUG("ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_2");
                break;
            default:
                break;
            }
        }

        // Init capabilities
        {
            DXGI_ADAPTER_DESC1 desc;
            ThrowIfFailed(adapter->GetDesc1(&desc));

            caps.vendorId = desc.VendorId;
            caps.deviceId = desc.DeviceId;

            std::wstring deviceName(desc.Description);
            caps.adapterName = alimer::ToUtf8(deviceName);

            // Detect adapter type.

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                caps.adapterType = GPUAdapterType::CPU;
            }
            else {
                D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
                ThrowIfFailed(pData->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch)));

                caps.adapterType = arch.UMA ? GPUAdapterType::IntegratedGPU : GPUAdapterType::DiscreteGPU;
            }

            // Determine maximum supported feature level for this device
            static const D3D_FEATURE_LEVEL s_featureLevels[] =
            {
                D3D_FEATURE_LEVEL_12_1,
                D3D_FEATURE_LEVEL_12_0,
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
            };

            D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
            {
                _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
            };

            HRESULT hr = pData->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
            if (SUCCEEDED(hr))
            {
                pData->featureLevel = featLevels.MaxSupportedFeatureLevel;
            }
            else
            {
                pData->featureLevel = D3D_FEATURE_LEVEL_11_0;
            }

            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

            // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

            if (FAILED(pData->d3dDevice->CheckFeatureSupport(
                D3D12_FEATURE_ROOT_SIGNATURE,
                &featureData, sizeof(featureData))))
            {
                pData->rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }

            // Features
            caps.features.independentBlend = true;
            caps.features.computeShader = true;
            caps.features.geometryShader = true;
            caps.features.tessellationShader = true;
            caps.features.logicOp = true;
            caps.features.multiViewport = true;
            caps.features.fullDrawIndexUint32 = true;
            caps.features.multiDrawIndirect = true;
            caps.features.fillModeNonSolid = true;
            caps.features.samplerAnisotropy = true;
            caps.features.textureCompressionETC2 = false;
            caps.features.textureCompressionASTC_LDR = false;
            caps.features.textureCompressionBC = true;
            caps.features.textureCubeArray = true;

            D3D12_FEATURE_DATA_D3D12_OPTIONS5 d3d12options5 = {};
            if (SUCCEEDED(pData->d3dDevice->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS5,
                &d3d12options5, sizeof(d3d12options5)))
                && d3d12options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
            {
                caps.features.raytracing = true;
            }
            else
            {
                caps.features.raytracing = false;
            }

            // Limits
            caps.limits.maxVertexAttributes = kMaxVertexAttributes;
            caps.limits.maxVertexBindings = kMaxVertexAttributes;
            caps.limits.maxVertexAttributeOffset = kMaxVertexAttributeOffset;
            caps.limits.maxVertexBindingStride = kMaxVertexBufferStride;

            //caps.limits.maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
            caps.limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            caps.limits.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            caps.limits.maxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
            caps.limits.maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            caps.limits.maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
            caps.limits.maxUniformBufferSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            caps.limits.minUniformBufferOffsetAlignment = 256u;
            caps.limits.maxStorageBufferSize = UINT32_MAX;
            caps.limits.minStorageBufferOffsetAlignment = 16;
            caps.limits.maxSamplerAnisotropy = D3D12_MAX_MAXANISOTROPY;
            caps.limits.maxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
            caps.limits.maxViewportWidth = D3D12_VIEWPORT_BOUNDS_MAX;
            caps.limits.maxViewportHeight = D3D12_VIEWPORT_BOUNDS_MAX;
            caps.limits.maxTessellationPatchSize = D3D12_IA_PATCH_MAX_CONTROL_POINT_COUNT;
            caps.limits.pointSizeRangeMin = 1.0f;
            caps.limits.pointSizeRangeMax = 1.0f;
            caps.limits.lineWidthRangeMin = 1.0f;
            caps.limits.lineWidthRangeMax = 1.0f;
            caps.limits.maxComputeSharedMemorySize = D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL;
            caps.limits.maxComputeWorkGroupCountX = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
            caps.limits.maxComputeWorkGroupCountY = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
            caps.limits.maxComputeWorkGroupCountZ = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
            caps.limits.maxComputeWorkGroupInvocations = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
            caps.limits.maxComputeWorkGroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X;
            caps.limits.maxComputeWorkGroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y;
            caps.limits.maxComputeWorkGroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z;

            /* see: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_format_support */
            /*UINT dxgi_fmt_caps = 0;
            for (int fmt = (VGPUTextureFormat_Undefined + 1); fmt < VGPUTextureFormat_Count; fmt++)
            {
                DXGI_FORMAT dxgi_fmt = _vgpu_d3d_get_format((VGPUTextureFormat)fmt);
                HRESULT hr = ID3D11Device1_CheckFormatSupport(renderer->d3d_device, dxgi_fmt, &dxgi_fmt_caps);
                VGPU_ASSERT(SUCCEEDED(hr));
                sg_pixelformat_info* info = &_sg.formats[fmt];
                info->sample = 0 != (dxgi_fmt_caps & D3D11_FORMAT_SUPPORT_TEXTURE2D);
                info->filter = 0 != (dxgi_fmt_caps & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
                info->render = 0 != (dxgi_fmt_caps & D3D11_FORMAT_SUPPORT_RENDER_TARGET);
                info->blend = 0 != (dxgi_fmt_caps & D3D11_FORMAT_SUPPORT_BLENDABLE);
                info->msaa = 0 != (dxgi_fmt_caps & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET);
                info->depth = 0 != (dxgi_fmt_caps & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL);
                if (info->depth) {
                    info->render = true;
                }
            }*/
        }

        SAFE_RELEASE(adapter);

        d3d12.deviceCount++;

        return true;
    }

    void GPUDevice::ApiDestroy()
    {
        // Allocator
        if (apiData != nullptr)
        {
            D3D12MA::Stats stats;
            apiData->allocator->CalculateStats(&stats);

            if (stats.Total.UsedBytes > 0) {
                ALIMER_LOGE("Total device memory leaked: %llu bytes.", stats.Total.UsedBytes);
            }

            apiData->allocator->Release();
            apiData->allocator = nullptr;
        }

#if !defined(NDEBUG)
        ULONG refCount = apiData->d3dDevice->Release();
        if (refCount > 0)
        {
            ALIMER_LOGD("Direct3D12: There are %d unreleased references left on the device", refCount);

            ID3D12DebugDevice* debugDevice;
            if (SUCCEEDED(apiData->d3dDevice->QueryInterface(IID_PPV_ARGS(&debugDevice))))
            {
                debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_SUMMARY | D3D12_RLDO_IGNORE_INTERNAL);
                debugDevice->Release();
            }
        }
#else
        d3dDevice.Reset();
#endif

        SafeDelete(apiData);

        d3d12.deviceCount--;

        if (d3d12.deviceCount == 0)
        {
            SAFE_RELEASE(d3d12.dxgiFactory);

#ifdef _DEBUG
            IDXGIDebug1* dxgiDebug;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
                dxgiDebug->Release();
            }
#endif
        }
    }

    void GPUDevice::WaitForIdle()
    {

    }

    DeviceHandle GPUDevice::GetHandle() const
    {
        return apiData->d3dDevice;
    }
}
