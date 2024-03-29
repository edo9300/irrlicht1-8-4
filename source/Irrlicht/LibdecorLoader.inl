LIBDECOR_FUNC(libdecor_unref, void, libdecor* context);
LIBDECOR_FUNC(libdecor_new, libdecor*, wl_display* display, libdecor_interface* iface);
LIBDECOR_FUNC(libdecor_decorate, libdecor_frame*, libdecor* context, wl_surface* surface, libdecor_frame_interface* iface, void* user_data);
LIBDECOR_FUNC(libdecor_frame_set_title, void, libdecor_frame* frame, const char* title);
LIBDECOR_FUNC(libdecor_frame_set_app_id, void, libdecor_frame* frame, const char* app_id);
LIBDECOR_FUNC(libdecor_frame_move, void, libdecor_frame* frame, wl_seat* wl_seat, uint32_t serial);
LIBDECOR_FUNC(libdecor_frame_commit, void, libdecor_frame* frame, libdecor_state* state, libdecor_configuration* configuration);
LIBDECOR_FUNC(libdecor_frame_set_minimized, void, libdecor_frame* frame);
LIBDECOR_FUNC(libdecor_frame_set_maximized, void, libdecor_frame* frame);
LIBDECOR_FUNC(libdecor_frame_unset_maximized, void, libdecor_frame* frame);
LIBDECOR_FUNC(libdecor_frame_map, void, libdecor_frame* frame);
LIBDECOR_FUNC(libdecor_state_new, libdecor_state*, int width, int height);
LIBDECOR_FUNC(libdecor_state_free, void, libdecor_state* state);
LIBDECOR_FUNC(libdecor_configuration_get_content_size, bool, libdecor_configuration* configuration, libdecor_frame* frame, int* width, int* height);
LIBDECOR_FUNC(libdecor_configuration_get_window_state, bool, libdecor_configuration* configuration, libdecor_window_state* window_state);
LIBDECOR_FUNC(libdecor_frame_set_fullscreen, void, libdecor_frame* frame, wl_output* output);
LIBDECOR_FUNC(libdecor_frame_unset_fullscreen, void, libdecor_frame* frame);

//unused
// LIBDECOR_FUNC(libdecor_get_fd, int, libdecor* context);
// LIBDECOR_FUNC(libdecor_dispatch, int, libdecor* context, int timeout);
// LIBDECOR_FUNC(libdecor_frame_ref, void, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_unref, void, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_set_visibility, void, libdecor_frame* frame, bool visible);
// LIBDECOR_FUNC(libdecor_frame_is_visible, bool, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_set_parent, void, libdecor_frame* frame, libdecor_frame* parent);
// LIBDECOR_FUNC(libdecor_frame_get_title, const char*, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_set_capabilities, void, libdecor_frame* frame, libdecor_capabilities capabilities);
// LIBDECOR_FUNC(libdecor_frame_unset_capabilities, void, libdecor_frame* frame, libdecor_capabilities capabilities);
// LIBDECOR_FUNC(libdecor_frame_has_capability, bool, libdecor_frame* frame, libdecor_capabilities capability);
// LIBDECOR_FUNC(libdecor_frame_show_window_menu, void, libdecor_frame* frame, wl_seat* wl_seat, uint32_t serial, int x, int y);
// LIBDECOR_FUNC(libdecor_frame_popup_grab, void, libdecor_frame* frame, const char* seat_name);
// LIBDECOR_FUNC(libdecor_frame_popup_ungrab, void, libdecor_frame* frame, const char* seat_name);
// LIBDECOR_FUNC(libdecor_frame_translate_coordinate, void, libdecor_frame* frame, int surface_x, int surface_y, int* frame_x, int* frame_y);
// LIBDECOR_FUNC(libdecor_frame_set_max_content_size, void, libdecor_frame* frame, int content_width, int content_height);
// LIBDECOR_FUNC(libdecor_frame_set_min_content_size, void, libdecor_frame* frame, int content_width, int content_height);
// LIBDECOR_FUNC(libdecor_frame_resize, void, libdecor_frame* frame, wl_seat* wl_seat, uint32_t serial, libdecor_resize_edge edge);
// LIBDECOR_FUNC(libdecor_frame_is_floating, bool, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_close, void, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_get_xdg_surface, xdg_surface*, libdecor_frame* frame);
// LIBDECOR_FUNC(libdecor_frame_get_xdg_toplevel, xdg_toplevel*, libdecor_frame* frame);