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

#include "GLFW_Window.h"
#include "Games/Game.h"
#include "Diagnostics/Log.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Alimer
{
    static void OnGlfwError(int code, const char* description) {
        ALIMER_LOGERROR(description);
    }

    static void* agpu_get_gl_proc_address(const char* function) {
        return (void*)glfwGetProcAddress(function);
    }

    void Game::PlatformRun()
    {
        glfwSetErrorCallback(OnGlfwError);

#ifdef __APPLE__
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

        if (!glfwInit()) {
            ALIMER_LOGERROR("Failed to initialize GLFW");
            return;
        }

        bool opengl = true;
        if (agpu_is_backend_supported(AGPU_BACKEND_VULKAN)) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            opengl = false;
        }
        else {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        mainWindow.reset(new GLFW_Window(opengl, config.windowTitle, config.windowSize, WindowStyle::Default));

        agpu_swapchain_desc swapchain_desc = {};
        swapchain_desc.width = mainWindow->GetSize().width;
        swapchain_desc.height = mainWindow->GetSize().height;
        swapchain_desc.native_handle = mainWindow->GetNativeHandle();

        agpu_config config = {};
#if defined(_DEBUG)
        config.debug = true;
#endif
        if (opengl)
        {
            config.get_gl_proc_address = agpu_get_gl_proc_address;
        }
        else
        {
            config.swapchain_desc = &swapchain_desc;
        }

        if (!agpu_init(&config)) {

        }

        InitBeforeRun();

        // Main message loop
        while (!mainWindow->ShouldClose() && !exiting)
        {
            // Check for window messages to process.
            glfwPollEvents();
            Tick();
        }

        glfwTerminate();
    }
}
