// Copyright (c) 2021-2022 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.
// 
//  Original license
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016-2017 Dawid Gan
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "CIrrDeviceWaylandKeycodesWorkaround.h"

#include "CIrrDeviceWayland.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <time.h>
#include <cerrno>
#include <poll.h>

#ifdef __FreeBSD__
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

#if defined _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef __FreeBSD__
#include <sys/joystick.h>
#else
#include <linux/joystick.h>
#endif
#endif
#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
#include <dlfcn.h>
#endif

#include "DbusLoader.h"

#include "CColorConverter.h"
#include "COSOperator.h"
#include "CTimer.h"
#include "CVideoModeList.h"
#include "IEventReceiver.h"
#include "IGUIEnvironment.h"
#include "IGUIElement.h"
#include "IGUISpriteBank.h"
#include "irrString.h"
#include "ISceneManager.h"
#include "Keycodes.h"
#include "os.h"
#include "SIrrCreationParameters.h"
#include "CEGLManager.h"

namespace irr
{
    namespace video
    {
        extern bool useCoreContext;
        IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
                io::IFileSystem* io, IContextManager* device);
        IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
                io::IFileSystem* io, IContextManager* device);
        IVideoDriver* createOGLES1Driver(const SIrrlichtCreationParameters& params,
                io::IFileSystem* io, IContextManager* device);
    }
}



#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
#define WAYLAND_FUNC(name, ret_type, ...) ret_type(* irr::CIrrDeviceWayland::p##name)(__VA_ARGS__) = nullptr;
#define WAYLAND_INTERFACE(name) wl_interface* irr::CIrrDeviceWayland::p##name = nullptr;
void* irr::CIrrDeviceWayland::LibWaylandEGL = nullptr;
void* irr::CIrrDeviceWayland::LibWaylandCursor = nullptr;
void* irr::CIrrDeviceWayland::LibXKBCommon = nullptr;
void* irr::CIrrDeviceWayland::LibWaylandClient = nullptr;
int irr::CIrrDeviceWayland::WaylandLoadCount = 0;
#else
#define WAYLAND_FUNC(name, ret_type, ...) ret_type(* irr::CIrrDeviceWayland::p##name)(__VA_ARGS__) = &name;
#define WAYLAND_INTERFACE(name)
#endif
#define WAYLAND_EGL_CORE
#define WAYLAND_CURSOR
#define XKB_COMMMON
#define WAYLAND_CLIENT
#include "CWaylandFunctions.inl"
#undef WAYLAND_CLIENT
#undef XKB_COMMMON
#undef WAYLAND_CURSOR
#undef WAYLAND_EGL_CORE
#undef WAYLAND_INTERFACE
#undef WAYLAND_FUNC

#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
#define WAYLAND_CLIENT
#define ONLY_PROXY
#define WAYLAND_FUNC(name, ret_type, ...) ret_type(* irr__internal__p__##name)(__VA_ARGS__) = nullptr;
#define WAYLAND_INTERFACE(name) const wl_interface* irr__internal__p__##name = nullptr;
#include "CWaylandFunctions.inl"
#undef ONLY_PROXY
#undef WAYLAND_CLIENT
#undef WAYLAND_INTERFACE
#undef WAYLAND_FUNC
#endif




/* Decodes URI escape sequences in string buf of len bytes
   (excluding the terminating NULL byte) in-place. Since
   URI-encoded characters take three times the space of
   normal characters, this should not be an issue.
   Returns the number of decoded bytes that wound up in
   the buffer, excluding the terminating NULL byte.
   The buffer is guaranteed to be NULL-terminated but
   may contain embedded NULL bytes.
   On error, -1 is returned.
 */
static int URIDecode(char* buf, int len) {
    int ri, wi, di;
    char decode = '\0';
    if(buf == nullptr || len < 0) {
        return -1;
    }
    if(len == 0) {
        len = strlen(buf);
    }
    for(ri = 0, wi = 0, di = 0; ri < len && wi < len; ri += 1) {
        if(di == 0) {
            /* start decoding */
            if(buf[ri] == '%') {
                decode = '\0';
                di += 1;
                continue;
            }
            /* normal write */
            buf[wi] = buf[ri];
            wi += 1;
            continue;
        } else if(di == 1 || di == 2) {
            char off = '\0';
            char isa = buf[ri] >= 'a' && buf[ri] <= 'f';
            char isA = buf[ri] >= 'A' && buf[ri] <= 'F';
            char isn = buf[ri] >= '0' && buf[ri] <= '9';
            if(!(isa || isA || isn)) {
                /* not a hexadecimal */
                int sri;
                for(sri = ri - di; sri <= ri; sri += 1) {
                    buf[wi] = buf[sri];
                    wi += 1;
                }
                di = 0;
                continue;
            }
            /* itsy bitsy magicsy */
            if(isn) {
                off = 0 - '0';
            } else if(isa) {
                off = 10 - 'a';
            } else if(isA) {
                off = 10 - 'A';
            }
            decode |= (buf[ri] + off) << (2 - di) * 4;
            if(di == 2) {
                buf[wi] = decode;
                wi += 1;
                di = 0;
            } else {
                di += 1;
            }
            continue;
        }
    }
    buf[wi] = '\0';
    return wi;
}

/* Convert URI to local filename
   return filename if possible, else nullptr
*/
static char* URIToLocal(char* uri) {
    char* file = nullptr;
    bool local;

    if(memcmp(uri, "file:/", 6) == 0) uri += 6;      /* local file? */
    else if(strstr(uri, ":/") != nullptr) return file; /* wrong scheme */

    local = uri[0] != '/' || (uri[0] != '\0' && uri[1] == '/');

    /* got a hostname? */
    if(!local && uri[0] == '/' && uri[2] != '/') {
        char* hostname_end = strchr(uri + 1, '/');
        if(hostname_end != nullptr) {
            char hostname[257];
            if(gethostname(hostname, 255) == 0) {
                hostname[256] = '\0';
                if(memcmp(uri + 1, hostname, hostname_end - (uri + 1)) == 0) {
                    uri = hostname_end + 1;
                    local = true;
                }
            }
        }
    }
    if(local) {
        file = uri;
        /* Convert URI escape sequences to real characters */
        URIDecode(file, 0);
        if(uri[1] == '/') {
            file++;
        } else {
            file--;
        }
    }
    return file;
}

enum IOR {
	READ	= 0x1,
	WRITE	= 0x2,
};

static int PipeReady(int fd, IOR flags)
{
	static constexpr int PIPE_MS_TIMEOUT = 10;
    int result;

    /* Note: We don't bother to account for elapsed time if we get EINTR */
    do
    {
        struct pollfd info;

        info.fd = fd;
        info.events = 0;
        if (flags & IOR::READ) {
            info.events |= POLLIN | POLLPRI;
        }
        if (flags & IOR::WRITE) {
            info.events |= POLLOUT;
        }
        result = poll(&info, 1, PIPE_MS_TIMEOUT);

    } while (result < 0 && errno == EINTR);

    return result;
}



template <typename>
struct basefunc;

template <typename ...A>
struct basefunc<void(*)(A...)> {
    static void value(A...) {
        return;
    }
};

#define MAKENOOP(func) basefunc<decltype(func)>::value

namespace irr
{

class WaylandCallbacks
{
public:
    static const wl_pointer_listener pointer_listener;
    static const wl_seat_listener seat_listener;
    static const wl_keyboard_listener keyboard_listener;
    static const wl_touch_listener touch_listener;
    static const wl_output_listener output_listener;
    static const wl_shell_surface_listener shell_surface_listener;
    static const wl_registry_listener registry_listener;
    static const xdg_wm_base_listener wm_base_listener;
    static const xdg_surface_listener surface_listener;
    static const xdg_toplevel_listener toplevel_listener;
    static const zxdg_shell_v6_listener zxdg_shell_listener;
    static const zxdg_surface_v6_listener zxdg_surface_listener;
    static const zxdg_toplevel_v6_listener zxdg_toplevel_listener;
    static const zwp_text_input_v3_listener text_input_v3_listener;
    static const wl_data_device_listener data_device_listener;
    static const wl_data_offer_listener data_offer_listener;
    static const wl_data_source_listener data_source_listener;
    static const wl_callback_listener surface_frame_listener;
#ifdef IRR_USE_LIBDECOR
	static struct libdecor_interface libdecor_interface;
	static struct libdecor_frame_interface libdecor_frame_interface;
#endif

    static void pointer_enter(void* data, wl_pointer* pointer, uint32_t serial,
                              wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->m_enter_serial = serial;
        device->updateCursor();
    }

    static void pointer_motion(void* data, wl_pointer* pointer, uint32_t time,
                               wl_fixed_t sx, wl_fixed_t sy)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->getCursorControl()->setPosition(wl_fixed_to_int(sx),
                                                wl_fixed_to_int(sy));

        SEvent irrevent;
        irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
        irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
        irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
        irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
        irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
        irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;

        device->signalEvent(irrevent);
    }

