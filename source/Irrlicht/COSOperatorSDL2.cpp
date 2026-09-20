// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#include "COSOperatorSDL2.h"

#ifdef _IRR_COMPILE_WITH_SDL2_DEVICE_

#include <SDL2/SDL_clipboard.h>

namespace irr
{

COSOperatorSDL2::COSOperatorSDL2(const core::stringc& osversion) :
	base(osversion) {
#ifdef _DEBUG
	setDebugName("COSOperatorSDL2");
#endif
}

void COSOperatorSDL2::copyToClipboard(const wchar_t* wtext) const {
	if(wtext == nullptr)
		return;
	auto wlen = wcslen(wtext);
	if(wlen == 0)
		return;
	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);
	char* ctext = new char[lenOld];
	core::wcharToUtf8(wtext, ctext, lenOld);
	SDL_SetClipboardText(ctext);
	delete[] ctext;
}

const wchar_t* COSOperatorSDL2::getTextFromClipboard() const {
	if(!SDL_HasClipboardText())
		return nullptr;
	char* cbuffer = SDL_GetClipboardText();
	size_t lenOld = strlen(cbuffer);
	wchar_t* ws = new wchar_t[lenOld + 1];
	core::utf8ToWchar(cbuffer, ws, (lenOld + 1) * sizeof(wchar_t));
	ClipboardString = ws;
	delete[] ws;
	SDL_free(cbuffer);
	return ClipboardString.data();
}

}

#endif