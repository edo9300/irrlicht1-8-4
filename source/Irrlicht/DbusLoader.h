#ifndef DBUS_LOADER_H
#define DBUS_LOADER_H
#include <cstdint>
#include "IrrCompileConfig.h"

// Local copy of functions and types required from dbus.h

extern "C" {

struct DBusMessage;
struct DBusConnection;
enum DBusBusType {
	DBUS_BUS_SESSION, /**< The login session bus */
	DBUS_BUS_SYSTEM, /**< The systemwide bus */
	DBUS_BUS_STARTER /**< The bus that started us, if any */
};

struct DBusMessageIter {
	void *dummy1; /**< Don't use this */
	void *dummy2; /**< Don't use this */
	uint32_t dummy3; /**< Don't use this */
	int dummy4; /**< Don't use this */
	int dummy5; /**< Don't use this */
	int dummy6; /**< Don't use this */
	int dummy7; /**< Don't use this */
	int dummy8; /**< Don't use this */
	int dummy9; /**< Don't use this */
	int dummy10; /**< Don't use this */
	int dummy11; /**< Don't use this */
	int pad1; /**< Don't use this */
	void* pad2; /**< Don't use this */
	void* pad3; /**< Don't use this */
};

struct DBusError {
	const char *name; /**< public error name field */
	const char *message; /**< public error message field */

	unsigned int dummy1 : 1; /**< placeholder */
	unsigned int dummy2 : 1; /**< placeholder */
	unsigned int dummy3 : 1; /**< placeholder */
	unsigned int dummy4 : 1; /**< placeholder */
	unsigned int dummy5 : 1; /**< placeholder */

	void* padding1; /**< placeholder */
};

#define DBUS_TYPE_STRING ((int) 's')

/** Type code that is never equal to a legitimate type code */
#define DBUS_TYPE_INVALID ((int) '\0')	

/** Type code marking a 32-bit signed integer */
#define DBUS_TYPE_INT32 ((int) 'i')
/** Type code marking a D-Bus variant type */
#define DBUS_TYPE_VARIANT ((int) 'v')

#define DBUS_TIMEOUT_USE_DEFAULT (-1)

#define DBUS_FUNC(name, ret_type, ...) ret_type(name)(__VA_ARGS__);
#include "DbusLoader.inl"
#undef DBUS_FUNC

}


namespace irr
{

#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
	struct DbusLoader {
#define DBUS_FUNC(name, ret_type, ...) static ret_type(*name)(__VA_ARGS__);
#include "DbusLoader.inl"
#undef DBUS_FUNC
		DbusLoader()=default;
		~DbusLoader();
		bool Init();
	private:
		static void* LibDbus;
		static int amt;
		void Load();
		void Unload();
		bool inited{ false };
	};
#else
	struct DbusLoader {
#define DBUS_FUNC(name, ret_type, ...) constexpr static auto name = ::name;
#include "DbusLoader.inl"
#undef DBUS_FUNC
		DbusLoader()=default;
		~DbusLoader()=default;
	};
#endif 	//_IRR_WAYLAND_DYNAMIC_LOAD_
} // end namespace irr

#endif //DBUS_LOADER_H
