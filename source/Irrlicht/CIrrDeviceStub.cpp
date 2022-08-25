// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CIrrDeviceStub.h"
#include "ISceneManager.h"
#include "IEventReceiver.h"
#include "IFileSystem.h"
#include "IGUIEnvironment.h"
#include "os.h"
#include "IrrCompileConfig.h"
#include "CTimer.h"
#include "CLogger.h"
#include "irrString.h"
#include "IRandomizer.h"

namespace irr
{
//! constructor
CIrrDeviceStub::CIrrDeviceStub(const SIrrlichtCreationParameters& params)
: IrrlichtDevice(), VideoDriver(0), GUIEnvironment(0), SceneManager(0),
	Timer(0), CursorControl(0), UserReceiver(params.EventReceiver),
	Logger(0), Operator(0), Randomizer(0), FileSystem(0),
	InputReceivingSceneManager(0), ShouldTransformTouchEvents(false), TouchEmulatedDoubleClickMaxOffset(0),
	VideoModeList(0), ContextManager(0),
	CreationParams(params), Close(false)
{
	Timer = new CTimer(params.UsePerformanceTimer);
	if (os::Printer::Logger)
	{
		os::Printer::Logger->grab();
		Logger = (CLogger*)os::Printer::Logger;
		Logger->setReceiver(UserReceiver);
	}
	else
	{
		Logger = new CLogger(UserReceiver);
		os::Printer::Logger = Logger;
	}
	Logger->setLogLevel(CreationParams.LoggingLevel);

	os::Printer::Logger = Logger;
	Randomizer = createDefaultRandomizer();

	FileSystem = io::createFileSystem();
	VideoModeList = new video::CVideoModeList();

	core::stringc s = "Irrlicht Engine version ";
	s.append(getVersion());
	os::Printer::log(s.c_str(), ELL_INFORMATION);

	checkVersion(params.SDK_version_do_not_use);
}


CIrrDeviceStub::~CIrrDeviceStub()
{
	VideoModeList->drop();

	if (GUIEnvironment)
		GUIEnvironment->drop();

	if (SceneManager)
		SceneManager->drop();

	if (VideoDriver)
		VideoDriver->drop();

	if (ContextManager)
		ContextManager->drop();

	if ( FileSystem )
		FileSystem->drop();

	if (InputReceivingSceneManager)
		InputReceivingSceneManager->drop();

	if (CursorControl)
		CursorControl->drop();

	if (Operator)
		Operator->drop();

	if (Randomizer)
		Randomizer->drop();

	CursorControl = 0;

	if (Timer)
		Timer->drop();

	if (Logger->drop())
		os::Printer::Logger = 0;
}


void CIrrDeviceStub::createGUIAndScene()
{
	#ifdef _IRR_COMPILE_WITH_GUI_
	// create gui environment
	GUIEnvironment = gui::createGUIEnvironment(FileSystem, VideoDriver, Operator);
	#endif

	// create Scene manager
	SceneManager = scene::createSceneManager(VideoDriver, FileSystem, CursorControl, GUIEnvironment);

	setEventReceiver(UserReceiver);
}


//! returns the video driver
video::IVideoDriver* CIrrDeviceStub::getVideoDriver()
{
	return VideoDriver;
}


//! return file system
io::IFileSystem* CIrrDeviceStub::getFileSystem()
{
	return FileSystem;
}



//! returns the gui environment
gui::IGUIEnvironment* CIrrDeviceStub::getGUIEnvironment()
{
	return GUIEnvironment;
}



//! returns the scene manager
scene::ISceneManager* CIrrDeviceStub::getSceneManager()
{
	return SceneManager;
}


//! \return Returns a pointer to the ITimer object. With it the
//! current Time can be received.
ITimer* CIrrDeviceStub::getTimer()
{
	return Timer;
}


//! Returns the version of the engine.
const char* CIrrDeviceStub::getVersion() const
{
	return IRRLICHT_SDK_VERSION;
}

//! \return Returns a pointer to the mouse cursor control interface.
gui::ICursorControl* CIrrDeviceStub::getCursorControl()
{
	return CursorControl;
}


//! \return Returns a pointer to a list with all video modes supported
//! by the gfx adapter.
video::IVideoModeList* CIrrDeviceStub::getVideoModeList()
{
	return VideoModeList;
}

//! return the context manager
video::IContextManager* CIrrDeviceStub::getContextManager()
{
	return ContextManager;
}

//! checks version of sdk and prints warning if there might be a problem
bool CIrrDeviceStub::checkVersion(const char* version)
{
	if (strcmp(getVersion(), version))
	{
		core::stringc w;
		w = "Warning: The library version of the Irrlicht Engine (";
		w += getVersion();
		w += ") does not match the version the application was compiled with (";
		w += version;
		w += "). This may cause problems.";
		os::Printer::log(w.c_str(), ELL_WARNING);

		return false;
	}

	return true;
}


//! Compares to the last call of this function to return double and triple clicks.
u32 CIrrDeviceStub::checkSuccessiveClicks(s32 mouseX, s32 mouseY, EMOUSE_INPUT_EVENT inputEvent, s32 maxMouseOffset)
{
	irr::u32 clickTime = getTimer()->getRealTime();

	if ( (clickTime-MouseMultiClicks.LastClickTime) < MouseMultiClicks.DoubleClickTime
		&& core::abs_(MouseMultiClicks.LastClick.X - mouseX ) <= maxMouseOffset
		&& core::abs_(MouseMultiClicks.LastClick.Y - mouseY ) <= maxMouseOffset
		&& MouseMultiClicks.CountSuccessiveClicks < 3
		&& MouseMultiClicks.LastMouseInputEvent == inputEvent
	   )
	{
		++MouseMultiClicks.CountSuccessiveClicks;
	}
	else
	{
		MouseMultiClicks.CountSuccessiveClicks = 1;
	}

	MouseMultiClicks.LastMouseInputEvent = inputEvent;
	MouseMultiClicks.LastClickTime = clickTime;
	MouseMultiClicks.LastClick.X = mouseX;
	MouseMultiClicks.LastClick.Y = mouseY;

	return MouseMultiClicks.CountSuccessiveClicks;
}

bool CIrrDeviceStub::transformToMultiClickEvent(irr::SEvent& event, s32 maxMouseOffset) {
	if(event.MouseInput.Event >= irr::EMIE_LMOUSE_PRESSED_DOWN && event.MouseInput.Event <= irr::EMIE_MMOUSE_PRESSED_DOWN) {
		irr::u32 clicks = checkSuccessiveClicks(event.MouseInput.X, event.MouseInput.Y, event.MouseInput.Event);
		if(clicks == 2) {
			event.MouseInput.Event = (irr::EMOUSE_INPUT_EVENT)(irr::EMIE_LMOUSE_DOUBLE_CLICK + event.MouseInput.Event - irr::EMIE_LMOUSE_PRESSED_DOWN);
			return true;
		} else if(clicks == 3) {
			event.MouseInput.Event = (irr::EMOUSE_INPUT_EVENT)(irr::EMIE_LMOUSE_TRIPLE_CLICK + event.MouseInput.Event - irr::EMIE_LMOUSE_PRESSED_DOWN);
			return true;
		}
	}
	return false;
}

bool CIrrDeviceStub::SendTransformedTouchEvent(const SEvent& event) {
	if(event.EventType != irr::EET_TOUCH_INPUT_EVENT)
		return false;
	irr::SEvent translated;
	memset(&translated, 0, sizeof(irr::SEvent));
	translated.EventType = irr::EET_MOUSE_INPUT_EVENT;

	translated.MouseInput.X = event.TouchInput.X;
	translated.MouseInput.Y = event.TouchInput.Y;

	switch(event.TouchInput.touchedCount) {
	case 1: {
		switch(event.TouchInput.Event) {
		case irr::ETIE_PRESSED_DOWN: {
			LastTouchPosition = core::position2di(event.TouchInput.X, event.TouchInput.Y);
			translated.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
			translated.MouseInput.ButtonStates = irr::EMBSM_LEFT;
			irr::SEvent hoverEvent = translated;
			hoverEvent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			hoverEvent.MouseInput.ButtonStates = 0;
			postEventFromUser(hoverEvent);
			break;
		}
		case irr::ETIE_MOVED:
			LastTouchPosition = core::position2di(event.TouchInput.X, event.TouchInput.Y);
			translated.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			translated.MouseInput.ButtonStates = irr::EMBSM_LEFT;
			break;
		case irr::ETIE_LEFT_UP:
			translated.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
			translated.MouseInput.ButtonStates = 0;
			// we don't have a valid pointer element use last
			// known pointer pos
			translated.MouseInput.X = LastTouchPosition.X;
			translated.MouseInput.Y = LastTouchPosition.Y;
			LastTouchPosition = core::position2di(0, 0);
			break;
		default:
			return true;
		}

		bool retval = postEventFromUser(translated);
		if(transformToMultiClickEvent(translated, TouchEmulatedDoubleClickMaxOffset))
			postEventFromUser(translated);
		return retval;
	}
	case 2: {
		if(event.TouchInput.Event != irr::ETIE_PRESSED_DOWN)
			return false;
		translated.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
		translated.MouseInput.ButtonStates = irr::EMBSM_LEFT | irr::EMBSM_RIGHT;
		translated.MouseInput.X = LastTouchPosition.X;
		translated.MouseInput.Y = LastTouchPosition.Y;
		postEventFromUser(translated);

		translated.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
		translated.MouseInput.ButtonStates = irr::EMBSM_LEFT;

		postEventFromUser(translated);
		break;
	}
	default:
		return false;
	}
	return true;
}


//! send the event to the right receiver
bool CIrrDeviceStub::postEventFromUser(const SEvent& event)
{
	bool absorbed = false;

	if (UserReceiver)
		absorbed = UserReceiver->OnEvent(event);

	if (!absorbed && GUIEnvironment)
		absorbed = GUIEnvironment->postEventFromUser(event);

	scene::ISceneManager* inputReceiver = InputReceivingSceneManager;
	if (!inputReceiver)
		inputReceiver = SceneManager;

	if (!absorbed && inputReceiver)
		absorbed = inputReceiver->postEventFromUser(event);

	if(ShouldTransformTouchEvents)
		absorbed = SendTransformedTouchEvent(event);

	return absorbed;
}


//! Sets a new event receiver to receive events
void CIrrDeviceStub::setEventReceiver(IEventReceiver* receiver)
{
	UserReceiver = receiver;
	Logger->setReceiver(receiver);
	if (GUIEnvironment)
		GUIEnvironment->setUserEventReceiver(receiver);
}


//! Returns poinhter to the current event receiver. Returns 0 if there is none.
IEventReceiver* CIrrDeviceStub::getEventReceiver()
{
	return UserReceiver;
}


//! \return Returns a pointer to the logger.
ILogger* CIrrDeviceStub::getLogger()
{
	return Logger;
}


//! Returns the operation system opertator object.
IOSOperator* CIrrDeviceStub::getOSOperator()
{
	return Operator;
}


//! Provides access to the engine's currently set randomizer.
IRandomizer* CIrrDeviceStub::getRandomizer() const
{
	return Randomizer;
}

//! Sets a new randomizer.
void CIrrDeviceStub::setRandomizer(IRandomizer* r)
{
	if (r!=Randomizer)
	{
		if (Randomizer)
			Randomizer->drop();
		Randomizer=r;
		if (Randomizer)
			Randomizer->grab();
	}
}

namespace
{
	struct SDefaultRandomizer : public IRandomizer
	{
		virtual void reset(s32 value=0x0f0f0f0f) _IRR_OVERRIDE_
		{
			os::Randomizer::reset(value);
		}

