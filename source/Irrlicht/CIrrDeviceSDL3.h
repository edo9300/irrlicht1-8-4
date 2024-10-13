// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This device code is based on the original SDL device implementation
// contributed by Shane Parker (sirshane).

#ifndef __C_IRR_DEVICE_SDL3_H_INCLUDED__
#define __C_IRR_DEVICE_SDL3_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_SDL3_DEVICE_

#include "IrrlichtDevice.h"
#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"

#include <SDL3/SDL.h>

#ifdef _IRR_EMSCRIPTEN_PLATFORM_
#include <emscripten/html5.h>
#endif

namespace irr
{

	class CIrrDeviceSDL3 : public CIrrDeviceStub, video::IImagePresenter
	{
	public:
#ifdef _IRR_EMSCRIPTEN_PLATFORM_
		friend class EMScriptenCallbacks;
#endif

		//! constructor
		CIrrDeviceSDL3(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceSDL3() override;

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run() override;

		//! pause execution temporarily
		virtual void yield() override;

		//! pause execution for a specified time
		virtual void sleep(u32 timeMs, bool pauseTimer) override;

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text) override;

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const override;

		//! returns if window has focus.
		virtual bool isWindowFocused() const override;

		//! returns if window is minimized.
		virtual bool isWindowMinimized() const override;

		//! returns color format of the window.
		virtual video::ECOLOR_FORMAT getColorFormat() const override;

		//! presents a surface in the client area
		virtual bool present(video::IImage* surface, void* windowId=0, core::rect<s32>* src=0) override;

		//! notifies the device that it should close itself
		virtual void closeDevice() override;

		//! \return Returns a pointer to a list with all video modes supported
		virtual video::IVideoModeList* getVideoModeList() override;

		//! Sets if the window should be resizable in windowed mode.
		virtual void setResizable(bool resize=false) override;

		//! Resize the render window.
		virtual void setWindowSize(const irr::core::dimension2d<u32>& size) override { resizeWindow(size.Width, size.Height); };

		//! Minimizes the window.
		virtual void minimizeWindow() override;

		//! Maximizes the window.
		virtual void maximizeWindow() override;

		//! Restores the window size.
		virtual void restoreWindow() override;

		//! Toggle the windowed/borderless fullscreen status of the window.
		virtual void toggleFullscreen(bool fullscreen = true) override;

		//! Checks if the Irrlicht window is running in fullscreen mode
		/** \return True if window is fullscreen. */
		virtual bool isFullscreen() const override;

		//! Get the position of this window on screen
		virtual core::position2di getWindowPosition() override;

		//! Activate any joysticks, and generate events for them.
		virtual bool activateJoysticks(core::array<SJoystickInfo> & joystickInfo) override;

		//! Set the current Gamma Value for the Display
		virtual bool setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast ) override;

		//! Get the current Gamma Value for the Display
		virtual bool getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast ) override;

		virtual void enableDragDrop(bool enable, drop_callback_function_t dragCheck = nullptr) override;

		//! Get the device type
		virtual E_DEVICE_TYPE getType() const override
		{
				return EIDT_SDL;
		}

		//! Implementation of the linux cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(CIrrDeviceSDL3* dev);

			~CCursorControl();

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible) override
			{
				IsVisible = visible;
				if (visible)
					SDL_ShowCursor();
				else
					SDL_HideCursor();
			}

			//! Returns if the cursor is currently visible.
			virtual bool isVisible() const override
			{
				return IsVisible;
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<f32> &pos) override
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(f32 x, f32 y) override
			{
				setPosition((s32)(x*Device->Width), (s32)(y*Device->Height));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos) override
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y) override
			{
				SDL_WarpMouseInWindow(Device->window, static_cast<float>(x), static_cast<float>(y));
			}

			//! Returns the current position of the mouse cursor.
			virtual const core::position2d<s32>& getPosition(bool updateCursor) override
			{
				if(updateCursor)
					updateCursorPos();
				return CursorPos;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<f32> getRelativePosition(bool updateCursor) override
			{
				if(updateCursor)
					updateCursorPos();
				return core::position2d<f32>(CursorPos.X / (f32)Device->Width,
					CursorPos.Y / (f32)Device->Height);
			}

			virtual void setReferenceRect(core::rect<s32>* rect=0) override
			{
			}


			//! Sets the active cursor icon
			virtual void setActiveIcon(gui::ECURSOR_ICON iconId) override;

			//! Gets the currently active icon
			virtual gui::ECURSOR_ICON getActiveIcon() const override {
				return CurCursor;
			}

		private:

			void updateCursorPos()
			{
#ifdef _IRR_EMSCRIPTEN_PLATFORM_
				EmscriptenPointerlockChangeEvent pointerlockStatus; // let's hope that test is not expensive ...
				if(emscripten_get_pointerlock_status(&pointerlockStatus) == EMSCRIPTEN_RESULT_SUCCESS) {
					if(pointerlockStatus.isActive) {
						CursorPos.X += Device->MouseXRel;
						CursorPos.Y += Device->MouseYRel;
						Device->MouseXRel = 0;
						Device->MouseYRel = 0;
					} else {
						CursorPos.X = Device->MouseX;
						CursorPos.Y = Device->MouseY;
					}
				}
#else
				CursorPos.X = Device->MouseX;
				CursorPos.Y = Device->MouseY;

				if (CursorPos.X < 0)
					CursorPos.X = 0;
				if (CursorPos.X > (s32)Device->Width)
					CursorPos.X = Device->Width;
				if (CursorPos.Y < 0)
					CursorPos.Y = 0;
				if (CursorPos.Y > (s32)Device->Height)
					CursorPos.Y = Device->Height;
#endif
			}

			CIrrDeviceSDL3* Device;
			core::position2d<s32> CursorPos;
			bool IsVisible;
			gui::ECURSOR_ICON CurCursor;
			SDL_Cursor* cursors[gui::ECI_COUNT];
		};

		void SwapBuffers() {
			SDL_GL_SwapWindow(window);
		}

	private:

		//! create the driver
		void createDriver();

		bool createWindow();

		void createKeyMap();

		void resizeWindow(u32 x, u32 y);

		void updateScreenTexture(core::dimension2d<u32> size);

		void checkAndUpdateIMEState();

		SDL_Window* window;
		int SDL_Flags;
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
		core::array<SDL_Joystick*> Joysticks;
#endif

		s32 MouseX, MouseY;
		s32 MouseXRel, MouseYRel;
		u32 MouseButtonStates;

		u32 Width, Height;

		bool Resizable;
		bool WindowHasFocus;
		bool WindowMinimized;

		irr::gui::IGUIElement* lastFocusedElement;
		core::rect<s32> lastFocusedElementPosition;
		bool isEditingText;

		struct SKeyMap
		{
			SKeyMap() {}
			SKeyMap(s32 x11, s32 win32)
				: SDLKey(x11), Win32Key(win32)
			{
			}

			s32 SDLKey;
			s32 Win32Key;

			bool operator<(const SKeyMap& o) const
			{
				return SDLKey<o.SDLKey;
			}
		};

		core::array<SKeyMap> KeyMap;
		SDL_Renderer* renderer;
		SDL_Texture* screen_texture;
		int screen_texture_pitch;
		core::dimension2d<u32> screen_texture_size;
		SDL_PixelFormat screen_texture_color_format;
		bool is_fullscreen;
		bool is_ctrl_pressed;
		bool is_shift_pressed;
	};

} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL3_DEVICE_
#endif // __C_IRR_DEVICE_SDL3_H_INCLUDED__