    static void pointer_button(void* data, wl_pointer* wl_pointer,
                               uint32_t serial, uint32_t time, uint32_t button,
                               uint32_t state)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_decoration && !device->CreationParams.Fullscreen &&
            state == WL_POINTER_BUTTON_STATE_PRESSED &&
            device->m_xkb_alt_pressed)
        {
#ifdef IRR_USE_LIBDECOR
            if (device->m_libdecor) {
                if (device->m_libdecor_surface) {
                    LibdecorLoader::libdecor_frame_move(device->m_libdecor_surface, device->m_seat, serial);
                }
            } else
#endif
            if (device->m_xdg_toplevel)
            {
                xdg_toplevel_move(device->m_xdg_toplevel, device->m_seat, serial);
            }
            else if (device->m_zxdg_toplevel)
            {
                zxdg_toplevel_v6_move(device->m_zxdg_toplevel, device->m_seat, serial);
            }
            else if (device->m_shell_surface)
            {
                wl_shell_surface_move(device->m_shell_surface, device->m_seat, serial);
            }
            
            return;
        }
        
        SEvent irrevent;
        irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
        irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
        irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
        irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
        irrevent.MouseInput.Event = irr::EMIE_COUNT;

        switch (button)
        {
        case BTN_LEFT:
            if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                irrevent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
                device->m_mouse_button_states |= irr::EMBSM_LEFT;
            }
            else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            {
                irrevent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
                device->m_mouse_button_states &= ~(irr::EMBSM_LEFT);
            }
            break;
        case BTN_RIGHT:
            if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                irrevent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
                device->m_mouse_button_states |= irr::EMBSM_RIGHT;
            }
            else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            {
                irrevent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
                device->m_mouse_button_states &= ~(irr::EMBSM_RIGHT);
            }
            break;
        case BTN_MIDDLE:
            if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                irrevent.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
                device->m_mouse_button_states |= irr::EMBSM_MIDDLE;
            }
            else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            {
                irrevent.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
                device->m_mouse_button_states &= ~(irr::EMBSM_MIDDLE);
            }
            break;
        default:
            break;
        }

        if (irrevent.MouseInput.Event == irr::EMIE_COUNT)
            return;

        irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;

        device->signalEvent(irrevent);

        if (irrevent.MouseInput.Event >= EMIE_LMOUSE_PRESSED_DOWN &&
            irrevent.MouseInput.Event <= EMIE_MMOUSE_PRESSED_DOWN)
        {
            u32 clicks = device->checkSuccessiveClicks(
                                                irrevent.MouseInput.X,
                                                irrevent.MouseInput.Y,
                                                irrevent.MouseInput.Event);
            if (clicks == 2)
            {
                irrevent.MouseInput.Event =
                        (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_DOUBLE_CLICK +
                        irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);

                device->signalEvent(irrevent);
            }
            else if (clicks == 3)
            {
                irrevent.MouseInput.Event =
                        (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_TRIPLE_CLICK +
                        irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);

                device->signalEvent(irrevent);
            }
        }
        device->setSelectionSerial(serial);
    }

    static void pointer_axis(void* data, wl_pointer* wl_pointer, uint32_t time,
                             uint32_t axis, wl_fixed_t value)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        {
            SEvent irrevent;
            irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
            irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
            irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
            irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
            irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
            irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;
            irrevent.MouseInput.Event = EMIE_MOUSE_WHEEL;
            irrevent.MouseInput.Wheel = wl_fixed_to_double(value) / -10.0f;

            device->signalEvent(irrevent);
        }
    }

    static void keyboard_keymap(void* data, wl_keyboard* keyboard,
                                uint32_t format, int fd, uint32_t size)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device)
        {
            close(fd);
            return;
        }

        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
        {
            close(fd);
            return;
        }

        char* map_str = static_cast<char*>(mmap(nullptr, size, PROT_READ,
                                                MAP_SHARED, fd, 0));

        if (map_str == MAP_FAILED)
        {
            close(fd);
            return;
        }

        device->m_xkb_keymap = CIrrDeviceWayland::pxkb_keymap_new_from_string(
                                                   device->m_xkb_context,
                                                   map_str,
                                                   XKB_KEYMAP_FORMAT_TEXT_V1,
                                                   XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(map_str, size);
        close(fd);

        if (!device->m_xkb_keymap)
            return;

        device->m_xkb_state = CIrrDeviceWayland::pxkb_state_new(device->m_xkb_keymap);

        if (!device->m_xkb_state)
        {
            CIrrDeviceWayland::pxkb_keymap_unref(device->m_xkb_keymap);
            device->m_xkb_keymap = nullptr;
            return;
        }

        device->m_xkb_alt_mask =
            1 << CIrrDeviceWayland::pxkb_keymap_mod_get_index(device->m_xkb_keymap, "Mod1");
        device->m_xkb_ctrl_mask =
            1 << CIrrDeviceWayland::pxkb_keymap_mod_get_index(device->m_xkb_keymap, "Control");
        device->m_xkb_shift_mask =
            1 << CIrrDeviceWayland::pxkb_keymap_mod_get_index(device->m_xkb_keymap, "Shift");

        const char* locale = getenv("LC_ALL");

        if (!locale)
            locale = getenv("LC_ALL");
        if (!locale)
            locale = getenv("LC_CTYPE");
        if (!locale)
            locale = getenv("LANG");
        if (!locale)
            locale = "C";

        device->m_xkb_compose_table = CIrrDeviceWayland::pxkb_compose_table_new_from_locale(
                                                  device->m_xkb_context,
                                                  locale,
                                                  XKB_COMPOSE_COMPILE_NO_FLAGS);

        if (!device->m_xkb_compose_table)
            return;

        device->m_xkb_compose_state = CIrrDeviceWayland::pxkb_compose_state_new(
                                                device->m_xkb_compose_table,
                                                XKB_COMPOSE_STATE_NO_FLAGS);

        if (!device->m_xkb_compose_state)
        {
            CIrrDeviceWayland::pxkb_compose_table_unref(device->m_xkb_compose_table);
            device->m_xkb_compose_table = nullptr;
        }
    }

    static void keyboard_leave(void* data, wl_keyboard* keyboard,
                               uint32_t serial, wl_surface* surface)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
		
		device->m_repeat_enabled = false;
    }

    static void keyboard_key(void* data, wl_keyboard* keyboard, uint32_t serial,
                             uint32_t time, uint32_t key, uint32_t state)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_xkb_state)
            return;
		
        wchar_t key_char = 0;

        if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
        {
            xkb_keysym_t sym = XKB_KEY_NoSymbol;

            const xkb_keysym_t* syms;
            uint32_t num_syms = CIrrDeviceWayland::pxkb_state_key_get_syms(device->m_xkb_state,
                                                       key + 8, &syms);

            if (num_syms == 1)
                sym = syms[0];

            if (sym != XKB_KEY_NoSymbol && device->m_xkb_compose_state)
            {
                xkb_compose_feed_result result = CIrrDeviceWayland::pxkb_compose_state_feed(
                                              device->m_xkb_compose_state, sym);

                if (result == XKB_COMPOSE_FEED_ACCEPTED)
                {
                    xkb_compose_status status = CIrrDeviceWayland::pxkb_compose_state_get_status(
                                                   device->m_xkb_compose_state);
                    switch (status)
                    {
                    case XKB_COMPOSE_COMPOSING:
                    case XKB_COMPOSE_CANCELLED:
                        sym = XKB_KEY_NoSymbol;
                        break;
                    case XKB_COMPOSE_COMPOSED:
                        sym = CIrrDeviceWayland::pxkb_compose_state_get_one_sym(
                                                   device->m_xkb_compose_state);
                        break;
                    default:
                        break;
                    }
                }
            }

            if (sym != XKB_KEY_NoSymbol)
            {
                key_char = CIrrDeviceWayland::pxkb_keysym_to_utf32(sym);
            }
        }

        SEvent irrevent;
        irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
        irrevent.KeyInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.KeyInput.Shift = device->m_xkb_shift_pressed;
        irrevent.KeyInput.PressedDown = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
        irrevent.KeyInput.Char = key_char;
        irrevent.KeyInput.Key = device->m_key_map[key];

        if (irrevent.KeyInput.Key == 0 && key > 0)
        {
            irrevent.KeyInput.Key = (EKEY_CODE)(KEY_KEY_CODES_COUNT + key);
        }

        device->signalEvent(irrevent);

        bool repeats = CIrrDeviceWayland::pxkb_keymap_key_repeats(device->m_xkb_keymap, key + 8);

        if (repeats && state == WL_KEYBOARD_KEY_STATE_PRESSED)
        {
            device->m_repeat_enabled = true;
            device->m_repeat_time = os::Timer::getRealTime();
            device->m_repeat_event = irrevent;
            device->setSelectionSerial(serial);
        }
        else
        {
            device->m_repeat_enabled = false;
        }
    }

    static void keyboard_modifiers(void* data, wl_keyboard* keyboard,
                                   uint32_t serial, uint32_t mods_depressed,
                                   uint32_t mods_latched, uint32_t mods_locked,
                                   uint32_t group)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_xkb_keymap)
            return;

        CIrrDeviceWayland::pxkb_state_update_mask(device->m_xkb_state, mods_depressed, mods_latched,
                              mods_locked, 0, 0, group);
        xkb_state_component state_component = (xkb_state_component)(
                            XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);

        xkb_mod_mask_t mods = CIrrDeviceWayland::pxkb_state_serialize_mods(device->m_xkb_state,
                                                       state_component);

        device->m_xkb_alt_pressed = (mods & device->m_xkb_alt_mask) != 0;
        device->m_xkb_ctrl_pressed = (mods & device->m_xkb_ctrl_mask) != 0;
        device->m_xkb_shift_pressed = (mods & device->m_xkb_shift_mask) != 0;
    }

    static void keyboard_repeat_info(void* data, wl_keyboard* keyboard,
                                     int32_t rate, int32_t delay)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->m_repeat_rate = rate == 0 ? 0 : 1000 / rate;
        device->m_repeat_delay = delay;
    }
    
    static void touch_handle_down(void* data, wl_touch* touch, uint32_t serial,
                                  uint32_t time, wl_surface *surface,
                                  int32_t id, wl_fixed_t x, wl_fixed_t y)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        SEvent event;
        event.EventType = EET_TOUCH_INPUT_EVENT;
        event.TouchInput.Event = ETIE_PRESSED_DOWN;
        event.TouchInput.ID = id;
        event.TouchInput.X = wl_fixed_to_int(x);
        event.TouchInput.Y = wl_fixed_to_int(y);
        
        device->signalEvent(event);
             
        if (device->m_touches_count == 0)
        {
            pointer_motion(data, nullptr, 0, x, y);
            pointer_button(data, nullptr, 0, 0, BTN_LEFT, 
                           WL_POINTER_BUTTON_STATE_PRESSED);
        }

        device->m_touches_count++;
    }
    
    static void touch_handle_up(void* data, wl_touch* touch, uint32_t serial,
                                uint32_t time, int32_t id)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        SEvent event;
        event.EventType = EET_TOUCH_INPUT_EVENT;
        event.TouchInput.Event = ETIE_LEFT_UP;
        event.TouchInput.ID = id;
        event.TouchInput.X = 0;
        event.TouchInput.Y = 0;
        
        device->signalEvent(event);
        
        if (device->m_touches_count == 1)
        {
            pointer_button(data, nullptr, 0, 0, BTN_LEFT, 
                           WL_POINTER_BUTTON_STATE_RELEASED);
        }

        device->m_touches_count--;
    }
    
    static void touch_handle_motion(void* data, wl_touch* touch, uint32_t time,
                                  int32_t id, wl_fixed_t x, wl_fixed_t y)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        SEvent event;
        event.EventType = EET_TOUCH_INPUT_EVENT;
        event.TouchInput.Event = ETIE_MOVED;
        event.TouchInput.ID = id;
        event.TouchInput.X = wl_fixed_to_int(x);
        event.TouchInput.Y = wl_fixed_to_int(y);
        
        device->signalEvent(event);
        
        if (device->m_touches_count == 1)
        {
            pointer_motion(data, nullptr, 0, x, y);
        }
    }
    
    static void touch_handle_cancel(void* data, wl_touch* touch)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        device->m_touches_count = 0;
    }

    static void seat_capabilities(void* data, wl_seat* seat, uint32_t caps)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if ((caps & WL_SEAT_CAPABILITY_POINTER) && !device->m_pointer)
        {
            device->m_pointer = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(device->m_pointer, &pointer_listener, device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && device->m_pointer)
        {
            wl_pointer_destroy(device->m_pointer);
            device->m_pointer = nullptr;
        }

        if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !device->m_keyboard)
        {
            device->m_has_hardware_keyboard = true;
            device->m_keyboard = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(device->m_keyboard, &keyboard_listener,
                                     device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->m_keyboard)
        {
            wl_keyboard_destroy(device->m_keyboard);
            device->m_keyboard = nullptr;
        }
        
        if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !device->m_touch)
        {
            device->m_has_touch_device = true;
            device->m_touch = wl_seat_get_touch(seat);
            wl_touch_add_listener(device->m_touch, &touch_listener,
                                  device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && device->m_touch)
        {
            wl_touch_destroy(device->m_touch);
            device->m_touch = nullptr;
        }
    }

    static void output_mode(void* data, struct wl_output* wl_output,
                            uint32_t flags, int32_t width, int32_t height,
                            int32_t refresh)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->VideoModeList->addMode(core::dimension2du(width, height), 24);

        if (flags & WL_OUTPUT_MODE_CURRENT)
        {
            device->VideoModeList->setDesktop(24, core::dimension2du(width,
                                                                    height));
        }
    }

    static void shell_surface_ping(void* data, wl_shell_surface* shell_surface,
                                   uint32_t serial)
    {
        wl_shell_surface_pong(shell_surface, serial);
    }

    static void shell_surface_configure(void* data,
                                        wl_shell_surface* shell_surface,
                                        uint32_t edges, int32_t width,
                                        int32_t height)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        if(width == 0 || height == 0) {
            return;
        }

        if(!device->CreationParams.Fullscreen && !device->CreationParams.WindowResizable)
            return;

        device->m_resizing_state.width = width;
        device->m_resizing_state.height = height;
        device->m_resizing_state.pending = true;
    }
    
    static void xdg_wm_base_ping(void* data, xdg_wm_base* shell, 
                                 uint32_t serial)
    {
        xdg_wm_base_pong(shell, serial);
    }
    
    static void zxdg_wm_shell_ping(void* data, zxdg_shell_v6* shell, 
                                 uint32_t serial)
    {
        zxdg_shell_v6_pong(shell, serial);
    }
    
    static void xdg_surface_configure(void* data, xdg_surface* surface,
                                      uint32_t serial)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if(!device->m_surface_configured) {

            device->m_width = device->m_resizing_state.width;
            device->m_height = device->m_resizing_state.height;
            CIrrDeviceWayland::pwl_egl_window_resize(device->m_egl_window, device->m_width, device->m_height, 0, 0);

            xdg_surface_ack_configure(surface, serial);

            wl_region* region = wl_compositor_create_region(device->m_compositor);
            wl_region_add(region, 0, 0, device->m_width, device->m_height);
            wl_surface_set_opaque_region(device->m_surface, region);
            wl_region_destroy(region);

            device->m_surface_configured = true;
        } else {
            device->m_resizing_state.pending = true;
            device->m_resizing_state.configure = true;
            device->m_resizing_state.serial = serial;
        }
    }
    
    static void xdg_toplevel_configure(void* data, xdg_toplevel* toplevel,
                                       int32_t width, int32_t height,
                                       wl_array* states)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        bool fullscreen = false;
        {
            auto* state = static_cast<xdg_toplevel_state*>(states->data);
            const auto array_end = state + (states->size / sizeof(xdg_toplevel_state*));
            for(; state < array_end; state++) {
                if(*state == XDG_TOPLEVEL_STATE_FULLSCREEN)
                    fullscreen = true;
            }
        }
        if(width == 0 || height == 0) {
            width = device->m_width;
            height = device->m_height;
        }

        if(!fullscreen) {

            /* zxdg_toplevel spec states that this is a suggestion.
               Ignore if less than or greater than max/min size. */

            if(!device->CreationParams.WindowResizable) {
                width = device->m_width;
                height = device->m_height;
            }
        }
        device->m_resizing_state.width = width;
        device->m_resizing_state.height = height;
    }
    
    static void xdg_toplevel_close(void* data, xdg_toplevel* xdg_toplevel)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        device->closeDevice();
    }
    
    static void zxdg_surface_configure(void* data, zxdg_surface_v6* surface,
                                      uint32_t serial)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if(!device->m_surface_configured) {

            device->m_width = device->m_resizing_state.width;
            device->m_height = device->m_resizing_state.height;
            CIrrDeviceWayland::pwl_egl_window_resize(device->m_egl_window, device->m_width, device->m_height, 0, 0);

            zxdg_surface_v6_ack_configure(surface, serial);

            wl_region* region = wl_compositor_create_region(device->m_compositor);
            wl_region_add(region, 0, 0, device->m_width, device->m_height);
            wl_surface_set_opaque_region(device->m_surface, region);
            wl_region_destroy(region);

            device->m_surface_configured = true;
        } else {
            device->m_resizing_state.pending = true;
            device->m_resizing_state.configure = true;
            device->m_resizing_state.serial = serial;
        }
    }
    
    static void zxdg_toplevel_configure(void* data, zxdg_toplevel_v6* toplevel,
                                       int32_t width, int32_t height,
                                       wl_array* states)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        bool fullscreen = false;
        {
            auto* state = static_cast<zxdg_toplevel_v6_state*>(states->data);
            const auto array_end = state + (states->size / sizeof(zxdg_toplevel_v6_state*));
            for(; state < array_end; state++) {
                if(*state == ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN)
                    fullscreen = true;
            }
        }
        if(width == 0 || height == 0) {
            width = device->m_width;
            height = device->m_height;
        }

        if(!fullscreen) {

            /* zxdg_toplevel spec states that this is a suggestion.
               Ignore if less than or greater than max/min size. */

            if(!device->CreationParams.WindowResizable) {
                width = device->m_width;
                height = device->m_height;
            }
        }
        device->m_resizing_state.width = width;
        device->m_resizing_state.height = height;
    }

    static void text_input_v3_commit_string(void* data,
                                 struct zwp_text_input_v3* zwp_text_input_v3,
                                 const char* text) {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        if(text && *text) {
            SEvent event;
            event.EventType = irr::EET_KEY_INPUT_EVENT;
            event.KeyInput.PressedDown = true;
            event.KeyInput.Key = irr::KEY_ACCEPT;
            event.KeyInput.Shift = 0;
            event.KeyInput.Control = 0;
            size_t lenOld = strlen(text);
            wchar_t* ws = new wchar_t[lenOld + 1];
            core::utf8ToWchar(text, ws, (lenOld + 1) * sizeof(wchar_t));
            wchar_t* cur = ws;
            while(*cur) {
                event.KeyInput.Char = *cur;
                cur++;
                device->postEventFromUser(event);
            }
            delete[] ws;
        }
    }
    
    static void zxdg_toplevel_close(void* data, zxdg_toplevel_v6* zxdg_toplevel_v6)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        device->closeDevice();
    }

    static void registry_global(void* data, wl_registry* registry,
                                uint32_t name, const char* interface,
                                uint32_t version)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (interface == nullptr)
            return;

        if (strcmp(interface, wl_compositor_interface.name) == 0)
        {
            device->m_compositor = static_cast<wl_compositor*>(wl_registry_bind(
                                  registry, name, &wl_compositor_interface, 1));
        }
        else if (strcmp(interface, wl_shell_interface.name) == 0)
        {
            device->m_has_wl_shell = true;
            device->m_wl_shell_name = name;
        }
        else if (strcmp(interface, wl_seat_interface.name) == 0)
        {
            device->m_seat = static_cast<wl_seat*>(wl_registry_bind(registry,
                                                   name, &wl_seat_interface,
                                                   version < 4 ? version : 4));
                                                   
            wl_seat_add_listener(device->m_seat, &seat_listener, device);
            if(!device->m_data_device && device->m_data_device_manager) {
                device->m_data_device = wl_data_device_manager_get_data_device(device->m_data_device_manager, device->m_seat);

                wl_data_device_add_listener(device->m_data_device, &data_device_listener, data);
            }
            if(!device->m_input_v3 && device->m_input_manager_v3) {
                device->m_input_v3 = zwp_text_input_manager_v3_get_text_input(device->m_input_manager_v3, device->m_seat);

                zwp_text_input_v3_add_listener(device->m_input_v3, &text_input_v3_listener, device);
            }
        }
        else if (strcmp(interface, wl_data_device_manager_interface.name) == 0)
        {
            device->m_data_device_manager = static_cast<wl_data_device_manager*>(wl_registry_bind(registry,
                                                   name, &wl_data_device_manager_interface, 3));
            if(!device->m_data_device && device->m_seat) {
                device->m_data_device = wl_data_device_manager_get_data_device(device->m_data_device_manager, device->m_seat);

                wl_data_device_add_listener(device->m_data_device, &data_device_listener, data);
            }
        }
        else if (strcmp(interface, wl_shm_interface.name) == 0)
        {
            device->m_shm = static_cast<wl_shm*>(wl_registry_bind(registry, 
                                                   name, &wl_shm_interface, 1));
        }
        else if (strcmp(interface, wl_output_interface.name) == 0)
        {
            device->m_output = static_cast<wl_output*>(wl_registry_bind(
                                           registry, name, &wl_output_interface,
                                           version < 2 ? version : 2));
                                           
            wl_output_add_listener(device->m_output, &output_listener, device);
        }
        else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
        {
            device->m_decoration_manager = 
                                    static_cast<zxdg_decoration_manager_v1*>(
                                    wl_registry_bind(registry, name, 
                                    &zxdg_decoration_manager_v1_interface, 1));
        }
        else if (strcmp(interface, org_kde_kwin_server_decoration_manager_interface.name) == 0)
        {
            device->m_kwin_server_decoration_manager = 
                                    static_cast<org_kde_kwin_server_decoration_manager*>(
                                    wl_registry_bind(registry, name, 
                                    &org_kde_kwin_server_decoration_manager_interface, 1));
        }
        else if (strcmp(interface, zwp_text_input_manager_v3_interface.name) == 0)
        {
            device->m_input_manager_v3 =
                                    static_cast<zwp_text_input_manager_v3*>(
                                    wl_registry_bind(registry, name, 
                                    &zwp_text_input_manager_v3_interface, 1));
            if(!device->m_input_v3 && device->m_seat) {
                device->m_input_v3 = zwp_text_input_manager_v3_get_text_input(device->m_input_manager_v3, device->m_seat);

                zwp_text_input_v3_add_listener(device->m_input_v3, &text_input_v3_listener, device);
            }
        }
        else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
        {
            device->m_has_xdg_wm_base = true;
            device->m_xdg_wm_base_name = name;
        }
        else if (strcmp(interface, zxdg_shell_v6_interface.name) == 0)
        {
            device->m_has_zxdg_shell = true;
            device->m_zxdg_shell_name = name;
        }
    }


    static void data_offer_handle_offer(void* data, wl_data_offer* wl_data_offer, const char* mime_type)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        u32& cur_mime = device->m_current_selection_mime;
        if((cur_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT) == 0 && strcmp(mime_type, "text/plain") == 0)
            cur_mime |= CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT;
        else if((cur_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8) == 0 &&
                strcmp(mime_type, "text/plain;charset=UTF-8") == 0)
            cur_mime |= CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8;
        else if((cur_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2) == 0 &&
                strcmp(mime_type, "text/plain;charset=utf-8") == 0)
            cur_mime |= CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2;
        else if((cur_mime & CIrrDeviceWayland::DATA_MIME::URI_LIST) == 0 &&
                strcmp(mime_type, "text/uri-list") == 0)
            cur_mime |= CIrrDeviceWayland::DATA_MIME::URI_LIST;
    }

    static void data_offer_handle_source_actions(void* data, wl_data_offer* wl_data_offer, uint32_t source_actions)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        device->m_drag_has_copy = source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }

    static void data_device_handle_data_offer(void* data, wl_data_device* wl_data_device, wl_data_offer* id)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        device->m_current_selection_mime = 0;
        wl_data_offer_add_listener(id, &data_offer_listener, data);
    }

    static void data_device_handle_selection(void* data, wl_data_device* wl_data_device, wl_data_offer* id)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        if(device->m_clipboard_data_offer != id) {
            if(device->m_clipboard_data_offer)
                wl_data_offer_destroy(device->m_clipboard_data_offer);
            device->m_clipboard_data_offer = id;
            device->m_clipboard_changed = true;
            device->m_clipboard_mime = device->m_current_selection_mime;
        }
    }

    static void data_device_handle_enter(void* data, wl_data_device* wl_data_device,
                                 uint32_t serial, wl_surface* surface,
                                 wl_fixed_t x, wl_fixed_t y, wl_data_offer* offer)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;

        device->m_drag_data_offer = offer;
        device->m_drag_data_offer_serial = serial;

        uint32_t dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
        const char* accept = nullptr;

        if(!device->m_drag_has_copy || device->m_current_selection_mime == 0)
            dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;

        bool is_file = device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::URI_LIST;

        if(dnd_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
            device->m_drop_pos = core::vector2di(wl_fixed_to_int(x), wl_fixed_to_int(y));
            if(device->m_drag_and_drop_check && !device->m_drag_and_drop_check(device->m_drop_pos, is_file))
                dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
            else {
                if(is_file)
                    accept = "text/uri-list";
                else if(device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8)
                    accept = "text/plain;charset=UTF-8";
                else if(device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2)
                    accept = "text/plain;charset=utf-8";
                else
                    accept = "text/plain";
            }
        }

        wl_data_offer_accept(offer, serial, accept);


        if(wl_data_offer_get_version(offer) >= 3) {
            wl_data_offer_set_actions(offer, dnd_action, dnd_action);
        }
    }

    static void data_device_handle_motion(void* data, wl_data_device* wl_data_device,
                                  uint32_t time, wl_fixed_t x, wl_fixed_t y)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        uint32_t dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
        const char* accept = nullptr;

        bool is_file = device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::URI_LIST;

        if(dnd_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
            device->m_drop_pos = core::vector2di(wl_fixed_to_int(x), wl_fixed_to_int(y));
            if(device->m_drag_and_drop_check && !device->m_drag_and_drop_check(device->m_drop_pos, is_file))
                dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
            else {
                if(is_file)
                    accept = "text/uri-list";
                else if(device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8)
                    accept = "text/plain;charset=UTF-8";
                else if(device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2)
                    accept = "text/plain;charset=utf-8";
                else
                    accept = "text/plain";
            }
        }

        wl_data_offer_accept(device->m_drag_data_offer, device->m_drag_data_offer_serial, accept);


        if(wl_data_offer_get_version(device->m_drag_data_offer) >= 3) {
            wl_data_offer_set_actions(device->m_drag_data_offer, dnd_action, dnd_action);
        }
    }

    static void data_device_handle_drop(void* data, wl_data_device* wl_data_device)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        if(!device->m_drag_data_offer)
            return;

        device->m_drag_is_dropping = true;

        bool is_file = device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::URI_LIST;

        core::stringc read_text;

        int pipefd[2];
        if(pipe2(pipefd, O_CLOEXEC | O_NONBLOCK) != -1) {

            const char* accept = nullptr;

            if(is_file)
                accept = "text/uri-list";
            else if(device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8)
                accept = "text/plain;charset=UTF-8";
            else if(device->m_current_selection_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2)
                accept = "text/plain;charset=utf-8";
            else
                accept = "text/plain";

            wl_data_offer_receive(device->m_drag_data_offer, accept, pipefd[1]);

			CIrrDeviceWayland::pwl_display_flush(device->m_display);
			CIrrDeviceWayland::pwl_display_roundtrip(device->m_display);
			close(pipefd[1]);

			while(true) {
				auto ready = PipeReady(pipefd[0], IOR::READ);
				if (ready <= 0)
					break;
				char buf[PIPE_BUF];
				ssize_t n = read(pipefd[0], buf, sizeof(buf));
				if(n <= 0)
					break;
				read_text.append(buf, n);
			}
			
			close(pipefd[0]);
        }

        read_text.append("", 0);

        irr::SEvent event;
        event.EventType = irr::EET_DROP_EVENT;
        event.DropEvent.DropType = irr::DROP_START;
        event.DropEvent.X = device->m_drop_pos.X;
        event.DropEvent.Y = device->m_drop_pos.Y;
        event.DropEvent.Text = nullptr;
        device->postEventFromUser(event);

        event.DropEvent.DropType = is_file ? irr::DROP_FILE : irr::DROP_TEXT;

        if(is_file) {
            char* saveptr = nullptr;
            char* buffer = &read_text[0];
            char* token = strtok_r(buffer, "\r\n", &saveptr);
            while(token != nullptr) {
                char* fn = URIToLocal(token);
                if(fn) {
                    size_t lenOld = strlen(fn);
                    wchar_t* ws = new wchar_t[lenOld + 1];
                    core::utf8ToWchar(fn, ws, (lenOld + 1) * sizeof(wchar_t));
                    event.DropEvent.Text = ws;
                    device->postEventFromUser(event);
                    delete[] ws;
                }
                token = strtok_r(nullptr, "\r\n", &saveptr);
            }
        } else {
            size_t lenOld = read_text.size();
            wchar_t* ws = new wchar_t[lenOld + 1];
            core::utf8ToWchar(read_text.c_str(), ws, (lenOld + 1) * sizeof(wchar_t));
            event.DropEvent.Text = ws;
            device->postEventFromUser(event);
            delete[] ws;
        }

        event.DropEvent.DropType = irr::DROP_END;
        device->postEventFromUser(event);

        wl_data_offer_finish(device->m_drag_data_offer);
        wl_data_offer_destroy(device->m_drag_data_offer);
        device->m_drag_data_offer = nullptr;
        device->m_drag_data_offer_serial = 0;
        device->m_drag_is_dropping = false;
    }

    static void data_device_handle_leave(void* data, wl_data_device* wl_data_device)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        if(!device->m_drag_is_dropping) {
            if(device->m_drag_data_offer)
                wl_data_offer_destroy(device->m_drag_data_offer);
            device->m_drag_data_offer = nullptr;
            device->m_drag_data_offer_serial = 0;
        }
    }

    static void data_source_handle_send(void* data, wl_data_source* source,
                                        const char* mime_type, int fd)
    {
        CIrrDeviceWayland* device = (CIrrDeviceWayland*)data;
        // An application wants to paste the clipboard contents
        if(strcmp(mime_type, "text/plain") == 0 ||
           strcmp(mime_type, "text/plain;charset=utf-8") == 0 ||
           strcmp(mime_type, "text/plain;charset=UTF-8") == 0) {
				auto& clipboard = device->m_clipboard;
				write(fd, clipboard.data(), clipboard.size());
        }	
        close(fd);
    }

    static void data_source_handle_cancelled(void* data, wl_data_source* source)
    {
        // An application has replaced the clipboard contents
        wl_data_source_destroy(source);
    }

    static void surface_frame_done(void* data, wl_callback* cb, uint32_t time)
    {
        CIrrDeviceWayland::CCursorControl* cursor_control = (CIrrDeviceWayland::CCursorControl*)data;
        wl_callback_destroy(cb);

        if(cursor_control->m_is_animated && cursor_control->m_is_visible) {

            wl_surface* surface = cursor_control->m_device->m_cursor_surface;
            cb = wl_surface_frame(surface);
            wl_callback_add_listener(cb, &WaylandCallbacks::surface_frame_listener, data);

            CIrrDeviceWayland* device = cursor_control->m_device;

            wl_cursor_image* image = cursor_control->getNextFrame(time);
            wl_buffer* buffer = CIrrDeviceWayland::pwl_cursor_image_get_buffer(image);

            wl_pointer_set_cursor(device->m_pointer, device->m_enter_serial, surface,
                                  image->hotspot_x, image->hotspot_y);

            wl_surface_attach(surface, buffer, 0, 0);
            wl_surface_damage(surface, 0, 0, image->width, image->height);
            wl_surface_commit(surface);
        }
    }
