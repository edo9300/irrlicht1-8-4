#ifndef C_DRIVER_CREATION_PROTOTYPES
#define C_DRIVER_CREATION_PROTOTYPES
#include "IrrCompileConfig.h"
namespace irr
{
	namespace video
	{

		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		IVideoDriver* createDirectX9Driver(const irr::SIrrlichtCreationParameters& params,
			io::IFileSystem* io, HWND window);
		#endif

		#ifdef _IRR_COMPILE_WITH_OPENGL_
		IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
				io::IFileSystem* io, IContextManager* contextManager);
		#endif

		#ifdef _IRR_COMPILE_WITH_OGLES1_
		IVideoDriver* createOGLES1Driver(const irr::SIrrlichtCreationParameters& params, io::IFileSystem* io, IContextManager* contextManager);
		#endif

		#ifdef _IRR_COMPILE_WITH_OGLES2_
		IVideoDriver* createOGLES2Driver(const irr::SIrrlichtCreationParameters& params, io::IFileSystem* io, IContextManager* contextManager);
		#endif

		#ifdef _IRR_COMPILE_WITH_WEBGL1_
		IVideoDriver* createWebGL1Driver(const irr::SIrrlichtCreationParameters& params, io::IFileSystem* io, IContextManager* contextManager);
		#endif
	} // end namespace video

} // end namespace irr
#endif //C_DRIVER_CREATION_PROTOTYPES