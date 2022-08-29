// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_EAGL_MANAGER_H_INCLUDED__
#define __C_EAGL_MANAGER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_EAGL_MANAGER_

#include "SIrrCreationParameters.h"
#include "SExposedVideoData.h"
#include "IContextManager.h"

namespace irr
{
namespace video
{
	// EAGL manager.
	class CEAGLManager : public IContextManager
	{
	public:
		//! Constructor.
		CEAGLManager();

		//! Destructor.
		virtual ~CEAGLManager();

		// Initialize EAGL.
		/* This method checks if a view has CAEAGLLayer and grabs it if it does, anyway surface and context
		aren't create. */
		bool initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data) _IRR_OVERRIDE_;

		// Terminate EAGL.
		/* Terminate EAGL context. This method break both existed surface and context. */
		void terminate() _IRR_OVERRIDE_;

		// Create EAGL surface.
		/* This method configure CAEAGLLayer. */
		bool generateSurface() _IRR_OVERRIDE_;

		// Destroy EAGL surface.
		/* This method reset CAEAGLLayer states. */
		void destroySurface() _IRR_OVERRIDE_;

		// Create EAGL context.
		/* This method create and activate EAGL context. */
		bool generateContext() _IRR_OVERRIDE_;

		// Destroy EAGL context.
		/* This method destroy EAGL context. */
		void destroyContext() _IRR_OVERRIDE_;

		const SExposedVideoData& getContext() const _IRR_OVERRIDE_;

		bool activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero) _IRR_OVERRIDE_;

		// Swap buffers.
		bool swapBuffers() _IRR_OVERRIDE_;
		
		// Generic vsync setting method for several extensions
		void swapInterval(int interval) _IRR_OVERRIDE_;

        // Context dependent getProcAddress or equivalent function
        void* loadFunction(const char* function_name) { return 0; };

	private:
		SIrrlichtCreationParameters Params;
		SExposedVideoData Data;

		bool Configured;

        void* DataStorage;

		struct SFrameBuffer
		{
			SFrameBuffer() : BufferID(0), ColorBuffer(0), DepthBuffer(0)
			{
			}

			u32 BufferID;
			u32 ColorBuffer;
			u32 DepthBuffer;
		};

		SFrameBuffer FrameBuffer;
	};
}
}

#endif
#endif
