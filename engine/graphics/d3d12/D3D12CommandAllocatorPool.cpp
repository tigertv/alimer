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

#include "D3D12CommandAllocatorPool.h"
#include <algorithm>

namespace alimer
{
    D3D12CommandAllocatorPool::D3D12CommandAllocatorPool(ID3D12Device* device_, D3D12_COMMAND_LIST_TYPE commandListType_)
        : device(device_)
        , commandListType(commandListType_)
    {

    }

    D3D12CommandAllocatorPool::~D3D12CommandAllocatorPool()
    {
        Destroy();
    }

    void D3D12CommandAllocatorPool::Destroy()
    {
        for (size_t i = 0; i < allocators.size(); ++i)
        {
            allocators[i]->Release();
        }

        allocators.clear();
    }

    ID3D12CommandAllocator* D3D12CommandAllocatorPool::RequestAllocator(uint64_t fenceValue)
    {
        std::lock_guard<std::mutex> LockGuard(allocatorMutex);

        ID3D12CommandAllocator* commandAllocator = nullptr;

        if (!freeAllocators.empty())
        {
            auto& allocatorPair = freeAllocators.front();

            if (allocatorPair.first <= fenceValue)
            {
                commandAllocator = allocatorPair.second;
                ThrowIfFailed(commandAllocator->Reset());
                freeAllocators.pop();
            }
        }

        // If no allocator's were ready to be reused, create a new one
        if (commandAllocator == nullptr)
        {
            ThrowIfFailed(device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator)));
#if defined(_DEBUG)
            wchar_t AllocatorName[32];
#if (EASTL_SIZE_T_32BIT == 0) || (EA_PLATFORM_WORD_SIZE == 4)
            swprintf(AllocatorName, 32, L"CommandAllocator %zu", allocators.size());
#else
            swprintf(AllocatorName, 32, L"CommandAllocator %u", allocators.size());
#endif
            commandAllocator->SetName(AllocatorName);
#endif

            allocators.push_back(commandAllocator);
        }

        return commandAllocator;
    }

    void D3D12CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator)
    {
        std::lock_guard<std::mutex> LockGuard(allocatorMutex);

        // That fence value indicates we are free to reset the allocator
        freeAllocators.push(std::make_pair(fenceValue, commandAllocator));
    }
}
