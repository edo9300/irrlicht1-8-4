// Copyright (C) 2020-2021 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef C_WIN32_DROP_TARGET_H
#define C_WIN32_DROP_TARGET_H
#include <oleidl.h>
#include <vector2d.h>

namespace irr {
class IrrlichtDevice;

class CDropTarget : public IDropTarget {
public:
	using callback_function = bool(*)(core::vector2di pos, bool isFile);
	CDropTarget(HWND hwnd, callback_function callback, IrrlichtDevice* dev) :
		window(hwnd), ref_count(1), isDragging(false), isFile(false), dragCheck(callback), device(dev) {
	};

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) _IRR_OVERRIDE_;

	virtual ULONG STDMETHODCALLTYPE AddRef() _IRR_OVERRIDE_ {
		return static_cast<ULONG>(InterlockedIncrement(&ref_count));
	}

	virtual ULONG STDMETHODCALLTYPE Release() _IRR_OVERRIDE_ {
		if(InterlockedDecrement(&ref_count) <= 0) {
			delete this;
			return 0;
		}
		return ref_count;
	}

	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) _IRR_OVERRIDE_;

	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) _IRR_OVERRIDE_;

	virtual HRESULT STDMETHODCALLTYPE DragLeave() _IRR_OVERRIDE_;

	virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) _IRR_OVERRIDE_;
private:
	HWND window;
	LONG ref_count;
	bool isDragging;
	bool isFile;
	callback_function dragCheck;
	IrrlichtDevice* device;

	bool CheckTarget(POINTL& point) const;
};

}

#endif //C_WIN32_DROP_TARGET_H