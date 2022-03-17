#ifndef LIBDECOR_LOADER_H
#define LIBDECOR_LOADER_H

#include <wayland-client.h>

extern "C" {
enum libdecor_error {
	LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
	LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION,
};

struct libdecor;
struct libdecor_interface {
	void (*error)(libdecor* context, libdecor_error error, const char* message);
	void (*reserved0)(void);
	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
	void (*reserved9)(void);
};
struct libdecor_frame;
struct libdecor_configuration;
struct libdecor_frame_interface {
	void (*configure)(libdecor_frame* frame, libdecor_configuration* configuration, void* user_data);
	void (*close)(libdecor_frame* frame, void* user_data);
	void (*commit)(libdecor_frame* frame, void* user_data);
	void (*dismiss_popup)(libdecor_frame* frame, const char* seat_name, void* user_data);
	void (*reserved0)(void);
	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
	void (*reserved9)(void);
};

struct xdg_toplevel;

struct libdecor_state;

enum libdecor_window_state {
	LIBDECOR_WINDOW_STATE_NONE = 0,
	LIBDECOR_WINDOW_STATE_ACTIVE = 1 << 0,
	LIBDECOR_WINDOW_STATE_MAXIMIZED = 1 << 1,
	LIBDECOR_WINDOW_STATE_FULLSCREEN = 1 << 2,
	LIBDECOR_WINDOW_STATE_TILED_LEFT = 1 << 3,
	LIBDECOR_WINDOW_STATE_TILED_RIGHT = 1 << 4,
	LIBDECOR_WINDOW_STATE_TILED_TOP = 1 << 5,
	LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 1 << 6,
};

enum libdecor_resize_edge {
	LIBDECOR_RESIZE_EDGE_NONE,
	LIBDECOR_RESIZE_EDGE_TOP,
	LIBDECOR_RESIZE_EDGE_BOTTOM,
	LIBDECOR_RESIZE_EDGE_LEFT,
	LIBDECOR_RESIZE_EDGE_TOP_LEFT,
	LIBDECOR_RESIZE_EDGE_BOTTOM_LEFT,
	LIBDECOR_RESIZE_EDGE_RIGHT,
	LIBDECOR_RESIZE_EDGE_TOP_RIGHT,
	LIBDECOR_RESIZE_EDGE_BOTTOM_RIGHT,
};

enum libdecor_capabilities {
	LIBDECOR_ACTION_MOVE = 1 << 0,
	LIBDECOR_ACTION_RESIZE = 1 << 1,
	LIBDECOR_ACTION_MINIMIZE = 1 << 2,
	LIBDECOR_ACTION_FULLSCREEN = 1 << 3,
	LIBDECOR_ACTION_CLOSE = 1 << 4,
};

#define LIBDECOR_FUNC(name, ret, ...) ret name(__VA_ARGS__);
#include "LibdecorLoader.inl"
#undef LIBDECOR_FUNC

}


namespace irr
{

class LibdecorLoader {
	public:
#ifdef IRR_LIBDECOR_DYNAMIC_LOAD
	static bool Init();
	static void Unload();
#define LIBDECOR_FUNC(name, ret, ...) static ret(*name)(__VA_ARGS__);
#include "LibdecorLoader.inl"
#undef LIBDECOR_FUNC
#else
	static constexpr bool Init() { return true; }
	static constexpr void Unload() { }
#define LIBDECOR_FUNC(name, ...) static constexpr auto name = ::name;
#include "LibdecorLoader.inl"
#undef LIBDECOR_FUNC
#endif
	
	
};

} // end namespace irr

#endif /* LIBDECOR_LOADER_H */
