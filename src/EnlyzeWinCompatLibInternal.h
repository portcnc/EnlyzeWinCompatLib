//
// EnlyzeWinCompatLib - Let Clang-compiled applications run on older Windows versions
// Copyright (c) 2021 Colin Finck, ENLYZE GmbH <c.finck@enlyze.com>
// SPDX-License-Identifier: MIT
//

#pragma once

#include "targetver.h"
#include <Windows.h>
#include <intrin.h>

template<typename FuncPtrToT, typename FuncPtrFromT>
FuncPtrToT CastFuncPtr(FuncPtrFromT ptr) {
	FuncPtrToT result{};

	memcpy(&result, &ptr, min(sizeof(result), sizeof(ptr)));

	return result;
}