		virtual s32 rand() const _IRR_OVERRIDE_
		{
			return os::Randomizer::rand();
		}

		virtual f32 frand() const _IRR_OVERRIDE_
		{
			return os::Randomizer::frand();
		}

		virtual s32 randMax() const _IRR_OVERRIDE_
		{
			return os::Randomizer::randMax();
		}
	};
}

//! Creates a new default randomizer.
IRandomizer* CIrrDeviceStub::createDefaultRandomizer() const
{
	IRandomizer* r = new SDefaultRandomizer();
	if (r)
		r->reset();
	return r;
}


//! Sets the input receiving scene manager.
void CIrrDeviceStub::setInputReceivingSceneManager(scene::ISceneManager* sceneManager)
{
	if (sceneManager)
		sceneManager->grab();
	if (InputReceivingSceneManager)
		InputReceivingSceneManager->drop();

	InputReceivingSceneManager = sceneManager;
}


//! Checks if the window is running in fullscreen mode
bool CIrrDeviceStub::isFullscreen() const
{
	return CreationParams.Fullscreen;
}


//! returns color format
video::ECOLOR_FORMAT CIrrDeviceStub::getColorFormat() const
{
	return video::ECF_R5G6B5;
}

//! No-op in this implementation
bool CIrrDeviceStub::activateJoysticks(core::array<SJoystickInfo> & joystickInfo)
{
	return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::activateAccelerometer(float updateInterval)
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::deactivateAccelerometer()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::isAccelerometerActive()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::isAccelerometerAvailable()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::activateGyroscope(float updateInterval)
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::deactivateGyroscope()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::isGyroscopeActive()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::isGyroscopeAvailable()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::activateDeviceMotion(float updateInterval)
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::deactivateDeviceMotion()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::isDeviceMotionActive()
{
    return false;
}

//! No-op in this implementation
bool CIrrDeviceStub::isDeviceMotionAvailable()
{
    return false;
}

/*!
*/
void CIrrDeviceStub::calculateGammaRamp ( u16 *ramp, f32 gamma, f32 relativebrightness, f32 relativecontrast )
{
	s32 i;
	s32 value;
	s32 rbright = (s32) ( relativebrightness * (65535.f / 4 ) );
	f32 rcontrast = 1.f / (255.f - ( relativecontrast * 127.5f ) );

	gamma = gamma > 0.f ? 1.0f / gamma : 0.f;

	for ( i = 0; i < 256; ++i )
	{
		value = (s32)(pow( rcontrast * i, gamma)*65535.f + 0.5f );
		ramp[i] = (u16) core::s32_clamp ( value + rbright, 0, 65535 );
	}

}

void CIrrDeviceStub::calculateGammaFromRamp ( f32 &gamma, const u16 *ramp )
{
	/* The following is adapted from a post by Garrett Bass on OpenGL
	Gamedev list, March 4, 2000.
	*/
	f32 sum = 0.0;
	s32 i, count = 0;

	gamma = 1.0;
	for ( i = 1; i < 256; ++i ) {
		if ( (ramp[i] != 0) && (ramp[i] != 65535) ) {
			f32 B = (f32)i / 256.f;
			f32 A = ramp[i] / 65535.f;
			sum += (f32) ( logf(A) / logf(B) );
			count++;
		}
	}
	if ( count && sum ) {
		gamma = 1.0f / (sum / count);
	}

}

//! Set the current Gamma Value for the Display
bool CIrrDeviceStub::setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast )
{
	return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceStub::getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast )
{
	return false;
}

//! Set the maximal elapsed time between 2 clicks to generate doubleclicks for the mouse. It also affects tripleclick behavior.
void CIrrDeviceStub::setDoubleClickTime( u32 timeMs )
{
	MouseMultiClicks.DoubleClickTime = timeMs;
}

//! Get the maximal elapsed time between 2 clicks to generate double- and tripleclicks for the mouse.
u32 CIrrDeviceStub::getDoubleClickTime() const
{
	return MouseMultiClicks.DoubleClickTime;
}

void CIrrDeviceStub::toggleTouchEventMouseTranslation(bool enable, int doubleClickMaxOffset) {
	if((ShouldTransformTouchEvents = enable) == false)
		return;
	TouchEmulatedDoubleClickMaxOffset = doubleClickMaxOffset;
}



} // end namespace irr

