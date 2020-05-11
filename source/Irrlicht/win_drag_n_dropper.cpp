// Copyright (C) 2020 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <IrrCompileConfig.h>
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
#include "win_drag_n_dropper.h"

#include <vector>
#include <string>
#include <Shlobj.h>
#include <IrrlichtDevice.h>

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

FORMATETC CheckFormat(IDataObject* pDataObj) {
	IEnumFORMATETC* Enum;
	FORMATETC type;
	FORMATETC nonunicode{ 0 };
	pDataObj->EnumFormatEtc(DATADIR_GET, &Enum);
	while(Enum->Next(1, &type, 0) == S_OK) {
		if(type.cfFormat == CF_UNICODETEXT) {
			return type;
		}
		if(type.cfFormat == CF_TEXT)
			nonunicode = type;
		if(type.cfFormat == CF_HDROP) {
			STGMEDIUM stg;
			if(pDataObj->GetData(&type, &stg) == S_OK) {
				DROPFILES* filelist = (DROPFILES*)GlobalLock(stg.hGlobal);
				bool unicode = filelist->fWide;
				GlobalUnlock(stg.hGlobal);
				ReleaseStgMedium(&stg);
				if(unicode)
					return type;
				else
					nonunicode = type;
			}
		}
	}
	return nonunicode;
}
#define checktarget (!dragCheck || (ScreenToClient(window, reinterpret_cast<LPPOINT>(&pt)) && dragCheck({ pt.x,pt.y }, isFile)))

HRESULT __stdcall edoproDropper::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	auto format = CheckFormat(pDataObj);
	if(!format.cfFormat) {
		isDragging = false;
		*pdwEffect = DROPEFFECT_NONE;
		return S_FALSE;
	}
	isFile = (format.cfFormat == CF_HDROP);
	isDragging = true;
	if(checktarget)
		*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall edoproDropper::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	*pdwEffect = DROPEFFECT_NONE;
	if(!checktarget)
		return S_FALSE;
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall edoproDropper::DragLeave(void) {
	isDragging = false;
	return S_OK;
}

template<typename T>
inline wchar_t* ToWideAllocated(T* _string) {
	if(std::is_same<T, wchar_t>::value)
		return (wchar_t*)_string;
	char* string = (char*)_string;
	size_t lenOld = mbstowcs(NULL, string, strlen(string));
	wchar_t *ws = new wchar_t[lenOld + 1];
	size_t len = mbstowcs(ws, string, lenOld);
	ws[len] = 0;
	return ws;
}

inline size_t len(const wchar_t* string) {
	return wcslen(string);
}
inline size_t len(const char* string) {
	return strlen(string);
}

template<typename T>
inline std::vector<wchar_t*> GetFileList(T* filelist) {
	std::vector<wchar_t*> res;
	while(*filelist) {
		res.push_back(ToWideAllocated(filelist));
		filelist += len(filelist) + 1;
	}
	return res;
}

HRESULT __stdcall edoproDropper::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	if(!isDragging || !ScreenToClient(window, reinterpret_cast<LPPOINT>(&pt)))
	   return S_FALSE;
	isDragging = false;
	*pdwEffect = DROPEFFECT_COPY;
	STGMEDIUM stg;
	FORMATETC fe = CheckFormat(pDataObj);
	if(pDataObj->GetData(&fe, &stg) != S_OK)
		return S_FALSE;
	char* data = nullptr;
	if(!(data = (char*)GlobalLock(stg.hGlobal))) {
		ReleaseStgMedium(&stg);
		return S_FALSE;
	}
	irr::SEvent event;
	event.EventType = irr::EET_DROP_EVENT;
	event.DropEvent.DropType = irr::DROP_START;
	event.DropEvent.X = pt.x;
	event.DropEvent.Y = pt.y;
	event.DropEvent.Text = nullptr;
	device->postEventFromUser(event);
	event.DropEvent.DropType = isFile ? irr::DROP_FILE : irr::DROP_TEXT;
	if(isFile) {
		DROPFILES* _filelist = (DROPFILES*)data;
		bool unicode = _filelist->fWide;
		void* files = (void*)(data + (_filelist->pFiles));
		auto filelist = unicode ? GetFileList((wchar_t*)files) : GetFileList((char*)files);
		for(auto& file : filelist) {
			event.DropEvent.Text = file;
			device->postEventFromUser(event);
			event.DropEvent.Text = nullptr;
			if(!unicode)
				delete[] file;
		}
	} else {
		if(fe.cfFormat == CF_TEXT)
			event.DropEvent.Text = ToWideAllocated(data);
		else
			event.DropEvent.Text = (wchar_t*)data;
		device->postEventFromUser(event);
		if(fe.cfFormat == CF_TEXT)
			delete[] event.DropEvent.Text;
		event.DropEvent.Text = nullptr;
	}
	event.DropEvent.DropType = irr::DROP_END;
	device->postEventFromUser(event);
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
	return S_OK;
}
#endif