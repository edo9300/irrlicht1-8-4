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

#ifndef CIRRDEVICEWAYLAND_H
#define CIRRDEVICEWAYLAND_H

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"
#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
#include "CWaylandProxyFunctionsWorkaround.h"
#endif
#include "xdg_decoration_unstable_v1_client_protocol.h"
#include "xdg_shell_client_protocol.h"
#include "org_kde_kwin_server_decoration_manager_client_protocol.h"
#include "zxdg_shell_unstable_v6_client_protocol.h"
#include "ztext_input_unstable_v3_client_protocol.h"

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#ifdef IRR_USE_LIBDECOR
#include "LibdecorLoader.h"
#endif

#include <map>
#include <irrArray.h>

namespace irr
{
    class CIrrDeviceWayland : public CIrrDeviceStub, public video::IImagePresenter
    {
    public:
        friend class WaylandCallbacks;
        friend class CCursorControl;

        //! constructor
        CIrrDeviceWayland(const SIrrlichtCreationParameters& param);

        //! destructor
        virtual ~CIrrDeviceWayland();

        //! runs the device. Returns false if device wants to be deleted
        virtual bool run();

        //! Cause the device to temporarily pause execution and let other
        //! processes to run. This should bring down processor usage without
        //! major performance loss for Irrlicht
        virtual void yield();

        //! Pause execution and let other processes to run for a specified
        //! amount of time.
        virtual void sleep(u32 timeMs, bool pauseTimer);

        //! sets the caption of the window
        virtual void setWindowCaption(const wchar_t* text);

        //! returns if window is active. if not, nothing need to be drawn
        virtual bool isWindowActive() const;

        //! returns if window has focus.
        virtual bool isWindowFocused() const;

        //! returns if window is minimized.
        virtual bool isWindowMinimized() const;

        //! returns color format of the window.
        virtual video::ECOLOR_FORMAT getColorFormat() const;

        //! presents a surface in the client area
        virtual bool present(video::IImage* surface, void* windowId=0,
                             core::rect<s32>* src=0);

        //! notifies the device that it should close itself
        virtual void closeDevice();

        //! \return Returns a pointer to a list with all video modes
        //! supported by the gfx adapter.
        video::IVideoModeList* getVideoModeList();

        //! Sets if the window should be resizable in windowed mode.
        virtual void setResizable(bool resize=false);

        //! Minimizes the window.
        virtual void minimizeWindow();

        //! Maximizes the window.
        virtual void maximizeWindow();

        //! Restores the window size.
        virtual void restoreWindow();
		
		//! Toggle the windowed/borderless fullscreen status of the window.
		virtual void toggleFullscreen(bool fullscreen=true) _IRR_OVERRIDE_;
        
        //! Move window to requested position
        virtual bool moveWindow(int x, int y);

        //! Get current window position.
        virtual core::position2di getWindowPosition();

        //! Activate any joysticks, and generate events for them.
        virtual bool activateJoysticks(core::array<SJoystickInfo>& joystickInfo);
        
        //! Returns true if system has touch device
        virtual bool supportsTouchDevice() const { return m_has_touch_device; }
        
        //! Returns true if system has hardware keyboard
        virtual bool hasHardwareKeyboard() const { return m_has_hardware_keyboard; }

        //! Set the current Gamma Value for the Display
        virtual bool setGammaRamp(f32 red, f32 green, f32 blue,
                                  f32 brightness, f32 contrast);

        //! Get the current Gamma Value for the Display
        virtual bool getGammaRamp(f32 &red, f32 &green, f32 &blue,
                                  f32 &brightness, f32 &contrast);

        //! gets text from the clipboard
        //! \return Returns 0 if no string is in there.
        virtual const c8* getTextFromClipboard() const;

        //! copies text to the clipboard
        virtual void copyToClipboard(const c8* text);

        //! Remove all messages pending in the system message loop
        virtual void clearSystemMessages();

        virtual void enableDragDrop(bool enable, drop_callback_function_t dragCheck = nullptr);

        //! Get the device type
        virtual E_DEVICE_TYPE getType() const
        {
                return EIDT_WAYLAND;
        }

        static bool isWaylandDeviceWorking();

        void checkPendingResizes();
		
        void updateCursor();
        unsigned int getWidth() {return m_width;}
        unsigned int getHeight() {return m_height;}
        

        //! Implementation of the linux cursor control
        class CCursorControl : public gui::ICursorControl
        {
            friend class WaylandCallbacks;
        public:
            CCursorControl(CIrrDeviceWayland* device) : m_device(device),
                m_is_visible(true), m_use_reference_rect(false), m_active_icon(irr::gui::ECI_NORMAL),
                m_last_time(0), m_last_frame(0), m_is_animated(false)
            {
                initCursors();
            };

