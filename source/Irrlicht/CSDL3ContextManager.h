// Copyright (C) 2013 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_SDL3_MANAGER_H_INCLUDED__
#define __C_SDL3_MANAGER_H_INCLUDED__

#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_SDL3_DEVICE_)

#include "SIrrCreationParameters.h"
#include "SExposedVideoData.h"
#include "IContextManager.h"
#include "SColor.h"
#include <SDL3/SDL_video.h>

namespace irr
{
namespace video
{
    // WGL manager.
    class CSDL3ContextManager : public IContextManager
    {
    public:
        //! Constructor.
		explicit CSDL3ContextManager(const SExposedVideoData& videodata);

		//! Destructor
		~CSDL3ContextManager();

        // Initialize
        virtual bool initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data) _IRR_OVERRIDE_;

        // Terminate
        virtual void terminate() _IRR_OVERRIDE_;

        // Create surface.
        virtual bool generateSurface() _IRR_OVERRIDE_;

        // Destroy surface.
        virtual void destroySurface() _IRR_OVERRIDE_;

        // Create context.
        virtual bool generateContext() _IRR_OVERRIDE_;

        // Destroy EGL context.
        virtual void destroyContext() _IRR_OVERRIDE_;

		//! Get current context
		virtual const SExposedVideoData& getContext() const _IRR_OVERRIDE_;

		//! Change render context, disable old and activate new defined by videoData
		virtual bool activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero) _IRR_OVERRIDE_;

        // Swap buffers.
        virtual bool swapBuffers() _IRR_OVERRIDE_;
		
		// Generic vsync setting method for several extensions
		virtual void swapInterval(int interval) _IRR_OVERRIDE_;

        // Context dependent getProcAddress or equivalent function
		virtual void* loadFunction(const char* function_name) _IRR_OVERRIDE_;

        static void SetWindowOGLProperties(const SIrrlichtCreationParameters& CreationParams);

    private:
        static SDL_Window* GetWindow(const SExposedVideoData& data) { return static_cast<SDL_Window*>(data.OGLSDL2.Window); }
        static SDL_GLContext GetContext(const SExposedVideoData& data) { return static_cast<SDL_GLContext>(data.OGLSDL2.Context); }
        SIrrlichtCreationParameters Params;
        SExposedVideoData PrimaryContext;
        SExposedVideoData CurrentContext;
        bool GLLibraryLoaded;
	};
}
}

#endif

#endif
