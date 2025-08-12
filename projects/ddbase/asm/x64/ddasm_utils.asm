;.586               ;Target processor.  Use instructions for Pentium class machines
;.MODEL FLAT, C      ;Use the flat memory model. Use C calling conventions
;.STACK             ;Define a stack segment of 1KB (Not required for this example)
;.DATA              ;Create a near data segment.  Local variables are declared after
                    ;this directive (Not required for this example)
.CODE               ;Indicates the start of a code segment.

ddcpu_pause_asm PROC
    pause
    ret 
ddcpu_pause_asm ENDP 
END 