            ~CCursorControl() {};

            //! Changes the visible state of the mouse cursor.
            virtual void setVisible(bool visible)
            {
                if (visible == m_is_visible)
                    return;

                m_is_visible = visible;
                m_device->updateCursor();
            }

            //! Returns if the cursor is currently visible.
            virtual bool isVisible() const
            {
                return m_is_visible;
            }

            //! Sets the new position of the cursor.
            virtual void setPosition(const core::position2d<f32> &pos)
            {
                setPosition(pos.X, pos.Y);
            }

            //! Sets the new position of the cursor.
            virtual void setPosition(f32 x, f32 y)
            {
                setPosition((s32)(x * m_device->getWidth()),
                            (s32)(y * m_device->getHeight()));
            }

            //! Sets the new position of the cursor.
            virtual void setPosition(const core::position2d<s32> &pos)
            {
                setPosition(pos.X, pos.Y);
            }

            //! Sets the new position of the cursor.
            virtual void setPosition(s32 x, s32 y)
            {
                m_cursor_pos.X = x;
                m_cursor_pos.Y = y;
            }

            //! Returns the current position of the mouse cursor.
            virtual const core::position2d<s32>& getPosition(bool updateCursor)
            {
                return m_cursor_pos;
            }

            virtual core::position2d<f32> getRelativePosition(bool updateCursor)
            {
                if (!m_use_reference_rect)
                {
                    return core::position2d<f32>(
                                    m_cursor_pos.X / (f32)m_device->getWidth(),
                                    m_cursor_pos.Y / (f32)m_device->getHeight());
                }

                return core::position2d<f32>(
                                m_cursor_pos.X / (f32)m_reference_rect.getWidth(),
                                m_cursor_pos.Y / (f32)m_reference_rect.getHeight());
            }

            virtual void setReferenceRect(core::rect<s32>* rect=0)
            {
                m_use_reference_rect = false;

                if (rect)
                {
                    m_reference_rect = *rect;
                    m_use_reference_rect = true;

                    // prevent division through zero and uneven sizes
                    if (m_reference_rect.getHeight() == 0 ||
                        m_reference_rect.getHeight() % 2)
                        m_reference_rect.LowerRightCorner.Y += 1;

                    if (m_reference_rect.getWidth() == 0 ||
                        m_reference_rect.getWidth() % 2)
                        m_reference_rect.LowerRightCorner.X += 1;
                }
            }

            //! Sets the active cursor icon
            virtual void setActiveIcon(gui::ECURSOR_ICON iconId);

            //! Gets the currently active icon
            virtual gui::ECURSOR_ICON getActiveIcon() const
            {
                return m_active_icon;
            }

            //! Add a custom sprite as cursor icon.
            virtual gui::ECURSOR_ICON addIcon(const gui::SCursorSprite& icon)
            {
                return irr::gui::ECI_NORMAL;
            }

            //! replace the given cursor icon.
            virtual void changeIcon(gui::ECURSOR_ICON iconId,
                                    const gui::SCursorSprite& icon) {}

            /** Return a system-specific size which is supported for cursors.
                Larger icons will fail, smaller icons might work. */
            virtual core::dimension2di getSupportedIconSize() const
            {
                return core::dimension2di(0, 0);
            }

        private:

            CIrrDeviceWayland* m_device;
            core::position2d<s32> m_cursor_pos;
            core::rect<s32> m_reference_rect;
            bool m_is_visible;
            bool m_use_reference_rect;
            gui::ECURSOR_ICON m_active_icon;

            uint32_t m_last_time;
            uint32_t m_last_frame;
            bool m_is_animated;

            core::array<wl_cursor*> Cursors;
            void initCursors();
            wl_cursor_image* getNextFrame(uint32_t time);
        };

    private:

        #if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
        struct JoystickInfo
        {
            int fd;
            int axes;
            int buttons;
            SEvent persistentData;

            JoystickInfo() : fd(-1), axes(0), buttons(0) { }
        };

        core::array<JoystickInfo> m_active_joysticks;
        #endif

        wl_compositor* m_compositor;
        wl_cursor_theme* m_cursor_theme;
        wl_display* m_display;
        wl_egl_window* m_egl_window;
        wl_keyboard* m_keyboard;
        wl_touch* m_touch;
        wl_output* m_output;
        wl_pointer* m_pointer;
        wl_registry* m_registry;
        wl_seat* m_seat;
        wl_data_device* m_data_device;
        wl_shm* m_shm;
        wl_surface* m_cursor_surface;
        wl_surface* m_surface;
        wl_data_device_manager* m_data_device_manager;
        uint32_t m_enter_serial;
        
        wl_shell* m_shell;
        wl_shell_surface* m_shell_surface;
        bool m_has_wl_shell;
        uint32_t m_wl_shell_name;