#ifdef IRR_USE_LIBDECOR
	static void libdecor_error(libdecor* context, libdecor_error error, const char* message)
	{
		os::Printer::log((core::stringc("libdecor error (")+core::stringc(error)+"): "+message).data(), ELL_ERROR);
	}
    static void libdecor_frame_configure(libdecor_frame* frame, libdecor_configuration* configuration, void* data) {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
		libdecor_window_state window_state;
        bool fullscreen = false;
		if (LibdecorLoader::libdecor_configuration_get_window_state(configuration, &window_state)) {
			fullscreen = (window_state & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
		}
		int32_t width, height;
        if(!LibdecorLoader::libdecor_configuration_get_content_size(configuration, frame,
                                                     &width, &height)) {
            width = device->m_width;
            height = device->m_height;
        }

        if(!fullscreen) {

            /* zxdg_toplevel spec states that this is a suggestion.
               Ignore if less than or greater than max/min size. */

            if(!device->CreationParams.WindowResizable) {
                width = device->m_width;
                height = device->m_height;
            }
        }
		device->m_surface_configured = true;
        device->m_resizing_state.pending = true;
        device->m_resizing_state.width = width;
        device->m_resizing_state.height = height;
		auto state = LibdecorLoader::libdecor_state_new(width, height);
		LibdecorLoader::libdecor_frame_commit(frame, state, configuration);
		LibdecorLoader::libdecor_state_free(state);
	}
    static void libdecor_frame_close(libdecor_frame* frame, void* data) {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        device->closeDevice();		
	}
    static void libdecor_frame_commit(libdecor_frame* frame, void* data) {
		(void)frame;
		(void)data;
		return;
	}
#endif
};

