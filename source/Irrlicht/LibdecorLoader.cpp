#include "IrrCompileConfig.h"

#if defined(IRR_USE_LIBDECOR) && defined(IRR_LIBDECOR_DYNAMIC_LOAD)

#include <dlfcn.h>
#include "os.h"
#include "LibdecorLoader.h"

namespace irr {

#define LIBDECOR_FUNC(name, ret, ...) ret(*LibdecorLoader::name)(__VA_ARGS__) = nullptr;
#include "LibdecorLoader.inl"
#undef LIBDECOR_FUNC

static void* LibLibdecor{ nullptr };
static int load_count{ 0 };


bool LibdecorLoader::Init() {
	if(load_count++ != 0)
		return LibLibdecor != nullptr;
	
	LibLibdecor = dlopen("libdecor-0.so", RTLD_LAZY);
	if(!LibLibdecor)
		LibLibdecor = dlopen("libdecor-0.so.0", RTLD_LAZY);
	if(!LibLibdecor)
		LibLibdecor = dlopen("libdecor-0.so.0.100.0", RTLD_LAZY);
	if(LibLibdecor) {
#define LIBDECOR_FUNC(name, ...) name=(decltype(name))dlsym(LibLibdecor, #name);\
	if(!name) {\
		os::Printer::log("Failed to load " #name " from libdecor-0.so, no window decorations will be provided", ELL_ERROR);\
		Unload();\
		return false;\
	}
#include "LibdecorLoader.inl"
#undef LIBDECOR_FUNC
	} else {
		os::Printer::log("Failed to load libdecor-0.so, no window decorations will be provided", ELL_ERROR);
		return false;
	}
	return true;
}

void LibdecorLoader::Unload() {
	if(--load_count <= 0 && LibLibdecor) {
		dlclose(LibLibdecor);
		
	}
}

}


#endif