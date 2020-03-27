// Copyright (C) 2005-2006 Etienne Petitjean
// Copyright (C) 2007-2012 Christian Stehno
// Copyright (C) 2019-2020 Kevin Lu, Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later
// Modified from the Irrlicht Engine 1.8.4. See LICENSE.

#import "AppDelegate.h"
#include <string>

#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_

@implementation AppDelegate

- (id)initWithDevice:(irr::CIrrDeviceMacOSX *)device
{
	self = [super init];
	if (self) _device = device;
	_dockMenu = [[[NSMenu alloc] init] autorelease];
	return (self);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	_quit = FALSE;
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
	[NSApp orderFrontStandardAboutPanel:sender];
}

- (void)unhideAllApplications:(id)sender
{
	[NSApp unhideAllApplications:sender];
}

- (void)hide:(id)sender
{
	[NSApp hide:sender];
}

- (void)hideOtherApplications:(id)sender
{
	[NSApp hideOtherApplications:sender];
}

- (void)terminate:(id)sender
{
	_quit = TRUE;
}

- (void)windowWillClose:(id)sender
{
	_quit = TRUE;
}

- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)proposedFrameSize
{
	if (_device->isResizable())
		return proposedFrameSize;
	else
		return [window frame].size;
}

- (void)windowDidResize:(NSNotification *)aNotification
{
	NSWindow	*window;
	NSRect		frame;

	window = [aNotification object];
	frame = [window frame];
	_device->setResize((int)frame.size.width,(int)frame.size.height);
}

- (BOOL)isQuit
{
	return (_quit);
}

- (void)keyDown:(NSEvent *)event
{
	[self interpretKeyEvents:@[event]];
}

- (void)insertText:(id)string
{
	[self setString: @""];
	if ([string isKindOfClass:[NSAttributedString class]])
	{
		_device->handleInputEvent([[string string] UTF8String]);
	}
	else
	{
		_device->handleInputEvent([string UTF8String]);
	}
}

- (void)doCommandBySelector:(SEL)selector
{
	_device->processKeyEvent();
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	return _dockMenu;
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	NSPoint dropPoint = [sender draggingLocation];
	NSPasteboard *pasteboard = [sender draggingPasteboard];
    _dropIsFile = [[pasteboard types] containsObject:NSFilenamesPboardType];

	if (_device->isDraggable((int)dropPoint.x, (int)dropPoint.y, _dropIsFile) &&
		([sender draggingSourceOperationMask] & NSDragOperationGeneric) == NSDragOperationGeneric) {
		return NSDragOperationCopy;
	}

	return NSDragOperationNone; /* no idea what to do with this, reject it. */
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	NSPoint dropPoint = [sender draggingLocation];
	if (_device->isDraggable((int)dropPoint.x, (int)dropPoint.y, _dropIsFile))
		return NSDragOperationCopy;
	return NSDragOperationNone;
}

static int DecodeUTF8(wchar_t * dest, const char * src, int size) {
	const char* p = src;
	wchar_t* wp = dest;
	while(*p != 0 && (wp - dest < size)) {
		if((*p & 0x80) == 0) {
			*wp = *p;
			p++;
		} else if((*p & 0xe0) == 0xc0) {
			*wp = (((unsigned)p[0] & 0x1f) << 6) | ((unsigned)p[1] & 0x3f);
			p += 2;
		} else if((*p & 0xf0) == 0xe0) {
			*wp = (((unsigned)p[0] & 0xf) << 12) | (((unsigned)p[1] & 0x3f) << 6) | ((unsigned)p[2] & 0x3f);
			p += 3;
		} else if((*p & 0xf8) == 0xf0) {
			if(sizeof(wchar_t) == 2) {
				unsigned unicode = (((unsigned)p[0] & 0x7) << 18) | (((unsigned)p[1] & 0x3f) << 12) | (((unsigned)p[2] & 0x3f) << 6) | ((unsigned)p[3] & 0x3f);
				unicode -= 0x10000;
				*wp++ = (unicode >> 10) | 0xd800;
				*wp = (unicode & 0x3ff) | 0xdc00;
			} else {
				*wp = (((unsigned)p[0] & 0x7) << 18) | (((unsigned)p[1] & 0x3f) << 12) | (((unsigned)p[2] & 0x3f) << 6) | ((unsigned)p[3] & 0x3f);
			}
			p += 4;
		} else
			p++;
		wp++;
	}
	*wp = 0;
	return wp - dest;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{ @autoreleasepool
{
	NSPoint dropPoint = [sender draggingLocation];
	NSPasteboard *pasteboard = [sender draggingPasteboard];
	NSArray *types = [NSArray arrayWithObjects:NSStringPboardType,NSFilenamesPboardType,nil];
	NSString *desiredType = [pasteboard availableTypeFromArray:types];

	if (desiredType == nil) {
		return NO;  /* can't accept anything that's being dropped here. */
	}

	NSData *data = [pasteboard dataForType:desiredType];
	if (data == nil) {
		return NO;
	}

	irr::SEvent	irrevent;
	irrevent.EventType = irr::EET_DROP_EVENT;
	irrevent.DropEvent.DropType = irr::DROP_START;
	irrevent.DropEvent.X = dropPoint.x;
	irrevent.DropEvent.Y = dropPoint.y;
	irrevent.DropEvent.Text = nullptr;
	_device->postEventFromUser(irrevent);

	auto dispatch = ^ (irr::SEvent& irrevent, NSString *str) {
		auto cstr = [str UTF8String];
		size_t lenUTF8 = strlen(cstr);
		std::wstring wstr(lenUTF8 + 1, 0);
		size_t len = DecodeUTF8(&wstr[0], cstr, lenUTF8);
		irrevent.DropEvent.Text = wstr.c_str();

		if (!_device->postEventFromUser(irrevent)) {
			irrevent.DropEvent.Text = nullptr;
			irrevent.DropEvent.DropType = irr::DROP_END;
			_device->postEventFromUser(irrevent);
			return false;
		}
		return true;
	};

    if ([pasteboard dataForType:NSStringPboardType]) {
        NSString *str = [pasteboard stringForType:NSStringPboardType];
        irrevent.DropEvent.DropType = irr::DROP_TEXT;
        if (!dispatch(irrevent, str))
            return NO;
    }

    NSArray *fileArray = [pasteboard propertyListForType:NSFilenamesPboardType];
	for (NSString *path in fileArray) {
		NSURL *fileURL = [NSURL fileURLWithPath:path];
		NSNumber *isAlias = nil;

		[fileURL getResourceValue:&isAlias forKey:NSURLIsAliasFileKey error:nil];

		/* If the URL is an alias, resolve it. */
		if ([isAlias boolValue]) {
			NSURLBookmarkResolutionOptions opts = NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithoutUI;
			NSData *bookmark = [NSURL bookmarkDataWithContentsOfURL:fileURL error:nil];
			if (bookmark != nil) {
				NSURL *resolvedURL = [NSURL URLByResolvingBookmarkData:bookmark
															   options:opts
														 relativeToURL:nil
												   bookmarkDataIsStale:nil
																 error:nil];

				if (resolvedURL != nil) {
					fileURL = resolvedURL;
				}
			}
		}
		irrevent.DropEvent.DropType = irr::DROP_FILE;
		if (!dispatch(irrevent, [fileURL path]))
			return NO;
	}

	irrevent.DropEvent.Text = nullptr;
	irrevent.DropEvent.DropType = irr::DROP_END;
	_device->postEventFromUser(irrevent);
	return YES;
}}

@end

#endif // _IRR_COMPILE_WITH_OSX_DEVICE_
