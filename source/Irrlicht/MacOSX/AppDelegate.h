// Copyright (C) 2005-2006 Etienne Petitjean
// Copyright (C) 2007-2012 Christian Stehno
// Copyright (C) 2019-2020 Kevin Lu, Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later
// Modified from the Irrlicht Engine 1.8.4. See LICENSE.

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_

#import <Cocoa/Cocoa.h>
#import "CIrrDeviceMacOSX.h"

@interface AppDelegate : NSTextView <NSApplicationDelegate>
{
	BOOL			_quit;
	BOOL			_dropIsFile;
	irr::CIrrDeviceMacOSX	*_device;
	NSMenu *_dockMenu;
}

- (id)initWithDevice:(irr::CIrrDeviceMacOSX *)device;
- (BOOL)isQuit;
- (NSMenu *)applicationDockMenu:(NSApplication *)sender;

@end

#endif // _IRR_COMPILE_WITH_OSX_DEVICE_
