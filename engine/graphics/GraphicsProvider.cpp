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

#include "config.h"
#include "core/Log.h"
#include "core/Assert.h"
#include "graphics/GraphicsProvider.h"

#if defined(ALIMER_VULKAN)
#include "graphics/vulkan/VulkanGraphicsProvider.h"
#endif

#if defined(ALIMER_D3D12)
#include "graphics/d3d12/D3D12GraphicsProvider.h"
#endif

#if defined(ALIMER_D3D11)
#include "graphics/d3d11/D3D11GPUDevice.h"
#endif

#if defined(ALIMER_OPENGL)
#   include "graphics/opengl/GLGPUDevice.h"
#endif

namespace alimer
{
    std::set<BackendType> GraphicsProvider::GetAvailableProviders()
    {
        static std::set<BackendType> availableProviders;

        if (availableProviders.empty())
        {
            availableProviders.insert(BackendType::Null);

#if defined(ALIMER_VULKAN)
            if (VulkanGraphicsProvider::IsAvailable())
                availableProviders.insert(BackendType::Vulkan);
#endif

#if defined(ALIMER_D3D12)
            if (D3D12GraphicsProvider::IsAvailable())
                availableProviders.insert(BackendType::Direct3D12);
#endif

#if defined(ALIMER_D3D11)
            if (D3D11GPUDevice::IsAvailable())
                availableProviders.insert(BackendType::Direct3D11);
#endif

#if defined(ALIMER_OPENGL)
            availableProviders.insert(BackendType::OpenGL);
#endif
        }

        return availableProviders;
    }

    std::unique_ptr<GraphicsProvider> GraphicsProvider::Create(const std::string& applicationName, GraphicsProviderFlags flags, BackendType preferredBackend)
    {
        BackendType backend = preferredBackend;
        if (preferredBackend == BackendType::Count)
        {
            auto availableBackends = GetAvailableProviders();

            if (availableBackends.find(BackendType::Metal) != availableBackends.end())
                backend = BackendType::Metal;
            else if (availableBackends.find(BackendType::Direct3D12) != availableBackends.end())
                backend = BackendType::Direct3D12;
            else if (availableBackends.find(BackendType::Vulkan) != availableBackends.end())
                backend = BackendType::Vulkan;
            else if (availableBackends.find(BackendType::Direct3D11) != availableBackends.end())
                backend = BackendType::Direct3D11;
            else if (availableBackends.find(BackendType::OpenGL) != availableBackends.end())
                backend = BackendType::OpenGL;
            else
                backend = BackendType::Null;
        }

        std::unique_ptr<GraphicsProvider> provider = nullptr;
        switch (backend)
        {
#if defined(ALIMER_VULKAN)
        case BackendType::Vulkan:
            ALIMER_LOGINFO("Using Vulkan render driver");
            provider = std::make_unique<VulkanGraphicsProvider>(applicationName, flags);
            break;
#endif
#if defined(ALIMER_D3D12)
        case BackendType::Direct3D12:
            ALIMER_LOGINFO("Using Direct3D12 render driver");
            provider = std::make_unique<D3D12GraphicsProvider>(flags);
            break;
#endif

#if defined(ALIMER_D3D11)
        case BackendType::Direct3D11:
            ALIMER_LOGINFO("Using Direct3D11 render driver");
            gpuDevice = make_shared<D3D11GPUDevice>(window, desc);
            break;
#endif

#if defined(ALIMER_OPENGL)
        case BackendType::OpenGL:
            ALIMER_LOGINFO("Using OpenGL render driver");
            provider = make_shared<GLGPUDevice>(window, desc);
            break;
#endif

        case BackendType::Metal:
            break;
        default:
            /* TODO: create null backend. */
            break;
        }

        return provider;
    }
}