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

#include "graphics/PixelFormat.h"

namespace alimer
{
    const PixelFormatDesc FormatDesc[] =
    {
        // format                               name,                       type                            bpp         compression             bits
        { PixelFormat::Undefined,               "Undefined",                PixelFormatType::Unknown,       0,          {0, 0, 0, 0, 0},        {0, 0, 0, 0, 0, 0}},

        // 8-bit pixel formats
        { PixelFormat::R8UNorm,                 "R8",                       PixelFormatType::UNorm,         8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},
        { PixelFormat::R8SNorm,                 "R8S",                      PixelFormatType::SNorm,         8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},
        { PixelFormat::R8UInt,                  "R8U",                      PixelFormatType::UInt,          8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},
        { PixelFormat::R8SInt,                  "R8I",                      PixelFormatType::SInt,          8,          {1, 1, 1, 1, 1},        {0, 0, 8, 0, 0, 0}},

        // 16-bit pixel formats
        { PixelFormat::R16UNorm,                "R16",                      PixelFormatType::UNorm,         16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
        { PixelFormat::R16SNorm,                "R16S",                     PixelFormatType::SNorm,         16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
        { PixelFormat::R16UInt,                 "R16U",                     PixelFormatType::UInt,          16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
        { PixelFormat::R16SInt,                 "R16I",                     PixelFormatType::SInt,          16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
        { PixelFormat::R16Float,                "R16F",                     PixelFormatType::Float,         16,         {1, 1, 2, 1, 1},        {0, 0, 16, 0, 0, 0}},
        { PixelFormat::RG8UNorm,                "RG8",                      PixelFormatType::UNorm,         16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},
        { PixelFormat::RG8SNorm,                "RG8S",                     PixelFormatType::SNorm,         16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},
        { PixelFormat::RG8UInt,                 "RG8U",                     PixelFormatType::UInt,          16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},
        { PixelFormat::RG8SInt,                 "RG8I",                     PixelFormatType::SInt,          16,         {1, 1, 2, 1, 1},        {0, 0, 8, 8, 0, 0}},

        // 32-bit pixel formats
        { PixelFormat::R32UInt,                 "R32U",                     PixelFormatType::UInt,          32,         {1, 1, 4, 1, 1},        {0, 0, 32, 0, 0, 0}},
        { PixelFormat::R32SInt,                 "R32I",                     PixelFormatType::SInt,          32,         {1, 1, 4, 1, 1},        {0, 0, 32, 0, 0, 0}},
        { PixelFormat::R32Float,                "R32F",                     PixelFormatType::Float,         32,         {1, 1, 4, 1, 1},        {0, 0, 32, 0, 0, 0}},
        { PixelFormat::RG16UNorm,               "RG16",                     PixelFormatType::UNorm,         32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
        { PixelFormat::RG16SNorm,               "RG16S",                    PixelFormatType::SNorm,         32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
        { PixelFormat::RG16UInt,                "RG16U",                    PixelFormatType::UInt,          32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
        { PixelFormat::RG16SInt,                "RG16I",                    PixelFormatType::SInt,          32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
        { PixelFormat::RG16Float,               "RG16F",                    PixelFormatType::Float,         32,         {1, 1, 4, 1, 1},        {0, 0, 16, 16, 0, 0}},
        { PixelFormat::RGBA8UNorm,              "RGBA8",                    PixelFormatType::UNorm,         32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
        { PixelFormat::RGBA8UNormSrgb,          "RGBA8Srgb",                PixelFormatType::UNormSrgb,     32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
        { PixelFormat::RGBA8SNorm,              "RGBA8S",                   PixelFormatType::SNorm,         32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
        { PixelFormat::RGBA8UInt,               "RGBA8U",                   PixelFormatType::UInt,          32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
        { PixelFormat::RGBA8SInt,               "RGBA8I",                   PixelFormatType::SInt,          32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
        { PixelFormat::BGRA8UNorm,              "BGRA8",                    PixelFormatType::UNorm,         32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},
        { PixelFormat::BGRA8UNormSrgb,          "BGRA8UNormSrgb",           PixelFormatType::UNormSrgb,     32,         {1, 1, 4, 1, 1},        {0, 0, 8, 8, 8, 8}},

        // Packed 32-Bit Pixel formats
        { PixelFormat::RGB10A2UNorm,            "RGB10A2UNorm",             PixelFormatType::UNorm,         32,         {1, 1, 4, 1, 1},        {0, 0, 10, 10, 10, 2}},
        { PixelFormat::RG11B10Float,            "RG11B10Float",             PixelFormatType::Float,         32,         {1, 1, 4, 1, 1},        {0, 0, 11, 11, 10, 0}},

        // 64-Bit Pixel Formats
        { PixelFormat::RG32UInt,                "RG32U",                    PixelFormatType::UInt,          64,         {1, 1, 8, 1, 1},        {0, 0, 32, 32, 0, 0}},
        { PixelFormat::RG32SInt,                "RG3I",                     PixelFormatType::SInt,          64,         {1, 1, 8, 1, 1},        {0, 0, 32, 32, 0, 0}},
        { PixelFormat::RG32Float,               "RG3F",                     PixelFormatType::Float,         64,         {1, 1, 8, 1, 1},        {0, 0, 32, 32, 0, 0}},
        { PixelFormat::RGBA16UNorm,             "RGBA16",                   PixelFormatType::UNorm,         64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
        { PixelFormat::RGBA16SNorm,             "RGBA16S",                  PixelFormatType::SNorm,         64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
        { PixelFormat::RGBA16UInt,              "RGBA16U",                  PixelFormatType::UInt,          64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
        { PixelFormat::RGBA16SInt,              "RGBA16S",                  PixelFormatType::SInt,          64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},
        { PixelFormat::RGBA16Float,             "RGBA16Float",              PixelFormatType::Float,         64,         {1, 1, 8, 1, 1},        {0, 0, 16, 16, 16, 16}},

        // 128-Bit Pixel Formats
        { PixelFormat::RGBA32UInt,              "RGBA32U",                  PixelFormatType::UInt,          128,        {1, 1, 16, 1, 1},       {0, 0, 32, 32, 32, 32}},
        { PixelFormat::RGBA32SInt,              "RGBA32S",                  PixelFormatType::SInt,          128,        {1, 1, 16, 1, 1},       {0, 0, 32, 32, 32, 32}},
        { PixelFormat::RGBA32Float,             "RGBA32F",                  PixelFormatType::Float,         128,        {1, 1, 16, 1, 1},       {0, 0, 32, 32, 32, 32}},

        // Depth-stencil
        { PixelFormat::Depth16UNorm,            "Depth16UNorm",             PixelFormatType::UNorm,         16,         {1, 1, 2, 1, 1},        {16, 0, 0, 0, 0, 0}},
        { PixelFormat::Depth32Float,            "Depth32Float",             PixelFormatType::UNorm,         32,         {1, 1, 4, 1, 1},        {32, 0, 0, 0, 0, 0}},
        { PixelFormat::Depth24UNormStencil8,    "Depth24UNormStencil8",     PixelFormatType::UNorm,         32,         {1, 1, 4, 1, 1},        {24, 8, 0, 0, 0, 0}},
        { PixelFormat::Depth32FloatStencil8,    "Depth32FloatStencil8",     PixelFormatType::Float,         32,         {1, 1, 4, 1, 1},        {32, 8, 0, 0, 0, 0}},

        // Compressed formats
        { PixelFormat::BC1RGBAUNorm,            "BC1RGBAUNorm",             PixelFormatType::UNorm,         4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC1RGBAUNormSrgb,        "BC1RGBAUNormSrgb",         PixelFormatType::UNormSrgb,     4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC2RGBAUNorm,            "BC2RGBAUNorm",             PixelFormatType::UNorm,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC2RGBAUNormSrgb,        "BC2RGBAUNormSrgb",         PixelFormatType::UNormSrgb,     8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC3RGBAUNorm,            "BC3RGBAUNorm",             PixelFormatType::UNorm,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC3RGBAUNormSrgb,        "BC3RGBAUNormSrgb",         PixelFormatType::UNormSrgb,     8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC4RUNorm,               "BC4RUNorm",                PixelFormatType::UNorm,         4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC4RSNorm,               "BC4RSNorm",                PixelFormatType::SNorm,         4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC5RGUNorm,              "BC5RGUNorm",               PixelFormatType::UNorm,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC5RGSNorm,              "BC5RGSNorm",               PixelFormatType::SNorm,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC6HRGBUFloat,           "BC6HRGBUFloat",            PixelFormatType::Float,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC6HRGBSFloat,           "BC6HRGBSFloat",            PixelFormatType::Float,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC7RGBAUNorm,            "BC7RGBAUNorm",             PixelFormatType::UNorm,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::BC7RGBAUNormSrgb,        "BC7RGBAUNormSrgb",         PixelFormatType::UNormSrgb,     8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},

        // Compressed PVRTC Pixel Formats
        { PixelFormat::PVRTC_RGB2,              "PVRTC_RGB2",               PixelFormatType::UNorm,         2,          {8, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::PVRTC_RGBA2,             "PVRTC_RGBA2",              PixelFormatType::UNorm,         2,          {8, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::PVRTC_RGB4,              "PVRTC_RGB4",               PixelFormatType::UNorm,         4,          {4, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::PVRTC_RGBA4,             "PVRTC_RGBA4",              PixelFormatType::UNorm,         4,          {4, 4, 8, 2, 2},        {0, 0, 0, 0, 0, 0}},

        // Compressed ETC Pixel Formats
        { PixelFormat::ETC2_RGB8,               "ETC2_RGB8",                PixelFormatType::UNorm,         4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::ETC2_RGB8Srgb,           "ETC2_RGB8Srgb",            PixelFormatType::UNormSrgb,     4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::ETC2_RGB8A1,             "ETC2_RGB8A1",              PixelFormatType::UNorm,         4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},
        { PixelFormat::ETC2_RGB8A1Srgb,         "ETC2_RGB8A1Srgb",          PixelFormatType::UNormSrgb,     4,          {4, 4, 8, 1, 1},        {0, 0, 0, 0, 0, 0}},

        // Compressed ASTC Pixel Formats
        { PixelFormat::ASTC4x4,                 "ASTC4x4",                  PixelFormatType::UNorm,         8,          {4, 4, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::ASTC5x5,                 "ASTC5x5",                  PixelFormatType::UNorm,         6,          {5, 5, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::ASTC6x6,                 "ASTC6x6",                  PixelFormatType::UNorm,         4,          {6, 6, 16, 1, 1},       {0, 0, 0, 0, 0, 0}},
        { PixelFormat::ASTC8x5,                 "ASTC8x5",                  PixelFormatType::UNorm,         4,          {8, 5, 16, 1, 1},       {0, 0, 0, 0, 0, 0} },
        { PixelFormat::ASTC8x6,                 "ASTC8x6",                  PixelFormatType::UNorm,         3,          {8, 6, 16, 1, 1},       {0, 0, 0, 0, 0, 0} },
        { PixelFormat::ASTC8x8,                 "ASTC8x8",                  PixelFormatType::UNorm,         3,          {8, 8, 16, 1, 1},       {0, 0, 0, 0, 0, 0} },
        { PixelFormat::ASTC10x10,               "ASTC10x10",                PixelFormatType::UNorm,         3,          {10, 10, 16, 1, 1},     {0, 0, 0, 0, 0, 0} },
        { PixelFormat::ASTC12x12,               "ASTC12x12",                PixelFormatType::UNorm,         3,          {12, 12, 16, 1, 1},     {0, 0, 0, 0, 0, 0} },
    };
}