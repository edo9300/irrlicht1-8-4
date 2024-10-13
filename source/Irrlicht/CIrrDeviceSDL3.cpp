// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_SDL3_DEVICE_

#define SDL_MAIN_HANDLED

#include "CIrrDeviceSDL3.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include <stdio.h>
#include <stdlib.h>
#include "SIrrCreationParameters.h"
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_main.h>
#include "IGUIEnvironment.h"
#include "IGUIElement.h"
#include "CSDL3ContextManager.h"
#include "CDriverCreationPrototypes.h"


namespace irr
{

//! constructor
CIrrDeviceSDL3::CIrrDeviceSDL3(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	window((decltype(window))param.WindowId),
	MouseX(0), MouseY(0), MouseXRel(0), MouseYRel(0), MouseButtonStates(0),
	Width(param.WindowSize.Width), Height(param.WindowSize.Height),
	Resizable(false), WindowHasFocus(false), WindowMinimized(false),
	lastFocusedElement(nullptr), isEditingText(false),
	renderer(nullptr), screen_texture(nullptr), is_ctrl_pressed(false), is_shift_pressed(false)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceSDL3");
	#endif

	SDL_SetMainReady();
	SDL_SetHintWithPriority(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority(SDL_HINT_IME_IMPLEMENTED_UI, "1", SDL_HINT_OVERRIDE);
	// Initialize SDL... Timer for sleep, video for the obvious, and
	// noparachute prevents SDL from catching fatal errors.
	auto init_flags = SDL_INIT_VIDEO;
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	init_flags |= SDL_INIT_JOYSTICK;
#endif

	if(!SDL_Init(init_flags))
	{
		os::Printer::log( "Unable to initialize SDL!", SDL_GetError());
		Close = true;
	}

	// create window
	if(CreationParams.DriverType != video::EDT_NULL) {
		if(CreationParams.ClassName) {
#ifndef _IRR_WCHAR_FILESYSTEM
			SDL_SetHintWithPriority(SDL_HINT_APP_ID, CreationParams.ClassName, SDL_HINT_OVERRIDE);
#else
			const size_t lenOld = (wcslen(CreationParams.ClassName) + 1) * sizeof(wchar_t);
			char* name = new char[lenOld];
			core::wcharToUtf8(CreationParams.ClassName, name, lenOld);
			SDL_SetHintWithPriority(SDL_HINT_APP_ID, name, SDL_HINT_OVERRIDE);
			delete[] name;
#endif
		}
		// create the window, only if we do not use the null device
		if(!createWindow())
			return;
	}

	auto version = SDL_GetVersion();
	core::stringc sdlversion = "SDL Version ";
	sdlversion += SDL_VERSIONNUM_MAJOR(version);
	sdlversion += ".";
	sdlversion += SDL_VERSIONNUM_MINOR(version);
	sdlversion += ".";
	sdlversion += SDL_VERSIONNUM_MICRO(version);
	if(SDL_VERSION != version) {
		sdlversion += "(Compile version ";
		sdlversion += SDL_MAJOR_VERSION;
		sdlversion += ".";
		sdlversion += SDL_MINOR_VERSION;
		sdlversion += ".";
		sdlversion += SDL_MICRO_VERSION;
		sdlversion += ")";
	}
	sdlversion += " (";
	sdlversion += SDL_GetPlatform();
	sdlversion += ")";

	Operator = new COSOperator(sdlversion, EIDT_SDL3);
	os::Printer::log(sdlversion.c_str(), ELL_INFORMATION);

	// create keymap
	createKeyMap();

	// create cursor control
	CursorControl = new CCursorControl(this);

	// create driver
	createDriver();

	if(window)
		SDL_ShowWindow(window);
	if(VideoDriver) {
		// a default SDL3 window will have a black background, render it white
		VideoDriver->beginScene(true, true, { 255, 255, 255, 255 });
		VideoDriver->endScene();
		createGUIAndScene();
	}
}


//! destructor
CIrrDeviceSDL3::~CIrrDeviceSDL3()
{
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	const u32 numJoysticks = Joysticks.size();
	for (u32 i=0; i<numJoysticks; ++i)
		SDL_CloseJoystick(Joysticks[i]);
#endif
	if(window) {
		if(ContextManager)
			ContextManager->destroyContext();
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}


#ifdef _IRR_EMSCRIPTEN_PLATFORM_
class EMScriptenCallbacks {
public:
	static EM_BOOL MouseUpDownCallback(int eventType, const EmscriptenMouseEvent* event, void* userData);
	static EM_BOOL MouseEnterCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
	static EM_BOOL MouseLeaveCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
};

EM_BOOL EMScriptenCallbacks::MouseUpDownCallback(int eventType, const EmscriptenMouseEvent* event, void* userData) {
	// We need this callback so far only because otherwise "emscripten_request_pointerlock" calls will
	// fail as their request are infinitely deferred.
	// Not exactly certain why, maybe SDL does catch those mouse-events otherwise and not pass them on.
	return EM_FALSE;
}

EM_BOOL EMScriptenCallbacks::MouseEnterCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData) {
	CIrrDeviceSDL3* device = static_cast<CIrrDeviceSDL3*>(userData);

	SEvent irrevent;

	irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
	irrevent.MouseInput.Event = irr::EMIE_MOUSE_ENTER_CANVAS;
	device->MouseX = irrevent.MouseInput.X = mouseEvent->canvasX;
	device->MouseY = irrevent.MouseInput.Y = mouseEvent->canvasY;
	device->MouseXRel = mouseEvent->movementX; // should be 0 I guess? Or can it enter while pointer is locked()?
	device->MouseYRel = mouseEvent->movementY;
	irrevent.MouseInput.ButtonStates = device->MouseButtonStates;	// TODO: not correct, but couldn't figure out the bitset of mouseEvent->buttons yet.
	irrevent.MouseInput.Shift = mouseEvent->shiftKey;
	irrevent.MouseInput.Control = mouseEvent->ctrlKey;

	device->postEventFromUser(irrevent);

	return EM_FALSE;
}

EM_BOOL EMScriptenCallbacks::MouseLeaveCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData) {
	CIrrDeviceSDL3* device = static_cast<CIrrDeviceSDL3*>(userData);

	SEvent irrevent;

	irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
	irrevent.MouseInput.Event = irr::EMIE_MOUSE_LEAVE_CANVAS;
	device->MouseX = irrevent.MouseInput.X = mouseEvent->canvasX;
	device->MouseY = irrevent.MouseInput.Y = mouseEvent->canvasY;
	device->MouseXRel = mouseEvent->movementX; // should be 0 I guess? Or can it enter while pointer is locked()?
	device->MouseYRel = mouseEvent->movementY;
	irrevent.MouseInput.ButtonStates = device->MouseButtonStates;	// TODO: not correct, but couldn't figure out the bitset of mouseEvent->buttons yet.
	irrevent.MouseInput.Shift = mouseEvent->shiftKey;
	irrevent.MouseInput.Control = mouseEvent->ctrlKey;

	device->postEventFromUser(irrevent);

	return EM_FALSE;
}
#endif

bool CIrrDeviceSDL3::createWindow()
{
	Uint32 windowFlags = 0;
	if (CreationParams.Fullscreen)
		windowFlags |= SDL_WINDOW_FULLSCREEN;

	if(CreationParams.DriverType == video::EDT_OPENGL || CreationParams.DriverType == video::EDT_OGLES1
	   || CreationParams.DriverType == video::EDT_OGLES2 || CreationParams.DriverType == video::EDT_WEBGL1) {
		windowFlags |= SDL_WINDOW_OPENGL;
		video::CSDL3ContextManager::SetWindowOGLProperties(CreationParams);
	}

	if(CreationParams.WindowResizable)
		windowFlags |= SDL_WINDOW_RESIZABLE;

#ifdef _IRR_EMSCRIPTEN_PLATFORM_
	if(Width != 0 || Height != 0)
		emscripten_set_canvas_size(Width, Height);
	else {
		int w, h, fs;
		emscripten_get_canvas_size(&w, &h, &fs);
		CreationParams.WindowSize.Width = Width = w;
		CreationParams.WindowSize.Height = Height = h;
	}
#endif

	irr::core::stringc title = "Irrlicht (title not set)";
	if(CreationParams.WindowCaption) {
#ifndef _IRR_WCHAR_FILESYSTEM
		title = CreationParams.WindowCaption;
#else
		const size_t lenOld = (wcslen(CreationParams.WindowCaption) + 1) * sizeof(wchar_t);
		char* name = new char[lenOld];
		core::wcharToUtf8(CreationParams.WindowCaption, name, lenOld);
		title = name;
		delete[] name;
#endif
	}

	window = SDL_CreateWindow(
		title.data(),
		CreationParams.WindowSize.Width,
		CreationParams.WindowSize.Height,
		windowFlags | SDL_WINDOW_HIDDEN
	);

	if(window == nullptr) {
		os::Printer::log("Could not create SDL Window.", SDL_GetError(), ELL_ERROR);
		return false;
	}

	// create color map
	WindowMinimized=false;
#ifdef _IRR_EMSCRIPTEN_PLATFORM_
	emscripten_set_mousedown_callback("#canvas", (void*)this, true, EMScriptenCallbacks::MouseUpDownCallback);
	emscripten_set_mouseup_callback("#canvas", (void*)this, true, EMScriptenCallbacks::MouseUpDownCallback);
	emscripten_set_mouseenter_callback("#canvas", (void*)this, false, EMScriptenCallbacks::MouseEnterCallback);
	emscripten_set_mouseleave_callback("#canvas", (void*)this, false, EMScriptenCallbacks::MouseLeaveCallback);
#endif

	return true;
}


//! create the driver
void CIrrDeviceSDL3::createDriver()
{
	void* HWnd{ nullptr };
#ifdef _WIN32
	HWnd = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#endif
	switch(CreationParams.DriverType)
	{

	case video::EDT_DIRECT3D9_ON_12:
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		VideoDriver = video::createDirectX9Driver(CreationParams, FileSystem, HWnd);

		if(VideoDriver)
			break;
		os::Printer::log("Could not create DIRECT3D9on12 Driver.", ELL_ERROR);
		os::Printer::log("Falling back to DIRECT3D9 driver.", ELL_ERROR);
#else
		os::Printer::log("DIRECT3D9on12 Driver was not compiled into this dll. Try another one.", ELL_ERROR);
		break;
#endif
	case video::EDT_DIRECT3D9:
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		{
			VideoDriver = video::createDirectX9Driver(CreationParams, FileSystem, HWnd);
			if (!VideoDriver)
			{
				os::Printer::log("Could not create DIRECT3D9 Driver.", ELL_ERROR);
			}
		}
		#else
		os::Printer::log("DIRECT3D9 Driver was not compiled into this dll. Try another one.", ELL_ERROR);
		#endif // _IRR_COMPILE_WITH_DIRECT3D_9_

		break;

	case video::EDT_SOFTWARE:
		#ifdef _IRR_COMPILE_WITH_SOFTWARE_
		renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
		VideoDriver = video::createSoftwareDriver(CreationParams.WindowSize, CreationParams.Fullscreen, FileSystem, this);
		#else
		os::Printer::log("No Software driver support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_BURNINGSVIDEO:
		#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
		renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
		puts(SDL_GetError());
		VideoDriver = video::createBurningVideoDriver(CreationParams, FileSystem, this);
		#else
		os::Printer::log("Burning's video driver was not compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_OPENGL:
		#ifdef _IRR_COMPILE_WITH_OPENGL_
		{
			video::SExposedVideoData data;

			data.OGLSDL2.Window = window;
			data.OGLSDL2.Context = nullptr;
			data.OGLSDL2.HWnd = HWnd;

			ContextManager = new video::CSDL3ContextManager(data);
			ContextManager->initialize(CreationParams, data);

			VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, ContextManager);
		}
		#else
		os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_OGLES1:
		#ifdef _IRR_COMPILE_WITH_OGLES1_
		{
			video::SExposedVideoData data;

			data.OGLSDL2.Window = window;
			data.OGLSDL2.Context = nullptr;
			data.OGLSDL2.HWnd = HWnd;

			ContextManager = new video::CSDL3ContextManager(data);
			ContextManager->initialize(CreationParams, data);

			VideoDriver = video::createOGLES1Driver(CreationParams, FileSystem, ContextManager);
		}
		#else
		os::Printer::log("No OpenGL-ES1 support compiled in.", ELL_ERROR);
		#endif
		break;
	case video::EDT_OGLES2:
		#ifdef _IRR_COMPILE_WITH_OGLES2_
		{
			video::SExposedVideoData data;

			data.OGLSDL2.Window = window;
			data.OGLSDL2.Context = nullptr;
			data.OGLSDL2.HWnd = HWnd;

			ContextManager = new video::CSDL3ContextManager(data);
			ContextManager->initialize(CreationParams, data);

			VideoDriver = video::createOGLES2Driver(CreationParams, FileSystem, ContextManager);
		}
		#else
		os::Printer::log("No OpenGL-ES2 support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_WEBGL1:
	#ifdef _IRR_COMPILE_WITH_WEBGL1_
		{
			video::SExposedVideoData data;

			data.OGLSDL2.Window = window;
			data.OGLSDL2.Context = nullptr;
			data.OGLSDL2.HWnd = HWnd;

			ContextManager = new video::CSDL3ContextManager(data);
			ContextManager->initialize(CreationParams, data);

			VideoDriver = video::createWebGL1Driver(CreationParams, FileSystem, ContextManager);
		}
	#else
		os::Printer::log("No WebGL1 support compiled in.", ELL_ERROR);
	#endif
		break;

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}
}

void CIrrDeviceSDL3::checkAndUpdateIMEState() {
    auto* env = getGUIEnvironment();
    if(!env) {
        lastFocusedElement = nullptr;
        if(isEditingText) {
            isEditingText = false;
			SDL_StopTextInput(window);
        }
        return;
    }

    auto updateRectPosition = [&] {
        lastFocusedElementPosition = lastFocusedElement->getAbsolutePosition();
        auto& pos = lastFocusedElementPosition.UpperLeftCorner;
		
		SDL_Rect rect;
		rect.x = pos.X;
		rect.y = pos.Y;
		rect.w = lastFocusedElementPosition.getWidth();
		rect.h = lastFocusedElementPosition.getHeight();
		SDL_SetTextInputArea(window, &rect, 0);
    };

    irr::gui::IGUIElement* ele = env->getFocus();
    if(lastFocusedElement == ele) {
        if(!ele || !isEditingText)
            return;
        auto abs_pos = lastFocusedElement->getAbsolutePosition();
        if(abs_pos == lastFocusedElementPosition)
            return;
        updateRectPosition();
        return;
    }
    isEditingText = (ele && (ele->getType() == irr::gui::EGUIET_EDIT_BOX) && ele->isEnabled());
    lastFocusedElement = ele;

    SDL_StopTextInput(window);

    if(!isEditingText)
        return;
	
    updateRectPosition();
    SDL_StartTextInput(window);
}


//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceSDL3::run()
{
	os::Timer::tick();

	checkAndUpdateIMEState();

	SEvent irrevent;
	SDL_Event SDL_event;

	while ( !Close && SDL_PollEvent( &SDL_event ) )
	{
		switch ( SDL_event.type )
		{
		case SDL_EVENT_MOUSE_MOTION:
			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			MouseX = irrevent.MouseInput.X = static_cast<s32>(SDL_event.motion.x);
			MouseY = irrevent.MouseInput.Y = static_cast<s32>(SDL_event.motion.y);
			MouseXRel = static_cast<s32>(SDL_event.motion.xrel);
			MouseYRel = static_cast<s32>(SDL_event.motion.yrel);
			irrevent.MouseInput.ButtonStates = MouseButtonStates;

			postEventFromUser(irrevent);
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:

			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.X = static_cast<s32>(SDL_event.button.x);
			irrevent.MouseInput.Y = static_cast<s32>(SDL_event.button.y);
			irrevent.MouseInput.Shift = is_shift_pressed;
			irrevent.MouseInput.Control = is_ctrl_pressed;

			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;

			switch(SDL_event.button.button)
			{
			case SDL_BUTTON_LEFT:
				if (SDL_event.button.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
				{
					irrevent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
					MouseButtonStates |= irr::EMBSM_LEFT;
				}
				else
				{
					irrevent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
					MouseButtonStates &= !irr::EMBSM_LEFT;
				}
				break;

			case SDL_BUTTON_RIGHT:
				if (SDL_event.button.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
				{
					irrevent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
					MouseButtonStates |= irr::EMBSM_RIGHT;
				}
				else
				{
					irrevent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
					MouseButtonStates &= ~irr::EMBSM_RIGHT;
				}
				break;

			case SDL_BUTTON_MIDDLE:
				if (SDL_event.button.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
				{
					irrevent.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
					MouseButtonStates |= irr::EMBSM_MIDDLE;
				}
				else
				{
					irrevent.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
					MouseButtonStates &= ~irr::EMBSM_MIDDLE;
				}
				break;
			}

			irrevent.MouseInput.ButtonStates = MouseButtonStates;

			if (irrevent.MouseInput.Event != irr::EMIE_MOUSE_MOVED)
			{
				postEventFromUser(irrevent);
			}
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			if(SDL_event.wheel.y == 0)
				break;
			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
			irrevent.MouseInput.Wheel = SDL_event.wheel.y > 0 ? 1.0f : -1.0f;
			postEventFromUser(irrevent);
			break;

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			{
				SKeyMap mp;
				mp.SDLKey = SDL_event.key.key;
				s32 idx = KeyMap.binary_search(mp);

				EKEY_CODE key;
				if (idx == -1)
					key = (EKEY_CODE)0;
				else
					key = (EKEY_CODE)KeyMap[idx].Win32Key;

				is_ctrl_pressed = (SDL_event.key.mod & SDL_KMOD_CTRL) != 0;
				is_shift_pressed = (SDL_event.key.mod & SDL_KMOD_SHIFT) != 0;
				irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
				irrevent.KeyInput.Char = (isEditingText && key != 8) ? 0 : SDL_event.key.mod;
				irrevent.KeyInput.Key = key;
				irrevent.KeyInput.PressedDown = (SDL_event.type == SDL_EVENT_KEY_DOWN);
				irrevent.KeyInput.Shift = (SDL_event.key.mod & SDL_KMOD_SHIFT) != 0;
				irrevent.KeyInput.Control = (SDL_event.key.mod & SDL_KMOD_CTRL ) != 0;
				postEventFromUser(irrevent);
			}
			break;

		case SDL_EVENT_TEXT_INPUT: {
			auto text = SDL_event.text.text;
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
					postEventFromUser(event);
				}
				delete[] ws;
			}
			break;
		}

		case SDL_EVENT_QUIT:
			Close = true;
			break;

		case SDL_EVENT_DROP_BEGIN:
			{
				irrevent.EventType = irr::EET_DROP_EVENT;
				float x, y;
				SDL_GetMouseState(&x, &y);
				irrevent.DropEvent.X = static_cast<s32>(x);
				irrevent.DropEvent.Y = static_cast<s32>(y);
				irrevent.DropEvent.Text = nullptr;
				irrevent.DropEvent.DropType = irr::DROP_START;
				postEventFromUser(irrevent);
			}
			break;

		case SDL_EVENT_DROP_FILE:
		case SDL_EVENT_DROP_TEXT:
			{
				irrevent.EventType = irr::EET_DROP_EVENT;
				irrevent.DropEvent.DropType = (SDL_event.type == SDL_EVENT_DROP_FILE) ?  irr::DROP_FILE : irr::DROP_TEXT;
				size_t lenOld = strlen(SDL_event.drop.data);
				wchar_t *ws = new wchar_t[lenOld + 1];
				core::utf8ToWchar(SDL_event.drop.data, ws, (lenOld + 1) * sizeof(wchar_t));
				irrevent.DropEvent.Text = ws;
				irrevent.DropEvent.X = static_cast<s32>(SDL_event.drop.x);
				irrevent.DropEvent.Y = static_cast<s32>(SDL_event.drop.y);
				postEventFromUser(irrevent);
				delete[] ws;
			}
			break;

		case SDL_EVENT_DROP_COMPLETE:
			{
				irrevent.EventType = irr::EET_DROP_EVENT;
				irrevent.DropEvent.Text = 0;
				irrevent.DropEvent.DropType = irr::DROP_END;
				postEventFromUser(irrevent);
			}
			break;

		case SDL_EVENT_WINDOW_MOUSE_ENTER:
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			WindowHasFocus = true;
			break;

		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			WindowHasFocus = false;
			break;

		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		case SDL_EVENT_WINDOW_RESIZED:
			if((SDL_event.window.data1 != (int)Width) || (SDL_event.window.data2 != (int)Height)) {
				Width = SDL_event.window.data1;
				Height = SDL_event.window.data2;
				if(VideoDriver)
					VideoDriver->OnResize(core::dimension2d<u32>(Width, Height));
			}
			break;

		case SDL_EVENT_WINDOW_RESTORED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_SHOWN:
			WindowMinimized = false;
			break;

		case SDL_EVENT_WINDOW_MINIMIZED:
			WindowMinimized = true;
			break;

		case SDL_EVENT_USER:
			irrevent.EventType = irr::EET_USER_EVENT;
			irrevent.UserEvent.UserData1 = *(reinterpret_cast<s32*>(&SDL_event.user.data1));
			irrevent.UserEvent.UserData2 = *(reinterpret_cast<s32*>(&SDL_event.user.data2));

			postEventFromUser(irrevent);
			break;

		default:
			break;
		} // end switch

	} // end while

#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	// TODO: Check if the multiple open/close calls are too expensive, then
	// open/close in the constructor/destructor instead

	// update joystick states manually
	SDL_UpdateJoysticks();
	// we'll always send joystick input events...
	SEvent joyevent;
	joyevent.EventType = EET_JOYSTICK_INPUT_EVENT;
	for (u32 i=0; i<Joysticks.size(); ++i)
	{
		SDL_Joystick* joystick = Joysticks[i];
		if (joystick)
		{
			int j;
			// query all buttons
			const int numButtons = core::min_(SDL_GetNumJoystickButtons(joystick), 32);
			joyevent.JoystickEvent.ButtonStates=0;
			for (j=0; j<numButtons; ++j)
				joyevent.JoystickEvent.ButtonStates |= (SDL_GetJoystickButton(joystick, j)<<j);

			// query all axes, already in correct range
			const int numAxes = core::min_(SDL_GetNumJoystickAxes(joystick), (int)SEvent::SJoystickEvent::NUMBER_OF_AXES);
			joyevent.JoystickEvent.Axis[SEvent::SJoystickEvent::AXIS_X]=0;
			joyevent.JoystickEvent.Axis[SEvent::SJoystickEvent::AXIS_Y]=0;
			joyevent.JoystickEvent.Axis[SEvent::SJoystickEvent::AXIS_Z]=0;
			joyevent.JoystickEvent.Axis[SEvent::SJoystickEvent::AXIS_R]=0;
			joyevent.JoystickEvent.Axis[SEvent::SJoystickEvent::AXIS_U]=0;
			joyevent.JoystickEvent.Axis[SEvent::SJoystickEvent::AXIS_V]=0;
			for (j=0; j<numAxes; ++j)
				joyevent.JoystickEvent.Axis[j] = SDL_GetJoystickAxis(joystick, j);

			// we can only query one hat, SDL only supports 8 directions
			if (SDL_GetNumJoystickHats(joystick)>0)
			{
				switch (SDL_GetJoystickHat(joystick, 0))
				{
					case SDL_HAT_UP:
						joyevent.JoystickEvent.POV=0;
						break;
					case SDL_HAT_RIGHTUP:
						joyevent.JoystickEvent.POV=4500;
						break;
					case SDL_HAT_RIGHT:
						joyevent.JoystickEvent.POV=9000;
						break;
					case SDL_HAT_RIGHTDOWN:
						joyevent.JoystickEvent.POV=13500;
						break;
					case SDL_HAT_DOWN:
						joyevent.JoystickEvent.POV=18000;
						break;
					case SDL_HAT_LEFTDOWN:
						joyevent.JoystickEvent.POV=22500;
						break;
					case SDL_HAT_LEFT:
						joyevent.JoystickEvent.POV=27000;
						break;
					case SDL_HAT_LEFTUP:
						joyevent.JoystickEvent.POV=31500;
						break;
					case SDL_HAT_CENTERED:
					default:
						joyevent.JoystickEvent.POV=65535;
						break;
				}
			}
			else
			{
				joyevent.JoystickEvent.POV=65535;
			}

			// we map the number directly
			joyevent.JoystickEvent.Joystick=static_cast<u8>(i);
			// now post the event
			postEventFromUser(joyevent);
			// and close the joystick
		}
	}
#endif
	return !Close;
}

//! Activate any joysticks, and generate events for them.
bool CIrrDeviceSDL3::activateJoysticks(core::array<SJoystickInfo> & joystickInfo)
{
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	joystickInfo.clear();

	// we can name up to 256 different joysticks
	const int numJoysticks = core::min_(SDL_NumJoysticks(), 256);
	Joysticks.reallocate(numJoysticks);
	joystickInfo.reallocate(numJoysticks);

	int joystick = 0;
	for (; joystick<numJoysticks; ++joystick)
	{
		Joysticks.push_back(SDL_OpenJoystick(joystick));
		SJoystickInfo info;

		info.Joystick = joystick;
		info.Axes = SDL_GetNumJoystickAxes(Joysticks[joystick]);
		info.Buttons = SDL_GetNumJoystickButtons(Joysticks[joystick]);
		info.Name = SDL_JoystickNameForIndex(joystick);
		info.PovHat = (SDL_GetNumJoystickHats(Joysticks[joystick]) > 0)
						? SJoystickInfo::POV_HAT_PRESENT : SJoystickInfo::POV_HAT_ABSENT;

		joystickInfo.push_back(info);
	}

	for(joystick = 0; joystick < (int)joystickInfo.size(); ++joystick)
	{
		char logString[256];
		(void)sprintf(logString, "Found joystick %d, %d axes, %d buttons '%s'",
		joystick, joystickInfo[joystick].Axes,
		joystickInfo[joystick].Buttons, joystickInfo[joystick].Name.c_str());
		os::Printer::log(logString, ELL_INFORMATION);
	}

	return true;

#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_

	return false;
}



//! pause execution temporarily
void CIrrDeviceSDL3::yield()
{
	SDL_Delay(0);
}


//! pause execution for a specified time
void CIrrDeviceSDL3::sleep(u32 timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer ? Timer->isStopped() : true;
	if (pauseTimer && !wasStopped)
		Timer->stop();

	SDL_Delay(timeMs);

	if (pauseTimer && !wasStopped)
		Timer->start();
}


//! sets the caption of the window
void CIrrDeviceSDL3::setWindowCaption(const wchar_t* text)
{
	core::stringc textc = text;
	SDL_SetWindowTitle(window, textc.c_str());
}


//! presents a surface in the client area
bool CIrrDeviceSDL3::present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip)
{
	updateScreenTexture(surface->getDimension());
	SDL_UpdateTexture(screen_texture, nullptr, surface->getData(), surface->getDimension().Width * screen_texture_pitch);
	SDL_RenderTexture(renderer, screen_texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
	return true;
}


//! notifies the device that it should close itself
void CIrrDeviceSDL3::closeDevice()
{
	Close = true;
}


//! \return Pointer to a list with all video modes supported
video::IVideoModeList* CIrrDeviceSDL3::getVideoModeList()
{
	int total;
	auto* display_modes = SDL_GetFullscreenDisplayModes(0, &total);
	for (s32 i = 0; i < total; i++) {
		const SDL_DisplayMode& mode = *(*display_modes++);

		if (i == 0) {
			VideoModeList->setDesktop(1, core::dimension2d<u32>(
					mode.w, mode.h));
		}

		VideoModeList->addMode(core::dimension2d<u32>(
			mode.w, mode.h), 1);
	}

	return VideoModeList;
}


//! Sets if the window should be resizable in windowed mode.
void CIrrDeviceSDL3::setResizable(bool resize)
{
	SDL_SetWindowResizable(window, resize);
}


//! Minimizes window if possible
void CIrrDeviceSDL3::minimizeWindow()
{
	SDL_MinimizeWindow(window);
}


//! Maximize window
void CIrrDeviceSDL3::maximizeWindow()
{
	SDL_MaximizeWindow(window);
}


//! Restore original window size
void CIrrDeviceSDL3::restoreWindow()
{
	SDL_RestoreWindow(window);
}


//! Restore original window size
void CIrrDeviceSDL3::toggleFullscreen(bool fullscreen) {
	SDL_SetWindowFullscreen(window, fullscreen);
}


bool CIrrDeviceSDL3::isFullscreen() const {
	return CIrrDeviceStub::isFullscreen();
}

//! Get the position of this window on screen
core::position2di CIrrDeviceSDL3::getWindowPosition() {
	core::position2di ret;
	SDL_GetWindowPosition(window, &ret.X, &ret.Y);
	return ret;
}


//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceSDL3::isWindowActive() const
{
	return (WindowHasFocus && !WindowMinimized);
}


//! returns if window has focus.
bool CIrrDeviceSDL3::isWindowFocused() const
{
	return WindowHasFocus;
}


//! returns if window is minimized.
bool CIrrDeviceSDL3::isWindowMinimized() const
{
	return WindowMinimized;
}


//! Set the current Gamma Value for the Display
bool CIrrDeviceSDL3::setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast )
{
	return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceSDL3::getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast )
{
	return false;
}

void CIrrDeviceSDL3::enableDragDrop(bool enable, drop_callback_function_t dragCheck) {
	SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, enable);
	SDL_SetEventEnabled(SDL_EVENT_DROP_TEXT, enable);
	SDL_SetEventEnabled(SDL_EVENT_DROP_BEGIN, enable);
	SDL_SetEventEnabled(SDL_EVENT_DROP_COMPLETE, enable);
}

//! returns color format of the window.
video::ECOLOR_FORMAT CIrrDeviceSDL3::getColorFormat() const
{
	return CIrrDeviceStub::getColorFormat();
}


void CIrrDeviceSDL3::createKeyMap()
{
	// I don't know if this is the best method  to create
	// the lookuptable, but I'll leave it like that until
	// I find a better version.

	KeyMap.reallocate(105);

	// buttons missing

	KeyMap.push_back(SKeyMap(SDLK_BACKSPACE, KEY_BACK));
	KeyMap.push_back(SKeyMap(SDLK_TAB, KEY_TAB));
	KeyMap.push_back(SKeyMap(SDLK_CLEAR, KEY_CLEAR));
	KeyMap.push_back(SKeyMap(SDLK_RETURN, KEY_RETURN));

	// combined modifiers missing

	KeyMap.push_back(SKeyMap(SDLK_PAUSE, KEY_PAUSE));
	KeyMap.push_back(SKeyMap(SDLK_CAPSLOCK, KEY_CAPITAL));

	// asian letter keys missing

	KeyMap.push_back(SKeyMap(SDLK_ESCAPE, KEY_ESCAPE));

	// asian letter keys missing

	KeyMap.push_back(SKeyMap(SDLK_SPACE, KEY_SPACE));
	KeyMap.push_back(SKeyMap(SDLK_PAGEUP, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(SDLK_PAGEDOWN, KEY_NEXT));
	KeyMap.push_back(SKeyMap(SDLK_END, KEY_END));
	KeyMap.push_back(SKeyMap(SDLK_HOME, KEY_HOME));
	KeyMap.push_back(SKeyMap(SDLK_LEFT, KEY_LEFT));
	KeyMap.push_back(SKeyMap(SDLK_UP, KEY_UP));
	KeyMap.push_back(SKeyMap(SDLK_RIGHT, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(SDLK_DOWN, KEY_DOWN));

	// select missing
	KeyMap.push_back(SKeyMap(SDLK_PRINTSCREEN, KEY_PRINT));
	// execute missing
	KeyMap.push_back(SKeyMap(SDLK_PRINTSCREEN, KEY_SNAPSHOT));

	KeyMap.push_back(SKeyMap(SDLK_INSERT, KEY_INSERT));
	KeyMap.push_back(SKeyMap(SDLK_DELETE, KEY_DELETE));
	KeyMap.push_back(SKeyMap(SDLK_HELP, KEY_HELP));

	KeyMap.push_back(SKeyMap(SDLK_0, KEY_KEY_0));
	KeyMap.push_back(SKeyMap(SDLK_1, KEY_KEY_1));
	KeyMap.push_back(SKeyMap(SDLK_2, KEY_KEY_2));
	KeyMap.push_back(SKeyMap(SDLK_3, KEY_KEY_3));
	KeyMap.push_back(SKeyMap(SDLK_4, KEY_KEY_4));
	KeyMap.push_back(SKeyMap(SDLK_5, KEY_KEY_5));
	KeyMap.push_back(SKeyMap(SDLK_6, KEY_KEY_6));
	KeyMap.push_back(SKeyMap(SDLK_7, KEY_KEY_7));
	KeyMap.push_back(SKeyMap(SDLK_8, KEY_KEY_8));
	KeyMap.push_back(SKeyMap(SDLK_9, KEY_KEY_9));

	KeyMap.push_back(SKeyMap(SDLK_A, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(SDLK_B, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(SDLK_C, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(SDLK_D, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(SDLK_E, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(SDLK_F, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(SDLK_G, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(SDLK_H, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(SDLK_I, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(SDLK_J, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(SDLK_K, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(SDLK_L, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(SDLK_M, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(SDLK_N, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(SDLK_O, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(SDLK_P, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(SDLK_Q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(SDLK_R, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(SDLK_S, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(SDLK_T, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(SDLK_U, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(SDLK_V, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(SDLK_W, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(SDLK_X, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(SDLK_Y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(SDLK_Z, KEY_KEY_Z));

	KeyMap.push_back(SKeyMap(SDLK_LGUI, KEY_LWIN));
	KeyMap.push_back(SKeyMap(SDLK_RGUI, KEY_RWIN));
	// apps missing
	KeyMap.push_back(SKeyMap(SDLK_POWER, KEY_SLEEP)); //??

	KeyMap.push_back(SKeyMap(SDLK_KP_0, KEY_NUMPAD0));
	KeyMap.push_back(SKeyMap(SDLK_KP_1, KEY_NUMPAD1));
	KeyMap.push_back(SKeyMap(SDLK_KP_2, KEY_NUMPAD2));
	KeyMap.push_back(SKeyMap(SDLK_KP_3, KEY_NUMPAD3));
	KeyMap.push_back(SKeyMap(SDLK_KP_4, KEY_NUMPAD4));
	KeyMap.push_back(SKeyMap(SDLK_KP_5, KEY_NUMPAD5));
	KeyMap.push_back(SKeyMap(SDLK_KP_6, KEY_NUMPAD6));
	KeyMap.push_back(SKeyMap(SDLK_KP_7, KEY_NUMPAD7));
	KeyMap.push_back(SKeyMap(SDLK_KP_8, KEY_NUMPAD8));
	KeyMap.push_back(SKeyMap(SDLK_KP_9, KEY_NUMPAD9));
	KeyMap.push_back(SKeyMap(SDLK_KP_MULTIPLY, KEY_MULTIPLY));
	KeyMap.push_back(SKeyMap(SDLK_KP_PLUS, KEY_ADD));
//	KeyMap.push_back(SKeyMap(SDLK_KP_, KEY_SEPARATOR));
	KeyMap.push_back(SKeyMap(SDLK_KP_MINUS, KEY_SUBTRACT));
	KeyMap.push_back(SKeyMap(SDLK_KP_PERIOD, KEY_DECIMAL));
	KeyMap.push_back(SKeyMap(SDLK_KP_DIVIDE, KEY_DIVIDE));

	KeyMap.push_back(SKeyMap(SDLK_F1,  KEY_F1));
	KeyMap.push_back(SKeyMap(SDLK_F2,  KEY_F2));
	KeyMap.push_back(SKeyMap(SDLK_F3,  KEY_F3));
	KeyMap.push_back(SKeyMap(SDLK_F4,  KEY_F4));
	KeyMap.push_back(SKeyMap(SDLK_F5,  KEY_F5));
	KeyMap.push_back(SKeyMap(SDLK_F6,  KEY_F6));
	KeyMap.push_back(SKeyMap(SDLK_F7,  KEY_F7));
	KeyMap.push_back(SKeyMap(SDLK_F8,  KEY_F8));
	KeyMap.push_back(SKeyMap(SDLK_F9,  KEY_F9));
	KeyMap.push_back(SKeyMap(SDLK_F10, KEY_F10));
	KeyMap.push_back(SKeyMap(SDLK_F11, KEY_F11));
	KeyMap.push_back(SKeyMap(SDLK_F12, KEY_F12));
	KeyMap.push_back(SKeyMap(SDLK_F13, KEY_F13));
	KeyMap.push_back(SKeyMap(SDLK_F14, KEY_F14));
	KeyMap.push_back(SKeyMap(SDLK_F15, KEY_F15));
	// no higher F-keys

	KeyMap.push_back(SKeyMap(SDLK_NUMLOCKCLEAR, KEY_NUMLOCK));
	KeyMap.push_back(SKeyMap(SDLK_SCROLLLOCK, KEY_SCROLL));
	KeyMap.push_back(SKeyMap(SDLK_LSHIFT, KEY_LSHIFT));
	KeyMap.push_back(SKeyMap(SDLK_RSHIFT, KEY_RSHIFT));
	KeyMap.push_back(SKeyMap(SDLK_LCTRL,  KEY_LCONTROL));
	KeyMap.push_back(SKeyMap(SDLK_RCTRL,  KEY_RCONTROL));
	KeyMap.push_back(SKeyMap(SDLK_LALT,  KEY_LMENU));
	KeyMap.push_back(SKeyMap(SDLK_RALT,  KEY_RMENU));

	KeyMap.push_back(SKeyMap(SDLK_PLUS,   KEY_PLUS));
	KeyMap.push_back(SKeyMap(SDLK_COMMA,  KEY_COMMA));
	KeyMap.push_back(SKeyMap(SDLK_MINUS,  KEY_MINUS));
	KeyMap.push_back(SKeyMap(SDLK_PERIOD, KEY_PERIOD));

	// some special keys missing

	KeyMap.sort();
}

void CIrrDeviceSDL3::resizeWindow(u32 x, u32 y) {
	if(window)
		SDL_SetWindowSize(window, x, y);
}

void CIrrDeviceSDL3::updateScreenTexture(core::dimension2d<u32> size) {
	if(VideoDriver->getDriverType() != video::EDT_SOFTWARE && VideoDriver->getDriverType() != video::EDT_BURNINGSVIDEO)
		return;
	int pitch;
	auto pixel_format = [format = VideoDriver->getColorFormat(), &pitch]() {
		switch(format) {
		case video::ECF_R8G8B8:
			pitch = 3;
			return SDL_PIXELFORMAT_XRGB8888;
		case video::ECF_A8R8G8B8:
			pitch = 4;
			return SDL_PIXELFORMAT_ARGB8888;
		case video::ECF_R5G6B5:
			pitch = 2;
			return SDL_PIXELFORMAT_RGB565;
		case video::ECF_A1R5G5B5:
		default:
			pitch = 2;
			return SDL_PIXELFORMAT_ARGB1555;
		}
	}();
	if(screen_texture) {
		if(pitch == screen_texture_pitch && pixel_format == screen_texture_color_format && size == screen_texture_size)
			return;
		SDL_DestroyTexture(screen_texture);
	}
	screen_texture_pitch = pitch;
	screen_texture_color_format = pixel_format;
	screen_texture_size = size;
	screen_texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, size.Width, size.Height);
}

CIrrDeviceSDL3::CCursorControl::CCursorControl(CIrrDeviceSDL3* dev) : Device(dev), IsVisible(true), CurCursor(gui::ECI_NORMAL)
{
	cursors[gui::ECI_NORMAL] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	cursors[gui::ECI_CROSS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	cursors[gui::ECI_HAND] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	cursors[gui::ECI_HELP] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	cursors[gui::ECI_IBEAM] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	cursors[gui::ECI_NO] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
	cursors[gui::ECI_WAIT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
	cursors[gui::ECI_SIZEALL] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
	cursors[gui::ECI_SIZENESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
	cursors[gui::ECI_SIZENWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
	cursors[gui::ECI_SIZENS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
	cursors[gui::ECI_SIZEWE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
	cursors[gui::ECI_UP] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	SDL_SetCursor(cursors[CurCursor]);
}

CIrrDeviceSDL3::CCursorControl::~CCursorControl()
{
	for(size_t i = 0; i < gui::ECI_COUNT; i++) {
		SDL_DestroyCursor(cursors[i]);
	}
}
void CIrrDeviceSDL3::CCursorControl::setActiveIcon(gui::ECURSOR_ICON iconId)
{
	if(CurCursor == iconId)
		return;
	CurCursor = iconId;
	SDL_SetCursor(cursors[iconId]);
}

} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL_DEVICE_