const wl_pointer_listener WaylandCallbacks::pointer_listener =
{
    WaylandCallbacks::pointer_enter,
    MAKENOOP(wl_pointer_listener::leave),
    WaylandCallbacks::pointer_motion,
    WaylandCallbacks::pointer_button,
    WaylandCallbacks::pointer_axis
};

const wl_keyboard_listener WaylandCallbacks::keyboard_listener =
{
    WaylandCallbacks::keyboard_keymap,
    MAKENOOP(wl_keyboard_listener::enter),
    WaylandCallbacks::keyboard_leave,
    WaylandCallbacks::keyboard_key,
    WaylandCallbacks::keyboard_modifiers,
    WaylandCallbacks::keyboard_repeat_info
};

const wl_touch_listener WaylandCallbacks::touch_listener =
{
    WaylandCallbacks::touch_handle_down,
    WaylandCallbacks::touch_handle_up,
    WaylandCallbacks::touch_handle_motion,
    MAKENOOP(wl_touch_listener::frame),
    WaylandCallbacks::touch_handle_cancel
};

const wl_seat_listener WaylandCallbacks::seat_listener =
{
    WaylandCallbacks::seat_capabilities,
    MAKENOOP(wl_seat_listener::name)
};

const wl_output_listener WaylandCallbacks::output_listener =
{
    MAKENOOP(wl_output_listener::geometry),
    WaylandCallbacks::output_mode,
    MAKENOOP(wl_output_listener::done),
    MAKENOOP(wl_output_listener::scale)
};

const wl_shell_surface_listener WaylandCallbacks::shell_surface_listener =
{
    WaylandCallbacks::shell_surface_ping,
    WaylandCallbacks::shell_surface_configure,
    MAKENOOP(wl_shell_surface_listener::popup_done)
};

const wl_registry_listener WaylandCallbacks::registry_listener =
{
    WaylandCallbacks::registry_global,
    MAKENOOP(wl_registry_listener::global_remove)
};

const xdg_wm_base_listener WaylandCallbacks::wm_base_listener = 
{
    WaylandCallbacks::xdg_wm_base_ping
};

const xdg_surface_listener WaylandCallbacks::surface_listener = 
{
    WaylandCallbacks::xdg_surface_configure
};

const zxdg_shell_v6_listener WaylandCallbacks::zxdg_shell_listener = 
{
    WaylandCallbacks::zxdg_wm_shell_ping
};

const xdg_toplevel_listener WaylandCallbacks::toplevel_listener = 
{
    WaylandCallbacks::xdg_toplevel_configure,
    WaylandCallbacks::xdg_toplevel_close
};

const zxdg_surface_v6_listener WaylandCallbacks::zxdg_surface_listener = 
{
    WaylandCallbacks::zxdg_surface_configure
};

const zxdg_toplevel_v6_listener WaylandCallbacks::zxdg_toplevel_listener = 
{
    WaylandCallbacks::zxdg_toplevel_configure,
    WaylandCallbacks::zxdg_toplevel_close
};

const zwp_text_input_v3_listener WaylandCallbacks::text_input_v3_listener =
{
    MAKENOOP(zwp_text_input_v3_listener::enter),
    MAKENOOP(zwp_text_input_v3_listener::leave),
    MAKENOOP(zwp_text_input_v3_listener::preedit_string),
    WaylandCallbacks::text_input_v3_commit_string,
    MAKENOOP(zwp_text_input_v3_listener::delete_surrounding_text),
    MAKENOOP(zwp_text_input_v3_listener::done)
};

const wl_data_device_listener WaylandCallbacks::data_device_listener =
{
    WaylandCallbacks::data_device_handle_data_offer,
    WaylandCallbacks::data_device_handle_enter,
    WaylandCallbacks::data_device_handle_leave,
    WaylandCallbacks::data_device_handle_motion,
    WaylandCallbacks::data_device_handle_drop,
    WaylandCallbacks::data_device_handle_selection
};

const wl_data_offer_listener WaylandCallbacks::data_offer_listener =
{
    WaylandCallbacks::data_offer_handle_offer,
    WaylandCallbacks::data_offer_handle_source_actions, // Version 3
    MAKENOOP(wl_data_offer_listener::action)        // Version 3
};

const wl_data_source_listener WaylandCallbacks::data_source_listener =
{
    MAKENOOP(wl_data_source_listener::target),
    WaylandCallbacks::data_source_handle_send,
    WaylandCallbacks::data_source_handle_cancelled,
    MAKENOOP(wl_data_source_listener::dnd_drop_performed),
    MAKENOOP(wl_data_source_listener::dnd_finished),
    MAKENOOP(wl_data_source_listener::action)
};
const struct wl_callback_listener WaylandCallbacks::surface_frame_listener =
{
    WaylandCallbacks::surface_frame_done
};
#ifdef IRR_USE_LIBDECOR
struct libdecor_interface WaylandCallbacks::libdecor_interface =
{
    WaylandCallbacks::libdecor_error
};
struct libdecor_frame_interface WaylandCallbacks::libdecor_frame_interface =
{
    WaylandCallbacks::libdecor_frame_configure,
    WaylandCallbacks::libdecor_frame_close,
    WaylandCallbacks::libdecor_frame_commit
};
#endif



bool CIrrDeviceWayland::isWaylandDeviceWorking()
{
    bool is_working = false;

    wl_display* display = pwl_display_connect(nullptr);

    if (display != nullptr)
    {
        is_working = true;
        pwl_display_disconnect(display);
    }

    return is_working;
}

CIrrDeviceWayland::CIrrDeviceWayland(const SIrrlichtCreationParameters& params)
    : CIrrDeviceStub(params)
{
    m_compositor = nullptr;
    m_cursor_theme = nullptr;
    m_display = nullptr;
    m_egl_window = nullptr;
    m_keyboard = nullptr;
    m_touch = nullptr;
    m_output = nullptr;
    m_pointer = nullptr;
    m_registry = nullptr;
    m_seat = nullptr;
    m_data_device = nullptr;
    m_shm = nullptr;
    m_cursor_surface = nullptr;
    m_surface = nullptr;
    m_data_device_manager = nullptr;
    m_enter_serial = 0;

    m_shell = nullptr;
    m_shell_surface = nullptr;
    m_has_wl_shell = false;
    m_wl_shell_name = 0;
    
    m_xdg_wm_base = nullptr;
    m_xdg_surface = nullptr;
    m_xdg_toplevel = nullptr;
    m_has_xdg_wm_base = false;
    m_xdg_wm_base_name = 0;
    
    m_zxdg_shell = nullptr;
    m_zxdg_surface = nullptr;
    m_zxdg_toplevel = nullptr;
    m_has_zxdg_shell = false;
    m_zxdg_shell_name = 0;
	
#ifdef IRR_USE_LIBDECOR
	m_libdecor = nullptr;
	m_libdecor_surface = nullptr;
#endif
	
    m_surface_configured = false;
    
    m_decoration_manager = nullptr;
    m_decoration = nullptr;
    
    m_kwin_server_decoration_manager = nullptr;
    m_kwin_server_decoration = nullptr;

    m_input_manager_v3 = nullptr;
    m_input_v3 = nullptr;

    lastFocusedElement = nullptr;
    isEditingText = false;
    hasIMEInput = false;

    m_xkb_context = nullptr;
    m_xkb_compose_table = nullptr;
    m_xkb_compose_state = nullptr;
    m_xkb_keymap = nullptr;
    m_xkb_state = nullptr;
    m_xkb_alt_mask = 0;
    m_xkb_ctrl_mask = 0;
    m_xkb_shift_mask = 0;
    m_xkb_alt_pressed = false;
    m_xkb_ctrl_pressed = false;
    m_xkb_shift_pressed = false;

    m_repeat_enabled = false;
    m_repeat_time = 0;
    m_repeat_rate = 40;
    m_repeat_delay = 400;

    m_mouse_button_states = 0;
    m_width = params.WindowSize.Width;
    m_height = params.WindowSize.Height;
    m_touches_count = 0;
    m_has_touch_device = false;
    m_has_hardware_keyboard = false;
    m_window_has_focus = false;
    m_window_minimized = false;

    m_drag_and_drop_check = nullptr;
    m_dragging_file = false;

    m_clipboard_data_offer = nullptr;
    m_clipboard_changed = false;

    m_selection_serial = 0;
    m_data_source = nullptr;

    m_clipboard_mime = 0;
    m_current_selection_mime = 0;

    m_drag_has_copy = false;

    m_drag_data_offer = nullptr;
    m_drag_data_offer_serial = 0;

    m_drag_is_dropping = false;

    m_resizing_state.configure = false;
    m_resizing_state.pending = false;
    
    #ifdef _DEBUG
    setDebugName("CIrrDeviceWayland");
    #endif

    utsname LinuxInfo;
    uname(&LinuxInfo);

    core::stringc linuxversion;
    linuxversion += LinuxInfo.sysname;
    linuxversion += " ";
    linuxversion += LinuxInfo.release;
    linuxversion += " ";
    linuxversion += LinuxInfo.version;
    linuxversion += " ";
    linuxversion += LinuxInfo.machine;
    linuxversion += " (Wayland)";

    Operator = new COSOperator(linuxversion, this);
    os::Printer::log(linuxversion.c_str(), ELL_INFORMATION);

    createKeyMap();
    
    bool success = initWayland();

    if(!success)
        return;

    setResizable(params.WindowResizable);

    createDriver();

    CursorControl = new CCursorControl(this);

    if (VideoDriver)
    {
        createGUIAndScene();
    }
}

CIrrDeviceWayland::~CIrrDeviceWayland()
{    
    if (m_decoration)
        zxdg_toplevel_decoration_v1_destroy(m_decoration);
        
    if (m_decoration_manager)
        zxdg_decoration_manager_v1_destroy(m_decoration_manager);
	
    if (m_kwin_server_decoration)
        org_kde_kwin_server_decoration_release(m_kwin_server_decoration);

    if(m_input_v3)
        zwp_text_input_v3_destroy(m_input_v3);

    if(m_input_manager_v3)
        zwp_text_input_manager_v3_destroy(m_input_manager_v3);
        
    if (m_kwin_server_decoration_manager)
        org_kde_kwin_server_decoration_manager_destroy(m_kwin_server_decoration_manager);
    
    if (m_keyboard)
        wl_keyboard_destroy(m_keyboard);

    if (m_pointer)
        wl_pointer_destroy(m_pointer);

    if (m_cursor_surface)
        wl_surface_destroy(m_cursor_surface);

    if (m_cursor_theme)
        pwl_cursor_theme_destroy(m_cursor_theme);
        
    if (m_xdg_toplevel)
        xdg_toplevel_destroy(m_xdg_toplevel);

    if (m_xdg_surface)
        xdg_surface_destroy(m_xdg_surface);
        
    if (m_xdg_wm_base)
        xdg_wm_base_destroy(m_xdg_wm_base);
        
    if (m_zxdg_toplevel)
        zxdg_toplevel_v6_destroy(m_zxdg_toplevel);

    if (m_zxdg_surface)
        zxdg_surface_v6_destroy(m_zxdg_surface);
        
    if (m_zxdg_shell)
        zxdg_shell_v6_destroy(m_zxdg_shell);
        
    if (m_shell_surface)
        wl_shell_surface_destroy(m_shell_surface);
        
    if (m_shell)
        wl_shell_destroy(m_shell);

#ifdef IRR_USE_LIBDECOR	
	if (m_libdecor) {
		LibdecorLoader::libdecor_unref(m_libdecor);
		LibdecorLoader::Unload();
	}
#endif

    if (m_egl_window)
        pwl_egl_window_destroy(m_egl_window);
    
    if (m_surface)
        wl_surface_destroy(m_surface);
    
    if (m_data_device_manager)
        wl_data_device_manager_destroy(m_data_device_manager);
    
    if (m_clipboard_data_offer)
        wl_data_offer_destroy(m_clipboard_data_offer);
    
    if (m_drag_data_offer)
        wl_data_offer_destroy(m_drag_data_offer);
        
    if (m_shm)
        wl_shm_destroy(m_shm);
        
    if (m_compositor)
        wl_compositor_destroy(m_compositor);

    if (m_output)
        wl_output_destroy(m_output);

    if (m_seat)
        wl_seat_destroy(m_seat);

    if (m_data_device)
        wl_data_device_release(m_data_device);

    if (m_registry)
        wl_registry_destroy(m_registry);

    if (m_xkb_state)
        pxkb_state_unref(m_xkb_state);
        
    if (m_xkb_keymap)
        pxkb_keymap_unref(m_xkb_keymap);
        
    if (m_xkb_compose_state)
        pxkb_compose_state_unref(m_xkb_compose_state);
        
    if (m_xkb_compose_table)
        pxkb_compose_table_unref(m_xkb_compose_table);

    if (m_xkb_context)
        pxkb_context_unref(m_xkb_context);

    if (m_display)
    {
        pwl_display_flush(m_display);
        pwl_display_disconnect(m_display);
    }

    closeJoysticks();

#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
    if(LibWaylandEGL) {
        WaylandLoadCount--;
        if(WaylandLoadCount == 0) {
            clearWaylandFunctions();
        }
    }
#endif
}

