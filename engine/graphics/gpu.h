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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum {
    GPU_MAX_COLOR_ATTACHMENTS = 8u,
    GPU_MAX_VERTEX_BUFFER_BINDINGS = 8u,
    GPU_MAX_VERTEX_ATTRIBUTES = 16u,
    GPU_MAX_VERTEX_ATTRIBUTE_OFFSET = 2047u,
    GPU_MAX_VERTEX_BUFFER_STRIDE = 2048u,
    // Maximum object that can be created.
    GPU_MAX_COMMAND_BUFFERS = 16u,
    GPU_MAX_TEXTURES = 4096u,
    GPU_MAX_BUFFERS = 4096u,
    GPU_MAX_SAMPLERS = 4096u,
    GPU_MAX_SHADERS = 512u,
    GPU_MAX_FRAMEBUFFERS = 256u,
    GPU_MAX_PIPELINES = 256u,
};

/// Defines pixel format.
typedef enum agpu_pixel_format {
    AGPU_PIXEL_FORMAT_UNDEFINED = 0,
    // 8-bit pixel formats
    AGPU_PIXEL_FORMAT_R8_UNORM,
    AGPU_PIXEL_FORMAT_R8_SNORM,
    AGPU_PIXEL_FORMAT_R8_UINT,
    AGPU_PIXEL_FORMAT_R8_SINT,

    // 16-bit pixel formats
    AGPU_PIXEL_FORMAT_R16_UNORM,
    AGPU_PIXEL_FORMAT_R16_SNORM,
    AGPU_PIXEL_FORMAT_R16_UINT,
    AGPU_PIXEL_FORMAT_R16_SINT,
    AGPU_PIXEL_FORMAT_R16_FLOAT,
    AGPU_PIXEL_FORMAT_RG8_UNORM,
    AGPU_PIXEL_FORMAT_RG8_SNORM,
    AGPU_PIXEL_FORMAT_RG8_UINT,
    AGPU_PIXEL_FORMAT_RG8_SINT,

    // 32-bit pixel formats
    AGPU_PIXEL_FORMAT_R32_UINT,
    AGPU_PIXEL_FORMAT_R32_SINT,
    AGPU_PIXEL_FORMAT_R32_FLOAT,
    AGPU_PIXEL_FORMAT_RG16_UNORM,
    AGPU_PIXEL_FORMAT_RG16_SNORM,
    AGPU_PIXEL_FORMAT_RG16_UINT,
    AGPU_PIXEL_FORMAT_RG16_SINT,
    AGPU_PIXEL_FORMAT_RG16_FLOAT,
    AGPU_PIXEL_FORMAT_RGBA8_UNORM,
    AGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB,
    AGPU_PIXEL_FORMAT_RGBA8_SNORM,
    AGPU_PIXEL_FORMAT_RGBA8_UINT,
    AGPU_PIXEL_FORMAT_RGBA8_SINT,
    AGPU_PIXEL_FORMAT_BGRA8_UNORM,
    AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB,

    // Packed 32-Bit Pixel formats
    AGPU_PIXEL_FORMAT_RGB10A2_UNORM,
    AGPU_PIXEL_FORMAT_RG11B10_FLOAT,

    // 64-Bit Pixel Formats
    AGPU_PIXEL_FORMAT_RG32_UINT,
    AGPU_PIXEL_FORMAT_RG32_SINT,
    AGPU_PIXEL_FORMAT_RG32_FLOAT,
    AGPU_PIXEL_FORMAT_RGBA16_UNORM,
    AGPU_PIXEL_FORMAT_RGBA16_SNORM,
    AGPU_PIXEL_FORMAT_RGBA16_UINT,
    AGPU_PIXEL_FORMAT_RGBA16_SINT,
    AGPU_PIXEL_FORMAT_RGBA16_FLOAT,

    // 128-Bit Pixel Formats
    AGPU_PIXEL_FORMAT_RGBA32_UINT,
    AGPU_PIXEL_FORMAT_RGBA32_SINT,
    AGPU_PIXEL_FORMAT_RGBA32_FLOAT,

    // Depth-stencil
    AGPU_PIXEL_FORMAT_D16_UNORM,
    AGPU_PIXEL_FORMAT_D32_FLOAT,
    AGPU_PIXEL_FORMAT_D24_UNORM_S8_UINT,
    AGPU_PIXEL_FORMAT_D32_FLOAT_S8_UINT,

    // Compressed formats
    AGPU_PIXEL_FORMAT_BC1_RGBA_UNORM,
    AGPU_PIXEL_FORMAT_BC1_RGBA_UNORM_SRGB,
    AGPU_PIXEL_FORMAT_BC2_RGBA_UNORM,
    AGPU_PIXEL_FORMAT_BC2_RGBA_UNORM_SRGB,
    AGPU_PIXEL_FORMAT_BC3_RGBA_UNORM,
    AGPU_PIXEL_FORMAT_BC3_RGBA_UNORM_SRGB,
    AGPU_PIXEL_FORMAT_BC4_R_UNORM,
    AGPU_PIXEL_FORMAT_BC4_R_SNORM,
    AGPU_PIXEL_FORMAT_BC5_RG_UNORM,
    AGPU_PIXEL_FORMAT_BC5_RG_SNORM,
    AGPU_PIXEL_FORMAT_BC6H_RGB_FLOAT,
    AGPU_PIXEL_FORMAT_BC6H_RGB_UFLOAT,
    AGPU_PIXEL_FORMAT_BC7_RGBA_UNORM,
    AGPU_PIXEL_FORMAT_BC7_RGBA_UNORM_SRGB,

    // Compressed PVRTC Pixel Formats
    AGPU_PIXEL_FORMAT_PVRTC_RGB2,
    AGPU_PIXEL_FORMAT_PVRTC_RGBA2,
    AGPU_PIXEL_FORMAT_PVRTC_RGB4,
    AGPU_PIXEL_FORMAT_PVRTC_RGBA4,

    // Compressed ETC Pixel Formats
    AGPU_PIXEL_FORMAT_ETC2_RGB8,
    AGPU_PIXEL_FORMAT_ETC2_RGB8_SRGB,
    AGPU_PIXEL_FORMAT_ETC2_RGB8A1,
    AGPU_PIXEL_FORMAT_ETC2_RGB8A1_SRGB,

    // Compressed ASTC Pixel Formats
    AGPU_PIXEL_FORMAT_ASTC4x4,
    AGPU_PIXEL_FORMAT_ASTC5x5,
    AGPU_PIXEL_FORMAT_ASTC6x6,
    AGPU_PIXEL_FORMAT_ASTC8x5,
    AGPU_PIXEL_FORMAT_ASTC8x6,
    AGPU_PIXEL_FORMAT_ASTC8x8,
    AGPU_PIXEL_FORMAT_ASTC10x10,
    AGPU_PIXEL_FORMAT_ASTC12x12,
    /// Pixel format count.
    AGPU_PIXEL_FORMAT_COUNT
} agpu_pixel_format;

typedef struct {
    /// Native display type.
    void* native_display;
    /// Native window handle.
    void* native_handle;

    uint32_t width;
    uint32_t height;

    agpu_pixel_format   color_format;
    agpu_pixel_format   depth_stencil_format;
} gpu_swapchain_desc;

typedef struct {
    bool validation;
    void (*callback)(void* context, const char* message, int level);
    void* context;
    const gpu_swapchain_desc* swapchain;
} gpu_config;

bool gpu_init(const gpu_config* config);
void gpu_shutdown();