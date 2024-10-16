// Copyright (C) 2020-2021 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <IrrCompileConfig.h>
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
#include "CWin32DropTarget.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4091) //warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#endif
#include <Shlobj.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <IrrlichtDevice.h>
#include <irrArray.h>

namespace irr {

HRESULT STDMETHODCALLTYPE CDropTarget::QueryInterface(REFIID riid, void** ppv) {
	if(riid == IID_IUnknown) {
		*ppv = static_cast<IUnknown*>(this);
		return S_OK;
	} else if(riid == IID_IDropTarget) {
		*ppv = static_cast<IDropTarget*>(this);
		return S_OK;
	} else {
		*ppv = nullptr;
		return E_NOINTERFACE;
	}
}

namespace {

FORMATETC CheckFormat(IDataObject* pDataObj) {
	FORMATETC ret{ 0 };
	IEnumFORMATETC* Enum;
	if(pDataObj->EnumFormatEtc(DATADIR_GET, &Enum) != S_OK)
		return ret;
	bool unicode = false;
	bool found = false;
	FORMATETC cur_format;
	while(!unicode && Enum->Next(1, &cur_format, 0) == S_OK) {
		if(cur_format.cfFormat == CF_HDROP) {
			STGMEDIUM stg;
			if(pDataObj->GetData(&cur_format, &stg) == S_OK) {
				DROPFILES* filelist = static_cast<DROPFILES*>(GlobalLock(stg.hGlobal));
				if(filelist != nullptr) {
					unicode = filelist->fWide != 0;
					GlobalUnlock(stg.hGlobal);
					ret = cur_format;
					found = true;
				}
				ReleaseStgMedium(&stg);
			}
		} else if(cur_format.cfFormat == CF_UNICODETEXT || cur_format.cfFormat == CF_TEXT) {
			unicode = (cur_format.cfFormat == CF_UNICODETEXT);
			found = true;
			ret = cur_format;
		}
	}
	Enum->Release();
	if(!found)
		ret.cfFormat = 0;
	return ret;
}

inline bool ScreenToClient(HWND hWnd, POINTL& lpPoint) {
	return ScreenToClient(hWnd, reinterpret_cast<POINT*>(&lpPoint)) != 0;
}

}

bool CDropTarget::CheckTarget(POINTL& point) const {
	if(dragCheck == nullptr)
		return true;
	return ScreenToClient(window, point) && dragCheck({ point.x, point.y }, isFile);
}

HRESULT STDMETHODCALLTYPE CDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	const FORMATETC format = CheckFormat(pDataObj);
	if(!format.cfFormat) {
		isDragging = false;
		*pdwEffect = DROPEFFECT_NONE;
		return S_FALSE;
	}
	isFile = (format.cfFormat == CF_HDROP);
	isDragging = true;
	if(CheckTarget(pt))
		*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	*pdwEffect = DROPEFFECT_NONE;
	if(!CheckTarget(pt))
		return S_FALSE;
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDropTarget::DragLeave() {
	isDragging = false;
	return S_OK;
}

namespace {

inline wchar_t* ToWideAllocated(wchar_t* string, size_t* len = nullptr) {
	if(len)
		*len = wcslen(string);
	return string;
}

inline wchar_t* ToWideAllocated(char* string, size_t* ret_len = nullptr) {
	size_t len = strlen(string);
	if(ret_len)
		*ret_len = len;
	size_t lenOld = mbstowcs(nullptr, string, len);
	wchar_t* ws = new wchar_t[lenOld + 1];
	size_t outLen = mbstowcs(ws, string, lenOld);
	ws[outLen] = 0;
	return ws;
}

template<typename T>
inline core::array<wchar_t*> GetFileListFromText(T* filelist) {
	core::array<wchar_t*> res;
	while(*filelist) {
		size_t len;
		res.push_back(ToWideAllocated(filelist, &len));
		filelist += len + 1;
	}
	return res;
}

inline core::array<wchar_t*> GetFileList(void* data, bool& unicode) {
	DROPFILES* filelist = static_cast<DROPFILES*>(data);
	unicode = filelist->fWide != 0;
	void* files = (static_cast<char*>(data) + (filelist->pFiles));
	if(unicode)
		return GetFileListFromText(static_cast<wchar_t*>(files));
	return GetFileListFromText(static_cast<char*>(files));
}

}

HRESULT STDMETHODCALLTYPE CDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	if(!isDragging || !ScreenToClient(window, pt))
		return S_FALSE;
	isDragging = false;
	*pdwEffect = DROPEFFECT_COPY;
	FORMATETC fe = CheckFormat(pDataObj);
	if(fe.cfFormat == 0)
		return S_FALSE;
	STGMEDIUM stg;
	if(pDataObj->GetData(&fe, &stg) != S_OK)
		return S_FALSE;
	void* data = nullptr;
	if(!(data = GlobalLock(stg.hGlobal))) {
		ReleaseStgMedium(&stg);
		return S_FALSE;
	}
	SEvent event;
	event.EventType = EEVENT_TYPE::EET_DROP_EVENT;
	event.DropEvent.DropType = EDROP_TYPE::DROP_START;
	event.DropEvent.X = pt.x;
	event.DropEvent.Y = pt.y;
	event.DropEvent.Text = nullptr;
	device->postEventFromUser(event);
	if(isFile) {
		event.DropEvent.DropType = EDROP_TYPE::DROP_FILE;
		bool unicode;
		const core::array<wchar_t*> filelist = GetFileList(data, unicode);
		for(u32 i = 0; i < filelist.size(); i++) {
			wchar_t* file = filelist[i];
			event.DropEvent.Text = file;
			device->postEventFromUser(event);
			event.DropEvent.Text = nullptr;
			if(!unicode)
				delete[] file;
		}
	} else {
		event.DropEvent.DropType = EDROP_TYPE::DROP_TEXT;
		const bool unicode = fe.cfFormat != CF_TEXT;
		if(unicode)
			event.DropEvent.Text = static_cast<wchar_t*>(data);
		else
			event.DropEvent.Text = ToWideAllocated(static_cast<char*>(data));
		device->postEventFromUser(event);
		if(!unicode)
			delete[] event.DropEvent.Text;
		event.DropEvent.Text = nullptr;
	}
	event.DropEvent.DropType = EDROP_TYPE::DROP_END;
	device->postEventFromUser(event);
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
	return S_OK;
}

}
#endif
