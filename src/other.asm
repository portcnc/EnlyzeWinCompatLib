;
; EnlyzeWinCompatLib - Let Clang-compiled applications run on older Windows versions
; Copyright (c) 2025 Oliver Adams, Port CNC, Inc <oliver@portcnc.com>
; SPDX-License-Identifier: MIT
;

.model flat

.data

PUBLIC __fltused
__fltused dd 1

PUBLIC ___security_cookie
___security_cookie dd 0

.code

PUBLIC @__security_check_cookie@4
@__security_check_cookie@4 PROC
	ret
@__security_check_cookie@4 ENDP

PUBLIC ??_7type_info@@6B@
PUBLIC _type_info_dtor_stub
PUBLIC _type_info_deleting_dtor_stub

_type_info_dtor_stub PROC
	ret
_type_info_dtor_stub ENDP

_type_info_deleting_dtor_stub PROC
	ret
_type_info_deleting_dtor_stub ENDP

.data

ALIGN 4
	??_7type_info@@6B@ dd OFFSET _type_info_deleting_dtor_stub
	dd OFFSET _type_info_dtor_stub
	dd 0

END