void CIrrDeviceWayland::checkAndUpdateIMEState() {
    if(!m_input_v3)
        return;
    auto* env = getGUIEnvironment();
    if(!env) {
        lastFocusedElement = nullptr;
        if(isEditingText) {
            isEditingText = false;
            zwp_text_input_v3_disable(m_input_v3);
            zwp_text_input_v3_commit(m_input_v3);
        }
    }

    auto updateRectPosition = [&] {
        lastFocusedElementPosition = lastFocusedElement->getAbsolutePosition();
        auto& pos = lastFocusedElementPosition.UpperLeftCorner;

        zwp_text_input_v3_set_cursor_rectangle(m_input_v3, pos.X, pos.Y, lastFocusedElementPosition.getWidth(), lastFocusedElementPosition.getHeight());

        zwp_text_input_v3_commit(m_input_v3);
    };

    irr::gui::IGUIElement* ele = env->getFocus();
    if(lastFocusedElement == ele) {
        auto abs_pos = lastFocusedElement->getAbsolutePosition();
        if(abs_pos == lastFocusedElementPosition)
            return;
        updateRectPosition();
        return;
    }
    isEditingText = (ele && (ele->getType() == irr::gui::EGUIET_EDIT_BOX) && ele->isEnabled());
    lastFocusedElement = ele;

    zwp_text_input_v3_disable(m_input_v3);
    zwp_text_input_v3_commit(m_input_v3);

    if(!isEditingText)
        return;
    zwp_text_input_v3_enable(m_input_v3);
    zwp_text_input_v3_commit(m_input_v3);
    zwp_text_input_v3_enable(m_input_v3);
    zwp_text_input_v3_commit(m_input_v3);

    zwp_text_input_v3_set_content_type(m_input_v3,
                                       ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE,
                                       ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL);

    updateRectPosition();
}

bool CIrrDeviceWayland::initWayland()
{
#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
    do {
        if(!loadEglCoreFunctions()) {
            os::Printer::log("Couldn't load wayland egl core functions.", ELL_ERROR);
            break;
        }
        if(!loadWaylandCursorFunctions()) {
            os::Printer::log("Couldn't load wayland cursor functions.", ELL_ERROR);
            break;
        }
        if(!loadXKBCommonFunctions()) {
            os::Printer::log("Couldn't load XKB common functions.", ELL_ERROR);
            break;
        }
        if(!loadWaylandClientFunctions()) {
            os::Printer::log("Couldn't load wayland client functions.", ELL_ERROR);
            break;
        }
        WaylandLoadCount++;
    } while(0);

    if(WaylandLoadCount == 0) {
        clearWaylandFunctions();
        return false;
    }
#endif

    m_display = pwl_display_connect(nullptr);
    
    if (m_display == nullptr)
    {
        os::Printer::log("Couldn't open display.", ELL_ERROR);
        return false;
    }
    
    m_xkb_context = CIrrDeviceWayland::pxkb_context_new(XKB_CONTEXT_NO_FLAGS);
    
    if (m_xkb_context == nullptr)
    {
        os::Printer::log("Couldn't create xkb context.", ELL_ERROR);
        return false;
    }
    
    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &WaylandCallbacks::registry_listener, 
                             this);
    
    pwl_display_dispatch(m_display);
    pwl_display_roundtrip(m_display);
    
    if (m_compositor == nullptr || m_seat == nullptr || m_output == nullptr)
    {
        os::Printer::log("Important protocols are not available.", ELL_ERROR);
        return false;
    }
    
    if (!m_has_wl_shell && !m_has_xdg_wm_base && !m_has_zxdg_shell)
    {
        os::Printer::log("Shell protocol is not available.", ELL_ERROR);
        return false;
    }

    if (CreationParams.DriverType != video::EDT_NULL)
    {
        if(CreationParams.PrivateData)
            class_name = static_cast<const char*>(CreationParams.PrivateData);
#ifdef IRR_USE_LIBDECOR
		if(!m_decoration_manager && !m_kwin_server_decoration_manager) {
			if(LibdecorLoader::Init()) {
				m_libdecor = LibdecorLoader::libdecor_new(m_display, &WaylandCallbacks::libdecor_interface);

				/* If libdecor works, we don't need xdg-shell anymore. */
				if (m_libdecor) {
					m_has_xdg_wm_base = false;
					m_has_zxdg_shell = false;
					m_has_wl_shell = false;
                } else {
                    LibdecorLoader::Unload();
                    os::Printer::log("Failed to create libdecor instance, no window decorations will be provided.", ELL_ERROR);
                }
			}
		}
#endif
        if (m_has_xdg_wm_base)
        {
            m_xdg_wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(
                    m_registry, m_xdg_wm_base_name, &xdg_wm_base_interface, 1));
                                                 
            xdg_wm_base_add_listener(m_xdg_wm_base, 
                                     &WaylandCallbacks::wm_base_listener, this);
        }
        else if (m_has_zxdg_shell)
        {
            m_zxdg_shell = static_cast<zxdg_shell_v6*>(wl_registry_bind(
                    m_registry, m_zxdg_shell_name, &zxdg_shell_v6_interface, 1));
                                                 
            zxdg_shell_v6_add_listener(m_zxdg_shell, 
                                     &WaylandCallbacks::zxdg_shell_listener, this);
        }
        else if (m_has_wl_shell)
        {
            m_shell = static_cast<wl_shell*>(wl_registry_bind(m_registry, 
                                      m_wl_shell_name, &wl_shell_interface, 1));
        }
        
        bool success = createWindow();
        
        if (!success)
        {
            os::Printer::log("Couldn't create window.", ELL_ERROR);
            return false;
        }
    }
    
    return true;
}


// dbus related code to read the theme and size taken from https://gitlab.gnome.org/jadahl/libdecor,
// licensed under the MIT license
DBusMessage* get_setting_sync(DBusConnection *const connection, const char *key, const char *value) {
	DBusError error;
	uint32_t success;
	DBusMessage *message;
	DBusMessage *reply;

	DbusLoader::dbus_error_init(&error);

	message = DbusLoader::dbus_message_new_method_call(
		"org.freedesktop.portal.Desktop",
		"/org/freedesktop/portal/desktop",
		"org.freedesktop.portal.Settings",
		"Read");

	success = DbusLoader::dbus_message_append_args(message,
		DBUS_TYPE_STRING, &key,
		DBUS_TYPE_STRING, &value,
		DBUS_TYPE_INVALID);

	if (!success)
		return nullptr;

	reply = DbusLoader::dbus_connection_send_with_reply_and_block(
			     connection,
			     message,
			     DBUS_TIMEOUT_USE_DEFAULT,
			     &error);

	DbusLoader::dbus_message_unref(message);

	if (DbusLoader::dbus_error_is_set(&error))
		return nullptr;

	return reply;
}

bool parse_type(DBusMessage *const reply, const int type, void *value) {
	DBusMessageIter iter[3];

	DbusLoader::dbus_message_iter_init(reply, &iter[0]);
	if (DbusLoader::dbus_message_iter_get_arg_type(&iter[0]) != DBUS_TYPE_VARIANT)
		return false;

	DbusLoader::dbus_message_iter_recurse(&iter[0], &iter[1]);
	if (DbusLoader::dbus_message_iter_get_arg_type(&iter[1]) != DBUS_TYPE_VARIANT)
		return false;

	DbusLoader::dbus_message_iter_recurse(&iter[1], &iter[2]);
	if (DbusLoader::dbus_message_iter_get_arg_type(&iter[2]) != type)
		return false;

	DbusLoader::dbus_message_iter_get_basic(&iter[2], value);

	return true;
}

bool libdecor_get_cursor_settings(char*& theme, int& size)  {
	DbusLoader loader;
	if(!loader.Init())
		return false;
	static const char* name = "org.gnome.desktop.interface";
	static const char* key_theme = "cursor-theme";
	static const char* key_size = "cursor-size";

	DBusError error;
	DBusConnection *connection;
	DBusMessage *reply;
	const char* value_theme = nullptr;

	DbusLoader::dbus_error_init(&error);

	connection = DbusLoader::dbus_bus_get(DBUS_BUS_SESSION, &error);

	if (DbusLoader::dbus_error_is_set(&error))
		return false;

	reply = get_setting_sync(connection, name, key_theme);
	if (!reply)
		return false;

	if (!parse_type(reply, DBUS_TYPE_STRING, &value_theme)) {
		DbusLoader::dbus_message_unref(reply);
		return false;
	}

	theme = strdup(value_theme);

	DbusLoader::dbus_message_unref(reply);

	reply = get_setting_sync(connection, name, key_size);
	if (!reply)
		return false;

	if (!parse_type(reply, DBUS_TYPE_INT32, &size)) {
		DbusLoader::dbus_message_unref(reply);
		return false;
	}

	DbusLoader::dbus_message_unref(reply);

	return true;
}

bool CIrrDeviceWayland::createWindow()
{
    m_surface = wl_compositor_create_surface(m_compositor);

    m_egl_window = pwl_egl_window_create(m_surface, m_width, m_height);

    if (m_xdg_wm_base != nullptr)
    {
        m_xdg_surface = xdg_wm_base_get_xdg_surface(m_xdg_wm_base, m_surface);
        
        xdg_surface_add_listener(m_xdg_surface, 
                                 &WaylandCallbacks::surface_listener, this);
                                     
        m_xdg_toplevel = xdg_surface_get_toplevel(m_xdg_surface);

        if(class_name.size() > 0)
            xdg_toplevel_set_app_id(m_xdg_toplevel, class_name.data());
        xdg_toplevel_add_listener(m_xdg_toplevel,
                                  &WaylandCallbacks::toplevel_listener, this);

        wl_surface_commit(m_surface);
        pwl_display_flush(m_display);
                                    
        if (CreationParams.Fullscreen)
        {
            xdg_toplevel_set_fullscreen(m_xdg_toplevel, nullptr);
        }
        
        xdg_surface_set_window_geometry(m_xdg_surface, 0, 0, m_width, m_height);
                                    
        while (!m_surface_configured)
        {
            pwl_display_flush(m_display);
            pwl_display_dispatch(m_display);
            usleep(1000);
        }
    } else if (m_zxdg_shell != nullptr)
    {
        m_zxdg_surface = zxdg_shell_v6_get_xdg_surface(m_zxdg_shell, m_surface);
        
        zxdg_surface_v6_add_listener(m_zxdg_surface, 
                                 &WaylandCallbacks::zxdg_surface_listener, this);
                                     
        m_zxdg_toplevel = zxdg_surface_v6_get_toplevel(m_zxdg_surface);

        if(class_name.size() > 0)
            zxdg_toplevel_v6_set_app_id(m_zxdg_toplevel, class_name.data());
        zxdg_toplevel_v6_add_listener(m_zxdg_toplevel,
                                  &WaylandCallbacks::zxdg_toplevel_listener, this);

        wl_surface_commit(m_surface);
        pwl_display_flush(m_display);
                                    
        if (CreationParams.Fullscreen)
        {
            zxdg_toplevel_v6_set_fullscreen(m_zxdg_toplevel, nullptr);
        }
        
        zxdg_surface_v6_set_window_geometry(m_zxdg_surface, 0, 0, m_width, m_height);
                                    
        while (!m_surface_configured)
        {
            pwl_display_flush(m_display);
            pwl_display_dispatch(m_display);
            usleep(1000);
        }
    }
    else if (m_shell != nullptr)
    {
        m_shell_surface = wl_shell_get_shell_surface(m_shell, m_surface);

        wl_shell_surface_add_listener(m_shell_surface,
                                      &WaylandCallbacks::shell_surface_listener, 
                                      this);

        if (CreationParams.Fullscreen)
        {
            wl_shell_surface_set_fullscreen(m_shell_surface,
                       WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, m_output);
        }
        else
        {
            wl_shell_surface_set_toplevel(m_shell_surface);
        }
    }
#ifdef IRR_USE_LIBDECOR
	else if (m_libdecor != nullptr)
	{
        m_libdecor_surface = LibdecorLoader::libdecor_decorate(m_libdecor,
												m_surface,
												&WaylandCallbacks::libdecor_frame_interface,
												this);
        if (m_libdecor_surface == nullptr) {
			os::Printer::log("CFailed to create libdecor frame!", ELL_ERROR);
        } else {
			if(class_name.size() > 0)
				LibdecorLoader::libdecor_frame_set_app_id(m_libdecor_surface, class_name.data());
            LibdecorLoader::libdecor_frame_map(m_libdecor_surface);
        }
		                       
        while (!m_surface_configured)
        {
            pwl_display_flush(m_display);
            pwl_display_dispatch(m_display);
            usleep(1000);
        }
	}
#endif
    else
    {
        os::Printer::log("Cannot create shell surface.", ELL_ERROR);
        return false;
    }
        
    if (m_decoration_manager != nullptr)
    {
        m_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
                                    m_decoration_manager, m_xdg_toplevel);
    }
                                                       
    if (m_decoration != nullptr)
    {
        zxdg_toplevel_decoration_v1_set_mode(m_decoration, 
                            ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    } else {
		if (m_kwin_server_decoration_manager != nullptr)
		{
			m_kwin_server_decoration = org_kde_kwin_server_decoration_manager_create(
										m_kwin_server_decoration_manager, m_surface);
		}
			
														   
		if (m_kwin_server_decoration != nullptr)
		{
			org_kde_kwin_server_decoration_request_mode(m_kwin_server_decoration, ORG_KDE_KWIN_SERVER_DECORATION_MANAGER_MODE_SERVER);
		}
	}

    wl_region* region = wl_compositor_create_region(m_compositor);
    wl_region_add(region, 0, 0, m_width, m_height);
    wl_surface_set_opaque_region(m_surface, region);
    wl_region_destroy(region);

    if (m_shm)
    {
        m_cursor_surface = wl_compositor_create_surface(m_compositor);
        int size = 0;
		char* theme = nullptr;
		bool free_theme = true;
		if(!libdecor_get_cursor_settings(theme, size)) {
			auto xcursor_size = getenv("XCURSOR_SIZE");
			theme = getenv("XCURSOR_THEME");
			if (xcursor_size != nullptr)
				size = atoi(xcursor_size);
			if (size <= 0)
				size = 24;
			free_theme = false;
		}
        m_cursor_theme = pwl_cursor_theme_load(theme, size, m_shm);
		if(free_theme)
			free(theme);
    }

    if (!m_cursor_theme)
    {
        os::Printer::log("Couldn't load cursor theme.", ELL_ERROR);
    }

    pwl_display_flush(m_display);

    return true;
}