        xdg_wm_base* m_xdg_wm_base;
        xdg_surface* m_xdg_surface;
        xdg_toplevel* m_xdg_toplevel;
        bool m_has_xdg_wm_base;
        uint32_t m_xdg_wm_base_name;

        zxdg_shell_v6* m_zxdg_shell;
        zxdg_surface_v6* m_zxdg_surface;
        zxdg_toplevel_v6* m_zxdg_toplevel;
        bool m_has_zxdg_shell;
        uint32_t m_zxdg_shell_name;
		
#ifdef IRR_USE_LIBDECOR
		libdecor* m_libdecor;
		libdecor_frame* m_libdecor_surface;
#endif

        bool m_surface_configured;
        
        zxdg_decoration_manager_v1* m_decoration_manager;
        zxdg_toplevel_decoration_v1* m_decoration;
        
        org_kde_kwin_server_decoration_manager* m_kwin_server_decoration_manager;
        org_kde_kwin_server_decoration* m_kwin_server_decoration;

        zwp_text_input_manager_v3* m_input_manager_v3;
        zwp_text_input_v3* m_input_v3;

        irr::gui::IGUIElement* lastFocusedElement;
        bool isEditingText;
        bool hasIMEInput;

        void checkAndUpdateIMEState();

        xkb_context* m_xkb_context;
        xkb_compose_table* m_xkb_compose_table;
        xkb_compose_state* m_xkb_compose_state;
        xkb_keymap* m_xkb_keymap;
        xkb_state* m_xkb_state;
        xkb_mod_mask_t m_xkb_alt_mask;
        xkb_mod_mask_t m_xkb_ctrl_mask;
        xkb_mod_mask_t m_xkb_shift_mask;
        bool m_xkb_alt_pressed;
        bool m_xkb_ctrl_pressed;
        bool m_xkb_shift_pressed;
        
        bool m_repeat_enabled;
        SEvent m_repeat_event;
        uint32_t m_repeat_time;
        uint32_t m_repeat_rate;
        uint32_t m_repeat_delay;

        uint32_t m_selection_serial;
        wl_data_source* m_data_source;

        uint32_t m_mouse_button_states;
        unsigned int m_width;
        unsigned int m_height;
        unsigned int m_touches_count;
        bool m_has_touch_device;
        bool m_has_hardware_keyboard;
        bool m_window_has_focus;
        bool m_window_minimized;
        mutable core::stringc m_readclipboard;
        std::map<int, EKEY_CODE> m_key_map;
        irr::core::array<SEvent> m_events;

        drop_callback_function_t m_drag_and_drop_check;
        bool m_dragging_file;
        irr::core::vector2di m_drop_pos;

        mutable core::stringc m_clipboard;
        wl_data_offer* m_clipboard_data_offer;
        mutable bool m_clipboard_changed;

        enum DATA_MIME {
            PLAIN_TEXT = 1,
            PLAIN_TEXT_UTF8 = 2,
            PLAIN_TEXT_UTF8_2 = 4,
            URI_LIST = 8,
        };

        u32 m_clipboard_mime;
        u32 m_current_selection_mime;

        bool m_drag_has_copy;
        wl_data_offer* m_drag_data_offer;
        uint32_t m_drag_data_offer_serial;
        bool m_drag_is_dropping;

        struct {
            int32_t width;
            int32_t height;
            uint32_t serial;
            bool configure;
            bool pending;
        } m_resizing_state;

        bool initWayland();
        void createDriver();
        void createKeyMap();
        bool createWindow();
        void signalEvent(const SEvent&);
        void pollJoysticks();
        void closeJoysticks();
        void setSelectionSerial(uint32_t serial);
#define WAYLAND_FUNC(name, ret_type, ...) static ret_type(* p##name)(__VA_ARGS__);
#ifdef _IRR_WAYLAND_DYNAMIC_LOAD_
#define WAYLAND_INTERFACE(name) static wl_interface *p##name;
        void clearWaylandFunctions();
        bool loadEglCoreFunctions();
        bool loadWaylandCursorFunctions();
        bool loadXKBCommonFunctions();
        bool loadWaylandClientFunctions();
        static void* LibWaylandEGL;
        static void* LibWaylandCursor;
        static void* LibXKBCommon;
        static void* LibWaylandClient;
        static int WaylandLoadCount;
#else
#define WAYLAND_INTERFACE(name)
        bool loadEglCoreFunctions() { return true; }
        bool loadWaylandCursorFunctions() { return true; }
        bool loadXKBCommonFunctions() { return true; }
        bool loadWaylandClientFunctions() { return true; }
#endif
        core::stringc class_name;
    public:
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
    };

} // end namespace irr

#endif

#endif // CIRRDEVICEWAYLAND_H
