// Copyright (C) 2020-2021 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef WIN_DRAG_N_DROPPER_H
#define WIN_DRAG_N_DROPPER_H
#include <oleidl.h>
#include <vector2d.h>

namespace irr {
class IrrlichtDevice;

class CDropTarget : public IDropTarget {
public:
	using callback_function = bool(*)(irr::core::vector2di pos, bool isFile);
	CDropTarget(HWND hwnd, callback_function callback, irr::IrrlichtDevice* dev) :
		window(hwnd), dragCheck(callback), device(dev) {
	};

	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override;
	virtual ULONG __stdcall AddRef() override {
		return static_cast<ULONG>(InterlockedIncrement(&ref_count));
	}
	virtual ULONG __stdcall Release() override {
		if(InterlockedDecrement(&ref_count) <= 0) {
			delete this;
			return 0;
		}
		return ref_count;
	}

	virtual HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

	virtual HRESULT __stdcall DragLeave() override;

	virtual HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
private:
	HWND window{ nullptr };
	LONG ref_count{ 1L };
	bool isDragging{ false };
	bool isFile{ false };
	callback_function dragCheck{ nullptr };
	irr::IrrlichtDevice* device{ nullptr };
	~CDropTarget() = default;

	bool CheckTarget(POINTL& point) const;
};

}

#endif //WIN_DRAG_N_DROPPER_H