void CIrrDeviceWayland::createDriver()
{
    switch(CreationParams.DriverType)
    {
    case video::EDT_OPENGL:
	{
        #ifdef _IRR_COMPILE_WITH_OPENGL_
        video::SExposedVideoData data;
        data.OpenGLWayland.EGLWindow = m_egl_window;
        data.OpenGLWayland.EGLDisplay = m_display;
        ContextManager = new video::CEGLManager(this);
        if(!ContextManager->initialize(CreationParams, data)) {
            os::Printer::log("Failed to initialize OpenGL context.", ELL_ERROR);
            break;
        }
        VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, ContextManager);
        #else
        os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
        #endif
        break;
	}
    case video::EDT_OGLES1:
	{
        #ifdef _IRR_COMPILE_WITH_OGLES1_
        video::SExposedVideoData data;
        data.OpenGLWayland.EGLWindow = m_egl_window;
        data.OpenGLWayland.EGLDisplay = m_display;
        ContextManager = new video::CEGLManager(this);
        if(!ContextManager->initialize(CreationParams, data)) {
            os::Printer::log("Failed to initialize OpenGL-ES1 context.", ELL_ERROR);
            break;
        }
        VideoDriver = video::createOGLES1Driver(CreationParams, FileSystem, ContextManager);
        #else
        os::Printer::log("No OpenGL-ES1 support compiled in.", ELL_ERROR);
        #endif
        break;
	}
    case video::EDT_OGLES2:
	{
        #ifdef _IRR_COMPILE_WITH_OGLES2_
        video::SExposedVideoData data;
        data.OpenGLWayland.EGLWindow = m_egl_window;
        data.OpenGLWayland.EGLDisplay = m_display;
        ContextManager = new video::CEGLManager(this);
        if(!ContextManager->initialize(CreationParams, data)) {
            os::Printer::log("Failed to initialize OpenGL-ES2 context.", ELL_ERROR);
            break;
        }
        VideoDriver = video::createOGLES2Driver(CreationParams, FileSystem, ContextManager);
        #else
        os::Printer::log("No OpenGL-ES2 support compiled in.", ELL_ERROR);
        #endif
        break;
	}
    case video::EDT_NULL:
        VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
        break;
    default:
        os::Printer::log("Wayland driver only supports OpenGL, OpenGL-ES1 and OpenGL-ES2", ELL_ERROR);
        break;
    }
}

void CIrrDeviceWayland::checkPendingResizes()
{
    if(!m_resizing_state.pending)
        return;
    m_width = m_resizing_state.width;
    m_height = m_resizing_state.height;
    getVideoDriver()->OnResize(irr::core::dimension2du((u32)m_width, (u32)m_height));

    //wl_surface_set_buffer_scale(data->surface, data->scale_factor);
    pwl_egl_window_resize(m_egl_window, m_width, m_height, 0, 0);

    if(m_resizing_state.configure) {
        if(m_xdg_surface) {
            xdg_surface_set_window_geometry(m_xdg_surface, 0, 0, m_width, m_height);
            xdg_surface_ack_configure(m_xdg_surface, m_resizing_state.serial);
        } else if(m_zxdg_surface) {
            zxdg_surface_v6_set_window_geometry(m_zxdg_surface, 0, 0, m_width, m_height);
            zxdg_surface_v6_ack_configure(m_zxdg_surface, m_resizing_state.serial);
        }
        m_resizing_state.configure = false;
    }

    wl_region* region = wl_compositor_create_region(m_compositor);
    wl_region_add(region, 0, 0, m_width, m_height);
    wl_surface_set_opaque_region(m_surface, region);
    wl_region_destroy(region);

    m_resizing_state.pending = false;
    pwl_display_flush(m_display);
}

void CIrrDeviceWayland::updateCursor()
{
    if (!m_pointer)
        return;

    if (!getCursorControl()->isVisible())
    {
        wl_pointer_set_cursor(m_pointer, m_enter_serial, nullptr, 0, 0);
    }
    else
    {
        getCursorControl()->setActiveIcon(getCursorControl()->getActiveIcon());
    }
}

void CIrrDeviceWayland::signalEvent(const SEvent &event)
{
    m_events.push_back(event);
}

//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceWayland::run()
{
    if(!VideoDriver)
        return false;
    os::Timer::tick();

    checkAndUpdateIMEState();

    if (pwl_display_dispatch_pending(m_display) == -1)
    {
        closeDevice();
    }

    for (unsigned int i = 0; i < m_events.size(); i++)
    {
        postEventFromUser(m_events[i]);
    }

    m_events.clear();

    if (m_repeat_enabled && m_repeat_rate > 0)
    {
        uint32_t curr_time = os::Timer::getRealTime();

        while (curr_time - m_repeat_time > m_repeat_delay + m_repeat_rate)
        {
            postEventFromUser(m_repeat_event);
            m_repeat_time += m_repeat_rate;
        }
    }

    if (!Close)
    {
        pollJoysticks();
    }

    return !Close;
}

//! Pause the current process for the minimum time allowed only to allow other
//! processes to execute
void CIrrDeviceWayland::yield()
{
    struct timespec ts = {0,1};
    nanosleep(&ts, nullptr);
}

//! Pause execution and let other processes to run for a specified amount of time.
void CIrrDeviceWayland::sleep(u32 timeMs, bool pauseTimer = false)
{
    const bool wasStopped = Timer ? Timer->isStopped() : true;

    struct timespec ts;
    ts.tv_sec = (time_t) (timeMs / 1000);
    ts.tv_nsec = (long) (timeMs % 1000)*  1000000;

    if (pauseTimer && !wasStopped)
    {
        Timer->stop();
    }

    nanosleep(&ts, nullptr);

    if (pauseTimer && !wasStopped)
    {
        Timer->start();
    }
}

//! sets the caption of the window
void CIrrDeviceWayland::setWindowCaption(const wchar_t* text)
{
    const size_t lenOld = (wcslen(text) + 1) * sizeof(wchar_t);
    char* title = new char[lenOld];
    core::wcharToUtf8(text, title, lenOld);

    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_title(m_xdg_toplevel, title);
    }
    else if (m_zxdg_toplevel)
    {
        zxdg_toplevel_v6_set_title(m_zxdg_toplevel, title);
    }
    else if (m_shell_surface)
    {
        wl_shell_surface_set_title(m_shell_surface, title);
    }
#ifdef IRR_USE_LIBDECOR
	else if (m_libdecor)
	{
		LibdecorLoader::libdecor_frame_set_title(m_libdecor_surface, title);
	}
#endif

    pwl_display_flush(m_display);

    delete[] title;
}

//! presents a surface in the client area
bool CIrrDeviceWayland::present(video::IImage* image, void* windowId,
                                core::rect<s32>* srcRect)
{
    return true;
}

//! notifies the device that it should close itself
void CIrrDeviceWayland::closeDevice()
{
    Close = true;
}

//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceWayland::isWindowActive() const
{
    return (m_window_has_focus && !m_window_minimized);
}

//! returns if window has focus.
bool CIrrDeviceWayland::isWindowFocused() const
{
    return m_window_has_focus;
}

//! returns if window is minimized.
bool CIrrDeviceWayland::isWindowMinimized() const
{
    return m_window_minimized;
}

//! returns color format of the window.
video::ECOLOR_FORMAT CIrrDeviceWayland::getColorFormat() const
{
    return video::ECF_R8G8B8;
}

//! Sets if the window should be resizable in windowed mode.
void CIrrDeviceWayland::setResizable(bool resize)
{
    CreationParams.WindowResizable = resize;
    int width = resize ? 0 : m_width;
    int height = resize ? 0 : m_height;
    if (m_xdg_toplevel)
    {        
        xdg_toplevel_set_min_size(m_xdg_toplevel, width, height);
        xdg_toplevel_set_max_size(m_xdg_toplevel, width, height);
    } else if(m_zxdg_shell) {
        zxdg_toplevel_v6_set_min_size(m_zxdg_toplevel, width, height);
        zxdg_toplevel_v6_set_max_size(m_zxdg_toplevel, width, height);
    }
}

//! Return pointer to a list with all video modes supported by the gfx adapter.
video::IVideoModeList* CIrrDeviceWayland::getVideoModeList()
{
    return VideoModeList;
}

//! Minimize window
void CIrrDeviceWayland::minimizeWindow()
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_minimized(m_xdg_toplevel);
    } else if(m_zxdg_shell) {
        zxdg_toplevel_v6_set_minimized(m_zxdg_toplevel);
    }
#ifdef IRR_USE_LIBDECOR
	else if (m_libdecor)
	{
		LibdecorLoader::libdecor_frame_set_minimized(m_libdecor_surface);
	}
#endif
}

//! Maximize window
void CIrrDeviceWayland::maximizeWindow()
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_maximized(m_xdg_toplevel);
    } else if(m_zxdg_shell) {
        zxdg_toplevel_v6_set_maximized(m_zxdg_toplevel);
    }
#ifdef IRR_USE_LIBDECOR
	else if (m_libdecor)
	{
		LibdecorLoader::libdecor_frame_set_maximized(m_libdecor_surface);
	}
#endif
}

//! Restore original window size
void CIrrDeviceWayland::restoreWindow()
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_unset_maximized(m_xdg_toplevel);
    } else if(m_zxdg_shell) {
        zxdg_toplevel_v6_unset_maximized(m_zxdg_toplevel);
    }
#ifdef IRR_USE_LIBDECOR
	else if (m_libdecor)
	{
		LibdecorLoader::libdecor_frame_unset_maximized(m_libdecor_surface);
	}
#endif
}

//! Restore original window size
void CIrrDeviceWayland::toggleFullscreen(bool fullscreen)
{
    if (m_xdg_toplevel)
    {
		if(fullscreen)
			xdg_toplevel_set_fullscreen(m_xdg_toplevel, nullptr);
		else
			xdg_toplevel_unset_fullscreen(m_xdg_toplevel);
    } else if(m_zxdg_shell) {
		if(fullscreen)
			zxdg_toplevel_v6_set_fullscreen(m_zxdg_toplevel, nullptr);
		else
			zxdg_toplevel_v6_unset_fullscreen(m_zxdg_toplevel);
    }
#ifdef IRR_USE_LIBDECOR
	else if (m_libdecor)
	{
		if(fullscreen)
			LibdecorLoader::libdecor_frame_set_fullscreen(m_libdecor_surface, nullptr);
		else
			LibdecorLoader::libdecor_frame_unset_fullscreen(m_libdecor_surface);
	}
#endif
}

//! Move window to requested position
bool CIrrDeviceWayland::moveWindow(int x, int y)
{
    return false;
}

//! Get current window position.
core::position2di CIrrDeviceWayland::getWindowPosition()
{
    return core::position2di{};
}

