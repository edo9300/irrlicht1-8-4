#define WAYLAND_CLIENT
#define ONLY_PROXY
struct wl_interface;
struct wl_proxy;
struct wl_event_queue;
struct wl_display;
struct wl_surface;
struct wl_shm;
union wl_argument;
#include <stdint.h>
#include <wayland-version.h>
#ifdef __cplusplus
extern "C" {
#endif
#define WAYLAND_FUNC(name, ret_type, ...) extern ret_type(* irr__internal__p__##name)(__VA_ARGS__);
#define WAYLAND_INTERFACE(name) extern const struct wl_interface *irr__internal__p__##name;
#include "CWaylandFunctions.inl"
#undef WAYLAND_INTERFACE
#undef WAYLAND_FUNC
#ifdef __cplusplus
}
#endif
#include <wayland-client-core.h>
#define wl_proxy_create (*irr__internal__p__wl_proxy_create)
#define wl_proxy_destroy (*irr__internal__p__wl_proxy_destroy)
#define wl_proxy_marshal (*irr__internal__p__wl_proxy_marshal)
#define wl_proxy_set_user_data (*irr__internal__p__wl_proxy_set_user_data)
//#define wl_proxy_get_user_data (*irr__internal__p__wl_proxy_get_user_data)
#define wl_proxy_get_version (*irr__internal__p__wl_proxy_get_version)
#define wl_proxy_add_listener (*irr__internal__p__wl_proxy_add_listener)
#define wl_proxy_marshal_constructor (*irr__internal__p__wl_proxy_marshal_constructor)
#define wl_proxy_marshal_constructor_versioned (*irr__internal__p__wl_proxy_marshal_constructor_versioned)

#define wl_seat_interface (*irr__internal__p__wl_seat_interface)
#define wl_surface_interface (*irr__internal__p__wl_surface_interface)
#define wl_shm_pool_interface (*irr__internal__p__wl_shm_pool_interface)
#define wl_buffer_interface (*irr__internal__p__wl_buffer_interface)
#define wl_registry_interface (*irr__internal__p__wl_registry_interface)
#define wl_shell_surface_interface (*irr__internal__p__wl_shell_surface_interface)
#define wl_region_interface (*irr__internal__p__wl_region_interface)
#define wl_pointer_interface (*irr__internal__p__wl_pointer_interface)
#define wl_keyboard_interface (*irr__internal__p__wl_keyboard_interface)
#define wl_compositor_interface (*irr__internal__p__wl_compositor_interface)
#define wl_output_interface (*irr__internal__p__wl_output_interface)
#define wl_shell_interface (*irr__internal__p__wl_shell_interface)
#define wl_shm_interface (*irr__internal__p__wl_shm_interface)
#define wl_data_device_interface (*irr__internal__p__wl_data_device_interface)
#define wl_data_offer_interface (*irr__internal__p__wl_data_offer_interface)
#define wl_data_source_interface (*irr__internal__p__wl_data_source_interface)
#define wl_data_device_manager_interface (*irr__internal__p__wl_data_device_manager_interface)
#define wl_callback_interface (*irr__internal__p__wl_callback_interface)
#define wl_touch_interface (*irr__internal__p__wl_touch_interface)
#if WAYLAND_VERSION_MINOR >= 20
#define wl_proxy_marshal_flags (*irr__internal__p__wl_proxy_marshal_flags)
#define wl_proxy_marshal_array_flags (*irr__internal__p__wl_proxy_marshal_array_flags)
#endif

#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#undef ONLY_PROXY
#undef WAYLAND_CLIENT
