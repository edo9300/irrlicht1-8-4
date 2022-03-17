#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_) && defined(_IRR_WAYLAND_DYNAMIC_LOAD_)

#include <dlfcn.h>
#include "DbusLoader.h"
#include "os.h"

namespace irr
{
#define DBUS_FUNC(name, return, ...) decltype(::name)* DbusLoader::name{ nullptr };
#include "DbusLoader.inl"
#undef DBUS_FUNC

void* DbusLoader::LibDbus{ nullptr };
int DbusLoader::amt{ 0 };

void DbusLoader::Load() {
	LibDbus = dlopen("libdbus-1.so.3", RTLD_LAZY);
	if(!LibDbus)
		LibDbus = dlopen("libdbus-1.so", RTLD_LAZY);
	if(LibDbus) {
#define DBUS_FUNC(name, ...) name=(decltype(name))dlsym(LibDbus, #name);\
	if(!name) {\
		os::Printer::log("Failed to load " #name " from libdbus-1.so", ELL_WARNING);\
		Unload();\
		return;\
	}
#include "DbusLoader.inl"
#undef DBUS_FUNC
	} else {
		os::Printer::log("Failed to load libdbus-1.so", ELL_WARNING);
		return;
	}
}

void DbusLoader::Unload() {
	if(LibDbus) {
		dlclose(LibDbus);
#define DBUS_FUNC(name, ...) name=nullptr;
#include "DbusLoader.inl"
#undef DBUS_FUNC
		LibDbus = nullptr;
	}
}

bool DbusLoader::Init()
{
	if(amt==0 && !LibDbus)
		Load();
	inited = true;
	amt++;
	return LibDbus;
}


//! destructor
DbusLoader::~DbusLoader()
{
	if(!inited)
		return;
	if(--amt <= 0 && LibDbus) {
		Unload();
	}
}

} // end namespace

#endif