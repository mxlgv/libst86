; SPDX-License-Identifier: MIT
; Copyright (c) 2024 Maxim Logaev

        global  st86_get_frame_addr

        section .text

st86_get_frame_addr:
%if __BITS__ = 64
        mov     rax, rbp 
%else
        mov     eax, ebp
%endif
        ret