//! Set the current Gamma Value for the Display
bool CIrrDeviceWayland::setGammaRamp(f32 red, f32 green, f32 blue,
                                     f32 brightness, f32 contrast)
{
    return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceWayland::getGammaRamp(f32 &red, f32 &green, f32 &blue,
                                     f32 &brightness, f32 &contrast)
{
    brightness = 0.0f;
    contrast = 0.0f;
    return false;
}

//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const c8* CIrrDeviceWayland::getTextFromClipboard() const
{
    if(!m_clipboard_changed)
        return m_readclipboard.c_str();

    m_readclipboard.clear();

    if(!m_clipboard_data_offer)
        return m_readclipboard.c_str();

    if((m_clipboard_mime & (CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT |
                           CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8 |
                           CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2)) == 0)
        return m_readclipboard.c_str();

    int pipefd[2];
    if(pipe2(pipefd, O_CLOEXEC | O_NONBLOCK) == -1)
        return m_readclipboard.c_str();
    wl_data_offer_receive(m_clipboard_data_offer,
                          (m_clipboard_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8) ? "text/plain;charset=UTF-8" :
                          (m_clipboard_mime & CIrrDeviceWayland::DATA_MIME::PLAIN_TEXT_UTF8_2) ? "text/plain;charset=utf-8" :
                          "text/plain",
                          pipefd[1]);

    pwl_display_flush(m_display);
    pwl_display_roundtrip(m_display);
    close(pipefd[1]);

    while(true) {
		auto ready = PipeReady(pipefd[0], IOR::READ);
		if (ready <= 0)
			break;
        char buf[PIPE_BUF];
        ssize_t n = read(pipefd[0], buf, sizeof(buf));
		if(n <= 0)
            break;
        m_readclipboard.append(buf, n);
	}
    m_readclipboard.append("", 0);
	
    close(pipefd[0]);
    m_clipboard_changed = false;
    return m_readclipboard.c_str();
}

//! copies text to the clipboard
void CIrrDeviceWayland::copyToClipboard(const c8* text)
{
    m_clipboard = text;
    if(m_clipboard.empty()) {
        wl_data_device_set_selection(m_data_device, nullptr, 0);
        m_data_source = nullptr;
    } else {
        m_data_source = wl_data_device_manager_create_data_source(m_data_device_manager);
        wl_data_source_set_user_data(m_data_source, const_cast<CIrrDeviceWayland*>(this));
        wl_data_source_add_listener(m_data_source, &WaylandCallbacks::data_source_listener, const_cast<CIrrDeviceWayland*>(this));
        wl_data_source_offer(m_data_source, "text/plain");
        wl_data_source_offer(m_data_source, "text/plain;charset=UTF-8");
        wl_data_source_offer(m_data_source, "text/plain;charset=utf-8");
        if(m_selection_serial) {
            wl_data_device_set_selection(m_data_device, m_data_source, m_selection_serial);
        }
    }
}

//! Remove all messages pending in the system message loop
void CIrrDeviceWayland::clearSystemMessages()
{
}

void CIrrDeviceWayland::enableDragDrop(bool enable, drop_callback_function_t dragCheck)
{
    if(enable)
        m_drag_and_drop_check = dragCheck;
    else
        m_drag_and_drop_check = nullptr;
}

void CIrrDeviceWayland::createKeyMap()
{
    m_key_map[KEY_RESERVED] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_ESC] = (EKEY_CODE)IRR_KEY_ESCAPE;
    m_key_map[KEY_1] = (EKEY_CODE)IRR_KEY_1;
    m_key_map[KEY_2] = (EKEY_CODE)IRR_KEY_2;
    m_key_map[KEY_3] = (EKEY_CODE)IRR_KEY_3;
    m_key_map[KEY_4] = (EKEY_CODE)IRR_KEY_4;
    m_key_map[KEY_5] = (EKEY_CODE)IRR_KEY_5;
    m_key_map[KEY_6] = (EKEY_CODE)IRR_KEY_6;
    m_key_map[KEY_7] = (EKEY_CODE)IRR_KEY_7;
    m_key_map[KEY_8] = (EKEY_CODE)IRR_KEY_8;
    m_key_map[KEY_9] = (EKEY_CODE)IRR_KEY_9;
    m_key_map[KEY_0] = (EKEY_CODE)IRR_KEY_0;
    m_key_map[KEY_MINUS] = (EKEY_CODE)IRR_KEY_MINUS;
    m_key_map[KEY_EQUAL] = (EKEY_CODE)IRR_KEY_PLUS;
    m_key_map[KEY_BACKSPACE] = (EKEY_CODE)IRR_KEY_BACK;
    m_key_map[KEY_TAB] = (EKEY_CODE)IRR_KEY_TAB;
    m_key_map[KEY_Q] = (EKEY_CODE)IRR_KEY_Q;
    m_key_map[KEY_W] = (EKEY_CODE)IRR_KEY_W;
    m_key_map[KEY_E] = (EKEY_CODE)IRR_KEY_E;
    m_key_map[KEY_R] = (EKEY_CODE)IRR_KEY_R;
    m_key_map[KEY_T] = (EKEY_CODE)IRR_KEY_T;
    m_key_map[KEY_Y] = (EKEY_CODE)IRR_KEY_Y;
    m_key_map[KEY_U] = (EKEY_CODE)IRR_KEY_U;
    m_key_map[KEY_I] = (EKEY_CODE)IRR_KEY_I;
    m_key_map[KEY_P] = (EKEY_CODE)IRR_KEY_P;
    m_key_map[KEY_O] = (EKEY_CODE)IRR_KEY_O;
    m_key_map[KEY_LEFTBRACE] = (EKEY_CODE)IRR_KEY_OEM_4;
    m_key_map[KEY_RIGHTBRACE] = (EKEY_CODE)IRR_KEY_OEM_6;
    m_key_map[KEY_ENTER] = (EKEY_CODE)IRR_KEY_RETURN;
    m_key_map[KEY_LEFTCTRL] = (EKEY_CODE)IRR_KEY_LCONTROL;
    m_key_map[KEY_A] = (EKEY_CODE)IRR_KEY_A;
    m_key_map[KEY_S] = (EKEY_CODE)IRR_KEY_S; 
    m_key_map[KEY_D] = (EKEY_CODE)IRR_KEY_D;
    m_key_map[KEY_F] = (EKEY_CODE)IRR_KEY_F;
    m_key_map[KEY_G] = (EKEY_CODE)IRR_KEY_G;
    m_key_map[KEY_H] = (EKEY_CODE)IRR_KEY_H;
    m_key_map[KEY_J] = (EKEY_CODE)IRR_KEY_J;
    m_key_map[KEY_K] = (EKEY_CODE)IRR_KEY_K;
    m_key_map[KEY_L] = (EKEY_CODE)IRR_KEY_L;
    m_key_map[KEY_SEMICOLON] = (EKEY_CODE)IRR_KEY_OEM_1;
    m_key_map[KEY_APOSTROPHE] = (EKEY_CODE)IRR_KEY_OEM_7;
    m_key_map[KEY_GRAVE] = (EKEY_CODE)IRR_KEY_OEM_3;
    m_key_map[KEY_LEFTSHIFT] = (EKEY_CODE)IRR_KEY_LSHIFT;
    m_key_map[KEY_BACKSLASH] = (EKEY_CODE)IRR_KEY_OEM_5;
    m_key_map[KEY_Z] = (EKEY_CODE)IRR_KEY_Z;
    m_key_map[KEY_X] = (EKEY_CODE)IRR_KEY_X;
    m_key_map[KEY_C] = (EKEY_CODE)IRR_KEY_C;
    m_key_map[KEY_V] = (EKEY_CODE)IRR_KEY_V;
    m_key_map[KEY_B] = (EKEY_CODE)IRR_KEY_B;
    m_key_map[KEY_N] = (EKEY_CODE)IRR_KEY_N;
    m_key_map[KEY_M] = (EKEY_CODE)IRR_KEY_M;
    m_key_map[KEY_COMMA] = (EKEY_CODE)IRR_KEY_COMMA;
    m_key_map[KEY_DOT] = (EKEY_CODE)IRR_KEY_PERIOD;
    m_key_map[KEY_SLASH] = (EKEY_CODE)IRR_KEY_OEM_2; 
    m_key_map[KEY_RIGHTSHIFT] = (EKEY_CODE)IRR_KEY_RSHIFT;
    m_key_map[KEY_KPASTERISK] = (EKEY_CODE)IRR_KEY_MULTIPLY;
    m_key_map[KEY_LEFTALT] = (EKEY_CODE)IRR_KEY_LMENU;
    m_key_map[KEY_SPACE] = (EKEY_CODE)IRR_KEY_SPACE;
    m_key_map[KEY_CAPSLOCK] = (EKEY_CODE)IRR_KEY_CAPITAL;
    m_key_map[KEY_F1] = (EKEY_CODE)IRR_KEY_F1;
    m_key_map[KEY_F2] = (EKEY_CODE)IRR_KEY_F2;
    m_key_map[KEY_F3] = (EKEY_CODE)IRR_KEY_F3;
    m_key_map[KEY_F4] = (EKEY_CODE)IRR_KEY_F4;
    m_key_map[KEY_F5] = (EKEY_CODE)IRR_KEY_F5;
    m_key_map[KEY_F6] = (EKEY_CODE)IRR_KEY_F6;
    m_key_map[KEY_F7] = (EKEY_CODE)IRR_KEY_F7;
    m_key_map[KEY_F8] = (EKEY_CODE)IRR_KEY_F8;
    m_key_map[KEY_F9] = (EKEY_CODE)IRR_KEY_F9;
    m_key_map[KEY_F10] = (EKEY_CODE)IRR_KEY_F10;
    m_key_map[KEY_NUMLOCK] = (EKEY_CODE)IRR_KEY_NUMLOCK;
    m_key_map[KEY_SCROLLLOCK] = (EKEY_CODE)IRR_KEY_SCROLL;
    m_key_map[KEY_KP7] = (EKEY_CODE)IRR_KEY_NUMPAD7;
    m_key_map[KEY_KP8] = (EKEY_CODE)IRR_KEY_NUMPAD8;
    m_key_map[KEY_KP9] = (EKEY_CODE)IRR_KEY_NUMPAD9;
    m_key_map[KEY_KPMINUS] = (EKEY_CODE)IRR_KEY_SUBTRACT;
    m_key_map[KEY_KP4] = (EKEY_CODE)IRR_KEY_NUMPAD4;
    m_key_map[KEY_KP5] = (EKEY_CODE)IRR_KEY_NUMPAD5;
    m_key_map[KEY_KP6] = (EKEY_CODE)IRR_KEY_NUMPAD6;
    m_key_map[KEY_KPPLUS] = (EKEY_CODE)IRR_KEY_ADD;
    m_key_map[KEY_KP1] = (EKEY_CODE)IRR_KEY_NUMPAD1;
    m_key_map[KEY_KP2] = (EKEY_CODE)IRR_KEY_NUMPAD2;
    m_key_map[KEY_KP3] = (EKEY_CODE)IRR_KEY_NUMPAD3;
    m_key_map[KEY_KP0] = (EKEY_CODE)IRR_KEY_NUMPAD0;
    m_key_map[KEY_KPDOT] = (EKEY_CODE)IRR_KEY_SEPARATOR;
    m_key_map[KEY_ZENKAKUHANKAKU] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_102ND] = (EKEY_CODE)IRR_KEY_OEM_102;
    m_key_map[KEY_F11] = (EKEY_CODE)IRR_KEY_F11;
    m_key_map[KEY_F12] = (EKEY_CODE)IRR_KEY_F12;
    m_key_map[KEY_RO] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_KATAKANA] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_HIRAGANA] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_HENKAN] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_KATAKANAHIRAGANA] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_MUHENKAN] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_KPJPCOMMA] = (EKEY_CODE)IRR_KEY_SEPARATOR;
    m_key_map[KEY_KPENTER] = (EKEY_CODE)IRR_KEY_RETURN;
    m_key_map[KEY_RIGHTCTRL] = (EKEY_CODE)IRR_KEY_RCONTROL;
    m_key_map[KEY_KPSLASH] = (EKEY_CODE)IRR_KEY_DIVIDE;
    m_key_map[KEY_SYSRQ] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_RIGHTALT] = (EKEY_CODE)IRR_KEY_RMENU;
    m_key_map[KEY_LINEFEED] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_HOME] = (EKEY_CODE)IRR_KEY_HOME;
    m_key_map[KEY_UP] = (EKEY_CODE)IRR_KEY_UP;
    m_key_map[KEY_PAGEUP] = (EKEY_CODE)IRR_KEY_PRIOR;
    m_key_map[KEY_LEFT] = (EKEY_CODE)IRR_KEY_LEFT;
    m_key_map[KEY_RIGHT] = (EKEY_CODE)IRR_KEY_RIGHT;
    m_key_map[KEY_END] = (EKEY_CODE)IRR_KEY_END;
    m_key_map[KEY_DOWN] = (EKEY_CODE)IRR_KEY_DOWN;
    m_key_map[KEY_PAGEDOWN] = (EKEY_CODE)IRR_KEY_NEXT;
    m_key_map[KEY_INSERT] = (EKEY_CODE)IRR_KEY_INSERT;
    m_key_map[KEY_DELETE] = (EKEY_CODE)IRR_KEY_DELETE;
    m_key_map[KEY_MACRO] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_MUTE] = (EKEY_CODE)IRR_KEY_VOLUME_MUTE;
    m_key_map[KEY_VOLUMEDOWN] = (EKEY_CODE)IRR_KEY_VOLUME_DOWN;
    m_key_map[KEY_VOLUMEUP] = (EKEY_CODE)IRR_KEY_VOLUME_UP;
    m_key_map[KEY_POWER] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_KPEQUAL] = (EKEY_CODE)IRR_KEY_RETURN;
    m_key_map[KEY_KPPLUSMINUS] = (EKEY_CODE)IRR_KEY_PLUS;
    m_key_map[KEY_PAUSE] = (EKEY_CODE)IRR_KEY_PAUSE;
    m_key_map[KEY_SCALE] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_KPCOMMA] = (EKEY_CODE)IRR_KEY_COMMA;
    m_key_map[KEY_HANGEUL] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_HANJA] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_YEN] = (EKEY_CODE)IRR_KEY_UNKNOWN;
    m_key_map[KEY_LEFTMETA] = (EKEY_CODE)IRR_KEY_LWIN;
    m_key_map[KEY_RIGHTMETA] = (EKEY_CODE)IRR_KEY_RWIN;
    m_key_map[KEY_COMPOSE] = (EKEY_CODE)IRR_KEY_MENU;
}

// The joystick code is mostly copied from CIrrDeviceLinux.

