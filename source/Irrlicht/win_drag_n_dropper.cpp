// Copyright (C) 2020 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <IrrCompileConfig.h>
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
#include "win_drag_n_dropper.h"

#include <objidl.h>
#include <vector>
#include <string>
#include <Shlobj.h>
#include <iostream>
#include <IrrlichtDevice.h>

constexpr FORMATETC textProperty = { CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
constexpr FORMATETC fileProperty = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

HRESULT __stdcall edoproDropper::QueryInterface(REFIID riid, void** ppv) {
	if(IsEqualIID(riid, IID_IUnknown)) {
		*ppv = static_cast<IUnknown*>(this);
		return S_OK;
	} else if(IsEqualIID(riid, IID_IDropTarget)) {
		*ppv = static_cast<IDropTarget*>(this);
		return S_OK;
	} else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}
#define checktarget (!dragCheck || (ScreenToClient(window, reinterpret_cast<LPPOINT>(&pt)) && dragCheck({ pt.x,pt.y }, isFile)))

HRESULT __stdcall edoproDropper::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	auto checkType = [&pDataObj](FORMATETC type) -> bool {
		return pDataObj->QueryGetData(&type) == S_OK;
	};
	isFile = checkType(fileProperty);
	if(!checkType(textProperty) && !isFile) {
		isDragging = false;
		*pdwEffect = DROPEFFECT_NONE;
		return S_FALSE;
	}
	isDragging = true;
	if(checktarget)
		*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall edoproDropper::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	*pdwEffect = DROPEFFECT_NONE;
	if(checktarget)
		*pdwEffect = DROPEFFECT_MOVE;
	return *pdwEffect == DROPEFFECT_MOVE ? S_OK : S_FALSE;
}

HRESULT __stdcall edoproDropper::DragLeave(void) {
	isDragging = false;
	return S_OK;
}

HRESULT __stdcall edoproDropper::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	if(!isDragging || !ScreenToClient(window, reinterpret_cast<LPPOINT>(&pt)))
	   return S_FALSE;
	isDragging = false;
	*pdwEffect = DROPEFFECT_COPY;
	STGMEDIUM stg;
	FORMATETC fe = (isFile ? fileProperty : textProperty);
	char* data = nullptr;
	if(pDataObj->GetData(&fe, &stg) != S_OK || !(data = (char*)GlobalLock(stg.hGlobal)))
		return S_FALSE;
	irr::SEvent event;
	event.EventType = irr::EET_DROP_EVENT;
	event.DropEvent.DropType = irr::DROP_START;
	event.DropEvent.X = pt.x;
	event.DropEvent.Y = pt.y;
	event.DropEvent.Text = nullptr;
	device->postEventFromUser(event);
	event.DropEvent.DropType = isFile ? irr::DROP_FILE : irr::DROP_TEXT;
	if(isFile) {
		DROPFILES* filelist = (DROPFILES*)data;
		wchar_t* files = (wchar_t*)(data + (filelist->pFiles));
		while(*files) {
			event.DropEvent.Text = files;
			device->postEventFromUser(event);
			event.DropEvent.Text = nullptr;
			files += wcslen(files) + 1;
		}
	} else {
		event.DropEvent.Text = (wchar_t*)data;
		device->postEventFromUser(event);
		event.DropEvent.Text = nullptr;
	}
	event.DropEvent.DropType = irr::DROP_END;
	device->postEventFromUser(event);
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
	return S_OK;
}
#endif
