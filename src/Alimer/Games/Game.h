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

#include "Core/Object.h"
#include "Games/GameWindow.h"
#include "Games/GameSystem.h"
#include "Graphics/GraphicsDevice.h"
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>

namespace Alimer
{
    struct Configuration
    {
        eastl::string windowTitle = "Alimer";

        uint32_t windowWidth = 1280;
        uint32_t windowHeight = 720;
    };

    class InputManager;

    class ALIMER_API Game : public Object
    {
        ALIMER_OBJECT(Game, Object);

    public:
        Game(const Configuration& config_);

        /// Destructor.
        virtual ~Game();

        /// Run main application loop and setup all required systems.
        int Run();

        /// Tick one frame.
        void Tick();

        /// Get the main (primary window)
        GameWindow* GetMainWindow() const { return mainWindow.get(); }

        inline InputManager* GetInput() const noexcept { return input; }

    protected:
        /// Setup before modules initialization. 
        virtual void Setup() {}

        /// Setup after window and graphics setup, by default initializes all GameSystems.
        virtual void Initialize();

    private:
        /// Called by platform backend.
        void InitBeforeRun();
        void PlatformRun();

    protected:
        int exitCode = 0;
        Configuration config;
        bool running = false;
        bool exiting = false;
        eastl::unique_ptr<GameWindow> mainWindow;
        eastl::vector<GameSystem*> gameSystems;
        GraphicsDevice* graphicsDevice = nullptr;
        InputManager* input;
    };

    extern Game* GameCreate(const eastl::vector<eastl::string>& args);

    // Call this to ensure application-main is linked in correctly without having to mess around
    // with -Wl,--whole-archive.
    ALIMER_API void GameDummy();
}