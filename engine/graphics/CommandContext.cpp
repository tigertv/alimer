//
// Copyright (c) 2019-2020 Amer Koleci and contributors.
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

#include "graphics/CommandContext.h"
#include "graphics/GPUDevice.h"
#include "graphics/Texture.h"
#include "graphics/SwapChain.h"

namespace alimer
{
    /* CopyContext */
    CopyContext::CopyContext(GraphicsDevice& device_)
        : device(device_)
    {
    }

    CopyContext::~CopyContext()
    {
        //device.GetImpl()->DestroyCommandBuffer(handle);
    }

    void CopyContext::BeginMarker(const std::string& name)
    {

    }

    void CopyContext::EndMarker()
    {

    }

    void CopyContext::Flush(bool wait)
    {
        //handle->Flush(wait);
    }

    /* ComputeContext */
    ComputeContext::ComputeContext(GraphicsDevice& device_)
        : CopyContext(device_)
    {

    }

    /* GraphicsContext */
    GraphicsContext::GraphicsContext(GraphicsDevice& device_)
        : ComputeContext(device_)
    {
        //handle = device_.GetImpl()->CreateCommandBuffer(QueueType::Graphics);
    }

    void GraphicsContext::BeginRenderPass(SwapChain* swapchain, const Color& clearColor)
    {
        //handle->BeginRenderPass(swapchain->GetCurrentTexture()->GetHandle(), clearColor);
    }

    void GraphicsContext::EndRenderPass()
    {
        //handle->EndRenderPass();
    }
}