bool CIrrDeviceWayland::activateJoysticks(core::array<SJoystickInfo>& joystickInfo)
{
#undef _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
#if defined (_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)

    joystickInfo.clear();

    u32 joystick;
    for (joystick = 0; joystick < 32; ++joystick)
    {
        // The joystick device could be here...
        core::stringc devName = "/dev/js";
        devName += joystick;

        JoystickInfo info;
        info.fd = open(devName.c_str(), O_RDONLY);
        if (info.fd == -1)
        {
            // ...but Ubuntu and possibly other distros
            // create the devices in /dev/input
            devName = "/dev/input/js";
            devName += joystick;
            info.fd = open(devName.c_str(), O_RDONLY);
        }

        if (info.fd == -1)
        {
            // and BSD here
            devName = "/dev/joy";
            devName += joystick;
            info.fd = open(devName.c_str(), O_RDONLY);
        }

        if (info.fd == -1)
            continue;

#ifdef __FreeBSD__
        info.axes=2;
        info.buttons=2;
#else
        ioctl( info.fd, JSIOCGAXES, &(info.axes) );
        ioctl( info.fd, JSIOCGBUTTONS, &(info.buttons) );
        fcntl( info.fd, F_SETFL, O_NONBLOCK );
#endif

        (void)memset(&info.persistentData, 0, sizeof(info.persistentData));
        info.persistentData.EventType = irr::EET_JOYSTICK_INPUT_EVENT;
        info.persistentData.JoystickEvent.Joystick = m_active_joysticks.size();

        // There's no obvious way to determine which (if any) axes represent a POV
        // hat, so we'll just set it to "not used" and forget about it.
        info.persistentData.JoystickEvent.POV = 65535;

        m_active_joysticks.push_back(info);

        SJoystickInfo returnInfo;
        returnInfo.HasGenericName = false;
        returnInfo.Joystick = joystick;
        returnInfo.PovHat = SJoystickInfo::POV_HAT_UNKNOWN;
        returnInfo.Axes = info.axes;
        returnInfo.Buttons = info.buttons;

#ifndef __FreeBSD__
        char name[80];
        ioctl( info.fd, JSIOCGNAME(80), name);
        returnInfo.Name = name;
#endif

        joystickInfo.push_back(returnInfo);
    }

    for (joystick = 0; joystick < joystickInfo.size(); ++joystick)
    {
        char logString[256];
        (void)sprintf(logString, "Found joystick %u, %u axes, %u buttons '%s'",
            joystick, joystickInfo[joystick].Axes,
            joystickInfo[joystick].Buttons, joystickInfo[joystick].Name.c_str());
        os::Printer::log(logString, ELL_INFORMATION);
    }

    return true;
#else
    return false;
#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
}


void CIrrDeviceWayland::pollJoysticks()
{
#if defined (_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
    if (m_active_joysticks.size() == 0)
        return;

    for (unsigned int i = 0; i < m_active_joysticks.size(); i++)
    {
        JoystickInfo& info = m_active_joysticks[i];

#ifdef __FreeBSD__
        struct joystick js;
        if (read(info.fd, &js, sizeof(js)) == sizeof(js))
        {
            /* should be a two-bit field*/
            info.persistentData.JoystickEvent.ButtonStates = js.b1 | (js.b2 << 1);
            info.persistentData.JoystickEvent.Axis[0] = js.x; /* X axis*/
            info.persistentData.JoystickEvent.Axis[1] = js.y; /* Y axis*/
        }
#else
        struct js_event event;
        while (sizeof(event) == read(info.fd, &event, sizeof(event)))
        {
            switch(event.type & ~JS_EVENT_INIT)
            {
            case JS_EVENT_BUTTON:
            {
                if (event.value)
                {
                    info.persistentData.JoystickEvent.ButtonStates |= (1 << event.number);
                }
                else
                {
                    info.persistentData.JoystickEvent.ButtonStates &= ~(1 << event.number);
                }
                break;
            }
            case JS_EVENT_AXIS:
            {
                if (event.number < SEvent::SJoystickEvent::NUMBER_OF_AXES)
                {
                    info.persistentData.JoystickEvent.Axis[event.number] = event.value;
                }
                break;
            }
            default:
                break;
            }
        }
#endif

        // Send an irrlicht joystick event once per ::run() even if no new data were received.
        (void)postEventFromUser(info.persistentData);
    }
#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
}

void CIrrDeviceWayland::closeJoysticks()
{
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
    for (unsigned int i = 0; i < m_active_joysticks.size(); i++)
    {
        if (m_active_joysticks[i].fd < 0)
            continue;

        close(m_active_joysticks[i].fd);
    }
#endif
}

void CIrrDeviceWayland::setSelectionSerial(uint32_t serial) {
    m_selection_serial = serial;
    if(m_selection_serial == 0 && m_data_source != nullptr) {
        wl_data_device_set_selection(m_data_device, m_data_source, serial);
    }
}
#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_

#define WAYLAND_EGL_CORE
bool CIrrDeviceWayland::loadEglCoreFunctions() {
    if(LibWaylandEGL) {
        WaylandLoadCount++;
        return true;
    }
    if((LibWaylandEGL = dlopen("libwayland-egl.so.1", RTLD_NOW)) == nullptr) {
        os::Printer::log("Couldn't load libwayland-egl.so.1.", ELL_ERROR);
        return false;
    }
    do {
#define WAYLAND_FUNC(name, ret_type, ...) p##name = (ret_type(*)(__VA_ARGS__))dlsym(LibWaylandEGL, #name); if(!p##name) break;
#include "CWaylandFunctions.inl"
#undef WAYLAND_FUNC
        WaylandLoadCount++;
        return true;
    } while(0);
    return false;
}
#undef WAYLAND_EGL_CORE

#define WAYLAND_CURSOR
bool CIrrDeviceWayland::loadWaylandCursorFunctions() {
    if(LibWaylandCursor)
        return true;

    if((LibWaylandCursor = dlopen("libwayland-cursor.so.0", RTLD_NOW)) == nullptr) {
        os::Printer::log("Couldn't load libwayland-cursor.so.0.", ELL_ERROR);
        return false;
    }
    do {
#define WAYLAND_FUNC(name, ret_type, ...) p##name = (ret_type(*)(__VA_ARGS__))dlsym(LibWaylandCursor, #name); if(!p##name) break;
#include "CWaylandFunctions.inl"
#undef WAYLAND_FUNC
        return true;
    } while(0);
    return false;
}
#undef WAYLAND_CURSOR

#define XKB_COMMMON
bool CIrrDeviceWayland::loadXKBCommonFunctions() {
    if(LibXKBCommon)
        return true;

    if((LibXKBCommon = dlopen("libxkbcommon.so.0", RTLD_NOW)) == nullptr) {
        os::Printer::log("Couldn't load libxkbcommon.so.0.", ELL_ERROR);
        return false;
    }
    do {
#define WAYLAND_FUNC(name, ret_type, ...) p##name = (ret_type(*)(__VA_ARGS__))dlsym(LibXKBCommon, #name); if(!p##name) break;
#include "CWaylandFunctions.inl"
#undef WAYLAND_FUNC
        return true;
    } while(0);
    return false;
}
#undef XKB_COMMMON

#define WAYLAND_CLIENT
bool CIrrDeviceWayland::loadWaylandClientFunctions() {
    if(LibWaylandClient)
        return true;

    if((LibWaylandClient = dlopen("libwayland-client.so.0", RTLD_NOW)) == nullptr) {
        os::Printer::log("Couldn't load libwayland-client.so.0.", ELL_ERROR);
        return false;
    }
    do {
#define WAYLAND_FUNC(name, ret_type, ...) p##name = (ret_type(*)(__VA_ARGS__))dlsym(LibWaylandClient, #name); if(!p##name) break;
#define WAYLAND_INTERFACE(name) p##name = (wl_interface *)dlsym(LibWaylandClient, #name); if(!p##name) break;
#include "CWaylandFunctions.inl"
#undef WAYLAND_INTERFACE
#undef WAYLAND_FUNC
#define ONLY_PROXY
#define WAYLAND_FUNC(name, ret_type, ...) irr__internal__p__##name = p##name;
#define WAYLAND_INTERFACE(name) irr__internal__p__##name = p##name;
#include "CWaylandFunctions.inl"
#undef ONLY_PROXY
#undef WAYLAND_INTERFACE
#undef WAYLAND_FUNC
        return true;
    } while(0);
    return false;
}
#undef WAYLAND_CLIENT

#define WAYLAND_CLIENT
#define XKB_COMMMON
#define WAYLAND_CURSOR
#define WAYLAND_EGL_CORE
void CIrrDeviceWayland::clearWaylandFunctions() {
#define WAYLAND_FUNC(name, ret_type, ...) p##name = nullptr;
#define WAYLAND_INTERFACE(name)
#include "CWaylandFunctions.inl"
#undef WAYLAND_INTERFACE
#undef WAYLAND_FUNC
#undef WAYLAND_EGL_CORE
#undef WAYLAND_CURSOR
#undef XKB_COMMMON

#define ONLY_PROXY
#define WAYLAND_FUNC(name, ret_type, ...) irr__internal__p__##name = nullptr;
#define WAYLAND_INTERFACE(name) irr__internal__p__##name = nullptr;
#include "CWaylandFunctions.inl"
#undef ONLY_PROXY
#undef WAYLAND_FUNC
#undef WAYLAND_INTERFACE
#undef WAYLAND_CLIENT
    if(LibWaylandEGL) {
        dlclose(LibWaylandEGL);
        LibWaylandEGL = nullptr;
    }
    if(LibWaylandCursor) {
        dlclose(LibWaylandCursor);
        LibWaylandCursor = nullptr;
    }
    if(LibXKBCommon) {
        dlclose(LibXKBCommon);
        LibXKBCommon = nullptr;
    }
    if(LibWaylandClient) {
        dlclose(LibWaylandClient);
        LibXKBCommon = nullptr;
    }
}

#endif

static wl_cursor* LoadWaylandCursor(wl_cursor_theme* theme, const char* const cursors[]) {
    if(!theme)
        return nullptr;
    const char* first = *cursors;
    while(*cursors) {
        if(wl_cursor* ret = CIrrDeviceWayland::pwl_cursor_theme_get_cursor(theme, *cursors))
            return ret;
        cursors++;
    }
    os::Printer::log(core::stringc("Couldn't load ").append(first).append(" cursor.").c_str(), ELL_ERROR);
    return nullptr;
}

void CIrrDeviceWayland::CCursorControl::initCursors() {
    wl_cursor_theme* theme = m_device->m_cursor_theme;
    {
        constexpr const char* const ArrowCursor[] = { "left_ptr", "default", "top_left_arrow", "left_arrow", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, ArrowCursor));
    }
    {
        constexpr const char* const CrossCursor[] = { "cross", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, CrossCursor));
    }
    {
        constexpr const char* const PointingHandCursor[] = { "pointing_hand", "pointer", "hand1", "e29285e634086352946a0e7090d73106", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, PointingHandCursor));
    }
    {
        constexpr const char* const WhatsThisCursor[] = { "whats_this", "help", "question_arrow", "5c6cd98b3f3ebcb1f9c7f1c204630408", "d9ce0ab605698f320427677b458ad60b", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, WhatsThisCursor));
    }
    {
        constexpr const char* const IBeamCursor[] = { "ibeam", "text", "xterm", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, IBeamCursor));
    }
    {
        constexpr const char* const ForbiddenCursor[] = { "forbidden", "not-allowed", "crossed_circle", "circle", "03b6e0fcb3499374a867c041f52298f0", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, ForbiddenCursor));
    }
    {
        constexpr const char* const WaitCursor[] = { "wait", "watch", "0426c94ea35c87780ff01dc239897213", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, WaitCursor));
    }
    {
        constexpr const char* const SizeAllCursor[] = { "size_all", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, SizeAllCursor));
    }
    {
        constexpr const char* const SizeBDiagCursor[] = { "size_bdiag", "nesw-resize", "50585d75b494802d0151028115016902", "fcf1c3c7cd4491d801f1e1c78f100000", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, SizeBDiagCursor));
    }
    {
        constexpr const char* const SizeFDiagCursor[] = { "size_fdiag", "not-nwse-resize", "38c5dff7c7b8962045400281044508d2", "c7088f0f3e6c8088236ef8e1e3e70000", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, SizeFDiagCursor));
    }
    {
        constexpr const char* const SizeVerCursor[] = { "size_ver", "ns-resize", "v_double_arrow", "00008160000006810000408080010102", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, SizeVerCursor));
    }
    {
        constexpr const char* const SizeHorCursor[] = { "size_hor", "ew-resize", "h_double_arrow", "028006030e0e7ebffc7f7070c0600140", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, SizeHorCursor));
    }
    {
        constexpr const char* const UpArrowCursor[] = { "up_arrow", nullptr };
        Cursors.push_back(LoadWaylandCursor(theme, UpArrowCursor));
    }
}

//! Sets the active cursor icon
void CIrrDeviceWayland::CCursorControl::setActiveIcon(gui::ECURSOR_ICON iconId)
{
	if ( iconId >= (s32)Cursors.size() )
		return;

    m_active_icon = iconId;

    wl_cursor* cursor = Cursors[iconId];
    if(!cursor) {
        if(iconId != irr::gui::ECI_NORMAL) {
            cursor = Cursors[irr::gui::ECI_NORMAL];
            m_active_icon = iconId;
        }
        if(!cursor)
            return;
    }

    wl_cursor_image* image = cursor->images[0];
    wl_buffer* buffer = pwl_cursor_image_get_buffer(image);

    if(!buffer)
        return;

    wl_pointer_set_cursor(m_device->m_pointer, m_device->m_enter_serial, m_device->m_cursor_surface,
                          image->hotspot_x, image->hotspot_y);
    wl_surface_attach(m_device->m_cursor_surface, buffer, 0, 0);
    wl_surface_damage(m_device->m_cursor_surface, 0, 0, image->width, image->height);
    wl_surface_commit(m_device->m_cursor_surface);

    bool was_animated = m_is_animated;

    m_is_animated = cursor->image_count > 1;

    if(m_is_animated) {
        m_last_time = 0;
        m_last_frame = 0;
        if(!was_animated) {
            wl_callback* cb = wl_surface_frame(m_device->m_cursor_surface);
            wl_callback_add_listener(cb, &WaylandCallbacks::surface_frame_listener, this);
        }
    }
}

wl_cursor_image* CIrrDeviceWayland::CCursorControl::getNextFrame(uint32_t time)
{
    wl_cursor* cursor = Cursors[m_active_icon];
    wl_cursor_image* current = cursor->images[m_last_frame];
    if(m_last_time != 0) {
        uint32_t elapsed = time - m_last_time;
        while(elapsed >= current->delay) {
            elapsed -= current->delay;
            m_last_frame = (m_last_frame + 1) % cursor->image_count;
            current = cursor->images[m_last_frame];
        }
    }
    m_last_time = time;

    return current;
}

} // end namespace

#endif
