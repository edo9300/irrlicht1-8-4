/*
copyright 2020 edo9300 see LICENSE
*/
#include "win_drag_n_dropper.h"

#include <objidl.h>
#include <vector>
#include <Shlobj.h>
#include <iostream>

constexpr FORMATETC textProperty = { CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
constexpr FORMATETC fileProperty = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

HRESULT __stdcall edoproDropper::QueryInterface(REFIID riid, void ** ppv) {
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
HRESULT __stdcall edoproDropper::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
	ready = false;
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
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall edoproDropper::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	*pdwEffect = DROPEFFECT_NONE;
	if(!dragCheck || (ScreenToClient(window, reinterpret_cast<LPPOINT>(&pt)) && dragCheck({ pt.x,pt.y }, isFile)))
		*pdwEffect = DROPEFFECT_MOVE;
	return *pdwEffect == DROPEFFECT_MOVE ? S_OK : S_FALSE;
}

HRESULT __stdcall edoproDropper::DragLeave(void) {
	ready = false;
	isDragging = false;
	return S_OK;
}

HRESULT __stdcall edoproDropper::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	std::lock_guard<std::mutex> lk(retrieving);
	isDragging = false;
	returns.clear();
	*pdwEffect = DROPEFFECT_COPY;
	STGMEDIUM stg;
	FORMATETC fe = (isFile ? fileProperty : textProperty);
	if(pDataObj->GetData(&fe, &stg) != S_OK)
		return S_FALSE;
	char* data = (char*)GlobalLock(stg.hGlobal);
	if(!data) {
		return S_FALSE;
	}
	if(isFile) {
		DROPFILES* filelist = (DROPFILES*)data;
		wchar_t* files = (wchar_t*)(data + (filelist->pFiles));
		while(*files) {
			returns.emplace_back(files);
			files += returns.back().size() + 1;
		}
	} else {
		returns.emplace_back((wchar_t*)data, GlobalSize(stg.hGlobal)/sizeof(wchar_t));
	}
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);
	ScreenToClient(window, reinterpret_cast<LPPOINT>(&pt));
	dropPosition = irr::core::vector2di({ pt.x,pt.y });
	ready = true;
	return S_OK;
}

bool edoproDropper::hasData() {
	std::lock_guard<std::mutex> lk(retrieving);
	return ready && returns.size();
}

std::vector<std::wstring> edoproDropper::getData(bool* isfile, irr::core::vector2di* dropPosition) {
	std::lock_guard<std::mutex> lk(retrieving);
	if(isfile)
		*isfile = isFile;
	if(dropPosition)
		*dropPosition = this->dropPosition;
	ready = false;
	return returns;
}
