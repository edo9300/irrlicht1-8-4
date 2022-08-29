// Copyright (C) 2014 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_NSOGL_MANAGER_H_INCLUDED__
#define __C_NSOGL_MANAGER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_NSOGL_MANAGER_

#include "SIrrCreationParameters.h"
#include "SExposedVideoData.h"
#include "IContextManager.h"
#include "SColor.h"

#import <AppKit/NSOpenGL.h>

namespace irr
{
namespace video
{
    // NSOpenGL manager.
    class CNSOGLManager : public IContextManager
    {
    public:
        //! Constructor.
        CNSOGLManager();

		//! Destructor
		~CNSOGLManager();

        // Initialize
        bool initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data) _IRR_OVERRIDE_;

        // Terminate
        void terminate() _IRR_OVERRIDE_;

        // Create surface.
        bool generateSurface() _IRR_OVERRIDE_;

        // Destroy surface.
        void destroySurface() _IRR_OVERRIDE_;

        // Create context.
        bool generateContext() _IRR_OVERRIDE_;

        // Destroy EGL context.
        void destroyContext() _IRR_OVERRIDE_;

		//! Get current context
		const SExposedVideoData& getContext() const _IRR_OVERRIDE_;

		//! Change render context, disable old and activate new defined by videoData
		bool activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero) _IRR_OVERRIDE_;

        // Swap buffers.
        bool swapBuffers() _IRR_OVERRIDE_;
		
		// Generic vsync setting method for several extensions
		void swapInterval(int interval) _IRR_OVERRIDE_;

        // Context dependent getProcAddress or equivalent function
        void* loadFunction(const char* function_name) _IRR_OVERRIDE_ { return 0; };

    private:
        SIrrlichtCreationParameters Params;
		SExposedVideoData PrimaryContext;
        SExposedVideoData CurrentContext;

        NSOpenGLPixelFormat* PixelFormat;
	};
}
}

#endif

#endif
