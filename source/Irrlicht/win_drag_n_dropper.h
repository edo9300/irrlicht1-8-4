// Copyright (C) 2020 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef WIN_DRAG_N_DROPPER_H
#define WIN_DRAG_N_DROPPER_H
#include <oleidl.h>
#include <vector2d.h>

namespace irr {
class IrrlichtDevice;
}

class IrrDropTarget : public IDropTarget {
public:
	IrrDropTarget(HWND hwnd, bool(*callback)(irr::core::vector2di pos, bool isFile), irr::IrrlichtDevice* dev) :window(hwnd), dragCheck(callback), device(dev), ref_count(1){};
	~IrrDropTarget() = default;

	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override;
	virtual ULONG __stdcall AddRef() override { InterlockedIncrement((long*)&ref_count); return ref_count; }
	virtual ULONG __stdcall Release() override {
		if(InterlockedDecrement((long*)&ref_count) <= 0) {
			delete this;
			return 0;
		}
		return ref_count;
	}

	virtual HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;

	virtual HRESULT __stdcall DragLeave() override;

	virtual HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
private:
	HWND window;
	ULONG ref_count{0UL};
	bool isDragging{false};
	bool isFile{false};
	bool(*dragCheck)(irr::core::vector2di pos, bool isFile){nullptr};
	irr::IrrlichtDevice* device{nullptr};

};

#endif //WIN_DRAG_N_DROPPER_Hs