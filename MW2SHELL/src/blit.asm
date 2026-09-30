; Hand-written assembly: the blit, shape, font, picture and palette routines, a MASM object
; assembled with MASM 6.11 (ML). It starts at 0x10032250, flush against the compiled C code that
; ends at 0x1003224f, and its data at 0x100687cc. The evidence that it was MASM:
;
; - Every routine's frame is the one ML generates for a PROC with USES and parameters: push ebp;
;   mov ebp, esp; add esp, -N for LOCALs (the VC++ 4.1 front ends emit sub esp, N); push ebx; push
;   esi; push edi; and the full epilogue at every ret.
; - Every routine saves and reloads es (push es; cld; push ds; pop es), which the compiler never
;   does for flat-model code.
; - Jumps are short (rel8) wherever they reach, as ML sizes them.
; - Its data is packed without alignment: the "MCGA.DLL" buffer at 0x10068800 is followed by
;   variables from 0x1006880d onward.
;
; The cosine table (0x10035af0), the IFF chunk tags after WriteViewRow and the LFSR tap table
; between FUN_100376f9 and DissolveView sit in .text, as in the original. Annotated by name in
; blit.h; COMPAT_MODE builds take blit.c's stubs.

	.386
	.model flat, c
	option noscoped
	option casemap:none

	.data

; The display driver's 13 entry points (SetDisplayDriver). The first returns the driver's name
; (GetDisplayDriverName); FadeViewColors calls others.
	public g_displayDriver
g_displayDriver_t struct
m_data dd 0dh dup (0)
g_displayDriver_t ends
g_displayDriver g_displayDriver_t <>

; The display driver's name: GetDisplayDriverName copies it in and returns it. The next variable
; the assembly references starts at 0x1006880d, so the buffer is 0xd bytes.
	public g_displayDriverName
g_displayDriverName_t struct
m_data db "MCGA.DLL", 5 dup (0)
g_displayDriverName_t ends
g_displayDriverName g_displayDriverName_t <>

; The run-length encoder's state (EncodeViewRle, EncodeRleRow, EmitRleRun): the output
; buffer (NULL only measures), the pending skip, the row and run pointers, the output
; cursor and the start of the current run.
	public g_rleOutput
g_rleOutput dd 0

	public g_rleSkip
g_rleSkip dd 0

	public g_rleRow
g_rleRow dd 0

	public g_rleRun
g_rleRun dd 0

	public g_rleCursor
g_rleCursor dd 0

	public g_rleRunStart
g_rleRunStart_t struct
m_data dd 5 dup (0)
g_rleRunStart_t ends
g_rleRunStart g_rleRunStart_t <>

; The bounding box of the opaque pixels that EncodeViewRle finds: left, top, right, bottom.
	public g_rleLeft
g_rleLeft dd 0

	public g_rleTop
g_rleTop dd 0

	public g_rleRight
g_rleRight dd 0

	public g_rleBottom
g_rleBottom dd 0

; A dword table that DissolveView fills and reads. The next variable starts at 0x10068d45.
; What its entries hold hasn't been worked out, so it and the second table keep placeholders.
	public g_unk0x10068845
g_unk0x10068845_t struct
m_data dd 140h dup (0)
g_unk0x10068845_t ends
g_unk0x10068845 g_unk0x10068845_t <>

; Scanline buffer: BlitPicture decodes one RLE scanline (up to the ushort width at data
; header +0x42) into it, and GifPutPixel collects one GIF row, before blitting it through
; WriteViewRow. FadeViewColors also keeps its working palette here. The next variable starts at
; 0x10069045.
	public g_scanline
g_scanline_t struct
m_data db 300h dup (0)
g_scanline_t ends
g_scanline g_scanline_t <>

; The colors FadeViewColors found in the view.
	public g_fadeColors
g_fadeColors_t struct
m_data db 100h dup (0)
g_fadeColors_t ends
g_fadeColors g_fadeColors_t <>

; FadeViewColors's per-component distances to the target palette.
	public g_fadeDistances
g_fadeDistances_t struct
m_data db 300h dup (0)
g_fadeDistances_t ends
g_fadeDistances g_fadeDistances_t <>

; A second dword table of DissolveView. The next variable starts at 0x10069a45.
	public g_unk0x10069445
g_unk0x10069445_t struct
m_data dd 180h dup (0)
g_unk0x10069445_t ends
g_unk0x10069445 g_unk0x10069445_t <>

; Flags per color index (FadeViewColors, CountViewColors), then FadeViewColors's per-component
; directions. It serves the two routines differently, so no one name fits.
	public g_unk0x10069a45
g_unk0x10069a45_t struct
m_data db 300h dup (0)
g_unk0x10069a45_t ends
g_unk0x10069a45 g_unk0x10069a45_t <>

; FadeViewColors's per-component error accumulators.
	public g_fadeErrors
g_fadeErrors_t struct
m_data db 300h dup (0)
g_fadeErrors_t ends
g_fadeErrors g_fadeErrors_t <>

; Masks of the low n bits, indexed by the code width.
	public g_gifCodeMasks
g_gifCodeMasks_t struct
m_data db 0, 1, 3, 7, 0fh, 1fh, 3fh, 7fh, 0ffh
g_gifCodeMasks_t ends
g_gifCodeMasks g_gifCodeMasks_t <>

; GIF interlacing: the row step of each pass...
	public g_gifPassSteps
g_gifPassSteps_t struct
m_data db 8, 8, 4, 2, 0
g_gifPassSteps_t ends
g_gifPassSteps g_gifPassSteps_t <>

; ...and the first row of the next pass.
	public g_gifPassStarts
g_gifPassStarts_t struct
m_data db 0, 4, 2, 1, 0
g_gifPassStarts_t ends
g_gifPassStarts g_gifPassStarts_t <>

; The view BlitGif decodes into.
	public g_gifView
g_gifView dd 0

; The color remap table that SetRemapTable loads and BlitShpFrameRemappedUnclipped and
; RemapShpFrame apply.
	public g_remapTable
g_remapTable_t struct
m_data db 100h dup (0)
g_remapTable_t ends
g_remapTable g_remapTable_t <>

; BlitRotated's four transformed corners, five dwords each.
	public g_rotatedCorners
g_rotatedCorners_t struct
m_data dd 14h dup (0)
g_rotatedCorners_t ends
g_rotatedCorners g_rotatedCorners_t <>

; BlitRotated's per-corner steps, indexed by corner.
	public g_rotatedCornerSteps
g_rotatedCornerSteps_t struct
m_data dd 4 dup (0)
g_rotatedCornerSteps_t ends
g_rotatedCornerSteps g_rotatedCornerSteps_t <>

	.code

; Returns the name of a display driver: calls its first entry point and copies the string it
; returns into g_displayDriverName.
GetDisplayDriverName proc uses ebx esi edi, p_driver:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_driver
	call dword ptr [esi]
	mov edi, offset g_displayDriverName
jmp_10032264:
	mov bl, byte ptr [eax]
	mov byte ptr [edi], bl
	inc edi
	inc eax
	or bl, bl
	jne jmp_10032264
	mov eax, offset g_displayDriverName
	pop es
	ret
GetDisplayDriverName endp

; Installs a display driver: copies its 13 entry points into g_displayDriver.
SetDisplayDriver proc uses ebx esi edi, p_driver:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_driver
	mov edi, offset g_displayDriver
	mov ecx, 0dh
	rep movsd
	pop es
	ret
SetDisplayDriver endp

; Writes one pixel at (p_x, p_y), relative to the view, and returns the pixel it replaced.
; Returns -1 for an empty buffer, -2 for an empty view and -3 when the point is clipped.
PutViewPixel proc uses ebx esi edi, p_view:dword, p_x:dword, p_y:dword, p_color:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x18, eax
	jle jmp_10032317
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10032317
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x1c, eax
	cmp eax, 0
	jg jmp_100322cb
	mov eax, 0
jmp_100322cb:
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x20, eax
	cmp eax, 0
	jg jmp_100322de
	mov eax, 0
jmp_100322de:
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x18
	dec edx
	cmp eax, edx
	jl jmp_100322ee
	mov eax, edx
jmp_100322ee:
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_100322fd
	mov eax, edx
jmp_100322fd:
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr l_unk0x0c
	cmp eax, dword ptr l_unk0x04
	jl jmp_10032322
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr l_unk0x08
	jl jmp_10032322
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x14, eax
	jmp jmp_1003232d
jmp_10032317:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10032322:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_1003232d:
	mov ecx, dword ptr p_x
	mov ebx, dword ptr p_y
	add ecx, dword ptr l_unk0x1c
	add ebx, dword ptr l_unk0x20
	cmp ecx, dword ptr l_unk0x04
	jl jmp_10032368
	cmp ecx, dword ptr l_unk0x0c
	jg jmp_10032368
	cmp ebx, dword ptr l_unk0x08
	jl jmp_10032368
	cmp ebx, dword ptr l_unk0x10
	jg jmp_10032368
	mov eax, ebx
	imul dword ptr l_unk0x18
	add eax, dword ptr l_unk0x14
	add eax, ecx
	mov ebx, eax
	xor eax, eax
	mov al, byte ptr [ebx]
	mov dl, byte ptr p_color
	mov byte ptr [ebx], dl
	pop es
	ret
jmp_10032368:
	mov eax, 0fffffffdh
	pop es
	ret
PutViewPixel endp

; Reads the pixel at (p_x, p_y), relative to the view. Returns -1 for an empty buffer, -2 for
; an empty view and -3 when the point is clipped.
GetViewPixel proc uses ebx esi edi, p_view:dword, p_x:dword, p_y:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x18, eax
	jle jmp_100323f2
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_100323f2
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x1c, eax
	cmp eax, 0
	jg jmp_100323a6
	mov eax, 0
jmp_100323a6:
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x20, eax
	cmp eax, 0
	jg jmp_100323b9
	mov eax, 0
jmp_100323b9:
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x18
	dec edx
	cmp eax, edx
	jl jmp_100323c9
	mov eax, edx
jmp_100323c9:
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_100323d8
	mov eax, edx
jmp_100323d8:
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr l_unk0x0c
	cmp eax, dword ptr l_unk0x04
	jl jmp_100323fd
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr l_unk0x08
	jl jmp_100323fd
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x14, eax
	jmp jmp_10032408
jmp_100323f2:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_100323fd:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10032408:
	mov ecx, dword ptr p_x
	mov ebx, dword ptr p_y
	add ecx, dword ptr l_unk0x1c
	add ebx, dword ptr l_unk0x20
	cmp ecx, dword ptr l_unk0x04
	jl jmp_1003243e
	cmp ecx, dword ptr l_unk0x0c
	jg jmp_1003243e
	cmp ebx, dword ptr l_unk0x08
	jl jmp_1003243e
	cmp ebx, dword ptr l_unk0x10
	jg jmp_1003243e
	mov eax, ebx
	imul dword ptr l_unk0x18
	add eax, dword ptr l_unk0x14
	add eax, ecx
	mov ebx, eax
	xor eax, eax
	mov al, byte ptr [ebx]
	pop es
	ret
jmp_1003243e:
	mov eax, 0fffffffdh
	pop es
	ret
GetViewPixel endp

; Fills or processes the clipped rectangle of the view. p_unk0x14 selects the mode; in some
; modes p_color is a callback the routine calls. Returns a negative code when the view is empty
; or the rectangle is fully clipped.
BlitLine proc uses ebx esi edi, p_view:dword, p_left:dword, p_top:dword, p_right:dword, p_bottom:dword, p_unk0x14:dword, p_color:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword, l_unk0x48:dword
	local l_unk0x4c:dword, l_unk0x50:dword, l_unk0x54:dword, l_unk0x58:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x50, eax
	jle jmp_100324c8
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_100324c8
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x54, eax
	cmp eax, 0
	jg jmp_1003247c
	mov eax, 0
jmp_1003247c:
	mov dword ptr l_unk0x3c, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x58, eax
	cmp eax, 0
	jg jmp_1003248f
	mov eax, 0
jmp_1003248f:
	mov dword ptr l_unk0x40, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x50
	dec edx
	cmp eax, edx
	jl jmp_1003249f
	mov eax, edx
jmp_1003249f:
	mov dword ptr l_unk0x44, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_100324ae
	mov eax, edx
jmp_100324ae:
	mov dword ptr l_unk0x48, eax
	mov eax, dword ptr l_unk0x44
	cmp eax, dword ptr l_unk0x3c
	jl jmp_100324d3
	mov eax, dword ptr l_unk0x48
	cmp eax, dword ptr l_unk0x40
	jl jmp_100324d3
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x4c, eax
	jmp jmp_100324de
jmp_100324c8:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_100324d3:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_100324de:
	mov eax, dword ptr l_unk0x54
	add dword ptr p_left, eax
	add dword ptr p_right, eax
	mov eax, dword ptr l_unk0x58
	add dword ptr p_top, eax
	add dword ptr p_bottom, eax
	mov eax, dword ptr p_right
	sub eax, dword ptr p_left
	mov dword ptr l_unk0x04, eax
	cdq
	mov dword ptr l_unk0x0c, edx
	xor eax, edx
	sub eax, edx
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr p_bottom
	sub eax, dword ptr p_top
	mov dword ptr l_unk0x10, eax
	cdq
	mov dword ptr l_unk0x18, edx
	xor eax, edx
	sub eax, edx
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr p_left
	mov dword ptr l_unk0x24, eax
	mov eax, dword ptr p_right
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr p_top
	mov dword ptr l_unk0x28, eax
	mov eax, dword ptr p_bottom
	mov dword ptr l_unk0x30, eax
	cmp dword ptr l_unk0x04, 0
	je jmp_10032c5f
	cmp dword ptr l_unk0x10, 0
	je jmp_10032cea
	mov eax, dword ptr l_unk0x0c
	xor eax, dword ptr l_unk0x18
	mov dword ptr l_unk0x1c, eax
	mov edx, dword ptr l_unk0x08
	mov ebx, dword ptr l_unk0x14
	mov eax, 0ffffffffh
	cmp edx, ebx
	je jmp_10032564
	jl jmp_10032560
	xchg edx, ebx
jmp_10032560:
	xor eax, eax
	div ebx
jmp_10032564:
	mov dword ptr l_unk0x20, eax
	mov dword ptr l_unk0x34, 0
jmp_1003256e:
	xor edx, edx
	mov eax, dword ptr l_unk0x24
	sub eax, dword ptr l_unk0x3c
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x44
	sub eax, dword ptr l_unk0x24
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr l_unk0x40
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x48
	sub eax, dword ptr l_unk0x28
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x2c
	sub eax, dword ptr l_unk0x3c
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x44
	sub eax, dword ptr l_unk0x2c
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x30
	sub eax, dword ptr l_unk0x40
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x48
	sub eax, dword ptr l_unk0x30
	shl eax, 1
	adc dh, dh
	or dword ptr l_unk0x34, edx
	or edx, edx
	je jmp_1003292d
	test dl, dh
	jne jmp_10032e40
	mov ebx, dword ptr l_unk0x08
	cmp ebx, dword ptr l_unk0x14
	jl jmp_10032628
	test dl, 8
	jne jmp_10032671
	test dl, 4
	jne jmp_100326c5
	test dl, 2
	jne jmp_10032749
	test dl, 1
	jne jmp_100327a1
	test dh, 8
	jne jmp_100327d1
	test dh, 4
	jne jmp_1003282c
	test dh, 2
	jne jmp_100328ab
	test dh, 1
	jne jmp_10032902
	jmp jmp_1003256e
jmp_10032628:
	test dl, 8
	jne jmp_10032699
	test dl, 4
	jne jmp_100326f1
	test dl, 2
	jne jmp_10032721
	test dl, 1
	jne jmp_10032775
	test dh, 8
	jne jmp_100327fd
	test dh, 4
	jne jmp_10032854
	test dh, 2
	jne jmp_1003287f
	test dh, 1
	jne jmp_100328da
	jmp jmp_1003256e
jmp_10032671:
	mov eax, dword ptr l_unk0x3c
	mov dword ptr l_unk0x24, eax
	sub eax, dword ptr p_left
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x28, eax
	jmp jmp_1003256e
jmp_10032699:
	mov eax, dword ptr l_unk0x3c
	mov dword ptr l_unk0x24, eax
	sub eax, dword ptr p_left
	mov edx, eax
	dec edx
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, -1
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x28, eax
	jmp jmp_1003256e
jmp_100326c5:
	mov eax, dword ptr l_unk0x44
	mov dword ptr l_unk0x24, eax
	sub eax, dword ptr p_left
	neg eax
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x28, eax
	jmp jmp_1003256e
jmp_100326f1:
	mov eax, dword ptr l_unk0x44
	mov dword ptr l_unk0x24, eax
	sub eax, dword ptr p_left
	neg eax
	mov edx, eax
	dec edx
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, -1
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x28, eax
	jmp jmp_1003256e
jmp_10032721:
	mov eax, dword ptr l_unk0x40
	mov dword ptr l_unk0x28, eax
	sub eax, dword ptr p_top
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x24, eax
	jmp jmp_1003256e
jmp_10032749:
	mov eax, dword ptr l_unk0x40
	mov dword ptr l_unk0x28, eax
	sub eax, dword ptr p_top
	mov edx, eax
	dec edx
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, -1
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x24, eax
	jmp jmp_1003256e
jmp_10032775:
	mov eax, dword ptr l_unk0x48
	mov dword ptr l_unk0x28, eax
	sub eax, dword ptr p_top
	neg eax
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x24, eax
	jmp jmp_1003256e
jmp_100327a1:
	mov eax, dword ptr l_unk0x48
	mov dword ptr l_unk0x28, eax
	sub eax, dword ptr p_top
	neg eax
	mov edx, eax
	dec edx
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, -1
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x24, eax
	jmp jmp_1003256e
jmp_100327d1:
	mov eax, dword ptr l_unk0x3c
	mov dword ptr l_unk0x2c, eax
	sub eax, dword ptr p_left
	neg eax
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x30, eax
	jmp jmp_1003256e
jmp_100327fd:
	mov eax, dword ptr l_unk0x3c
	mov dword ptr l_unk0x2c, eax
	sub eax, dword ptr p_left
	neg eax
	mov edx, eax
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, 0
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x30, eax
	jmp jmp_1003256e
jmp_1003282c:
	mov eax, dword ptr l_unk0x44
	mov dword ptr l_unk0x2c, eax
	sub eax, dword ptr p_left
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x30, eax
	jmp jmp_1003256e
jmp_10032854:
	mov eax, dword ptr l_unk0x44
	mov dword ptr l_unk0x2c, eax
	sub eax, dword ptr p_left
	mov edx, eax
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, 0
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_top
	mov dword ptr l_unk0x30, eax
	jmp jmp_1003256e
jmp_1003287f:
	mov eax, dword ptr l_unk0x40
	mov dword ptr l_unk0x30, eax
	sub eax, dword ptr p_top
	neg eax
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x2c, eax
	jmp jmp_1003256e
jmp_100328ab:
	mov eax, dword ptr l_unk0x40
	mov dword ptr l_unk0x30, eax
	sub eax, dword ptr p_top
	neg eax
	mov edx, eax
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, 0
	mov edx, dword ptr l_unk0x1c
	not edx
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x2c, eax
	jmp jmp_1003256e
jmp_100328da:
	mov eax, dword ptr l_unk0x48
	mov dword ptr l_unk0x30, eax
	sub eax, dword ptr p_top
	mul dword ptr l_unk0x20
	add eax, 80000000h
	adc edx, 0
	mov eax, edx
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x2c, eax
	jmp jmp_1003256e
jmp_10032902:
	mov eax, dword ptr l_unk0x48
	mov dword ptr l_unk0x30, eax
	sub eax, dword ptr p_top
	mov edx, eax
	mov eax, 80000000h
	div dword ptr l_unk0x20
	cmp edx, 1
	sbb eax, 0
	mov edx, dword ptr l_unk0x1c
	xor eax, edx
	sub eax, edx
	add eax, dword ptr p_left
	mov dword ptr l_unk0x2c, eax
	jmp jmp_1003256e
jmp_1003292d:
	mov eax, dword ptr l_unk0x28
	imul dword ptr l_unk0x50
	add eax, dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x24
	mov edi, eax
	mov esi, dword ptr l_unk0x50
	xor esi, dword ptr l_unk0x18
	sub esi, dword ptr l_unk0x18
	mov ebx, dword ptr l_unk0x20
	mov eax, dword ptr l_unk0x08
	cmp eax, dword ptr l_unk0x14
	je jmp_10032d70
	jg jmp_10032ac1
	mov eax, dword ptr l_unk0x30
	sub eax, dword ptr l_unk0x28
	cdq
	xor eax, edx
	sub eax, edx
	inc eax
	mov ecx, eax
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr p_top
	cdq
	xor eax, edx
	sub eax, edx
	mul ebx
	add eax, 80000000h
	mov edx, eax
	cmp dword ptr l_unk0x0c, -1
	je jmp_10032a22
	cmp dword ptr p_unk0x14, 1
	je jmp_100329b9
	jg jmp_100329f7
	mov eax, dword ptr p_color
jmp_10032990:
	mov byte ptr [edi], al
	add edx, ebx
	adc edi, esi
	dec ecx
	je jmp_100329b4
	mov byte ptr [edi], al
	add edx, ebx
	adc edi, esi
	dec ecx
	je jmp_100329b4
	mov byte ptr [edi], al
	add edx, ebx
	adc edi, esi
	dec ecx
	je jmp_100329b4
	mov byte ptr [edi], al
	add edx, ebx
	adc edi, esi
	dec ecx
	jne jmp_10032990
jmp_100329b4:
	jmp jmp_10032e31
jmp_100329b9:
	mov eax, dword ptr p_color
	push ebp
	mov ebp, ebx
	mov ebx, eax
jmp_100329c1:
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	adc edi, esi
	dec ecx
	je jmp_100329f1
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	adc edi, esi
	dec ecx
	je jmp_100329f1
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	adc edi, esi
	dec ecx
	je jmp_100329f1
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	adc edi, esi
	dec ecx
	jne jmp_100329c1
jmp_100329f1:
	pop ebp
	jmp jmp_10032e31
jmp_100329f7:
	mov esi, dword ptr p_view
	mov edi, dword ptr l_unk0x24
	sub edi, dword ptr [esi+4]
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr [esi+8]
	mov esi, eax
	mov eax, dword ptr l_unk0x18
	add eax, eax
	inc eax
jmp_10032a0e:
	pushad
	call dword ptr p_color
	popad
	add edx, ebx
	jae jmp_10032a18
	inc edi
jmp_10032a18:
	add esi, eax
	dec ecx
	jne jmp_10032a0e
	jmp jmp_10032e31
jmp_10032a22:
	neg esi
	cmp dword ptr p_unk0x14, 1
	je jmp_10032a58
	jg jmp_10032a96
	mov eax, dword ptr p_color
jmp_10032a2f:
	mov byte ptr [edi], al
	add edx, ebx
	sbb edi, esi
	dec ecx
	je jmp_10032a53
	mov byte ptr [edi], al
	add edx, ebx
	sbb edi, esi
	dec ecx
	je jmp_10032a53
	mov byte ptr [edi], al
	add edx, ebx
	sbb edi, esi
	dec ecx
	je jmp_10032a53
	mov byte ptr [edi], al
	add edx, ebx
	sbb edi, esi
	dec ecx
	jne jmp_10032a2f
jmp_10032a53:
	jmp jmp_10032e31
jmp_10032a58:
	mov eax, dword ptr p_color
	push ebp
	mov ebp, ebx
	mov ebx, eax
jmp_10032a60:
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	sbb edi, esi
	dec ecx
	je jmp_10032a90
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	sbb edi, esi
	dec ecx
	je jmp_10032a90
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	sbb edi, esi
	dec ecx
	je jmp_10032a90
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edx, ebp
	sbb edi, esi
	dec ecx
	jne jmp_10032a60
jmp_10032a90:
	pop ebp
	jmp jmp_10032e31
jmp_10032a96:
	mov esi, dword ptr p_view
	mov edi, dword ptr l_unk0x24
	sub edi, dword ptr [esi+4]
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr [esi+8]
	mov esi, eax
	mov eax, dword ptr l_unk0x18
	add eax, eax
	inc eax
jmp_10032aad:
	pushad
	call dword ptr p_color
	popad
	add edx, ebx
	jae jmp_10032ab7
	dec edi
jmp_10032ab7:
	add esi, eax
	dec ecx
	jne jmp_10032aad
	jmp jmp_10032e31
jmp_10032ac1:
	mov eax, dword ptr l_unk0x2c
	sub eax, dword ptr l_unk0x24
	cdq
	xor eax, edx
	sub eax, edx
	inc eax
	mov ecx, eax
	mov eax, dword ptr l_unk0x24
	sub eax, dword ptr p_left
	cdq
	xor eax, edx
	sub eax, edx
	mul ebx
	add eax, 80000000h
	mov edx, eax
	cmp dword ptr l_unk0x0c, -1
	je jmp_10032ba6
	cmp dword ptr p_unk0x14, 1
	je jmp_10032b31
	jg jmp_10032b7b
	mov eax, dword ptr p_color
jmp_10032afc:
	mov byte ptr [edi], al
	inc edi
	add edx, ebx
	jae jmp_10032b05
	add edi, esi
jmp_10032b05:
	dec ecx
	je jmp_10032b2c
	mov byte ptr [edi], al
	inc edi
	add edx, ebx
	jae jmp_10032b11
	add edi, esi
jmp_10032b11:
	dec ecx
	je jmp_10032b2c
	mov byte ptr [edi], al
	inc edi
	add edx, ebx
	jae jmp_10032b1d
	add edi, esi
jmp_10032b1d:
	dec ecx
	je jmp_10032b2c
	mov byte ptr [edi], al
	inc edi
	add edx, ebx
	jae jmp_10032b29
	add edi, esi
jmp_10032b29:
	dec ecx
	jne jmp_10032afc
jmp_10032b2c:
	jmp jmp_10032e31
jmp_10032b31:
	mov eax, dword ptr p_color
	push ebp
	mov ebp, ebx
	mov ebx, eax
jmp_10032b39:
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	inc edi
	add edx, ebp
	jae jmp_10032b45
	add edi, esi
jmp_10032b45:
	dec ecx
	je jmp_10032b75
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	inc edi
	add edx, ebp
	jae jmp_10032b54
	add edi, esi
jmp_10032b54:
	dec ecx
	je jmp_10032b75
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	inc edi
	add edx, ebp
	jae jmp_10032b63
	add edi, esi
jmp_10032b63:
	dec ecx
	je jmp_10032b75
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	inc edi
	add edx, ebp
	jae jmp_10032b72
	add edi, esi
jmp_10032b72:
	dec ecx
	jne jmp_10032b39
jmp_10032b75:
	pop ebp
	jmp jmp_10032e31
jmp_10032b7b:
	mov esi, dword ptr p_view
	mov edi, dword ptr l_unk0x24
	sub edi, dword ptr [esi+4]
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr [esi+8]
	mov esi, eax
	mov eax, dword ptr l_unk0x18
	add eax, eax
	inc eax
jmp_10032b92:
	pushad
	call dword ptr p_color
	popad
	add edx, ebx
	jae jmp_10032b9c
	inc esi
jmp_10032b9c:
	add edi, eax
	dec ecx
	jne jmp_10032b92
	jmp jmp_10032e31
jmp_10032ba6:
	cmp dword ptr p_unk0x14, 1
	je jmp_10032bea
	jg jmp_10032c34
	mov eax, dword ptr p_color
jmp_10032bb5:
	mov byte ptr [edi], al
	dec edi
	add edx, ebx
	jae jmp_10032bbe
	add edi, esi
jmp_10032bbe:
	dec ecx
	je jmp_10032be5
	mov byte ptr [edi], al
	dec edi
	add edx, ebx
	jae jmp_10032bca
	add edi, esi
jmp_10032bca:
	dec ecx
	je jmp_10032be5
	mov byte ptr [edi], al
	dec edi
	add edx, ebx
	jae jmp_10032bd6
	add edi, esi
jmp_10032bd6:
	dec ecx
	je jmp_10032be5
	mov byte ptr [edi], al
	dec edi
	add edx, ebx
	jae jmp_10032be2
	add edi, esi
jmp_10032be2:
	dec ecx
	jne jmp_10032bb5
jmp_10032be5:
	jmp jmp_10032e31
jmp_10032bea:
	mov eax, dword ptr p_color
	push ebp
	mov ebp, ebx
	mov ebx, eax
jmp_10032bf2:
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	dec edi
	add edx, ebp
	jae jmp_10032bfe
	add edi, esi
jmp_10032bfe:
	dec ecx
	je jmp_10032c2e
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	dec edi
	add edx, ebp
	jae jmp_10032c0d
	add edi, esi
jmp_10032c0d:
	dec ecx
	je jmp_10032c2e
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	dec edi
	add edx, ebp
	jae jmp_10032c1c
	add edi, esi
jmp_10032c1c:
	dec ecx
	je jmp_10032c2e
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	dec edi
	add edx, ebp
	jae jmp_10032c2b
	add edi, esi
jmp_10032c2b:
	dec ecx
	jne jmp_10032bf2
jmp_10032c2e:
	pop ebp
	jmp jmp_10032e31
jmp_10032c34:
	mov esi, dword ptr p_view
	mov edi, dword ptr l_unk0x24
	sub edi, dword ptr [esi+4]
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr [esi+8]
	mov esi, eax
	mov eax, dword ptr l_unk0x18
	add eax, eax
	inc eax
jmp_10032c4b:
	pushad
	call dword ptr p_color
	popad
	add edx, ebx
	jae jmp_10032c55
	dec esi
jmp_10032c55:
	add edi, eax
	dec ecx
	jne jmp_10032c4b
	jmp jmp_10032e31
jmp_10032c5f:
	mov eax, dword ptr p_left
	cmp eax, dword ptr l_unk0x3c
	jl jmp_10032e40
	cmp eax, dword ptr l_unk0x44
	jg jmp_10032e40
	mov eax, dword ptr p_top
	cmp eax, dword ptr p_bottom
	jg jmp_10032c7f
	mov eax, dword ptr p_bottom
jmp_10032c7f:
	cmp eax, dword ptr l_unk0x40
	jl jmp_10032e40
	mov eax, dword ptr p_top
	cmp eax, dword ptr p_bottom
	jl jmp_10032c93
	mov eax, dword ptr p_bottom
jmp_10032c93:
	cmp eax, dword ptr l_unk0x48
	jg jmp_10032e40
	mov eax, dword ptr p_top
	cmp eax, dword ptr l_unk0x40
	jg jmp_10032ca7
	mov eax, dword ptr l_unk0x40
jmp_10032ca7:
	cmp eax, dword ptr l_unk0x48
	jl jmp_10032caf
	mov eax, dword ptr l_unk0x48
jmp_10032caf:
	mov dword ptr l_unk0x28, eax
	mov eax, dword ptr p_bottom
	cmp eax, dword ptr l_unk0x40
	jg jmp_10032cbd
	mov eax, dword ptr l_unk0x40
jmp_10032cbd:
	cmp eax, dword ptr l_unk0x48
	jl jmp_10032cc5
	mov eax, dword ptr l_unk0x48
jmp_10032cc5:
	mov dword ptr l_unk0x30, eax
	mov eax, dword ptr p_left
	mov dword ptr l_unk0x24, eax
	mov esi, dword ptr l_unk0x50
	xor esi, dword ptr l_unk0x18
	sub esi, dword ptr l_unk0x18
	mov eax, dword ptr l_unk0x30
	sub eax, dword ptr l_unk0x28
	cdq
	xor eax, edx
	sub eax, edx
	mov ecx, eax
	inc ecx
	jmp jmp_10032d90
jmp_10032cea:
	mov eax, dword ptr p_top
	cmp eax, dword ptr l_unk0x40
	jl jmp_10032e40
	cmp eax, dword ptr l_unk0x48
	jg jmp_10032e40
	mov eax, dword ptr p_left
	cmp eax, dword ptr p_right
	jg jmp_10032d0a
	mov eax, dword ptr p_right
jmp_10032d0a:
	cmp eax, dword ptr l_unk0x3c
	jl jmp_10032e40
	mov eax, dword ptr p_left
	cmp eax, dword ptr p_right
	jl jmp_10032d1e
	mov eax, dword ptr p_right
jmp_10032d1e:
	cmp eax, dword ptr l_unk0x44
	jg jmp_10032e40
	mov eax, dword ptr p_left
	cmp eax, dword ptr l_unk0x3c
	jg jmp_10032d32
	mov eax, dword ptr l_unk0x3c
jmp_10032d32:
	cmp eax, dword ptr l_unk0x44
	jl jmp_10032d3a
	mov eax, dword ptr l_unk0x44
jmp_10032d3a:
	mov dword ptr l_unk0x24, eax
	mov eax, dword ptr p_right
	cmp eax, dword ptr l_unk0x3c
	jg jmp_10032d48
	mov eax, dword ptr l_unk0x3c
jmp_10032d48:
	cmp eax, dword ptr l_unk0x44
	jl jmp_10032d50
	mov eax, dword ptr l_unk0x44
jmp_10032d50:
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr p_top
	mov dword ptr l_unk0x28, eax
	mov esi, dword ptr l_unk0x0c
	inc esi
	or esi, dword ptr l_unk0x0c
	mov eax, dword ptr l_unk0x2c
	sub eax, dword ptr l_unk0x24
	cdq
	xor eax, edx
	sub eax, edx
	mov ecx, eax
	inc ecx
	jmp jmp_10032d90
jmp_10032d70:
	mov esi, dword ptr l_unk0x50
	xor esi, dword ptr l_unk0x18
	sub esi, dword ptr l_unk0x18
	mov eax, dword ptr l_unk0x0c
	inc eax
	or eax, dword ptr l_unk0x0c
	add esi, eax
	mov eax, dword ptr l_unk0x2c
	sub eax, dword ptr l_unk0x24
	cdq
	xor eax, edx
	sub eax, edx
	mov ecx, eax
	inc ecx
jmp_10032d90:
	mov eax, dword ptr l_unk0x28
	imul dword ptr l_unk0x50
	add eax, dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x24
	mov edi, eax
	cmp dword ptr p_unk0x14, 1
	je jmp_10032dc7
	jg jmp_10032df4
	mov eax, dword ptr p_color
jmp_10032da9:
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	je jmp_10032dc5
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	je jmp_10032dc5
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	je jmp_10032dc5
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	jne jmp_10032da9
jmp_10032dc5:
	jmp jmp_10032e31
jmp_10032dc7:
	mov ebx, dword ptr p_color
jmp_10032dca:
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	je jmp_10032df2
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	je jmp_10032df2
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	je jmp_10032df2
	mov al, byte ptr [edi]
	xlatb
	mov byte ptr [edi], al
	add edi, esi
	dec ecx
	jne jmp_10032dca
jmp_10032df2:
	jmp jmp_10032e31
jmp_10032df4:
	mov esi, dword ptr p_view
	mov edi, dword ptr l_unk0x24
	sub edi, dword ptr [esi+4]
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr [esi+8]
	mov esi, eax
	xor eax, eax
	test dword ptr l_unk0x10, 0ffffffffh
	setne al
	or eax, dword ptr l_unk0x18
	xor ebx, ebx
	test dword ptr l_unk0x04, 0ffffffffh
	setne bl
	or ebx, dword ptr l_unk0x0c
jmp_10032e23:
	pushad
	call dword ptr p_color
	popad
	add esi, eax
	add edi, ebx
	dec ecx
	jne jmp_10032e23
	jmp jmp_10032e31
jmp_10032e31:
	xor eax, eax
	cmp dword ptr l_unk0x34, 1
	setae al
	pop es
	ret
jmp_10032e40:
	mov eax, 2
	pop es
	ret
BlitLine endp

; Returns 0 when drawn, or a negative code when the view is empty or everything is clipped.
; Nothing calls it, and what it draws hasn't been worked out from the assembly, so it keeps its
; placeholder.
FUN_10032e4b proc uses ebx esi edi, p_view:dword, p_unk0x04:dword, p_unk0x08:dword, p_unk0x0c:dword, p_unk0x10:dword, p_unk0x14:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x18, eax
	jle jmp_10032eca
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10032eca
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x1c, eax
	cmp eax, 0
	jg jmp_10032e7e
	mov eax, 0
jmp_10032e7e:
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x20, eax
	cmp eax, 0
	jg jmp_10032e91
	mov eax, 0
jmp_10032e91:
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x18
	dec edx
	cmp eax, edx
	jl jmp_10032ea1
	mov eax, edx
jmp_10032ea1:
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10032eb0
	mov eax, edx
jmp_10032eb0:
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr l_unk0x0c
	cmp eax, dword ptr l_unk0x04
	jl jmp_10032ed5
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr l_unk0x08
	jl jmp_10032ed5
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x14, eax
	jmp jmp_10032ee0
jmp_10032eca:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10032ed5:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10032ee0:
	mov eax, dword ptr l_unk0x1c
	add dword ptr p_unk0x04, eax
	add dword ptr p_unk0x0c, eax
	mov eax, dword ptr l_unk0x20
	add dword ptr p_unk0x08, eax
	add dword ptr p_unk0x10, eax
	mov eax, dword ptr l_unk0x04
	cmp dword ptr p_unk0x04, eax
	jg jmp_10032efd
	mov dword ptr p_unk0x04, eax
jmp_10032efd:
	mov eax, dword ptr l_unk0x08
	cmp dword ptr p_unk0x08, eax
	jg jmp_10032f08
	mov dword ptr p_unk0x08, eax
jmp_10032f08:
	mov eax, dword ptr l_unk0x0c
	cmp dword ptr p_unk0x0c, eax
	jl jmp_10032f13
	mov dword ptr p_unk0x0c, eax
jmp_10032f13:
	mov eax, dword ptr l_unk0x10
	cmp dword ptr p_unk0x10, eax
	jl jmp_10032f1e
	mov dword ptr p_unk0x10, eax
jmp_10032f1e:
	mov ecx, dword ptr p_unk0x0c
	sub ecx, dword ptr p_unk0x04
	jl jmp_10032f79
	inc ecx
	mov eax, dword ptr p_unk0x08
	imul dword ptr l_unk0x18
	add eax, dword ptr l_unk0x14
	add eax, dword ptr p_unk0x04
	mov edi, eax
	mov edx, dword ptr p_unk0x10
	sub edx, dword ptr p_unk0x08
	jl jmp_10032f79
	mov eax, dword ptr p_unk0x14
	mov esi, edi
	mov ebx, ecx
	jmp jmp_10032f4d
jmp_10032f46:
	add esi, dword ptr l_unk0x18
	mov edi, esi
	mov ecx, ebx
jmp_10032f4d:
	push edx
	and edx, 1
	je jmp_10032f58
	pop edx
	inc edi
	dec ecx
	jmp jmp_10032f59
jmp_10032f58:
	pop edx
jmp_10032f59:
	mov byte ptr [edi], al
	add edi, 2
	sub ecx, 2
	jg jmp_10032f59
	dec edx
	jns jmp_10032f46
	xor eax, eax
	pop es
	ret
	mov eax, 0fffffffdh
	pop es
	ret
jmp_10032f79:
	mov eax, 0fffffffch
	pop es
	ret
FUN_10032e4b endp

; Draws frame p_frame of an SHP animation into the view, clipped; a frame that lies wholly inside
; the view goes through BlitShpFrameUnclipped. Returns 0 when drawn, or a negative code when the
; view is empty or everything is clipped.
BlitShpFrame proc uses ebx esi edi, p_view:dword, p_shp:dword, p_frame:dword, p_left:dword, p_top:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword, l_unk0x48:dword
	local l_unk0x4c:dword, l_unk0x50:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x48, eax
	jle jmp_10033003
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10033003
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x4c, eax
	cmp eax, 0
	jg jmp_10032fb7
	mov eax, 0
jmp_10032fb7:
	mov dword ptr l_unk0x34, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x50, eax
	cmp eax, 0
	jg jmp_10032fca
	mov eax, 0
jmp_10032fca:
	mov dword ptr l_unk0x38, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x48
	dec edx
	cmp eax, edx
	jl jmp_10032fda
	mov eax, edx
jmp_10032fda:
	mov dword ptr l_unk0x3c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10032fe9
	mov eax, edx
jmp_10032fe9:
	mov dword ptr l_unk0x40, eax
	mov eax, dword ptr l_unk0x3c
	cmp eax, dword ptr l_unk0x34
	jl jmp_1003300e
	mov eax, dword ptr l_unk0x40
	cmp eax, dword ptr l_unk0x38
	jl jmp_1003300e
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x44, eax
	jmp jmp_10033019
jmp_10033003:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_1003300e:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10033019:
	mov eax, dword ptr l_unk0x4c
	add dword ptr p_left, eax
	mov eax, dword ptr l_unk0x50
	add dword ptr p_top, eax
	mov esi, dword ptr p_frame
	shl esi, 3
	add esi, 8
	add esi, dword ptr p_shp
	mov esi, dword ptr [esi]
	add esi, dword ptr p_shp
	mov dword ptr l_unk0x30, esi
	mov eax, dword ptr [esi+8]
	add eax, dword ptr p_left
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+0ch]
	add eax, dword ptr p_top
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr [esi+10h]
	add eax, dword ptr p_left
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr [esi+14h]
	add eax, dword ptr p_top
	mov dword ptr l_unk0x18, eax
	add esi, 18h
	mov eax, dword ptr l_unk0x14
	cmp eax, dword ptr l_unk0x0c
	jl jmp_100333ed
	mov eax, dword ptr l_unk0x18
	cmp eax, dword ptr l_unk0x10
	jl jmp_100333ed
	xor edx, edx
	mov eax, dword ptr l_unk0x0c
	sub eax, dword ptr l_unk0x34
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x3c
	sub eax, dword ptr l_unk0x0c
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x10
	sub eax, dword ptr l_unk0x38
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x40
	sub eax, dword ptr l_unk0x10
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x14
	sub eax, dword ptr l_unk0x34
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x3c
	sub eax, dword ptr l_unk0x14
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x18
	sub eax, dword ptr l_unk0x38
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x40
	sub eax, dword ptr l_unk0x18
	shl eax, 1
	adc dh, dh
	mov dword ptr l_unk0x1c, edx
	test dl, dh
	jne jmp_100333e2
	or dl, dh
	jne jmp_10033104
	mov esi, dword ptr p_view
	mov eax, dword ptr [esi+4]
	sub dword ptr p_left, eax
	mov eax, dword ptr [esi+8]
	sub dword ptr p_top, eax
	push dword ptr l_unk0x48
	push dword ptr p_top
	push dword ptr p_left
	push dword ptr l_unk0x30
	push dword ptr p_view
	call BlitShpFrameUnclipped
	add esp, 14h
	jmp jmp_100333da
jmp_10033104:
	mov eax, dword ptr l_unk0x10
	imul dword ptr l_unk0x48
	add eax, dword ptr l_unk0x44
	add eax, dword ptr l_unk0x0c
	mov edi, eax
	mov ecx, dword ptr l_unk0x10
	mov dword ptr l_unk0x20, ecx
	jmp jmp_10033130
jmp_1003311a:
	movzx eax, al
	add esi, eax
	dec esi
jmp_10033120:
	inc esi
jmp_10033121:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033120
	jne jmp_1003311a
	jb jmp_10033120
	add edi, dword ptr l_unk0x48
	inc ecx
jmp_10033130:
	cmp ecx, dword ptr l_unk0x38
	jl jmp_10033121
	mov dword ptr l_unk0x24, edi
	mov dword ptr l_unk0x20, ecx
	mov eax, edi
	sub eax, dword ptr l_unk0x0c
	add eax, dword ptr l_unk0x34
	mov dword ptr l_unk0x28, eax
	mov eax, edi
	sub eax, dword ptr l_unk0x0c
	add eax, dword ptr l_unk0x3c
	mov dword ptr l_unk0x2c, eax
	jmp jmp_100333ce
jmp_10033156:
	mov eax, dword ptr l_unk0x20
	cmp eax, dword ptr l_unk0x40
	jg jmp_100333da
	mov edi, dword ptr l_unk0x24
	test dword ptr l_unk0x1c, 8
	jne jmp_1003321b
	test dword ptr l_unk0x1c, 400h
	jne jmp_100332a9
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100331a3
	jne jmp_100331e2
	jae jmp_10033216
jmp_1003318e:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100331a3
	jne jmp_100331e2
	jb jmp_1003318e
	jae jmp_10033216
jmp_100331a3:
	movzx ecx, al
jmp_100331a6:
	mov al, byte ptr [esi]
	inc esi
	cmp ecx, 4
	jle jmp_100331d5
	push ebx
	mov ah, al
	mov ebx, ecx
	mov ecx, edi
	rol eax, 8
	neg ecx
	and ecx, 3
	mov al, ah
	sub ebx, ecx
	rep stosb
	rol eax, 8
	mov ecx, ebx
	mov al, ah
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	pop ebx
jmp_100331d5:
	rep stosb
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100331a3
	jae jmp_10033216
	je jmp_1003318e
jmp_100331e2:
	movzx ecx, al
jmp_100331e5:
	cmp ecx, 4
	jle jmp_10033205
	push ebx
	mov ebx, ecx
	mov ecx, edi
	neg ecx
	and ecx, 3
	sub ebx, ecx
	rep movsb
	mov ecx, ebx
	and ebx, 3
	shr ecx, 2
	rep movsd
	mov ecx, ebx
	pop ebx
jmp_10033205:
	rep movsb
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100331a3
	jne jmp_100331e2
	jb jmp_1003318e
jmp_10033216:
	jmp jmp_100333bf
jmp_1003321b:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003323b
	jne jmp_1003326c
	jae jmp_100332a4
jmp_10033226:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003323b
	jne jmp_1003326c
	jb jmp_10033226
	jae jmp_100332a4
jmp_1003323b:
	movzx ecx, al
	mov eax, dword ptr l_unk0x28
	sub eax, edi
	cmp eax, ecx
	jge jmp_1003325e
	or eax, eax
	js jmp_1003324f
	add edi, eax
	sub ecx, eax
jmp_1003324f:
	test dword ptr l_unk0x1c, 400h
	je jmp_100331a6
	jne jmp_100332d4
jmp_1003325e:
	add edi, ecx
	inc esi
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003323b
	jae jmp_100332a4
	je jmp_10033226
jmp_1003326c:
	movzx ecx, al
	mov eax, dword ptr l_unk0x28
	sub eax, edi
	cmp eax, ecx
	jge jmp_10033295
	or eax, eax
	js jmp_10033282
	add edi, eax
	sub ecx, eax
	add esi, eax
jmp_10033282:
	test dword ptr l_unk0x1c, 400h
	je jmp_100331e5
	jne jmp_1003332d
jmp_10033295:
	add edi, ecx
	add esi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003323b
	jne jmp_1003326c
	jb jmp_10033226
jmp_100332a4:
	jmp jmp_100333bf
jmp_100332a9:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100332d1
	jne jmp_1003332a
	jae jmp_1003337a
jmp_100332b8:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100332d1
	jne jmp_1003332a
	jb jmp_100332b8
	jae jmp_1003337a
jmp_100332d1:
	movzx ecx, al
jmp_100332d4:
	cmp edi, dword ptr l_unk0x2c
	jg jmp_1003339f
	mov eax, edi
	add eax, ecx
	dec eax
	sub eax, dword ptr l_unk0x2c
	cdq
	not edx
	and edx, eax
	sub ecx, edx
	mov al, byte ptr [esi]
	inc esi
	cmp ecx, 4
	jle jmp_1003331b
	push ebx
	mov ah, al
	mov ebx, ecx
	mov ecx, edi
	rol eax, 8
	neg ecx
	and ecx, 3
	mov al, ah
	sub ebx, ecx
	rep stosb
	rol eax, 8
	mov ecx, ebx
	mov al, ah
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	pop ebx
jmp_1003331b:
	rep stosb
	add edi, edx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100332d1
	jae jmp_1003337a
	je jmp_100332b8
jmp_1003332a:
	movzx ecx, al
jmp_1003332d:
	cmp edi, dword ptr l_unk0x2c
	jg jmp_100333b0
	mov eax, edi
	add eax, ecx
	dec eax
	sub eax, dword ptr l_unk0x2c
	cdq
	not edx
	and edx, eax
	sub ecx, edx
	cmp ecx, 4
	jle jmp_10033361
	push ebx
	mov ebx, ecx
	mov ecx, edi
	neg ecx
	and ecx, 3
	sub ebx, ecx
	rep movsb
	mov ecx, ebx
	and ebx, 3
	shr ecx, 2
	rep movsd
	mov ecx, ebx
	pop ebx
jmp_10033361:
	rep movsb
	add edi, edx
	add esi, edx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100332d1
	jne jmp_1003332a
	jb jmp_100332b8
jmp_1003337a:
	jmp jmp_100333bf
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003339c
	jne jmp_100333ad
	jae jmp_100333bf
jmp_10033387:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003339c
	jne jmp_100333ad
	jb jmp_10033387
	jae jmp_100333bf
jmp_1003339c:
	movzx ecx, al
jmp_1003339f:
	add edi, ecx
	inc esi
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003339c
	jae jmp_100333bf
	je jmp_10033387
jmp_100333ad:
	movzx ecx, al
jmp_100333b0:
	add edi, ecx
	add esi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003339c
	jne jmp_100333ad
	jb jmp_10033387
jmp_100333bf:
	mov eax, dword ptr l_unk0x48
	add dword ptr l_unk0x24, eax
	add dword ptr l_unk0x28, eax
	add dword ptr l_unk0x2c, eax
	inc dword ptr l_unk0x20
jmp_100333ce:
	mov eax, dword ptr l_unk0x20
	cmp eax, dword ptr l_unk0x18
	jle jmp_10033156
jmp_100333da:
	xor eax, eax
	pop es
	ret
jmp_100333e2:
	mov eax, 0fffffffdh
	pop es
	ret
jmp_100333ed:
	mov eax, 0fffffffch
	pop es
	ret
BlitShpFrame endp

; BlitShpFrame's path for a frame that lies wholly inside the view.
BlitShpFrameUnclipped proc uses ebx esi edi, p_view:dword, p_unk0x04:dword, p_unk0x08:dword, p_unk0x0c:dword, p_unk0x10:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [esi+4]
	add dword ptr p_unk0x08, eax
	mov eax, dword ptr [esi+8]
	add dword ptr p_unk0x0c, eax
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr p_unk0x10, eax
	jle jmp_100334f3
	mov esi, dword ptr p_unk0x04
	mov edi, dword ptr [ebx]
	mov eax, dword ptr [esi+8]
	add eax, dword ptr p_unk0x08
	add edi, eax
	mov eax, dword ptr [esi+0ch]
	mov ebx, eax
	add eax, dword ptr p_unk0x0c
	mul dword ptr p_unk0x10
	add edi, eax
	mov edx, edi
	mov eax, dword ptr [esi+10h]
	mov eax, dword ptr [esi+14h]
	inc eax
	sub eax, ebx
	mov ebx, eax
	jle jmp_100334f3
	add esi, 18h
jmp_10033450:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033474
	jne jmp_100334b3
	jae jmp_100334e7
jmp_1003345f:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033474
	jne jmp_100334b3
	jb jmp_1003345f
	jae jmp_100334e7
jmp_10033474:
	movzx ecx, al
	mov al, byte ptr [esi]
	inc esi
	cmp ecx, 4
	jle jmp_100334a6
	push ebx
	mov ah, al
	mov ebx, ecx
	mov ecx, edi
	rol eax, 8
	neg ecx
	and ecx, 3
	mov al, ah
	sub ebx, ecx
	rep stosb
	rol eax, 8
	mov ecx, ebx
	mov al, ah
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	pop ebx
jmp_100334a6:
	rep stosb
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033474
	jae jmp_100334e7
	je jmp_1003345f
jmp_100334b3:
	movzx ecx, al
	cmp ecx, 4
	jle jmp_100334d6
	push ebx
	mov ebx, ecx
	mov ecx, edi
	neg ecx
	and ecx, 3
	sub ebx, ecx
	rep movsb
	mov ecx, ebx
	and ebx, 3
	shr ecx, 2
	rep movsd
	mov ecx, ebx
	pop ebx
jmp_100334d6:
	rep movsb
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033474
	jne jmp_100334b3
	jb jmp_1003345f
jmp_100334e7:
	add edx, dword ptr p_unk0x10
	mov edi, edx
	dec ebx
	jne jmp_10033450
jmp_100334f3:
	xor eax, eax
	pop es
	ret
BlitShpFrameUnclipped endp

; Loads the 256-entry color remap table from p_table.
SetRemapTable proc uses ebx esi edi, p_table:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_table
	mov edi, offset g_remapTable
	mov ecx, 40h
	rep movsd
	pop es
	ret
SetRemapTable endp

; BlitShpFrame with the pixels mapped through the color remap table (through
; BlitShpFrameRemappedUnclipped when the frame lies wholly inside the view).
BlitShpFrameRemapped proc uses ebx esi edi, p_view:dword, p_unk0x04:dword, p_unk0x08:dword, p_left:dword, p_top:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword, l_unk0x48:dword
	local l_unk0x4c:dword, l_unk0x50:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x48, eax
	jle jmp_10033599
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10033599
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x4c, eax
	cmp eax, 0
	jg jmp_1003354d
	mov eax, 0
jmp_1003354d:
	mov dword ptr l_unk0x34, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x50, eax
	cmp eax, 0
	jg jmp_10033560
	mov eax, 0
jmp_10033560:
	mov dword ptr l_unk0x38, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x48
	dec edx
	cmp eax, edx
	jl jmp_10033570
	mov eax, edx
jmp_10033570:
	mov dword ptr l_unk0x3c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_1003357f
	mov eax, edx
jmp_1003357f:
	mov dword ptr l_unk0x40, eax
	mov eax, dword ptr l_unk0x3c
	cmp eax, dword ptr l_unk0x34
	jl jmp_100335a4
	mov eax, dword ptr l_unk0x40
	cmp eax, dword ptr l_unk0x38
	jl jmp_100335a4
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x44, eax
	jmp jmp_100335af
jmp_10033599:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_100335a4:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_100335af:
	mov eax, dword ptr l_unk0x4c
	add dword ptr p_left, eax
	mov eax, dword ptr l_unk0x50
	add dword ptr p_top, eax
	mov esi, dword ptr p_unk0x08
	shl esi, 3
	add esi, 8
	add esi, dword ptr p_unk0x04
	mov esi, dword ptr [esi]
	add esi, dword ptr p_unk0x04
	mov dword ptr l_unk0x30, esi
	mov eax, dword ptr [esi+8]
	add eax, dword ptr p_left
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+0ch]
	add eax, dword ptr p_top
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr [esi+10h]
	add eax, dword ptr p_left
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr [esi+14h]
	add eax, dword ptr p_top
	mov dword ptr l_unk0x18, eax
	add esi, 18h
	mov eax, dword ptr l_unk0x14
	cmp eax, dword ptr l_unk0x0c
	jl jmp_10033975
	mov eax, dword ptr l_unk0x18
	cmp eax, dword ptr l_unk0x10
	jl jmp_10033975
	xor edx, edx
	mov eax, dword ptr l_unk0x0c
	sub eax, dword ptr l_unk0x34
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x3c
	sub eax, dword ptr l_unk0x0c
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x10
	sub eax, dword ptr l_unk0x38
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x40
	sub eax, dword ptr l_unk0x10
	shl eax, 1
	adc dl, dl
	mov eax, dword ptr l_unk0x14
	sub eax, dword ptr l_unk0x34
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x3c
	sub eax, dword ptr l_unk0x14
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x18
	sub eax, dword ptr l_unk0x38
	shl eax, 1
	adc dh, dh
	mov eax, dword ptr l_unk0x40
	sub eax, dword ptr l_unk0x18
	shl eax, 1
	adc dh, dh
	mov dword ptr l_unk0x1c, edx
	test dl, dh
	jne jmp_1003396a
	or dl, dh
	jne jmp_1003369a
	mov esi, dword ptr p_view
	mov eax, dword ptr [esi+4]
	sub dword ptr p_left, eax
	mov eax, dword ptr [esi+8]
	sub dword ptr p_top, eax
	push dword ptr l_unk0x48
	push dword ptr p_top
	push dword ptr p_left
	push dword ptr l_unk0x30
	push dword ptr p_view
	call BlitShpFrameRemappedUnclipped
	add esp, 14h
	jmp jmp_10033962
jmp_1003369a:
	mov eax, dword ptr l_unk0x10
	imul dword ptr l_unk0x48
	add eax, dword ptr l_unk0x44
	add eax, dword ptr l_unk0x0c
	mov edi, eax
	mov ecx, dword ptr l_unk0x10
	mov dword ptr l_unk0x20, ecx
	jmp jmp_100336c6
jmp_100336b0:
	movzx eax, al
	add esi, eax
	dec esi
jmp_100336b6:
	inc esi
jmp_100336b7:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100336b6
	jne jmp_100336b0
	jb jmp_100336b6
	add edi, dword ptr l_unk0x48
	inc ecx
jmp_100336c6:
	cmp ecx, dword ptr l_unk0x38
	jl jmp_100336b7
	mov dword ptr l_unk0x24, edi
	mov dword ptr l_unk0x20, ecx
	mov eax, edi
	sub eax, dword ptr l_unk0x0c
	add eax, dword ptr l_unk0x34
	mov dword ptr l_unk0x28, eax
	mov eax, edi
	sub eax, dword ptr l_unk0x0c
	add eax, dword ptr l_unk0x3c
	mov dword ptr l_unk0x2c, eax
	jmp jmp_10033956
jmp_100336ec:
	mov eax, dword ptr l_unk0x20
	cmp eax, dword ptr l_unk0x40
	jg jmp_10033962
	mov edi, dword ptr l_unk0x24
	test dword ptr l_unk0x1c, 8
	jne jmp_100337a4
	test dword ptr l_unk0x1c, 400h
	jne jmp_10033832
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033735
	jne jmp_1003377c
	jae jmp_1003379f
jmp_10033720:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033735
	jne jmp_1003377c
	jb jmp_10033720
	jae jmp_1003379f
jmp_10033735:
	movzx ecx, al
jmp_10033738:
	xor eax, eax
	mov al, byte ptr [esi]
	inc esi
	mov al, byte ptr [g_remapTable+eax]
	cmp ecx, 4
	jle jmp_1003376f
	push ebx
	mov ah, al
	mov ebx, ecx
	mov ecx, edi
	rol eax, 8
	neg ecx
	and ecx, 3
	mov al, ah
	sub ebx, ecx
	rep stosb
	rol eax, 8
	mov ecx, ebx
	mov al, ah
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	pop ebx
jmp_1003376f:
	rep stosb
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033735
	jae jmp_1003379f
	je jmp_10033720
jmp_1003377c:
	movzx ecx, al
jmp_1003377f:
	xor eax, eax
	or ecx, ecx
	je jmp_10033794
jmp_10033785:
	mov al, byte ptr [esi]
	inc esi
	mov al, byte ptr [g_remapTable+eax]
	mov byte ptr [edi], al
	inc edi
	dec ecx
	jne jmp_10033785
jmp_10033794:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033735
	jne jmp_1003377c
	jb jmp_10033720
jmp_1003379f:
	jmp jmp_10033947
jmp_100337a4:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100337c4
	jne jmp_100337f5
	jae jmp_1003382d
jmp_100337af:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100337c4
	jne jmp_100337f5
	jb jmp_100337af
	jae jmp_1003382d
jmp_100337c4:
	movzx ecx, al
	mov eax, dword ptr l_unk0x28
	sub eax, edi
	cmp eax, ecx
	jge jmp_100337e7
	or eax, eax
	js jmp_100337d8
	add edi, eax
	sub ecx, eax
jmp_100337d8:
	test dword ptr l_unk0x1c, 400h
	je jmp_10033738
	jne jmp_10033861
jmp_100337e7:
	add edi, ecx
	inc esi
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100337c4
	jae jmp_1003382d
	je jmp_100337af
jmp_100337f5:
	movzx ecx, al
	mov eax, dword ptr l_unk0x28
	sub eax, edi
	cmp eax, ecx
	jge jmp_1003381e
	or eax, eax
	js jmp_1003380b
	add edi, eax
	sub ecx, eax
	add esi, eax
jmp_1003380b:
	test dword ptr l_unk0x1c, 400h
	je jmp_1003377f
	jne jmp_100338c2
jmp_1003381e:
	add edi, ecx
	add esi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100337c4
	jne jmp_100337f5
	jb jmp_100337af
jmp_1003382d:
	jmp jmp_10033947
jmp_10033832:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003385e
	jne jmp_100338bf
	jae jmp_10033902
jmp_10033845:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003385e
	jne jmp_100338bf
	jb jmp_10033845
	jae jmp_10033902
jmp_1003385e:
	movzx ecx, al
jmp_10033861:
	cmp edi, dword ptr l_unk0x2c
	jg jmp_10033927
	mov eax, edi
	add eax, ecx
	dec eax
	sub eax, dword ptr l_unk0x2c
	cdq
	not edx
	and edx, eax
	sub ecx, edx
	xor eax, eax
	mov al, byte ptr [esi]
	inc esi
	mov al, byte ptr [g_remapTable+eax]
	cmp ecx, 4
	jle jmp_100338b0
	push ebx
	mov ah, al
	mov ebx, ecx
	mov ecx, edi
	rol eax, 8
	neg ecx
	and ecx, 3
	mov al, ah
	sub ebx, ecx
	rep stosb
	rol eax, 8
	mov ecx, ebx
	mov al, ah
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	pop ebx
jmp_100338b0:
	rep stosb
	add edi, edx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003385e
	jae jmp_10033902
	je jmp_10033845
jmp_100338bf:
	movzx ecx, al
jmp_100338c2:
	cmp edi, dword ptr l_unk0x2c
	jg jmp_10033938
	mov eax, edi
	add eax, ecx
	dec eax
	sub eax, dword ptr l_unk0x2c
	cdq
	not edx
	and edx, eax
	sub ecx, edx
	xor eax, eax
	or ecx, ecx
	je jmp_100338eb
jmp_100338dc:
	mov al, byte ptr [esi]
	inc esi
	mov al, byte ptr [g_remapTable+eax]
	mov byte ptr [edi], al
	inc edi
	dec ecx
	jne jmp_100338dc
jmp_100338eb:
	add edi, edx
	add esi, edx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_1003385e
	jne jmp_100338bf
	jb jmp_10033845
jmp_10033902:
	jmp jmp_10033947
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033924
	jne jmp_10033935
	jae jmp_10033947
jmp_1003390f:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033924
	jne jmp_10033935
	jb jmp_1003390f
	jae jmp_10033947
jmp_10033924:
	movzx ecx, al
jmp_10033927:
	add edi, ecx
	inc esi
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033924
	jae jmp_10033947
	je jmp_1003390f
jmp_10033935:
	movzx ecx, al
jmp_10033938:
	add edi, ecx
	add esi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10033924
	jne jmp_10033935
	jb jmp_1003390f
jmp_10033947:
	mov eax, dword ptr l_unk0x48
	add dword ptr l_unk0x24, eax
	add dword ptr l_unk0x28, eax
	add dword ptr l_unk0x2c, eax
	inc dword ptr l_unk0x20
jmp_10033956:
	mov eax, dword ptr l_unk0x20
	cmp eax, dword ptr l_unk0x18
	jle jmp_100336ec
jmp_10033962:
	xor eax, eax
	pop es
	ret
jmp_1003396a:
	mov eax, 0fffffffdh
	pop es
	ret
jmp_10033975:
	mov eax, 0fffffffch
	pop es
	ret
BlitShpFrameRemapped endp

; BlitShpFrameRemapped's path for a frame that lies wholly inside the view.
BlitShpFrameRemappedUnclipped proc uses ebx esi edi, p_view:dword, p_unk0x04:dword, p_unk0x08:dword, p_unk0x0c:dword, p_unk0x10:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [esi+4]
	add dword ptr p_unk0x08, eax
	mov eax, dword ptr [esi+8]
	add dword ptr p_unk0x0c, eax
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr p_unk0x10, eax
	jle jmp_10033a6e
	mov esi, dword ptr p_unk0x04
	mov edi, dword ptr [ebx]
	mov eax, dword ptr [esi+8]
	add eax, dword ptr p_unk0x08
	add edi, eax
	mov eax, dword ptr [esi+0ch]
	mov ebx, eax
	add eax, dword ptr p_unk0x0c
	mul dword ptr p_unk0x10
	add edi, eax
	mov edx, edi
	mov eax, dword ptr [esi+10h]
	mov eax, dword ptr [esi+14h]
	inc eax
	sub eax, ebx
	mov ebx, eax
	jle jmp_10033a6e
	add esi, 18h
jmp_100339d8:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100339f8
	jne jmp_10033a3f
	jae jmp_10033a62
jmp_100339e3:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100339f8
	jne jmp_10033a3f
	jb jmp_100339e3
	jae jmp_10033a62
jmp_100339f8:
	movzx ecx, al
	xor eax, eax
	mov al, byte ptr [esi]
	inc esi
	mov al, byte ptr [g_remapTable+eax]
	cmp ecx, 4
	jle jmp_10033a32
	push ebx
	mov ah, al
	mov ebx, ecx
	mov ecx, edi
	rol eax, 8
	neg ecx
	and ecx, 3
	mov al, ah
	sub ebx, ecx
	rep stosb
	rol eax, 8
	mov ecx, ebx
	mov al, ah
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	pop ebx
jmp_10033a32:
	rep stosb
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100339f8
	jae jmp_10033a62
	je jmp_100339e3
jmp_10033a3f:
	movzx ecx, al
	xor eax, eax
	or ecx, ecx
	je jmp_10033a57
jmp_10033a48:
	mov al, byte ptr [esi]
	inc esi
	mov al, byte ptr [g_remapTable+eax]
	mov byte ptr [edi], al
	inc edi
	dec ecx
	jne jmp_10033a48
jmp_10033a57:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100339f8
	jne jmp_10033a3f
	jb jmp_100339e3
jmp_10033a62:
	add edx, dword ptr p_unk0x10
	mov edi, edx
	dec ebx
	jne jmp_100339d8
jmp_10033a6e:
	xor eax, eax
	pop es
	ret
BlitShpFrameRemappedUnclipped endp

; Blits with rotation (p_angle) and 16.16 scaling (p_scaleX, p_scaleY). The rotated corners
; go into g_rotatedCorners; with no rotation and unit scale, it takes a plain copy path.
BlitRotated proc p_view:dword, p_unk0x04:dword, p_unk0x08:dword, p_unk0x0c:dword, p_unk0x10:dword, p_unk0x14:dword, p_unk0x18:dword, p_angle:dword, p_scaleX:dword, p_scaleY:dword
	add esp, 0ffffff10h
	push ebx
	push esi
	push edi
	push es
	cld
	push ds
	pop es
	cmp dword ptr p_angle, 10000h
	jne jmp_10033aa2
	cmp dword ptr p_scaleX, 10000h
	jne jmp_10033aa2
	cmp dword ptr p_unk0x18, 0
	je jmp_100345e0
jmp_10033aa2:
	push dword ptr p_unk0x08
	push dword ptr p_unk0x04
	call GetShpFrameExtent
	add esp, 8
	mov ecx, eax
	shr eax, 10h
	dec eax
	mov dword ptr [ebp-44h], eax
	and ecx, 0ffffh
	dec ecx
	mov dword ptr [ebp-48h], ecx
	lea ebx, [ebp-14h]
	lea esi, [ebp-28h]
	mov dword ptr [esi], ebx
	mov edx, dword ptr p_unk0x14
	mov dword ptr [ebx], edx
	mov dword ptr [esi+4], 0
	mov dword ptr [g_rotatedCorners+0ch], 0
	mov dword ptr [g_rotatedCorners+48h], 0
	mov dword ptr [esi+8], 0
	mov dword ptr [g_rotatedCorners+10h], 0
	mov dword ptr [g_rotatedCorners+24h], 0
	mov dword ptr [ebx+4], eax
	mov dword ptr [esi+0ch], eax
	mov dword ptr [g_rotatedCorners+20h], eax
	mov dword ptr [g_rotatedCorners+34h], eax
	mov dword ptr [ebx+8], ecx
	mov dword ptr [esi+10h], ecx
	mov dword ptr [g_rotatedCorners+38h], ecx
	mov dword ptr [g_rotatedCorners+4ch], ecx
	push dword ptr p_unk0x08
	push dword ptr p_unk0x04
	call GetShpFrameOrigin
	add esp, 8
	mov ebx, eax
	cwde
	neg eax
	mov dword ptr [ebp-3ch], eax
	sar ebx, 10h
	neg ebx
	mov dword ptr [ebp-40h], ebx
	test dword ptr p_scaleY, 2
	jne jmp_10033b9b
	push 0ffh
	lea eax, [ebp-28h]
	push eax
	call FillView
	add esp, 8
	test dword ptr p_scaleY, 1
	je jmp_10033b83
	push dword ptr [ebp-3ch]
	push dword ptr [ebp-40h]
	push dword ptr p_unk0x08
	push dword ptr p_unk0x04
	lea eax, [ebp-28h]
	push eax
	call BlitShpFrameRemapped
	add esp, 14h
	jmp jmp_10033b9b
jmp_10033b83:
	push dword ptr [ebp-3ch]
	push dword ptr [ebp-40h]
	push dword ptr p_unk0x08
	push dword ptr p_unk0x04
	lea eax, [ebp-28h]
	push eax
	call BlitShpFrame
	add esp, 14h
jmp_10033b9b:
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr [ebp-0e8h], eax
	jle jmp_10033c3c
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10033c3c
	mov eax, dword ptr [esi+4]
	mov dword ptr [ebp-0ech], eax
	cmp eax, 0
	jg jmp_10033bcf
	mov eax, 0
jmp_10033bcf:
	mov dword ptr [ebp-0d4h], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [ebp-0f0h], eax
	cmp eax, 0
	jg jmp_10033be8
	mov eax, 0
jmp_10033be8:
	mov dword ptr [ebp-0d8h], eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr [ebp-0e8h]
	dec edx
	cmp eax, edx
	jl jmp_10033bfe
	mov eax, edx
jmp_10033bfe:
	mov dword ptr [ebp-0dch], eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10033c10
	mov eax, edx
jmp_10033c10:
	mov dword ptr [ebp-0e0h], eax
	mov eax, dword ptr [ebp-0dch]
	cmp eax, dword ptr [ebp-0d4h]
	jl jmp_10033c47
	mov eax, dword ptr [ebp-0e0h]
	cmp eax, dword ptr [ebp-0d8h]
	jl jmp_10033c47
	mov eax, dword ptr [ebx]
	mov dword ptr [ebp-0e4h], eax
	jmp jmp_10033c52
jmp_10033c3c:
	mov eax, 0ffffffffh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_10033c47:
	mov eax, 0fffffffeh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_10033c52:
	mov eax, dword ptr p_unk0x0c
	sub eax, dword ptr [ebp-40h]
	mov dword ptr [ebp-4ch], eax
	mov eax, dword ptr p_unk0x10
	sub eax, dword ptr [ebp-3ch]
	mov dword ptr [ebp-50h], eax
	mov dword ptr [ebp-30h], 0
	mov dword ptr [ebp-2ch], 0
	push dword ptr p_scaleX
	push dword ptr p_angle
	push dword ptr p_unk0x18
	lea eax, [ebp-40h]
	push eax
	lea eax, [ebp-38h]
	push eax
	lea eax, [ebp-30h]
	push eax
	call RotateScalePoint
	add esp, 18h
	mov eax, dword ptr [ebp-38h]
	add eax, dword ptr [ebp-4ch]
	mov dword ptr [g_rotatedCorners], eax
	mov eax, dword ptr [ebp-34h]
	add eax, dword ptr [ebp-50h]
	mov dword ptr [g_rotatedCorners+4], eax
	mov eax, dword ptr [ebp-44h]
	mov dword ptr [ebp-30h], eax
	mov dword ptr [ebp-2ch], 0
	push dword ptr p_scaleX
	push dword ptr p_angle
	push dword ptr p_unk0x18
	lea eax, [ebp-40h]
	push eax
	lea eax, [ebp-38h]
	push eax
	lea eax, [ebp-30h]
	push eax
	call RotateScalePoint
	add esp, 18h
	mov eax, dword ptr [ebp-38h]
	add eax, dword ptr [ebp-4ch]
	mov dword ptr [g_rotatedCorners+14h], eax
	mov eax, dword ptr [ebp-34h]
	add eax, dword ptr [ebp-50h]
	mov dword ptr [g_rotatedCorners+18h], eax
	mov eax, dword ptr [ebp-0ech]
	add dword ptr [g_rotatedCorners], eax
	add dword ptr [g_rotatedCorners+14h], eax
	mov eax, dword ptr [ebp-0f0h]
	add dword ptr [g_rotatedCorners+4], eax
	add dword ptr [g_rotatedCorners+18h], eax
	mov eax, dword ptr [ebp-44h]
	mov ebx, dword ptr [ebp-48h]
	mov dword ptr [ebp-30h], eax
	mov dword ptr [ebp-2ch], ebx
	push dword ptr p_scaleX
	push dword ptr p_angle
	push dword ptr p_unk0x18
	lea eax, [ebp-40h]
	push eax
	lea eax, [ebp-38h]
	push eax
	lea eax, [ebp-30h]
	push eax
	call RotateScalePoint
	add esp, 18h
	mov eax, dword ptr [ebp-38h]
	add eax, dword ptr [ebp-4ch]
	mov dword ptr [g_rotatedCorners+28h], eax
	mov eax, dword ptr [ebp-34h]
	add eax, dword ptr [ebp-50h]
	mov dword ptr [g_rotatedCorners+2ch], eax
	mov eax, dword ptr [ebp-48h]
	mov dword ptr [ebp-30h], 0
	mov dword ptr [ebp-2ch], eax
	push dword ptr p_scaleX
	push dword ptr p_angle
	push dword ptr p_unk0x18
	lea eax, [ebp-40h]
	push eax
	lea eax, [ebp-38h]
	push eax
	lea eax, [ebp-30h]
	push eax
	call RotateScalePoint
	add esp, 18h
	mov eax, dword ptr [ebp-38h]
	add eax, dword ptr [ebp-4ch]
	mov dword ptr [g_rotatedCorners+3ch], eax
	mov eax, dword ptr [ebp-34h]
	add eax, dword ptr [ebp-50h]
	mov dword ptr [g_rotatedCorners+40h], eax
	mov eax, dword ptr [ebp-0ech]
	add dword ptr [g_rotatedCorners+28h], eax
	add dword ptr [g_rotatedCorners+3ch], eax
	mov eax, dword ptr [ebp-0f0h]
	add dword ptr [g_rotatedCorners+2ch], eax
	add dword ptr [g_rotatedCorners+40h], eax
	lea ebx, [ebp-14h]
	mov eax, dword ptr [ebx]
	mov dword ptr [ebp-58h], eax
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [ebp-54h], ecx
	push ds
	pop es
	mov ebx, offset g_rotatedCorners
	mov eax, ebx
	add eax, 50h
	mov dword ptr [ebp-64h], ebx
	mov dword ptr [ebp-68h], eax
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_10033ddc:
	mov edx, 0
	mov eax, dword ptr [ebx]
	sub eax, dword ptr [ebp-0d4h]
	shld edx, eax, 1
	mov eax, dword ptr [ebp-0dch]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	sub eax, dword ptr [ebp-0d8h]
	shld edx, eax, 1
	mov eax, dword ptr [ebp-0e0h]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10033e1f
	mov esi, eax
	mov dword ptr [ebp-6ch], ebx
jmp_10033e1f:
	cmp eax, edi
	jl jmp_10033e25
	mov edi, eax
jmp_10033e25:
	and ecx, edx
	add ebx, 14h
	cmp ebx, dword ptr [ebp-68h]
	jne jmp_10033ddc
	or ecx, ecx
	jne jmp_10034411
	mov eax, dword ptr [ebp-6ch]
	mov dword ptr [ebp-78h], eax
	mov dword ptr [ebp-7ch], eax
	mov dword ptr [ebp-8ch], esi
	cmp edi, esi
	je jmp_10034411
jmp_10033e4e:
	mov ebx, dword ptr [ebp-78h]
	mov dword ptr [ebp-70h], ebx
	mov esi, ebx
	sub esi, 14h
	cmp esi, dword ptr [ebp-64h]
	jge jmp_10033e64
	mov esi, dword ptr [ebp-68h]
	sub esi, 14h
jmp_10033e64:
	mov dword ptr [ebp-78h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, dword ptr [ebp-0d8h]
	jge jmp_10033e7d
	cmp ecx, dword ptr [ebp-0d8h]
	jle jmp_10033e4e
jmp_10033e7d:
	sub ecx, edx
	je jmp_10033e4e
	mov dword ptr [ebp-80h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0a8h], eax
	mov ecx, dword ptr [ebp-80h]
	mov edx, dword ptr [esi+0ch]
	sub edx, dword ptr [ebx+0ch]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0b0h], eax
	mov ecx, dword ptr [ebp-80h]
	mov edx, dword ptr [esi+10h]
	sub edx, dword ptr [ebx+10h]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0b8h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [ebp-90h], edx
	mov edx, dword ptr [ebx+0ch]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [ebp-98h], edx
	mov edx, dword ptr [ebx+10h]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [ebp-0a0h], edx
jmp_10033f14:
	mov ebx, dword ptr [ebp-7ch]
	mov dword ptr [ebp-74h], ebx
	mov esi, ebx
	add esi, 14h
	cmp esi, dword ptr [ebp-68h]
	jl jmp_10033f27
	mov esi, dword ptr [ebp-64h]
jmp_10033f27:
	mov dword ptr [ebp-7ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, dword ptr [ebp-0d8h]
	jge jmp_10033f40
	cmp ecx, dword ptr [ebp-0d8h]
	jle jmp_10033f14
jmp_10033f40:
	sub ecx, edx
	je jmp_10033f14
	mov dword ptr [ebp-84h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0ach], eax
	mov ecx, dword ptr [ebp-84h]
	mov edx, dword ptr [esi+0ch]
	sub edx, dword ptr [ebx+0ch]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0b4h], eax
	mov ecx, dword ptr [ebp-84h]
	mov edx, dword ptr [esi+10h]
	sub edx, dword ptr [ebx+10h]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0bch], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [ebp-94h], edx
	mov edx, dword ptr [ebx+0ch]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [ebp-9ch], edx
	mov edx, dword ptr [ebx+10h]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [ebp-0a4h], edx
	mov eax, dword ptr [ebp-0e0h]
	sub eax, dword ptr [ebp-8ch]
	sub edi, dword ptr [ebp-0e0h]
	jg jmp_10033ff6
	add eax, edi
jmp_10033ff6:
	mov dword ptr [ebp-88h], eax
	mov eax, dword ptr [ebp-0d8h]
	sub eax, dword ptr [ebp-8ch]
	jle jmp_100340ad
	sub dword ptr [ebp-88h], eax
	mov ecx, dword ptr [ebp-0d8h]
	mov dword ptr [ebp-8ch], ecx
	mov ebx, dword ptr [ebp-70h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [ebp-80h], ecx
	shl ecx, 10h
	mov eax, dword ptr [ebp-0a8h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-90h], eax
	mov eax, dword ptr [ebp-0b0h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-98h], eax
	mov eax, dword ptr [ebp-0b8h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-0a0h], eax
	mov ecx, dword ptr [ebp-0d8h]
	mov ebx, dword ptr [ebp-74h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [ebp-84h], ecx
	shl ecx, 10h
	mov eax, dword ptr [ebp-0ach]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-94h], eax
	mov eax, dword ptr [ebp-0b4h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-9ch], eax
	mov eax, dword ptr [ebp-0bch]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-0a4h], eax
jmp_100340ad:
	mov eax, dword ptr [ebp-8ch]
	imul dword ptr [ebp-0e8h]
	add eax, dword ptr [ebp-0e4h]
	add eax, 0
	mov dword ptr [ebp-5ch], eax
	mov eax, dword ptr [ebp-90h]
	mov ebx, dword ptr [ebp-94h]
	mov ecx, dword ptr [ebp-98h]
	mov edx, dword ptr [ebp-9ch]
	mov esi, dword ptr [ebp-0a0h]
	mov edi, dword ptr [ebp-0a4h]
jmp_100340e9:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	cmp ebx, eax
	jg jmp_100340f8
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_100340f8:
	sar eax, 10h
	cmp eax, dword ptr [ebp-0dch]
	jg jmp_100343ba
	sar ebx, 10h
	cmp ebx, dword ptr [ebp-0d4h]
	jl jmp_100343ba
	mov dword ptr [ebp-0c0h], eax
	mov dword ptr [ebp-0c4h], ebx
	mov dword ptr [ebp-0d0h], ecx
	sub ebx, eax
	je jmp_100341fd
	push ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov dword ptr [ebp-0c8h], eax
	shld edx, eax, 10h
	pop ebx
	and eax, 0ffffh
	and edx, 0ffffh
	mov ecx, 1
	test edx, 8000h
	je jmp_10034172
	or edx, 0ffff0000h
	neg ecx
	cmp eax, 1
	sbb edx, -1
jmp_10034172:
	add ecx, edx
	push ecx
	push edx
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov dword ptr [ebp-0cch], eax
	shld edx, eax, 10h
	and eax, 0ffffh
	and edx, 0ffffh
	mov ecx, dword ptr [ebp-54h]
	test edx, 8000h
	je jmp_100341b0
	neg ecx
	cmp eax, 1
	sbb edx, -1
jmp_100341b0:
	mov eax, dword ptr [ebp-54h]
	imul dx
	cwde
	pop edx
	pop ebx
	add edx, eax
	mov dword ptr [g_rotatedCornerSteps], edx
	add edx, ecx
	mov dword ptr [g_rotatedCornerSteps+4], edx
	add ebx, eax
	mov dword ptr [g_rotatedCornerSteps+8], ebx
	add ebx, ecx
	mov dword ptr [g_rotatedCornerSteps+0ch], ebx
	mov ecx, dword ptr [ebp-0d4h]
	sub ecx, dword ptr [ebp-0c0h]
	jg jmp_10034440
jmp_100341eb:
	mov eax, dword ptr [ebp-0c4h]
	sub eax, dword ptr [ebp-0dch]
	jg jmp_1003446e
jmp_100341fd:
	mov ecx, esi
	shr esi, 10h
	mov eax, esi
	mul dword ptr [ebp-54h]
	add eax, dword ptr [ebp-58h]
	mov esi, dword ptr [ebp-0d0h]
	shr esi, 10h
	add esi, eax
	mov eax, dword ptr [ebp-0c0h]
	mov edi, dword ptr [ebp-5ch]
	add edi, eax
	mov ebx, dword ptr [ebp-0c4h]
	sub ebx, eax
	push ebp
	mov edx, dword ptr [ebp-0d0h]
	mov eax, dword ptr [ebp-0c8h]
	or eax, eax
	jns jmp_1003423d
	neg eax
	not edx
jmp_1003423d:
	shl eax, 10h
	shl edx, 10h
	mov ebp, dword ptr [ebp-0cch]
	or ebp, ebp
	jns jmp_10034251
	neg ebp
	not ecx
jmp_10034251:
	shl ebp, 10h
	shl ecx, 10h
	push ebx
	xor ebx, ebx
	cmp dword ptr [esp], 5
	jl jmp_1003431c
jmp_10034264:
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1003426d
	mov byte ptr [edi], bl
jmp_1003426d:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_10034288
	mov byte ptr [edi+1], bl
jmp_10034288:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_100342a3
	mov byte ptr [edi+2], bl
jmp_100342a3:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_100342be
	mov byte ptr [edi+3], bl
jmp_100342be:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_100342d9
	mov byte ptr [edi+4], bl
jmp_100342d9:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_100342f4
	mov byte ptr [edi+5], bl
jmp_100342f4:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	add edi, 6
	sub dword ptr [esp], 6
	js jmp_100343b6
	cmp dword ptr [esp], 5
	jge jmp_10034264
jmp_1003431c:
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_10034325
	mov byte ptr [edi], bl
jmp_10034325:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	dec dword ptr [esp]
	js jmp_100343b6
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_10034345
	mov byte ptr [edi+1], bl
jmp_10034345:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	dec dword ptr [esp]
	js jmp_100343b6
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_10034365
	mov byte ptr [edi+2], bl
jmp_10034365:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	dec dword ptr [esp]
	js jmp_100343b6
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_10034385
	mov byte ptr [edi+3], bl
jmp_10034385:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
	dec dword ptr [esp]
	js jmp_100343b6
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_100343a5
	mov byte ptr [edi+4], bl
jmp_100343a5:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_rotatedCornerSteps+ebx*4]
jmp_100343b6:
	add esp, 4
	pop ebp
jmp_100343ba:
	mov edi, dword ptr [ebp-0e8h]
	add dword ptr [ebp-5ch], edi
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	dec dword ptr [ebp-88h]
	js jmp_10034411
	je jmp_10034417
	dec dword ptr [ebp-80h]
	je jmp_10034479
	add eax, dword ptr [ebp-0a8h]
	add ecx, dword ptr [ebp-0b0h]
	add esi, dword ptr [ebp-0b8h]
jmp_100343ee:
	dec dword ptr [ebp-84h]
	je jmp_10034529
	add ebx, dword ptr [ebp-0ach]
	add edx, dword ptr [ebp-0b4h]
	add edi, dword ptr [ebp-0bch]
	jmp jmp_100340e9
jmp_10034411:
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_10034417:
	add eax, dword ptr [ebp-0a8h]
	add ecx, dword ptr [ebp-0b0h]
	add esi, dword ptr [ebp-0b8h]
	add ebx, dword ptr [ebp-0ach]
	add edx, dword ptr [ebp-0b4h]
	add edi, dword ptr [ebp-0bch]
	jmp jmp_100340e9
jmp_10034440:
	add dword ptr [ebp-0c0h], ecx
	shl ecx, 10h
	mov eax, dword ptr [ebp-0c8h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [ebp-0d0h], eax
	mov eax, dword ptr [ebp-0cch]
	imul ecx
	shrd eax, edx, 10h
	add esi, eax
	jmp jmp_100341eb
jmp_1003446e:
	sub dword ptr [ebp-0c4h], eax
	jmp jmp_100341fd
jmp_10034479:
	push ebx
	push edx
	mov ebx, dword ptr [ebp-78h]
	mov dword ptr [ebp-70h], ebx
	mov esi, ebx
	sub esi, 14h
	cmp esi, dword ptr [ebp-64h]
	jge jmp_10034491
	mov esi, dword ptr [ebp-68h]
	sub esi, 14h
jmp_10034491:
	mov dword ptr [ebp-78h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [ebp-80h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0a8h], eax
	mov ecx, dword ptr [ebp-80h]
	mov edx, dword ptr [esi+0ch]
	sub edx, dword ptr [ebx+0ch]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0b0h], eax
	mov ecx, dword ptr [ebp-80h]
	mov edx, dword ptr [esi+10h]
	sub edx, dword ptr [ebx+10h]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0b8h], eax
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	mov ecx, dword ptr [ebx+0ch]
	shl ecx, 10h
	add ecx, 8000h
	mov esi, dword ptr [ebx+10h]
	shl esi, 10h
	add esi, 8000h
	pop edx
	pop ebx
	jmp jmp_100343ee
jmp_10034529:
	push eax
	push ecx
	mov ebx, dword ptr [ebp-7ch]
	mov dword ptr [ebp-74h], ebx
	mov edi, ebx
	add edi, 14h
	cmp edi, dword ptr [ebp-68h]
	jl jmp_1003453e
	mov edi, dword ptr [ebp-64h]
jmp_1003453e:
	mov dword ptr [ebp-7ch], edi
	mov ecx, dword ptr [edi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [ebp-84h], ecx
	mov edx, dword ptr [edi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0ach], eax
	mov ecx, dword ptr [ebp-84h]
	mov edx, dword ptr [edi+0ch]
	sub edx, dword ptr [ebx+0ch]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0b4h], eax
	mov ecx, dword ptr [ebp-84h]
	mov edx, dword ptr [edi+10h]
	sub edx, dword ptr [ebx+10h]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [ebp-0bch], eax
	mov edx, dword ptr [ebx+0ch]
	shl edx, 10h
	add edx, 8000h
	mov edi, dword ptr [ebx+10h]
	shl edi, 10h
	add edi, 8000h
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop ecx
	pop eax
	jmp jmp_100340e9
jmp_100345e0:
	test dword ptr p_scaleY, 1
	je jmp_10034605
	push dword ptr p_unk0x10
	push dword ptr p_unk0x0c
	push dword ptr p_unk0x08
	push dword ptr p_unk0x04
	push dword ptr p_view
	call BlitShpFrameRemapped
	add esp, 14h
	jmp jmp_10034411
jmp_10034605:
	push dword ptr p_unk0x10
	push dword ptr p_unk0x0c
	push dword ptr p_unk0x08
	push dword ptr p_unk0x04
	push dword ptr p_view
	call BlitShpFrame
	add esp, 14h
	pop es
	pop edi
	pop esi
	pop ebx
	ret
BlitRotated endp

; Takes an offset-table resource and an index, like the SHP helpers, and four more arguments.
; Nothing calls it, and what it does hasn't been worked out from the assembly, so it keeps its
; placeholder.
FUN_10034622 proc uses ebx esi edi, p_data:dword, p_index:dword, p_unk0x08:dword, p_unk0x0c:dword, p_unk0x10:dword, p_unk0x14:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword
	push es
	cld
	push ds
	pop es
	mov dword ptr l_unk0x18, 0
	mov dword ptr l_unk0x1c, 0
	mov dword ptr l_unk0x20, 0
	mov dword ptr l_unk0x24, 0
	mov esi, dword ptr p_index
	shl esi, 3
	add esi, 8
	add esi, dword ptr p_data
	mov esi, dword ptr [esi]
	add esi, dword ptr p_data
	mov dword ptr l_unk0x04, esi
	mov esi, dword ptr l_unk0x04
	mov eax, dword ptr [esi+8]
	add eax, dword ptr p_unk0x08
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr [esi+0ch]
	mov ebx, eax
	add eax, dword ptr p_unk0x0c
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+10h]
	add eax, dword ptr p_unk0x08
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr [esi+14h]
	mov ecx, eax
	add ecx, dword ptr p_unk0x0c
	mov dword ptr l_unk0x14, ecx
	inc eax
	sub eax, ebx
	mov ebx, eax
	jle jmp_10034747
	add esi, 18h
	mov dword ptr l_unk0x18, 7fffffffh
	mov dword ptr l_unk0x1c, 7fffffffh
	mov dword ptr l_unk0x20, 80000000h
	mov dword ptr l_unk0x24, 80000000h
	mov edx, dword ptr l_unk0x0c
jmp_100346b7:
	mov edi, dword ptr l_unk0x08
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100346da
	jne jmp_1003470d
	jae jmp_1003473f
jmp_100346c5:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	add edi, ecx
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100346da
	jne jmp_1003470d
	jb jmp_100346c5
	jae jmp_1003473f
jmp_100346da:
	movzx ecx, al
	mov al, byte ptr [esi]
	inc esi
	cmp dword ptr l_unk0x18, edi
	jl jmp_100346e8
	mov dword ptr l_unk0x18, edi
jmp_100346e8:
	add edi, ecx
	cmp dword ptr l_unk0x20, edi
	jg jmp_100346f2
	mov dword ptr l_unk0x20, edi
jmp_100346f2:
	cmp dword ptr l_unk0x1c, edx
	jl jmp_100346fa
	mov dword ptr l_unk0x1c, edx
jmp_100346fa:
	cmp dword ptr l_unk0x24, edx
	jg jmp_10034702
	mov dword ptr l_unk0x24, edx
jmp_10034702:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100346da
	jae jmp_1003473f
	je jmp_100346c5
jmp_1003470d:
	movzx ecx, al
	add esi, ecx
	cmp dword ptr l_unk0x18, edi
	jl jmp_1003471a
	mov dword ptr l_unk0x18, edi
jmp_1003471a:
	add edi, ecx
	cmp dword ptr l_unk0x20, edi
	jg jmp_10034724
	mov dword ptr l_unk0x20, edi
jmp_10034724:
	cmp dword ptr l_unk0x1c, edx
	jl jmp_1003472c
	mov dword ptr l_unk0x1c, edx
jmp_1003472c:
	cmp dword ptr l_unk0x24, edx
	jg jmp_10034734
	mov dword ptr l_unk0x24, edx
jmp_10034734:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_100346da
	jne jmp_1003470d
	jb jmp_100346c5
jmp_1003473f:
	inc edx
	dec ebx
	jne jmp_100346b7
jmp_10034747:
	mov eax, dword ptr p_unk0x10
	and eax, 1
	je jmp_10034763
	mov eax, dword ptr p_unk0x08
	add eax, dword ptr p_unk0x08
	mov ebx, eax
	sub eax, dword ptr l_unk0x18
	sub ebx, dword ptr l_unk0x20
	mov dword ptr l_unk0x20, eax
	mov dword ptr l_unk0x18, ebx
jmp_10034763:
	mov eax, dword ptr p_unk0x10
	and eax, 2
	je jmp_1003477f
	mov eax, dword ptr p_unk0x0c
	add eax, dword ptr p_unk0x0c
	mov ebx, eax
	sub eax, dword ptr l_unk0x1c
	sub ebx, dword ptr l_unk0x24
	mov dword ptr l_unk0x24, eax
	mov dword ptr l_unk0x1c, ebx
jmp_1003477f:
	mov edi, dword ptr p_unk0x14
	mov eax, dword ptr l_unk0x18
	stosd
	mov eax, dword ptr l_unk0x1c
	stosd
	mov eax, dword ptr l_unk0x20
	stosd
	mov eax, dword ptr l_unk0x24
	stosd
	xor eax, eax
	pop es
	ret
FUN_10034622 endp

; Run-length encodes the view, skipping pixels equal to p_transparent: finds the bounding box of
; the opaque pixels, writes a 0x18-byte header to p_out (when not NULL) followed by the rows
; (EncodeRleRow) and returns the encoded size.
EncodeViewRle proc uses ebx esi edi, p_view:dword, p_transparent:dword, p_x:dword, p_y:dword, p_out:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x30, eax
	jle jmp_10034819
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10034819
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x34, eax
	cmp eax, 0
	jg jmp_100347cd
	mov eax, 0
jmp_100347cd:
	mov dword ptr l_unk0x1c, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x38, eax
	cmp eax, 0
	jg jmp_100347e0
	mov eax, 0
jmp_100347e0:
	mov dword ptr l_unk0x20, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x30
	dec edx
	cmp eax, edx
	jl jmp_100347f0
	mov eax, edx
jmp_100347f0:
	mov dword ptr l_unk0x24, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_100347ff
	mov eax, edx
jmp_100347ff:
	mov dword ptr l_unk0x28, eax
	mov eax, dword ptr l_unk0x24
	cmp eax, dword ptr l_unk0x1c
	jl jmp_10034824
	mov eax, dword ptr l_unk0x28
	cmp eax, dword ptr l_unk0x20
	jl jmp_10034824
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x2c, eax
	jmp jmp_1003482f
jmp_10034819:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10034824:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_1003482f:
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov edi, dword ptr p_out
	mov dword ptr [g_rleOutput], edi
	or edi, edi
	je jmp_1003487d
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [esi+4]
	shl eax, 10h
	mov ax, word ptr [esi+10h]
	sub ax, word ptr [esi+8]
	mov dword ptr [edi], eax
	add edi, 4
	mov eax, dword ptr p_x
	shl eax, 10h
	mov ax, word ptr p_y
	mov dword ptr [edi], eax
	add edi, 4
	xor eax, eax
	mov dword ptr [edi], eax
	add edi, 4
	mov dword ptr [edi], eax
	add edi, 4
	dec eax
	mov dword ptr [edi], eax
	add edi, 4
	mov dword ptr [edi], eax
	add edi, 4
jmp_1003487d:
	mov eax, dword ptr l_unk0x34
	add dword ptr p_x, eax
	mov eax, dword ptr l_unk0x38
	add dword ptr p_y, eax
	mov eax, dword ptr l_unk0x24
	inc eax
	sub eax, dword ptr l_unk0x1c
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr l_unk0x24
	inc eax
	sub eax, dword ptr l_unk0x1c
	mov dword ptr l_unk0x0c, eax
	mov eax, 7fffffffh
	mov dword ptr [g_rleLeft], eax
	mov dword ptr [g_rleTop], eax
	neg eax
	mov dword ptr [g_rleRight], eax
	mov dword ptr [g_rleBottom], eax
	mov eax, dword ptr l_unk0x20
	imul dword ptr l_unk0x30
	add eax, dword ptr l_unk0x2c
	add eax, dword ptr l_unk0x1c
	mov dword ptr [g_rleRow], eax
	mov esi, eax
	mov eax, dword ptr l_unk0x20
	mov dword ptr l_unk0x10, eax
	jmp jmp_1003496d
jmp_100348d6:
	mov edi, dword ptr [g_rleRow]
	mov dword ptr [g_rleRun], edi
	mov ecx, dword ptr l_unk0x08
	mov dword ptr l_unk0x14, ecx
	mov al, byte ptr p_transparent
	repe scasb
	je jmp_10034961
	mov eax, dword ptr [g_rleTop]
	cmp eax, dword ptr l_unk0x10
	jl jmp_100348fc
	mov eax, dword ptr l_unk0x10
jmp_100348fc:
	mov dword ptr [g_rleTop], eax
	mov eax, dword ptr [g_rleBottom]
	cmp eax, dword ptr l_unk0x10
	jg jmp_1003490e
	mov eax, dword ptr l_unk0x10
jmp_1003490e:
	mov dword ptr [g_rleBottom], eax
	mov eax, dword ptr l_unk0x24
	sub eax, ecx
	cmp eax, dword ptr [g_rleLeft]
	jl jmp_10034925
	mov eax, dword ptr [g_rleLeft]
jmp_10034925:
	mov dword ptr [g_rleLeft], eax
	mov eax, dword ptr [g_rleRow]
	add eax, dword ptr l_unk0x08
	dec eax
	mov edi, eax
	mov dword ptr [g_rleRun], edi
	mov ecx, dword ptr l_unk0x08
	mov dword ptr l_unk0x14, ecx
	mov al, byte ptr p_transparent
	std
	repe scasb
	cld
	je jmp_10034961
	mov eax, dword ptr l_unk0x1c
	add eax, ecx
	cmp eax, dword ptr [g_rleRight]
	jg jmp_1003495c
	mov eax, dword ptr [g_rleRight]
jmp_1003495c:
	mov dword ptr [g_rleRight], eax
jmp_10034961:
	mov eax, dword ptr l_unk0x30
	add dword ptr [g_rleRow], eax
	inc dword ptr l_unk0x10
jmp_1003496d:
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr l_unk0x28
	jle jmp_100348d6
	mov edi, dword ptr p_out
	or edi, edi
	je jmp_100349ac
	mov eax, dword ptr [g_rleLeft]
	sub eax, dword ptr p_x
	mov dword ptr [edi+8], eax
	mov eax, dword ptr [g_rleTop]
	sub eax, dword ptr p_y
	mov dword ptr [edi+0ch], eax
	mov eax, dword ptr [g_rleRight]
	sub eax, dword ptr p_x
	mov dword ptr [edi+10h], eax
	mov eax, dword ptr [g_rleBottom]
	sub eax, dword ptr p_y
	mov dword ptr [edi+14h], eax
jmp_100349ac:
	add edi, 18h
	mov dword ptr [g_rleCursor], edi
	mov eax, dword ptr [g_rleRight]
	inc eax
	sub eax, dword ptr [g_rleLeft]
	mov dword ptr l_unk0x18, eax
	mov eax, dword ptr [g_rleTop]
	imul dword ptr l_unk0x30
	add eax, dword ptr l_unk0x2c
	add eax, dword ptr [g_rleLeft]
	mov esi, eax
	mov dword ptr [g_rleRow], esi
	mov eax, dword ptr [g_rleTop]
	mov dword ptr l_unk0x10, eax
	jmp jmp_10034a04
jmp_100349e7:
	push dword ptr l_unk0x1c
	push dword ptr p_transparent
	push dword ptr l_unk0x18
	call EncodeRleRow
	add esp, 0ch
	mov eax, dword ptr l_unk0x30
	add dword ptr [g_rleRow], eax
	inc dword ptr l_unk0x10
jmp_10034a04:
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr [g_rleBottom]
	jle jmp_100349e7
	mov eax, dword ptr [g_rleCursor]
	sub eax, dword ptr p_out
	pop es
	ret
EncodeViewRle endp

; Maps the pixels of an entry of the data's offset table through the color remap table, in
; place, following its run-length codes.
RemapShpFrame proc uses ebx esi edi, p_data:dword, p_index:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_index
	shl esi, 3
	add esi, 8
	add esi, dword ptr p_data
	mov esi, dword ptr [esi]
	add esi, dword ptr p_data
	mov ebx, dword ptr [esi+0ch]
	mov eax, dword ptr [esi+14h]
	inc eax
	sub eax, ebx
	mov ebx, eax
	jle jmp_10034aa7
	add esi, 18h
jmp_10034a48:
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10034a66
	jne jmp_10034a84
	jae jmp_10034aa4
jmp_10034a53:
	mov al, byte ptr [esi]
	inc esi
	movzx ecx, al
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10034a66
	jne jmp_10034a84
	jb jmp_10034a53
	jae jmp_10034aa4
jmp_10034a66:
	movzx ecx, al
	mov eax, 0
	mov al, byte ptr [esi]
	mov al, byte ptr [g_remapTable+eax]
	mov byte ptr [esi], al
	inc esi
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10034a66
	jae jmp_10034aa4
	je jmp_10034a53
jmp_10034a84:
	movzx ecx, al
	mov eax, 0
jmp_10034a8c:
	mov al, byte ptr [esi]
	mov al, byte ptr [g_remapTable+eax]
	mov byte ptr [esi], al
	inc esi
	loop jmp_10034a8c
	mov al, byte ptr [esi]
	inc esi
	shr al, 1
	ja jmp_10034a66
	jne jmp_10034a84
	jb jmp_10034a53
jmp_10034aa4:
	dec ebx
	jne jmp_10034a48
jmp_10034aa7:
	xor eax, eax
	pop es
	ret
RemapShpFrame endp

; Encodes one row of p_count pixels at g_rleRow for EncodeViewRle.
EncodeRleRow proc uses ebx esi edi, p_count:dword, p_transparent:dword, p_left:dword
	local l_unk0x04:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr [g_rleRow]
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 0
	push 0
	call EmitRleRun
	add esp, 0ch
	mov dword ptr l_unk0x04, 5
	mov ecx, dword ptr p_count
	or ecx, ecx
	je jmp_10034c0d
	mov al, byte ptr [esi]
	inc esi
	dec ecx
	mov ah, al
	cmp ah, byte ptr p_transparent
	je jmp_10034bde
jmp_10034af8:
	mov dword ptr l_unk0x04, 1
	or ecx, ecx
	je jmp_10034c0d
	mov al, byte ptr [esi]
	inc esi
	dec ecx
	xor al, ah
	xor ah, al
	or al, al
	je jmp_10034bab
	cmp ah, byte ptr p_transparent
	jne jmp_10034b36
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 1
	push 1
	call EmitRleRun
	add esp, 0ch
	jmp jmp_10034bde
jmp_10034b36:
	or ecx, ecx
	je jmp_10034c0d
	mov al, byte ptr [esi]
	inc esi
	dec ecx
	xor al, ah
	xor ah, al
	cmp ah, byte ptr p_transparent
	jne jmp_10034b62
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 1
	push 1
	call EmitRleRun
	add esp, 0ch
	jmp jmp_10034bde
jmp_10034b62:
	or al, al
	jne jmp_10034b36
	or ecx, ecx
	je jmp_10034c0d
	mov al, byte ptr [esi]
	inc esi
	dec ecx
	xor al, ah
	xor ah, al
	cmp ah, byte ptr p_transparent
	jne jmp_10034b92
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 1
	push 1
	call EmitRleRun
	add esp, 0ch
	jmp jmp_10034bde
jmp_10034b92:
	or al, al
	jne jmp_10034b36
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 3
	push 1
	call EmitRleRun
	add esp, 0ch
jmp_10034bab:
	mov dword ptr l_unk0x04, 2
	or ecx, ecx
	je jmp_10034c0d
	mov al, byte ptr [esi]
	inc esi
	dec ecx
	xor al, ah
	je jmp_10034bab
	xor ah, al
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 1
	push 2
	call EmitRleRun
	add esp, 0ch
	cmp ah, byte ptr p_transparent
	jne jmp_10034af8
jmp_10034bde:
	mov dword ptr l_unk0x04, 3
	or ecx, ecx
	je jmp_10034c0d
	mov al, byte ptr [esi]
	inc esi
	dec ecx
	xor al, ah
	je jmp_10034bde
	xor ah, al
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 1
	push 3
	call EmitRleRun
	add esp, 0ch
	jmp jmp_10034af8
jmp_10034c0d:
	mov dword ptr [g_rleRun], esi
	push dword ptr p_left
	push 0
	push dword ptr l_unk0x04
	call EmitRleRun
	add esp, 0ch
	push dword ptr p_left
	push 0
	push 4
	call EmitRleRun
	add esp, 0ch
	pop es
	ret
EncodeRleRow endp

; Emits one run of EncodeRleRow's row encoding; p_op selects the kind (0 starts a row, 1 a
; literal run, 2 a repeated run, 3 a skip, 4 ends the row).
EmitRleRun proc uses ebx esi edi, p_op:dword, p_back:dword, p_left:dword
	push es
	cld
	push ds
	pop es
	push eax
	push ecx
	mov esi, dword ptr [g_rleRunStart]
	mov edi, dword ptr [g_rleCursor]
	mov eax, dword ptr p_op
	cmp eax, 2
	je jmp_10034c94
	cmp eax, 1
	je jmp_10034d3b
	cmp eax, 3
	je jmp_10034de0
	cmp eax, 4
	je jmp_10034df3
	cmp eax, 0
	jne jmp_10034e01
	xor eax, eax
	mov dword ptr [g_rleSkip], eax
	mov esi, dword ptr [g_rleRun]
	mov dword ptr [g_rleRunStart], esi
	jmp jmp_10034e01
jmp_10034c94:
	mov ebx, dword ptr [g_rleSkip]
	or ebx, ebx
	je jmp_10034cd3
jmp_10034c9e:
	mov ecx, ebx
	cmp ecx, 0ffh
	jl jmp_10034cad
	mov ecx, 0ffh
jmp_10034cad:
	sub ebx, ecx
	cmp dword ptr [g_rleOutput], 0
	je jmp_10034cc4
	mov al, 1
	mov byte ptr [edi], al
	inc edi
	mov al, cl
	mov byte ptr [edi], al
	inc edi
	jmp jmp_10034cc7
jmp_10034cc4:
	add edi, 2
jmp_10034cc7:
	add esi, ecx
	or ebx, ebx
	jne jmp_10034c9e
	mov dword ptr [g_rleSkip], ebx
jmp_10034cd3:
	mov ebx, dword ptr [g_rleRun]
	sub ebx, esi
	sub ebx, dword ptr p_back
	mov eax, dword ptr p_left
	add eax, esi
	sub eax, dword ptr [g_rleRow]
	cmp eax, dword ptr [g_rleLeft]
	jge jmp_10034cf6
	mov dword ptr [g_rleLeft], eax
jmp_10034cf6:
	add eax, ebx
	dec eax
	cmp eax, dword ptr [g_rleRight]
	jle jmp_10034d08
	mov dword ptr [g_rleRight], eax
	jmp jmp_10034d32
jmp_10034d08:
	mov ecx, ebx
	cmp ecx, 7fh
	jl jmp_10034d14
	mov ecx, 7fh
jmp_10034d14:
	cmp dword ptr [g_rleOutput], 0
	je jmp_10034d2b
	mov al, cl
	add al, al
	mov byte ptr [edi], al
	inc edi
	mov al, byte ptr [esi]
	mov byte ptr [edi], al
	inc edi
	jmp jmp_10034d2e
jmp_10034d2b:
	add edi, 2
jmp_10034d2e:
	add esi, ecx
	sub ebx, ecx
jmp_10034d32:
	or ebx, ebx
	jne jmp_10034d08
	jmp jmp_10034e01
jmp_10034d3b:
	mov ebx, dword ptr [g_rleSkip]
	or ebx, ebx
	je jmp_10034d7a
jmp_10034d45:
	mov ecx, ebx
	cmp ecx, 0ffh
	jl jmp_10034d54
	mov ecx, 0ffh
jmp_10034d54:
	sub ebx, ecx
	cmp dword ptr [g_rleOutput], 0
	je jmp_10034d6b
	mov al, 1
	mov byte ptr [edi], al
	inc edi
	mov al, cl
	mov byte ptr [edi], al
	inc edi
	jmp jmp_10034d6e
jmp_10034d6b:
	add edi, 2
jmp_10034d6e:
	add esi, ecx
	or ebx, ebx
	jne jmp_10034d45
	mov dword ptr [g_rleSkip], ebx
jmp_10034d7a:
	mov ebx, dword ptr [g_rleRun]
	sub ebx, esi
	sub ebx, dword ptr p_back
	mov eax, dword ptr p_left
	add eax, esi
	sub eax, dword ptr [g_rleRow]
	cmp eax, dword ptr [g_rleLeft]
	jge jmp_10034d9d
	mov dword ptr [g_rleLeft], eax
jmp_10034d9d:
	add eax, ebx
	dec eax
	cmp eax, dword ptr [g_rleRight]
	jle jmp_10034daf
	mov dword ptr [g_rleRight], eax
	jmp jmp_10034dda
jmp_10034daf:
	mov ecx, ebx
	cmp ecx, 7fh
	jl jmp_10034dbb
	mov ecx, 7fh
jmp_10034dbb:
	mov edx, ecx
	mov al, cl
	add al, al
	inc al
	cmp dword ptr [g_rleOutput], 0
	je jmp_10034dd3
	mov byte ptr [edi], al
	inc edi
	rep movsb
	jmp jmp_10034dd8
jmp_10034dd3:
	inc edi
	add esi, ecx
	add edi, ecx
jmp_10034dd8:
	sub ebx, edx
jmp_10034dda:
	or ebx, ebx
	jne jmp_10034daf
	jmp jmp_10034e01
jmp_10034de0:
	mov ebx, dword ptr [g_rleRun]
	sub ebx, esi
	sub ebx, dword ptr p_back
	mov dword ptr [g_rleSkip], ebx
	jmp jmp_10034e01
jmp_10034df3:
	xor eax, eax
	cmp dword ptr [g_rleOutput], 0
	je jmp_10034e00
	mov byte ptr [edi], al
jmp_10034e00:
	inc edi
jmp_10034e01:
	mov dword ptr [g_rleCursor], edi
	mov dword ptr [g_rleRunStart], esi
	pop ecx
	pop eax
	pop es
	ret
EmitRleRun endp

; Fills the view's rectangle, clipped to its buffer, with p_color. Leaves -1 in eax for an empty
; buffer, -2 for an empty rectangle, else 0.
FillView proc uses ebx esi edi, p_view:dword, p_color:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x18, eax
	jle jmp_10034e94
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10034e94
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x1c, eax
	cmp eax, 0
	jg jmp_10034e48
	mov eax, 0
jmp_10034e48:
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x20, eax
	cmp eax, 0
	jg jmp_10034e5b
	mov eax, 0
jmp_10034e5b:
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x18
	dec edx
	cmp eax, edx
	jl jmp_10034e6b
	mov eax, edx
jmp_10034e6b:
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10034e7a
	mov eax, edx
jmp_10034e7a:
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr l_unk0x0c
	cmp eax, dword ptr l_unk0x04
	jl jmp_10034e9f
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr l_unk0x08
	jl jmp_10034e9f
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x14, eax
	jmp jmp_10034eaa
jmp_10034e94:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10034e9f:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10034eaa:
	mov eax, dword ptr l_unk0x08
	imul dword ptr l_unk0x18
	add eax, dword ptr l_unk0x14
	add eax, dword ptr l_unk0x04
	mov edi, eax
	mov ebx, dword ptr l_unk0x0c
	inc ebx
	sub ebx, dword ptr l_unk0x04
	mov esi, dword ptr l_unk0x18
	sub esi, ebx
	mov al, byte ptr p_color
	mov ah, al
	shl eax, 10h
	mov al, byte ptr p_color
	mov ah, al
	mov edx, dword ptr l_unk0x08
	mov dword ptr l_unk0x24, ebx
	cmp ebx, 4
	jle jmp_10034ee5
	jmp jmp_10034f0b
jmp_10034ede:
	mov ecx, ebx
	rep stosb
	add edi, esi
	inc edx
jmp_10034ee5:
	cmp edx, dword ptr l_unk0x10
	jle jmp_10034ede
	jmp jmp_10034f10
jmp_10034eec:
	mov ecx, edi
	mov ebx, dword ptr l_unk0x24
	neg ecx
	and ecx, 3
	sub ebx, ecx
	rep stosb
	mov ecx, ebx
	and ebx, 3
	shr ecx, 2
	rep stosd
	mov ecx, ebx
	rep stosb
	add edi, esi
	inc edx
jmp_10034f0b:
	cmp edx, dword ptr l_unk0x10
	jle jmp_10034eec
jmp_10034f10:
	xor eax, eax
	pop es
	ret
FillView endp

; Copies p_source's rectangle to p_dest at (p_destLeft, p_destTop), clipped to both views. A
; p_fillColor from 0 to 0xff fills the rectangle with that color instead.
BlitView proc p_source:dword, p_sourceLeft:dword, p_sourceTop:dword, p_dest:dword, p_destLeft:dword, p_destTop:dword, p_fillColor:dword
	add esp, 0ffffff64h
	push ebx
	push esi
	push edi
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_source
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr [ebp-70h], eax
	jle jmp_10034f9a
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10034f9a
	mov eax, dword ptr [esi+4]
	mov dword ptr [ebp-78h], eax
	cmp eax, 0
	jg jmp_10034f4e
	mov eax, 0
jmp_10034f4e:
	mov dword ptr [ebp-60h], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [ebp-7ch], eax
	cmp eax, 0
	jg jmp_10034f61
	mov eax, 0
jmp_10034f61:
	mov dword ptr [ebp-64h], eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr [ebp-70h]
	dec edx
	cmp eax, edx
	jl jmp_10034f71
	mov eax, edx
jmp_10034f71:
	mov dword ptr [ebp-68h], eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10034f80
	mov eax, edx
jmp_10034f80:
	mov dword ptr [ebp-6ch], eax
	mov eax, dword ptr [ebp-68h]
	cmp eax, dword ptr [ebp-60h]
	jl jmp_10034fa5
	mov eax, dword ptr [ebp-6ch]
	cmp eax, dword ptr [ebp-64h]
	jl jmp_10034fa5
	mov eax, dword ptr [ebx]
	mov dword ptr [ebp-74h], eax
	jmp jmp_10034fb0
jmp_10034f9a:
	mov eax, 0ffffffffh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_10034fa5:
	mov eax, 0fffffffeh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_10034fb0:
	mov eax, dword ptr [ebp-60h]
	mov dword ptr [ebp-2ch], eax
	mov eax, dword ptr [ebp-64h]
	mov dword ptr [ebp-30h], eax
	mov eax, dword ptr [ebp-68h]
	mov dword ptr [ebp-34h], eax
	mov eax, dword ptr [ebp-6ch]
	mov dword ptr [ebp-38h], eax
	mov eax, dword ptr [ebp-78h]
	sub dword ptr [ebp-2ch], eax
	sub dword ptr [ebp-34h], eax
	mov eax, dword ptr [ebp-7ch]
	sub dword ptr [ebp-30h], eax
	sub dword ptr [ebp-38h], eax
	mov esi, dword ptr p_dest
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr [ebp-94h], eax
	jle jmp_10035071
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10035071
	mov eax, dword ptr [esi+4]
	mov dword ptr [ebp-98h], eax
	cmp eax, 0
	jg jmp_1003500a
	mov eax, 0
jmp_1003500a:
	mov dword ptr [ebp-80h], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [ebp-9ch], eax
	cmp eax, 0
	jg jmp_10035020
	mov eax, 0
jmp_10035020:
	mov dword ptr [ebp-84h], eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr [ebp-94h]
	dec edx
	cmp eax, edx
	jl jmp_10035036
	mov eax, edx
jmp_10035036:
	mov dword ptr [ebp-88h], eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10035048
	mov eax, edx
jmp_10035048:
	mov dword ptr [ebp-8ch], eax
	mov eax, dword ptr [ebp-88h]
	cmp eax, dword ptr [ebp-80h]
	jl jmp_1003507c
	mov eax, dword ptr [ebp-8ch]
	cmp eax, dword ptr [ebp-84h]
	jl jmp_1003507c
	mov eax, dword ptr [ebx]
	mov dword ptr [ebp-90h], eax
	jmp jmp_10035087
jmp_10035071:
	mov eax, 0ffffffffh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_1003507c:
	mov eax, 0fffffffeh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_10035087:
	mov eax, dword ptr [ebp-80h]
	mov dword ptr [ebp-40h], eax
	mov eax, dword ptr [ebp-84h]
	mov dword ptr [ebp-44h], eax
	mov eax, dword ptr [ebp-88h]
	mov dword ptr [ebp-48h], eax
	mov eax, dword ptr [ebp-8ch]
	mov dword ptr [ebp-4ch], eax
	mov eax, dword ptr [ebp-98h]
	sub dword ptr [ebp-40h], eax
	sub dword ptr [ebp-48h], eax
	mov eax, dword ptr [ebp-9ch]
	sub dword ptr [ebp-44h], eax
	sub dword ptr [ebp-4ch], eax
	mov eax, dword ptr p_sourceLeft
	sub eax, dword ptr p_destLeft
	mov dword ptr [ebp-24h], eax
	mov eax, dword ptr p_sourceTop
	sub eax, dword ptr p_destTop
	mov dword ptr [ebp-28h], eax
	mov eax, dword ptr [ebp-2ch]
	mov edx, dword ptr [ebp-40h]
	add edx, dword ptr [ebp-24h]
	cmp eax, edx
	jg jmp_100350e1
	mov eax, edx
jmp_100350e1:
	mov dword ptr [ebp-4], eax
	mov eax, dword ptr [ebp-30h]
	mov edx, dword ptr [ebp-44h]
	add edx, dword ptr [ebp-28h]
	cmp eax, edx
	jg jmp_100350f3
	mov eax, edx
jmp_100350f3:
	mov dword ptr [ebp-8], eax
	mov eax, dword ptr [ebp-34h]
	mov edx, dword ptr [ebp-48h]
	add edx, dword ptr [ebp-24h]
	cmp eax, edx
	jl jmp_10035105
	mov eax, edx
jmp_10035105:
	mov dword ptr [ebp-0ch], eax
	mov eax, dword ptr [ebp-38h]
	mov edx, dword ptr [ebp-4ch]
	add edx, dword ptr [ebp-28h]
	cmp eax, edx
	jl jmp_10035117
	mov eax, edx
jmp_10035117:
	mov dword ptr [ebp-10h], eax
	mov eax, dword ptr [ebp-0ch]
	cmp eax, dword ptr [ebp-4]
	jl jmp_100352a9
	mov eax, dword ptr [ebp-10h]
	cmp eax, dword ptr [ebp-8]
	jl jmp_100352a9
	mov eax, dword ptr [ebp-40h]
	mov edx, dword ptr [ebp-2ch]
	sub edx, dword ptr [ebp-24h]
	cmp eax, edx
	jg jmp_10035141
	mov eax, edx
jmp_10035141:
	mov dword ptr [ebp-14h], eax
	mov eax, dword ptr [ebp-44h]
	mov edx, dword ptr [ebp-30h]
	sub edx, dword ptr [ebp-28h]
	cmp eax, edx
	jg jmp_10035153
	mov eax, edx
jmp_10035153:
	mov dword ptr [ebp-18h], eax
	mov eax, dword ptr [ebp-48h]
	mov edx, dword ptr [ebp-34h]
	sub edx, dword ptr [ebp-24h]
	cmp eax, edx
	jl jmp_10035165
	mov eax, edx
jmp_10035165:
	mov dword ptr [ebp-1ch], eax
	mov eax, dword ptr [ebp-4ch]
	mov edx, dword ptr [ebp-38h]
	sub edx, dword ptr [ebp-28h]
	cmp eax, edx
	jl jmp_10035177
	mov eax, edx
jmp_10035177:
	mov dword ptr [ebp-20h], eax
	mov eax, dword ptr [ebp-0ch]
	inc eax
	sub eax, dword ptr [ebp-4]
	mov dword ptr [ebp-58h], eax
	mov eax, dword ptr [ebp-10h]
	inc eax
	sub eax, dword ptr [ebp-8]
	mov dword ptr [ebp-54h], eax
	mov eax, dword ptr [ebp-7ch]
	imul dword ptr [ebp-70h]
	add eax, dword ptr [ebp-74h]
	add eax, dword ptr [ebp-78h]
	mov esi, eax
	mov eax, dword ptr [ebp-9ch]
	imul dword ptr [ebp-94h]
	add eax, dword ptr [ebp-90h]
	add eax, dword ptr [ebp-98h]
	mov edi, eax
	mov eax, dword ptr [ebp-8]
	mov ebx, dword ptr [ebp-18h]
	cmp eax, ebx
	jle jmp_100351e0
	mul dword ptr [ebp-70h]
	add esi, eax
	mov eax, ebx
	mul dword ptr [ebp-94h]
	add edi, eax
	mov eax, dword ptr [ebp-70h]
	mov dword ptr [ebp-3ch], eax
	mov eax, dword ptr [ebp-94h]
	mov dword ptr [ebp-50h], eax
	jmp jmp_10035206
jmp_100351e0:
	mov eax, dword ptr [ebp-10h]
	mul dword ptr [ebp-70h]
	add esi, eax
	mov eax, dword ptr [ebp-20h]
	mul dword ptr [ebp-94h]
	add edi, eax
	mov eax, dword ptr [ebp-70h]
	neg eax
	mov dword ptr [ebp-3ch], eax
	mov eax, dword ptr [ebp-94h]
	neg eax
	mov dword ptr [ebp-50h], eax
jmp_10035206:
	mov ecx, dword ptr [ebp-58h]
	mov eax, dword ptr [ebp-4]
	mov ebx, dword ptr [ebp-14h]
	cmp eax, ebx
	jle jmp_10035227
	add esi, eax
	add edi, ebx
	sub dword ptr [ebp-3ch], ecx
	sub dword ptr [ebp-50h], ecx
	cld
	mov dword ptr [ebp-5ch], 0
	jmp jmp_1003523b
jmp_10035227:
	add esi, dword ptr [ebp-0ch]
	add edi, dword ptr [ebp-1ch]
	add dword ptr [ebp-3ch], ecx
	add dword ptr [ebp-50h], ecx
	std
	mov dword ptr [ebp-5ch], 3
jmp_1003523b:
	mov eax, dword ptr p_fillColor
	test eax, 0ffffff00h
	je jmp_10035274
	mov edx, dword ptr [ebp-54h]
	mov eax, dword ptr [ebp-3ch]
	mov ebx, dword ptr [ebp-50h]
jmp_1003524e:
	mov ecx, dword ptr [ebp-58h]
	and ecx, 3
	rep movsb
	mov ecx, dword ptr [ebp-58h]
	shr ecx, 2
	sub esi, dword ptr [ebp-5ch]
	sub edi, dword ptr [ebp-5ch]
	rep movsd
	add esi, dword ptr [ebp-5ch]
	add edi, dword ptr [ebp-5ch]
	add esi, eax
	add edi, ebx
	dec edx
	jne jmp_1003524e
	cld
	jmp jmp_100352a1
jmp_10035274:
	mov dl, al
	mov ah, al
	shl eax, 10h
	mov al, dl
	mov ah, al
	mov edx, dword ptr [ebp-54h]
	mov ebx, dword ptr [ebp-50h]
jmp_10035285:
	mov ecx, dword ptr [ebp-58h]
	and ecx, 3
	rep stosb
	mov ecx, dword ptr [ebp-58h]
	shr ecx, 2
	sub edi, dword ptr [ebp-5ch]
	rep stosd
	add edi, dword ptr [ebp-5ch]
	add edi, ebx
	dec edx
	jne jmp_10035285
	cld
jmp_100352a1:
	xor eax, eax
	pop es
	pop edi
	pop esi
	pop ebx
	ret
jmp_100352a9:
	mov eax, 0fffffffdh
	pop es
	pop edi
	pop esi
	pop ebx
	ret
BlitView endp

; Scrolls the view by (p_dx, p_dy) with wrap-around, through nine BlitView blits.
ScrollView proc uses ebx esi edi, p_view:dword, p_dx:dword, p_dy:dword, p_mode:dword, p_color:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov eax, dword ptr [esi+0ch]
	inc eax
	sub eax, dword ptr [esi+4]
	mov dword ptr l_unk0x30, eax
	jle jmp_100354a6
	mov edx, dword ptr [esi+10h]
	inc edx
	sub edx, dword ptr [esi+8]
	mov dword ptr l_unk0x34, edx
	jle jmp_100354a6
	neg eax
	mov dword ptr l_unk0x38, eax
	neg edx
	mov dword ptr l_unk0x3c, edx
	cmp dword ptr p_mode, 1
	je jmp_1003533b
	mov eax, dword ptr p_dx
	cdq
	xor eax, edx
	sub eax, edx
	cmp eax, dword ptr l_unk0x30
	jge jmp_1003531f
	mov eax, dword ptr p_dy
	cdq
	xor eax, edx
	sub eax, edx
	cmp eax, dword ptr l_unk0x34
	jge jmp_1003531f
	mov eax, dword ptr p_view
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr p_color
	mov dword ptr l_unk0x44, eax
	jmp jmp_100353a6
jmp_1003531f:
	mov eax, dword ptr p_color
	push 0
	movzx ax, al
	push ax
	push dword ptr p_view
	call FillView
	add esp, 8
	pop es
	ret
jmp_1003533b:
	cmp dword ptr p_color, 0
	je jmp_1003549a
	lea eax, l_unk0x28
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr p_color
	mov dword ptr l_unk0x14, eax
	lea eax, l_unk0x14
	mov dword ptr l_unk0x28, eax
	xor eax, eax
	mov dword ptr l_unk0x24, eax
	mov dword ptr l_unk0x20, eax
	mov eax, dword ptr l_unk0x30
	dec eax
	mov dword ptr l_unk0x10, eax
	mov dword ptr l_unk0x1c, eax
	mov eax, dword ptr l_unk0x34
	dec eax
	mov dword ptr l_unk0x0c, eax
	mov dword ptr l_unk0x18, eax
	push -1
	push 0
	push 0
	push dword ptr l_unk0x2c
	push 0
	push 0
	push dword ptr p_view
	call BlitView
	add esp, 1ch
	mov eax, dword ptr p_dx
	cdq
	idiv dword ptr l_unk0x30
	mov dword ptr p_dx, edx
	mov eax, dword ptr p_dy
	cdq
	idiv dword ptr l_unk0x34
	mov dword ptr p_dy, edx
	mov dword ptr l_unk0x44, 0ffffffffh
jmp_100353a6:
	mov eax, dword ptr p_dx
	or eax, dword ptr p_dy
	je jmp_10035492
	mov esi, dword ptr l_unk0x2c
	mov edi, dword ptr p_view
	push -1
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push 0
	push 0
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push dword ptr l_unk0x34
	push dword ptr l_unk0x30
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push 0
	push dword ptr l_unk0x30
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push dword ptr l_unk0x3c
	push dword ptr l_unk0x30
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push dword ptr l_unk0x34
	push 0
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push dword ptr l_unk0x3c
	push 0
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push dword ptr l_unk0x34
	push dword ptr l_unk0x38
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push 0
	push dword ptr l_unk0x38
	push esi
	call BlitView
	add esp, 1ch
	push dword ptr l_unk0x44
	push dword ptr p_dy
	push dword ptr p_dx
	push edi
	push dword ptr l_unk0x3c
	push dword ptr l_unk0x38
	push esi
	call BlitView
	add esp, 1ch
jmp_10035492:
	xor eax, eax
	pop es
	ret
jmp_1003549a:
	mov eax, dword ptr l_unk0x30
	mul dword ptr l_unk0x34
	pop es
	ret
jmp_100354a6:
	mov eax, 0fffffffeh
	pop es
	ret
ScrollView endp

; Draws the outline of an ellipse centered on (p_x, p_y), clipped to the view.
DrawEllipse proc uses ebx esi edi, p_view:dword, p_x:dword, p_y:dword, p_radiusX:dword, p_radiusY:dword, p_color:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword, l_unk0x48:dword
	local l_unk0x4c:dword, l_unk0x50:dword, l_unk0x54:dword
	push es
	cld
	push ds
	pop es
	cmp dword ptr p_radiusX, 0
	je jmp_100354ca
	cmp dword ptr p_radiusY, 0
	jne jmp_100354fb
jmp_100354ca:
	mov eax, dword ptr p_y
	add eax, dword ptr p_radiusY
	mov ebx, dword ptr p_x
	add ebx, dword ptr p_radiusX
	mov ecx, dword ptr p_y
	sub ecx, dword ptr p_radiusY
	mov edx, dword ptr p_x
	sub edx, dword ptr p_radiusX
	push dword ptr p_color
	push 0
	push eax
	push ebx
	push ecx
	push edx
	push dword ptr p_view
	call BlitLine
	add esp, 1ch
	jmp jmp_100357ec
jmp_100354fb:
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x4c, eax
	jle jmp_1003556d
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_1003556d
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x50, eax
	cmp eax, 0
	jg jmp_10035521
	mov eax, 0
jmp_10035521:
	mov dword ptr l_unk0x38, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x54, eax
	cmp eax, 0
	jg jmp_10035534
	mov eax, 0
jmp_10035534:
	mov dword ptr l_unk0x3c, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x4c
	dec edx
	cmp eax, edx
	jl jmp_10035544
	mov eax, edx
jmp_10035544:
	mov dword ptr l_unk0x40, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10035553
	mov eax, edx
jmp_10035553:
	mov dword ptr l_unk0x44, eax
	mov eax, dword ptr l_unk0x40
	cmp eax, dword ptr l_unk0x38
	jl jmp_10035578
	mov eax, dword ptr l_unk0x44
	cmp eax, dword ptr l_unk0x3c
	jl jmp_10035578
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x48, eax
	jmp jmp_10035583
jmp_1003556d:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10035578:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10035583:
	mov eax, dword ptr p_color
	mov ah, al
	mov dword ptr p_color, eax
	mov word ptr [ebp+1eh], ax
	mov eax, dword ptr l_unk0x50
	add dword ptr p_x, eax
	mov eax, dword ptr l_unk0x54
	add dword ptr p_y, eax
	mov eax, dword ptr p_x
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr p_y
	mov dword ptr l_unk0x08, eax
	mov dword ptr l_unk0x0c, 0
	mov eax, dword ptr p_radiusY
	mov dword ptr l_unk0x10, eax
	mul eax
	mov dword ptr l_unk0x1c, eax
	shl eax, 1
	mov dword ptr l_unk0x20, eax
	mov eax, dword ptr p_radiusX
	mul eax
	mov dword ptr l_unk0x14, eax
	shl eax, 1
	mov dword ptr l_unk0x18, eax
	mov dword ptr l_unk0x24, 0
	mov eax, dword ptr l_unk0x18
	mul dword ptr p_radiusY
	mov dword ptr l_unk0x28, eax
	mov eax, dword ptr l_unk0x14
	shr eax, 2
	add eax, dword ptr l_unk0x1c
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr l_unk0x14
	mul dword ptr p_radiusY
	sub dword ptr l_unk0x2c, eax
	mov ebx, dword ptr p_radiusY
jmp_100355f3:
	mov eax, dword ptr l_unk0x24
	sub eax, dword ptr l_unk0x28
	jns jmp_100356e9
	push ebx
	mov ecx, dword ptr p_color
	mov edi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	add edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_10035631
	cmp edi, dword ptr l_unk0x40
	jg jmp_10035631
	cmp edx, dword ptr l_unk0x3c
	jl jmp_10035631
	cmp edx, dword ptr l_unk0x44
	jg jmp_10035631
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_10035631:
	mov edi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	sub edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_1003565f
	cmp edi, dword ptr l_unk0x40
	jg jmp_1003565f
	cmp edx, dword ptr l_unk0x3c
	jl jmp_1003565f
	cmp edx, dword ptr l_unk0x44
	jg jmp_1003565f
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_1003565f:
	mov edi, dword ptr l_unk0x04
	sub edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	add edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_1003568d
	cmp edi, dword ptr l_unk0x40
	jg jmp_1003568d
	cmp edx, dword ptr l_unk0x3c
	jl jmp_1003568d
	cmp edx, dword ptr l_unk0x44
	jg jmp_1003568d
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_1003568d:
	mov edi, dword ptr l_unk0x04
	sub edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	sub edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_100356bb
	cmp edi, dword ptr l_unk0x40
	jg jmp_100356bb
	cmp edx, dword ptr l_unk0x3c
	jl jmp_100356bb
	cmp edx, dword ptr l_unk0x44
	jg jmp_100356bb
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_100356bb:
	pop ebx
	cmp dword ptr l_unk0x2c, 0
	js jmp_100356d2
	dec dword ptr l_unk0x10
	dec ebx
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr l_unk0x18
	mov dword ptr l_unk0x28, eax
	sub dword ptr l_unk0x2c, eax
jmp_100356d2:
	inc dword ptr l_unk0x0c
	mov eax, dword ptr l_unk0x24
	add eax, dword ptr l_unk0x20
	mov dword ptr l_unk0x24, eax
	add eax, dword ptr l_unk0x1c
	add dword ptr l_unk0x2c, eax
	jmp jmp_100355f3
jmp_100356e9:
	mov eax, dword ptr l_unk0x14
	sub eax, dword ptr l_unk0x1c
	mov edx, eax
	sar eax, 1
	add eax, edx
	sub eax, dword ptr l_unk0x24
	sub eax, dword ptr l_unk0x28
	sar eax, 1
	add dword ptr l_unk0x2c, eax
jmp_10035700:
	push ebx
	mov ecx, dword ptr p_color
	mov edi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	add edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_10035732
	cmp edi, dword ptr l_unk0x40
	jg jmp_10035732
	cmp edx, dword ptr l_unk0x3c
	jl jmp_10035732
	cmp edx, dword ptr l_unk0x44
	jg jmp_10035732
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_10035732:
	mov edi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	sub edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_10035760
	cmp edi, dword ptr l_unk0x40
	jg jmp_10035760
	cmp edx, dword ptr l_unk0x3c
	jl jmp_10035760
	cmp edx, dword ptr l_unk0x44
	jg jmp_10035760
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_10035760:
	mov edi, dword ptr l_unk0x04
	sub edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	add edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_1003578e
	cmp edi, dword ptr l_unk0x40
	jg jmp_1003578e
	cmp edx, dword ptr l_unk0x3c
	jl jmp_1003578e
	cmp edx, dword ptr l_unk0x44
	jg jmp_1003578e
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_1003578e:
	mov edi, dword ptr l_unk0x04
	sub edi, dword ptr l_unk0x0c
	mov edx, dword ptr l_unk0x08
	sub edx, dword ptr l_unk0x10
	cmp edi, dword ptr l_unk0x38
	jl jmp_100357bc
	cmp edi, dword ptr l_unk0x40
	jg jmp_100357bc
	cmp edx, dword ptr l_unk0x3c
	jl jmp_100357bc
	cmp edx, dword ptr l_unk0x44
	jg jmp_100357bc
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov byte ptr [edi], cl
jmp_100357bc:
	pop ebx
	cmp dword ptr l_unk0x2c, 0
	jns jmp_100357d2
	inc dword ptr l_unk0x0c
	mov eax, dword ptr l_unk0x24
	add eax, dword ptr l_unk0x20
	mov dword ptr l_unk0x24, eax
	add dword ptr l_unk0x2c, eax
jmp_100357d2:
	dec dword ptr l_unk0x10
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr l_unk0x18
	mov dword ptr l_unk0x28, eax
	sub eax, dword ptr l_unk0x14
	sub dword ptr l_unk0x2c, eax
	dec ebx
	js jmp_100357ec
	jmp jmp_10035700
jmp_100357ec:
	pop es
	ret
DrawEllipse endp

; Fills an ellipse centered on (p_x, p_y), clipped to the view.
FillEllipse proc uses ebx esi edi, p_view:dword, p_x:dword, p_y:dword, p_radiusX:dword, p_radiusY:dword, p_color:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword, l_unk0x48:dword
	local l_unk0x4c:dword, l_unk0x50:dword, l_unk0x54:dword
	push es
	cld
	push ds
	pop es
	cmp dword ptr p_radiusX, 0
	je jmp_1003580b
	cmp dword ptr p_radiusY, 0
	jne jmp_1003583c
jmp_1003580b:
	mov eax, dword ptr p_y
	add eax, dword ptr p_radiusY
	mov ebx, dword ptr p_x
	add ebx, dword ptr p_radiusX
	mov ecx, dword ptr p_y
	sub ecx, dword ptr p_radiusY
	mov edx, dword ptr p_x
	sub edx, dword ptr p_radiusX
	push dword ptr p_color
	push 0
	push eax
	push ebx
	push ecx
	push edx
	push dword ptr p_view
	call BlitLine
	add esp, 1ch
	jmp jmp_10035aea
jmp_1003583c:
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x4c, eax
	jle jmp_100358ae
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_100358ae
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x50, eax
	cmp eax, 0
	jg jmp_10035862
	mov eax, 0
jmp_10035862:
	mov dword ptr l_unk0x38, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x54, eax
	cmp eax, 0
	jg jmp_10035875
	mov eax, 0
jmp_10035875:
	mov dword ptr l_unk0x3c, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x4c
	dec edx
	cmp eax, edx
	jl jmp_10035885
	mov eax, edx
jmp_10035885:
	mov dword ptr l_unk0x40, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10035894
	mov eax, edx
jmp_10035894:
	mov dword ptr l_unk0x44, eax
	mov eax, dword ptr l_unk0x40
	cmp eax, dword ptr l_unk0x38
	jl jmp_100358b9
	mov eax, dword ptr l_unk0x44
	cmp eax, dword ptr l_unk0x3c
	jl jmp_100358b9
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x48, eax
	jmp jmp_100358c4
jmp_100358ae:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_100358b9:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_100358c4:
	mov eax, dword ptr p_color
	mov ah, al
	mov dword ptr p_color, eax
	mov word ptr [ebp+1eh], ax
	mov eax, dword ptr l_unk0x50
	add dword ptr p_x, eax
	mov eax, dword ptr l_unk0x54
	add dword ptr p_y, eax
	mov eax, dword ptr p_x
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr p_y
	mov dword ptr l_unk0x08, eax
	mov dword ptr l_unk0x0c, 0
	mov eax, dword ptr p_radiusY
	mov dword ptr l_unk0x10, eax
	mul eax
	mov dword ptr l_unk0x1c, eax
	shl eax, 1
	mov dword ptr l_unk0x20, eax
	mov eax, dword ptr p_radiusX
	mul eax
	mov dword ptr l_unk0x14, eax
	shl eax, 1
	mov dword ptr l_unk0x18, eax
	mov dword ptr l_unk0x24, 0
	mov eax, dword ptr l_unk0x18
	mul dword ptr p_radiusY
	mov dword ptr l_unk0x28, eax
	mov eax, dword ptr l_unk0x14
	shr eax, 2
	add eax, dword ptr l_unk0x1c
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr l_unk0x14
	mul dword ptr p_radiusY
	sub dword ptr l_unk0x2c, eax
	mov ebx, dword ptr p_radiusY
jmp_10035934:
	mov eax, dword ptr l_unk0x24
	sub eax, dword ptr l_unk0x28
	js jmp_10035941
	jmp jmp_10035a09
jmp_10035941:
	mov edi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	cmp edi, dword ptr l_unk0x38
	jl jmp_100359dc
	cmp edi, dword ptr l_unk0x40
	jl jmp_10035958
	mov edi, dword ptr l_unk0x40
jmp_10035958:
	mov dword ptr l_unk0x34, edi
	mov edi, dword ptr l_unk0x04
	sub edi, dword ptr l_unk0x0c
	cmp edi, dword ptr l_unk0x40
	jg jmp_100359dc
	cmp edi, dword ptr l_unk0x38
	jg jmp_1003596e
	mov edi, dword ptr l_unk0x38
jmp_1003596e:
	mov dword ptr l_unk0x30, edi
	mov edx, dword ptr l_unk0x08
	add edx, dword ptr l_unk0x10
	cmp edx, dword ptr l_unk0x3c
	jl jmp_100359dc
	cmp edx, dword ptr l_unk0x44
	jg jmp_100359a5
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov ecx, dword ptr l_unk0x34
	sub ecx, dword ptr l_unk0x30
	inc ecx
	mov eax, dword ptr p_color
	mov edx, ecx
	and edx, 3
	shr ecx, 2
	rep stosd
	mov ecx, edx
	rep stosb
jmp_100359a5:
	mov edi, dword ptr l_unk0x30
	mov edx, dword ptr l_unk0x08
	sub edx, dword ptr l_unk0x10
	cmp edx, dword ptr l_unk0x3c
	jl jmp_100359dc
	cmp edx, dword ptr l_unk0x44
	jg jmp_100359dc
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov ecx, dword ptr l_unk0x34
	sub ecx, dword ptr l_unk0x30
	inc ecx
	mov eax, dword ptr p_color
	mov edx, ecx
	and edx, 3
	shr ecx, 2
	rep stosd
	mov ecx, edx
	rep stosb
jmp_100359dc:
	cmp dword ptr l_unk0x2c, 0
	js jmp_100359f2
	dec dword ptr l_unk0x10
	dec ebx
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr l_unk0x18
	mov dword ptr l_unk0x28, eax
	sub dword ptr l_unk0x2c, eax
jmp_100359f2:
	inc dword ptr l_unk0x0c
	mov eax, dword ptr l_unk0x24
	add eax, dword ptr l_unk0x20
	mov dword ptr l_unk0x24, eax
	add eax, dword ptr l_unk0x1c
	add dword ptr l_unk0x2c, eax
	jmp jmp_10035934
jmp_10035a09:
	mov eax, dword ptr l_unk0x14
	sub eax, dword ptr l_unk0x1c
	mov edx, eax
	sar eax, 1
	add eax, edx
	sub eax, dword ptr l_unk0x24
	sub eax, dword ptr l_unk0x28
	sar eax, 1
	add dword ptr l_unk0x2c, eax
jmp_10035a20:
	mov edi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	cmp edi, dword ptr l_unk0x38
	jl jmp_10035abb
	cmp edi, dword ptr l_unk0x40
	jl jmp_10035a37
	mov edi, dword ptr l_unk0x40
jmp_10035a37:
	mov dword ptr l_unk0x34, edi
	mov edi, dword ptr l_unk0x04
	sub edi, dword ptr l_unk0x0c
	cmp edi, dword ptr l_unk0x40
	jg jmp_10035abb
	cmp edi, dword ptr l_unk0x38
	jg jmp_10035a4d
	mov edi, dword ptr l_unk0x38
jmp_10035a4d:
	mov dword ptr l_unk0x30, edi
	mov edx, dword ptr l_unk0x08
	add edx, dword ptr l_unk0x10
	cmp edx, dword ptr l_unk0x3c
	jl jmp_10035abb
	cmp edx, dword ptr l_unk0x44
	jg jmp_10035a84
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov ecx, dword ptr l_unk0x34
	sub ecx, dword ptr l_unk0x30
	inc ecx
	mov eax, dword ptr p_color
	mov edx, ecx
	and edx, 3
	shr ecx, 2
	rep stosd
	mov ecx, edx
	rep stosb
jmp_10035a84:
	mov edi, dword ptr l_unk0x30
	mov edx, dword ptr l_unk0x08
	sub edx, dword ptr l_unk0x10
	cmp edx, dword ptr l_unk0x3c
	jl jmp_10035abb
	cmp edx, dword ptr l_unk0x44
	jg jmp_10035abb
	mov eax, edx
	imul dword ptr l_unk0x4c
	add eax, dword ptr l_unk0x48
	add eax, edi
	mov edi, eax
	mov ecx, dword ptr l_unk0x34
	sub ecx, dword ptr l_unk0x30
	inc ecx
	mov eax, dword ptr p_color
	mov edx, ecx
	and edx, 3
	shr ecx, 2
	rep stosd
	mov ecx, edx
	rep stosb
jmp_10035abb:
	cmp dword ptr l_unk0x2c, 0
	jns jmp_10035ad0
	inc dword ptr l_unk0x0c
	mov eax, dword ptr l_unk0x24
	add eax, dword ptr l_unk0x20
	mov dword ptr l_unk0x24, eax
	add dword ptr l_unk0x2c, eax
jmp_10035ad0:
	dec dword ptr l_unk0x10
	mov eax, dword ptr l_unk0x28
	sub eax, dword ptr l_unk0x18
	mov dword ptr l_unk0x28, eax
	sub eax, dword ptr l_unk0x14
	sub dword ptr l_unk0x2c, eax
	dec ebx
	js jmp_10035aea
	jmp jmp_10035a20
jmp_10035aea:
	pop es
	ret
FillEllipse endp

; The 16.16 cosine of 0 to 90 degrees in tenths of a degree; read backwards, the sine. The
; original keeps it in .text, between FillEllipse and GetCosSin.
	public g_cosTable
g_cosTable_t struct
m_data dd 3 dup (10000h), 0ffffh, 0fffeh, 0fffeh, 0fffch, 0fffbh, 0fffah, 0fff8h, 0fff6h, 0fff4h
	dd 0fff2h, 0ffefh, 0ffech, 0ffeah, 0ffe6h, 0ffe3h, 0ffe0h, 0ffdch, 0ffd8h, 0ffd4h, 0ffd0h
	dd 0ffcbh, 0ffc7h, 0ffc2h, 0ffbdh, 0ffb7h, 0ffb2h, 0ffach, 0ffa6h, 0ffa0h, 0ff9ah, 0ff93h
	dd 0ff8dh, 0ff86h, 0ff7fh, 0ff77h, 0ff70h, 0ff68h, 0ff60h, 0ff58h, 0ff50h, 0ff48h, 0ff3fh
	dd 0ff36h, 0ff2dh, 0ff24h, 0ff1ah, 0ff10h, 0ff07h, 0fefdh, 0fef2h, 0fee8h, 0feddh, 0fed2h
	dd 0fec7h, 0febch, 0feb1h, 0fea5h, 0fe99h, 0fe8dh, 0fe81h, 0fe74h, 0fe68h, 0fe5bh, 0fe4eh
	dd 0fe40h, 0fe33h, 0fe25h, 0fe18h, 0fe09h, 0fdfbh, 0fdedh, 0fddeh, 0fdcfh, 0fdc0h, 0fdb1h
	dd 0fda2h, 0fd92h, 0fd82h, 0fd72h, 0fd62h, 0fd52h, 0fd41h, 0fd30h, 0fd1fh, 0fd0eh, 0fcfdh
	dd 0fcebh, 0fcd9h, 0fcc7h, 0fcb5h, 0fca3h, 0fc90h, 0fc7dh, 0fc6ah, 0fc57h, 0fc44h, 0fc30h
	dd 0fc1ch, 0fc08h, 0fbf4h, 0fbe0h, 0fbcbh, 0fbb7h, 0fba2h, 0fb8dh, 0fb77h, 0fb62h, 0fb4ch
	dd 0fb36h, 0fb20h, 0fb0ah, 0faf3h, 0fadch, 0fac5h, 0faaeh, 0fa97h, 0fa80h, 0fa68h, 0fa50h
	dd 0fa38h, 0fa20h, 0fa07h, 0f9efh, 0f9d6h, 0f9bdh, 0f9a3h, 0f98ah, 0f970h, 0f956h, 0f93ch
	dd 0f922h, 0f908h, 0f8edh, 0f8d2h, 0f8b7h, 0f89ch, 0f881h, 0f865h, 0f84ah, 0f82eh, 0f811h
	dd 0f7f5h, 0f7d9h, 0f7bch, 0f79fh, 0f782h, 0f764h, 0f747h, 0f729h, 0f70bh, 0f6edh, 0f6cfh
	dd 0f6b0h, 0f692h, 0f673h, 0f654h, 0f635h, 0f615h, 0f5f6h, 0f5d6h, 0f5b6h, 0f596h, 0f575h
	dd 0f555h, 0f534h, 0f513h, 0f4f2h, 0f4d0h, 0f4afh, 0f48dh, 0f46bh, 0f449h, 0f427h, 0f404h
	dd 0f3e2h, 0f3bfh, 0f39ch, 0f378h, 0f355h, 0f331h, 0f30eh, 0f2eah, 0f2c5h, 0f2a1h, 0f27ch
	dd 0f258h, 0f233h, 0f20eh, 0f1e8h, 0f1c3h, 0f19dh, 0f177h, 0f151h, 0f12bh, 0f104h, 0f0deh
	dd 0f0b7h, 0f090h, 0f068h, 0f041h, 0f019h, 0eff2h, 0efcah, 0efa2h, 0ef79h, 0ef51h, 0ef28h
	dd 0eeffh, 0eed6h, 0eeadh, 0ee83h, 0ee5ah, 0ee30h, 0ee06h, 0eddch, 0edb1h, 0ed87h, 0ed5ch
	dd 0ed31h, 0ed06h, 0ecdbh, 0ecafh, 0ec83h, 0ec58h, 0ec2bh, 0ebffh, 0ebd3h, 0eba6h, 0eb79h
	dd 0eb4ch, 0eb1fh, 0eaf2h, 0eac4h, 0ea97h, 0ea69h, 0ea3bh, 0ea0dh, 0e9deh, 0e9b0h, 0e981h
	dd 0e952h, 0e923h, 0e8f3h, 0e8c4h, 0e894h, 0e864h, 0e834h, 0e804h, 0e7d3h, 0e7a3h, 0e772h
	dd 0e741h, 0e710h, 0e6deh, 0e6adh, 0e67bh, 0e649h, 0e617h, 0e5e5h, 0e5b3h, 0e580h, 0e54dh
	dd 0e51ah, 0e4e7h, 0e4b4h, 0e481h, 0e44dh, 0e419h, 0e3e5h, 0e3b1h, 0e37ch, 0e348h, 0e313h
	dd 0e2deh, 0e2a9h, 0e274h, 0e23eh, 0e209h, 0e1d3h, 0e19dh, 0e167h, 0e131h, 0e0fah, 0e0c3h
	dd 0e08dh, 0e056h, 0e01eh, 0dfe7h, 0dfb0h, 0df78h, 0df40h, 0df08h, 0ded0h, 0de97h, 0de5fh
	dd 0de26h, 0ddedh, 0ddb4h, 0dd7bh, 0dd41h, 0dd07h, 0dcceh, 0dc94h, 0dc5ah, 0dc1fh, 0dbe5h
	dd 0dbaah, 0db6fh, 0db34h, 0daf9h, 0dabeh, 0da82h, 0da47h, 0da0bh, 0d9cfh, 0d993h, 0d956h
	dd 0d91ah, 0d8ddh, 0d8a0h, 0d863h, 0d826h, 0d7e9h, 0d7abh, 0d76dh, 0d72fh, 0d6f1h, 0d6b3h
	dd 0d675h, 0d636h, 0d5f7h, 0d5b9h, 0d57ah, 0d53ah, 0d4fbh, 0d4bbh, 0d47ch, 0d43ch, 0d3fch
	dd 0d3bch, 0d37bh, 0d33bh, 0d2fah, 0d2b9h, 0d278h, 0d237h, 0d1f5h, 0d1b4h, 0d172h, 0d130h
	dd 0d0eeh, 0d0ach, 0d06ah, 0d027h, 0cfe5h, 0cfa2h, 0cf5fh, 0cf1ch, 0ced8h, 0ce95h, 0ce51h
	dd 0ce0eh, 0cdcah, 0cd85h, 0cd41h, 0ccfdh, 0ccb8h, 0cc73h, 0cc2eh, 0cbe9h, 0cba4h, 0cb5fh
	dd 0cb19h, 0cad3h, 0ca8eh, 0ca48h, 0ca01h, 0c9bbh, 0c975h, 0c92eh, 0c8e7h, 0c8a0h, 0c859h
	dd 0c812h, 0c7cah, 0c783h, 0c73bh, 0c6f3h, 0c6abh, 0c663h, 0c61ah, 0c5d2h, 0c589h, 0c540h
	dd 0c4f7h, 0c4aeh, 0c465h, 0c41bh, 0c3d2h, 0c388h, 0c33eh, 0c2f4h, 0c2aah, 0c260h, 0c215h
	dd 0c1cah, 0c180h, 0c135h, 0c0eah, 0c09eh, 0c053h, 0c007h, 0bfbch, 0bf70h, 0bf24h, 0bed8h
	dd 0be8bh, 0be3fh, 0bdf2h, 0bda5h, 0bd58h, 0bd0bh, 0bcbeh, 0bc71h, 0bc23h, 0bbd6h, 0bb88h
	dd 0bb3ah, 0baech, 0ba9eh, 0ba4fh, 0ba01h, 0b9b2h, 0b963h, 0b914h, 0b8c5h, 0b876h, 0b827h
	dd 0b7d7h, 0b787h, 0b738h, 0b6e8h, 0b698h, 0b647h, 0b5f7h, 0b5a6h, 0b556h, 0b505h, 0b4b4h
	dd 0b463h, 0b412h, 0b3c0h, 0b36fh, 0b31dh, 0b2cbh, 0b279h, 0b227h, 0b1d5h, 0b183h, 0b130h
	dd 0b0deh, 0b08bh, 0b038h, 0afe5h, 0af92h, 0af3eh, 0aeebh, 0ae97h, 0ae44h, 0adf0h, 0ad9ch
	dd 0ad48h, 0acf3h, 0ac9fh, 0ac4bh, 0abf6h, 0aba1h, 0ab4ch, 0aaf7h, 0aaa2h, 0aa4dh, 0a9f7h
	dd 0a9a1h, 0a94ch, 0a8f6h, 0a8a0h, 0a84ah, 0a7f3h, 0a79dh, 0a747h, 0a6f0h, 0a699h, 0a642h
	dd 0a5ebh, 0a594h, 0a53dh, 0a4e5h, 0a48eh, 0a436h, 0a3deh, 0a386h, 0a32eh, 0a2d6h, 0a27eh
	dd 0a225h, 0a1cdh, 0a174h, 0a11bh, 0a0c2h, 0a069h, 0a010h, 9fb7h, 9f5dh, 9f04h, 9eaah
	dd 9e50h, 9df6h, 9d9ch, 9d42h, 9ce7h, 9c8dh, 9c32h, 9bd8h, 9b7dh, 9b22h, 9ac7h, 9a6ch
	dd 9a11h, 99b5h, 995ah, 98feh, 98a2h, 9846h, 97eah, 978eh, 9732h, 96d6h, 9679h, 961ch
	dd 95c0h, 9563h, 9506h, 94a9h, 944ch, 93eeh, 9391h, 9334h, 92d6h, 9278h, 921ah, 91bch
	dd 915eh, 9100h, 90a2h, 9043h, 8fe5h, 8f86h, 8f27h, 8ec8h, 8e69h, 8e0ah, 8dabh, 8d4ch
	dd 8cech, 8c8dh, 8c2dh, 8bcdh, 8b6dh, 8b0dh, 8aadh, 8a4dh, 89edh, 898ch, 892ch, 88cbh
	dd 886bh, 880ah, 87a9h, 8748h, 86e7h, 8685h, 8624h, 85c2h, 8561h, 84ffh, 849dh, 843ch
	dd 83dah, 8377h, 8315h, 82b3h, 8251h, 81eeh, 818bh, 8129h, 80c6h, 8063h, 8000h, 7f9dh
	dd 7f3ah, 7ed6h, 7e73h, 7e0fh, 7dach, 7d48h, 7ce4h, 7c80h, 7c1ch, 7bb8h, 7b54h, 7af0h
	dd 7a8ch, 7a27h, 79c3h, 795eh, 78f9h, 7894h, 782fh, 77cah, 7765h, 7700h, 769bh, 7635h
	dd 75d0h, 756ah, 7504h, 749fh, 7439h, 73d3h, 736dh, 7307h, 72a0h, 723ah, 71d4h, 716dh
	dd 7107h, 70a0h, 7039h, 6fd2h, 6f6bh, 6f04h, 6e9dh, 6e36h, 6dcfh, 6d67h, 6d00h, 6c98h
	dd 6c31h, 6bc9h, 6b61h, 6af9h, 6a91h, 6a29h, 69c1h, 6959h, 68f1h, 6888h, 6820h, 67b7h
	dd 674fh, 66e6h, 667dh, 6614h, 65abh, 6542h, 64d9h, 6470h, 6407h, 639eh, 6334h, 62cbh
	dd 6261h, 61f8h, 618eh, 6124h, 60bah, 6050h, 5fe6h, 5f7ch, 5f12h, 5ea8h, 5e3dh, 5dd3h
	dd 5d69h, 5cfeh, 5c93h, 5c29h, 5bbeh, 5b53h, 5ae8h, 5a7dh, 5a12h, 59a7h, 593ch, 58d1h
	dd 5865h, 57fah, 578fh, 5723h, 56b8h, 564ch, 55e0h, 5574h, 5509h, 549dh, 5431h, 53c5h
	dd 5358h, 52ech, 5280h, 5214h, 51a7h, 513bh, 50ceh, 5062h, 4ff5h, 4f88h, 4f1ch, 4eafh
	dd 4e42h, 4dd5h, 4d68h, 4cfbh, 4c8eh, 4c21h, 4bb4h, 4b46h, 4ad9h, 4a6bh, 49feh, 4990h
	dd 4923h, 48b5h, 4848h, 47dah, 476ch, 46feh, 4690h, 4622h, 45b4h, 4546h, 44d8h, 446ah
	dd 43fbh, 438dh, 431fh, 42b0h, 4242h, 41d3h, 4165h, 40f6h, 4088h, 4019h, 3faah, 3f3bh
	dd 3ecch, 3e5eh, 3defh, 3d80h, 3d11h, 3ca1h, 3c32h, 3bc3h, 3b54h, 3ae5h, 3a75h, 3a06h
	dd 3996h, 3927h, 38b7h, 3848h, 37d8h, 3769h, 36f9h, 3689h, 3619h, 35aah, 353ah, 34cah
	dd 345ah, 33eah, 337ah, 330ah, 329ah, 322ah, 31b9h, 3149h, 30d9h, 3069h, 2ff8h, 2f88h
	dd 2f17h, 2ea7h, 2e37h, 2dc6h, 2d55h, 2ce5h, 2c74h, 2c04h, 2b93h, 2b22h, 2ab1h, 2a41h
	dd 29d0h, 295fh, 28eeh, 287dh, 280ch, 279bh, 272ah, 26b9h, 2648h, 25d7h, 2566h, 24f5h
	dd 2483h, 2412h, 23a1h, 2330h, 22beh, 224dh, 21dch, 216ah, 20f9h, 2087h, 2016h, 1fa4h
	dd 1f33h, 1ec1h, 1e50h, 1ddeh, 1d6dh, 1cfbh, 1c89h, 1c18h, 1ba6h, 1b34h, 1ac2h, 1a51h
	dd 19dfh, 196dh, 18fbh, 1889h, 1817h, 17a6h, 1734h, 16c2h, 1650h, 15deh, 156ch, 14fah
	dd 1488h, 1416h, 13a4h, 1332h, 12c0h, 124eh, 11dch, 1169h, 10f7h, 1085h, 1013h, 0fa1h
	dd 0f2fh, 0ebdh, 0e4ah, 0dd8h, 0d66h, 0cf4h, 0c81h, 0c0fh, 0b9dh, 0b2bh, 0ab8h, 0a46h
	dd 9d4h, 961h, 8efh, 87dh, 80bh, 798h, 726h, 6b4h, 641h, 5cfh, 55ch, 4eah, 478h, 405h
	dd 393h, 321h, 2aeh, 23ch, 1cah, 157h, 0e5h, 72h, 0
g_cosTable_t ends
g_cosTable g_cosTable_t <>

; Looks up the 16.16 cosine and sine of p_angle (in tenths of a degree) in g_cosTable.
GetCosSin proc uses ebx esi edi, p_angle:dword, p_cos:dword, p_sin:dword
	push es
	mov ebx, dword ptr p_angle
	and ebx, ebx
	jns jmp_10036922
jmp_10036912:
	add ebx, 0e10h
	js jmp_10036912
	jmp jmp_10036922
jmp_1003691c:
	sub ebx, 0e10h
jmp_10036922:
	cmp ebx, 0e10h
	jg jmp_1003691c
	cmp ebx, 708h
	ja jmp_1003696a
	cmp ebx, 384h
	ja jmp_1003694d
	shl ebx, 2
	mov eax, dword ptr [g_cosTable+ebx]
	neg ebx
	mov edx, dword ptr [g_cosTable+0e10h+ebx]
	jmp jmp_100369ac
jmp_1003694d:
	neg ebx
	add ebx, 708h
	shl ebx, 2
	mov eax, dword ptr [g_cosTable+ebx]
	neg eax
	neg ebx
	mov edx, dword ptr [g_cosTable+0e10h+ebx]
	jmp jmp_100369ac
jmp_1003696a:
	neg ebx
	add ebx, 0e10h
	cmp ebx, 384h
	ja jmp_1003698f
	shl ebx, 2
	mov eax, dword ptr [g_cosTable+ebx]
	neg ebx
	mov edx, dword ptr [g_cosTable+0e10h+ebx]
	neg edx
	jmp jmp_100369ac
jmp_1003698f:
	neg ebx
	add ebx, 708h
	shl ebx, 2
	mov eax, dword ptr [g_cosTable+ebx]
	neg eax
	neg ebx
	mov edx, dword ptr [g_cosTable+0e10h+ebx]
	neg edx
jmp_100369ac:
	mov ebx, dword ptr p_cos
	mov dword ptr [ebx], eax
	mov ebx, dword ptr p_sin
	mov dword ptr [ebx], edx
	pop es
	ret
GetCosSin endp

; Multiplies two 16.16 fixed-point values, rounding, into *p_result.
FixedMul16 proc uses ebx esi edi, p_a:dword, p_b:dword, p_result:dword
	push es
	mov eax, dword ptr p_a
	imul dword ptr p_b
	add eax, 8000h
	adc edx, 0
	mov ax, dx
	ror eax, 10h
	mov edi, dword ptr p_result
	mov dword ptr [edi], eax
	pop es
	ret
FixedMul16 endp

; Rotates the point p_point about p_origin by p_angle (in tenths of a degree) and scales it
; by the 16.16 factors p_scaleX and p_scaleY, storing the result in p_result.
RotateScalePoint proc uses ebx esi edi, p_point:dword, p_result:dword, p_origin:dword, p_angle:dword, p_scaleX:dword, p_scaleY:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword
	push es
	cld
	push ds
	pop es
	lea eax, l_unk0x08
	push eax
	lea eax, l_unk0x04
	push eax
	push dword ptr p_angle
	call GetCosSin
	add esp, 0ch
	mov esi, dword ptr p_point
	mov edi, dword ptr p_origin
	mov eax, dword ptr [esi]
	sub eax, dword ptr [edi]
	shl eax, 10h
	imul dword ptr p_scaleX
	add eax, 8000h
	adc edx, 0
	mov ebx, edx
	mov eax, ebx
	imul dword ptr l_unk0x04
	add eax, 8000h
	adc edx, 0
	mov ax, dx
	ror eax, 10h
	mov dword ptr l_unk0x0c, eax
	mov eax, ebx
	imul dword ptr l_unk0x08
	add eax, 8000h
	adc edx, 0
	mov ax, dx
	ror eax, 10h
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr [esi+4]
	sub eax, dword ptr [edi+4]
	shl eax, 10h
	imul dword ptr p_scaleY
	add eax, 8000h
	adc edx, 0
	mov ecx, edx
	mov eax, ecx
	imul dword ptr l_unk0x04
	add eax, 8000h
	adc edx, 0
	mov ax, dx
	ror eax, 10h
	mov dword ptr l_unk0x18, eax
	mov eax, ecx
	imul dword ptr l_unk0x08
	add eax, 8000h
	adc edx, 0
	mov ax, dx
	ror eax, 10h
	mov dword ptr l_unk0x10, eax
	mov esi, dword ptr p_result
	mov edx, dword ptr l_unk0x0c
	sub edx, dword ptr l_unk0x10
	add edx, dword ptr [edi]
	mov dword ptr [esi], edx
	mov edx, dword ptr l_unk0x18
	add edx, dword ptr l_unk0x14
	add edx, dword ptr [edi+4]
	mov dword ptr [esi+4], edx
	pop es
	ret
RotateScalePoint endp

; Returns the dword at +0x8 of the font data (Font keeps it as the line height).
FontGetHeight proc uses ebx esi edi, p_data:dword
	push es
	mov esi, dword ptr p_data
	mov eax, dword ptr [esi+8]
	pop es
	ret
FontGetHeight endp

; Looks up p_char in the font data's offset table at +0x10 and returns the dword at that
; offset (Font sums it as the character's width).
FontGetCharWidth proc uses ebx esi edi, p_data:dword, p_char:dword
	push es
	mov eax, dword ptr p_char
	shl eax, 2
	add eax, dword ptr p_data
	add eax, 10h
	mov esi, dword ptr [eax]
	add esi, dword ptr p_data
	mov eax, dword ptr [esi]
	pop es
	ret
FontGetCharWidth endp

; Draws one character of the font data into the view, clipped; a palette maps its pixels and
; skips 0xff. Returns the character's advance, -1 for an empty view or -2 when fully clipped.
BlitChar proc uses ebx esi edi, p_view:dword, p_left:dword, p_top:dword, p_font:dword, p_char:dword, p_palette:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x28, eax
	jle jmp_10036b5b
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10036b5b
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x2c, eax
	cmp eax, 0
	jg jmp_10036b0f
	mov eax, 0
jmp_10036b0f:
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x30, eax
	cmp eax, 0
	jg jmp_10036b22
	mov eax, 0
jmp_10036b22:
	mov dword ptr l_unk0x18, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x28
	dec edx
	cmp eax, edx
	jl jmp_10036b32
	mov eax, edx
jmp_10036b32:
	mov dword ptr l_unk0x1c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10036b41
	mov eax, edx
jmp_10036b41:
	mov dword ptr l_unk0x20, eax
	mov eax, dword ptr l_unk0x1c
	cmp eax, dword ptr l_unk0x14
	jl jmp_10036b66
	mov eax, dword ptr l_unk0x20
	cmp eax, dword ptr l_unk0x18
	jl jmp_10036b66
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x24, eax
	jmp jmp_10036b71
jmp_10036b5b:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10036b66:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10036b71:
	mov eax, dword ptr l_unk0x2c
	add dword ptr p_left, eax
	mov eax, dword ptr l_unk0x30
	add dword ptr p_top, eax
	mov esi, dword ptr p_font
	mov edx, dword ptr [esi+8]
	mov eax, dword ptr p_char
	shl eax, 2
	add eax, dword ptr p_font
	add eax, 10h
	mov esi, dword ptr [eax]
	add esi, dword ptr p_font
	mov dword ptr l_unk0x08, 0
	mov ecx, dword ptr [esi]
	mov dword ptr l_unk0x04, ecx
	cmp ecx, 0
	je jmp_10036c5b
	add esi, 4
	mov edi, dword ptr p_view
	mov eax, dword ptr l_unk0x1c
	inc eax
	sub eax, ecx
	sub eax, dword ptr p_left
	jns jmp_10036bc2
	add ecx, eax
	jle jmp_10036c5b
jmp_10036bc2:
	mov eax, dword ptr p_left
	sub eax, dword ptr l_unk0x14
	jns jmp_10036bd7
	add ecx, eax
	jle jmp_10036c5b
	sub esi, eax
	sub dword ptr p_left, eax
jmp_10036bd7:
	mov eax, dword ptr l_unk0x20
	inc eax
	sub eax, edx
	sub eax, dword ptr p_top
	jns jmp_10036be6
	add edx, eax
	jle jmp_10036c5b
jmp_10036be6:
	mov eax, dword ptr p_top
	sub eax, dword ptr l_unk0x18
	jns jmp_10036bfb
	add edx, eax
	jle jmp_10036c5b
	sub dword ptr p_top, eax
	imul eax, dword ptr l_unk0x04
	sub esi, eax
jmp_10036bfb:
	mov dword ptr l_unk0x10, edx
	mov eax, dword ptr p_top
	imul dword ptr l_unk0x28
	add eax, dword ptr l_unk0x24
	add eax, dword ptr p_left
	mov edi, eax
	mov dword ptr l_unk0x08, ecx
	sub dword ptr l_unk0x04, ecx
	mov eax, dword ptr l_unk0x28
	sub eax, ecx
	mov dword ptr l_unk0x0c, eax
	mov edx, dword ptr l_unk0x10
	cmp dword ptr p_palette, 0
	jne jmp_10036c3d
jmp_10036c23:
	rep movsb
	mov ecx, dword ptr l_unk0x08
	add esi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	dec edx
	jne jmp_10036c23
	mov eax, dword ptr l_unk0x04
	add eax, dword ptr l_unk0x08
	pop es
	ret
jmp_10036c3d:
	jecxz jmp_10036c5b
	mov ebx, dword ptr p_palette
jmp_10036c42:
	mov al, byte ptr [esi]
	xlatb
	cmp al, 0ffh
	je jmp_10036c4b
	mov byte ptr [edi], al
jmp_10036c4b:
	inc esi
	inc edi
	loop jmp_10036c42
	mov ecx, dword ptr l_unk0x08
	add esi, dword ptr l_unk0x04
	add edi, dword ptr l_unk0x0c
	dec edx
	jne jmp_10036c42
jmp_10036c5b:
	mov eax, dword ptr l_unk0x04
	add eax, dword ptr l_unk0x08
	pop es
	ret
BlitChar endp

BlitString proc uses ebx esi edi, p_view:dword, p_left:dword, p_top:dword, p_font:dword, p_text:dword, p_palette:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_text
	mov edi, dword ptr p_left
jmp_10036c77:
	movzx eax, byte ptr [esi]
	push dword ptr p_palette
	push eax
	push dword ptr p_font
	push dword ptr p_top
	push edi
	push dword ptr p_view
	call BlitChar
	add esp, 18h
	add edi, eax
	inc esi
	cmp byte ptr [esi], 0
	jne jmp_10036c77
	pop es
	ret
BlitString endp

; Copies p_count pixels into row p_index of the view, clipped.
WriteViewRow proc uses ebx esi edi, p_view:dword, p_index:dword, p_data:dword, p_count:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x1c, eax
	jle jmp_10036d1d
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10036d1d
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x20, eax
	cmp eax, 0
	jg jmp_10036cd1
	mov eax, 0
jmp_10036cd1:
	mov dword ptr l_unk0x08, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x24, eax
	cmp eax, 0
	jg jmp_10036ce4
	mov eax, 0
jmp_10036ce4:
	mov dword ptr l_unk0x0c, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x1c
	dec edx
	cmp eax, edx
	jl jmp_10036cf4
	mov eax, edx
jmp_10036cf4:
	mov dword ptr l_unk0x10, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10036d03
	mov eax, edx
jmp_10036d03:
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr l_unk0x10
	cmp eax, dword ptr l_unk0x08
	jl jmp_10036d28
	mov eax, dword ptr l_unk0x14
	cmp eax, dword ptr l_unk0x0c
	jl jmp_10036d28
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x18, eax
	jmp jmp_10036d33
jmp_10036d1d:
	mov eax, 0ffffffffh
	pop es
	ret
jmp_10036d28:
	mov eax, 0fffffffeh
	pop es
	ret
jmp_10036d33:
	mov dword ptr l_unk0x04, 0
	mov eax, dword ptr l_unk0x20
	add dword ptr l_unk0x04, eax
	mov eax, dword ptr l_unk0x24
	add dword ptr p_index, eax
	mov esi, dword ptr p_data
	mov edi, dword ptr p_view
	mov ecx, dword ptr p_count
	mov eax, dword ptr l_unk0x10
	sub eax, dword ptr l_unk0x04
	inc eax
	sub eax, ecx
	jns jmp_10036d5e
	add ecx, eax
	jle jmp_10036d9b
jmp_10036d5e:
	mov eax, dword ptr l_unk0x04
	sub eax, dword ptr l_unk0x08
	jns jmp_10036d6f
	add ecx, eax
	jle jmp_10036d9b
	sub esi, eax
	sub dword ptr l_unk0x04, eax
jmp_10036d6f:
	mov eax, dword ptr l_unk0x14
	sub eax, dword ptr p_index
	js jmp_10036d9b
	mov eax, dword ptr p_index
	sub eax, dword ptr l_unk0x0c
	js jmp_10036d9b
	mov eax, dword ptr p_index
	imul dword ptr l_unk0x1c
	add eax, dword ptr l_unk0x18
	add eax, dword ptr l_unk0x04
	mov edi, eax
	mov edx, ecx
	and ecx, 3
	rep movsb
	mov ecx, edx
	shr ecx, 2
	rep movsd
jmp_10036d9b:
	pop es
	ret
WriteViewRow endp

; The IFF chunk tags that BlitIff, ReadIffPalette and GetIffSize look up. The original
; keeps them in .text, right after WriteViewRow's ret.
	public g_iffBmhdTag
g_iffBmhdTag_t struct
m_data db "BMHD"
g_iffBmhdTag_t ends
g_iffBmhdTag g_iffBmhdTag_t <>

	public g_iffCmapTag
g_iffCmapTag_t struct
m_data db "CMAP"
g_iffCmapTag_t ends
g_iffCmapTag g_iffCmapTag_t <>

	public g_iffBodyTag
g_iffBodyTag_t struct
m_data db "BODY"
g_iffBodyTag_t ends
g_iffBodyTag g_iffBodyTag_t <>

; Scan big-endian IFF chunks for a four-byte tag and return its data pointer.
FindIffChunk proc uses ebx esi edi, p_tag:dword, p_chunks:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_chunks
	add esi, 0ch
jmp_10036dbd:
	cmp byte ptr [esi], 0
	jne jmp_10036dc5
	inc esi
	jmp jmp_10036dbd
jmp_10036dc5:
	mov ecx, 2
	mov edi, dword ptr p_tag
	mov eax, esi
	repe cmpsw
	je jmp_10036de6
	mov esi, eax
	add esi, 6
	lodsw
	xchg al, ah
	and eax, 0ffffh
	add esi, eax
	jmp jmp_10036dbd
jmp_10036de6:
	add eax, 8
	pop es
	ret
FindIffChunk endp

; Decodes the BODY chunk of an IFF ILBM or PBM image (p_data) into the view, one row at a
; time through WriteViewRow. Returns the BMHD compression byte.
BlitIff proc uses ebx esi edi, p_view:dword, p_data:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword
	push es
	cld
	push ds
	pop es
	mov edi, dword ptr p_view
	mov eax, dword ptr [edi+0ch]
	sub eax, dword ptr [edi+4]
	inc eax
	mov dword ptr l_unk0x24, eax
	mov eax, dword ptr [edi+10h]
	sub eax, dword ptr [edi+8]
	inc eax
	mov dword ptr l_unk0x20, eax
	mov edi, dword ptr [edi]
	mov edi, dword ptr [edi]
	mov dword ptr l_unk0x3c, edi
	mov dword ptr l_unk0x28, 0
	mov edi, dword ptr p_data
	mov eax, dword ptr [edi+8]
	xor eax, 4d424c49h
	mov dword ptr l_unk0x04, eax
	push dword ptr p_data
	push offset g_iffBmhdTag
	call FindIffChunk
	add esp, 8
	mov esi, eax
	lodsw
	xchg al, ah
	and eax, 0ffffh
	mov dword ptr l_unk0x0c, eax
	lodsw
	xchg al, ah
	and eax, 0ffffh
	cmp eax, dword ptr l_unk0x20
	jl jmp_10036e5e
	mov eax, dword ptr l_unk0x20
jmp_10036e5e:
	mov dword ptr l_unk0x08, eax
	add esi, 5
	lodsb
	cmp al, 1
	je jmp_10036fad
	mov eax, 0
	lodsb
	mov dword ptr l_unk0x2c, eax
	add esi, 2
	mov eax, 0
	lodsb
	mov dword ptr l_unk0x38, eax
	mov eax, dword ptr l_unk0x0c
	mov ebx, eax
	shr eax, 3
	and ebx, 7
	cmp ebx, 1
	sbb eax, -1
	mov ebx, eax
	and eax, 1
	add ebx, eax
	mov dword ptr l_unk0x30, ebx
	mov eax, dword ptr l_unk0x0c
	and eax, 1
	add eax, dword ptr l_unk0x0c
	mov dword ptr l_unk0x34, eax
	mov eax, dword ptr l_unk0x0c
	cmp eax, dword ptr l_unk0x24
	jl jmp_10036eb4
	mov eax, dword ptr l_unk0x24
jmp_10036eb4:
	mov dword ptr l_unk0x0c, eax
	push dword ptr p_data
	push offset g_iffBodyTag
	call FindIffChunk
	add esp, 8
	mov dword ptr l_unk0x10, eax
jmp_10036eca:
	mov esi, dword ptr l_unk0x10
	cmp dword ptr l_unk0x2c, 1
	jne jmp_10036f25
	mov edi, offset g_unk0x10069a45
	mov edx, dword ptr l_unk0x34
	add edx, edi
jmp_10036edd:
	cmp edi, edx
	jae jmp_10036f1b
	lodsb
	movzx ecx, al
	cmp ecx, 80h
	je jmp_10036edd
	ja jmp_10036efe
	inc ecx
	push ecx
	and ecx, 3
	rep movsb
	pop ecx
	shr ecx, 2
	rep movsd
	jmp jmp_10036edd
jmp_10036efe:
	lodsb
	mov ah, al
	mov ebx, eax
	shl eax, 10h
	mov ax, bx
	neg cl
	inc cl
	push ecx
	and ecx, 3
	rep stosb
	pop ecx
	shr ecx, 2
	rep stosd
	jmp jmp_10036edd
jmp_10036f1b:
	mov dword ptr l_unk0x10, esi
	mov esi, offset g_unk0x10069a45
	jmp jmp_10036f2d
jmp_10036f25:
	mov eax, esi
	add eax, dword ptr l_unk0x34
	mov dword ptr l_unk0x10, eax
jmp_10036f2d:
	cmp dword ptr l_unk0x04, 0
	jne jmp_10036f8f
	mov edi, offset g_scanline
	mov eax, dword ptr l_unk0x30
	mov dword ptr l_unk0x18, eax
	mov dword ptr l_unk0x1c, eax
	mov eax, dword ptr l_unk0x0c
	mov dword ptr l_unk0x14, eax
jmp_10036f47:
	mov edx, 80h
jmp_10036f4c:
	mov ebx, 0
	mov eax, 100h
jmp_10036f56:
	movzx ecx, byte ptr [ebx+esi]
	and ecx, edx
	je jmp_10036f60
	or al, ah
jmp_10036f60:
	add ebx, dword ptr l_unk0x1c
	shl ah, 1
	jne jmp_10036f56
	stosb
	dec dword ptr l_unk0x14
	je jmp_10036f77
	shr dl, 1
	jne jmp_10036f4c
	inc esi
	dec dword ptr l_unk0x18
	jne jmp_10036f47
jmp_10036f77:
	push dword ptr l_unk0x0c
	push offset g_scanline
	push dword ptr l_unk0x28
	push dword ptr p_view
	call WriteViewRow
	add esp, 10h
	jmp jmp_10036fa1
jmp_10036f8f:
	push dword ptr l_unk0x0c
	push esi
	push dword ptr l_unk0x28
	push dword ptr p_view
	call WriteViewRow
	add esp, 10h
jmp_10036fa1:
	inc dword ptr l_unk0x28
	dec dword ptr l_unk0x08
	jne jmp_10036eca
jmp_10036fad:
	mov eax, dword ptr l_unk0x38
	pop es
	ret
BlitIff endp

; Copies the IFF image's CMAP chunk into p_palette as 6-bit components.
ReadIffPalette proc uses ebx esi edi, p_data:dword, p_palette:dword
	push es
	cld
	push ds
	pop es
	push dword ptr p_data
	push offset g_iffCmapTag
	call FindIffChunk
	add esp, 8
	mov esi, eax
	mov edi, dword ptr p_palette
	mov ecx, 300h
jmp_10036fda:
	lodsb
	shr al, 2
	stosb
	loop jmp_10036fda
	pop es
	ret
ReadIffPalette endp

; Returns the IFF image's BMHD width and height, packed as width << 16 | height.
GetIffSize proc uses ebx esi edi, p_data:dword
	push es
	cld
	push ds
	pop es
	push dword ptr p_data
	push offset g_iffBmhdTag
	call FindIffChunk
	add esp, 8
	mov esi, eax
	lodsw
	xchg al, ah
	shl eax, 10h
	lodsw
	xchg al, ah
	pop es
	ret
GetIffSize endp

BlitPicture proc uses ebx esi edi, p_view:dword, p_data:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword
	push es
	cld
	push ds
	pop es
	mov dword ptr l_unk0x0c, 0
	mov edi, dword ptr p_view
	mov edi, dword ptr [edi]
	mov edi, dword ptr [edi]
	mov esi, dword ptr p_data
	movzx ebx, word ptr [esi+0ah]
	sub bx, word ptr [esi+6]
	mov dword ptr l_unk0x08, ebx
	mov ebx, 0
	movzx eax, word ptr [esi+42h]
	mov dword ptr l_unk0x04, eax
	add esi, 80h
jmp_1003704f:
	mov edi, offset g_scanline
	mov edx, edi
	add edx, dword ptr l_unk0x04
jmp_10037059:
	lodsb
	mov ah, al
	and ah, 0c0h
	xor ah, 0c0h
	jne jmp_1003706e
	and eax, 3fh
	mov ecx, eax
	lodsb
	rep stosb
	jmp jmp_1003706f
jmp_1003706e:
	stosb
jmp_1003706f:
	cmp edi, edx
	jl jmp_10037059
	push dword ptr l_unk0x04
	push offset g_scanline
	push ebx
	push dword ptr p_view
	call WriteViewRow
	add esp, 10h
	inc ebx
	cmp ebx, dword ptr l_unk0x08
	jle jmp_1003704f
	mov eax, dword ptr l_unk0x0c
	pop es
	ret
BlitPicture endp

ReadPicturePalette proc uses ebx esi edi, p_data:dword, p_size:dword, p_palette:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_data
	add esi, dword ptr p_size
	sub esi, 300h
	mov edi, dword ptr p_palette
	mov ecx, 300h
jmp_100370b4:
	lodsb
	shr al, 2
	stosb
	loop jmp_100370b4
	pop es
	ret
ReadPicturePalette endp

GetPictureSize proc uses ebx esi edi, p_data:dword
	push es
	mov esi, dword ptr p_data
	mov ax, word ptr [esi+8]
	sub ax, word ptr [esi+4]
	inc ax
	shl eax, 10h
	mov ax, word ptr [esi+0ah]
	sub ax, word ptr [esi+6]
	inc ax
	pop es
	ret
GetPictureSize endp

; Internal assembly helper: initializes the code tables using ECX and EDI.
GifInitCodes proc
	mov ebx, 0
	mov eax, ecx
	add eax, 2
	mov dword ptr [edi], eax
	mov eax, ecx
	shl eax, 1
	mov dword ptr [edi+4], eax
jmp_100370fb:
	cmp ebx, ecx
	jge jmp_1003711a
	mov byte ptr [edi+ebx+102eh], bl
	mov byte ptr [edi+ebx+202eh], bl
	mov word ptr [edi+ebx*2+302eh], 0ffffh
	inc ebx
	jmp jmp_100370fb
jmp_1003711a:
	cmp ebx, 1000h
	jge jmp_1003712f
	mov word ptr [edi+ebx*2+302eh], 0fffeh
	inc ebx
	jmp jmp_1003711a
jmp_1003712f:
	ret
GifInitCodes endp

; Internal assembly helper: reads a byte from ESI, tracking the run in EDI.
GifReadByte proc
	cmp dword ptr [edi+10h], 0
	jne jmp_1003713f
	lodsb
	and eax, 0ffh
	mov dword ptr [edi+10h], eax
jmp_1003713f:
	lodsb
	and eax, 0ffh
	dec dword ptr [edi+10h]
	ret
GifReadByte endp

; Internal assembly helper: returns the next EDX bits of the LZW code stream in EDI.
GifReadCode proc
	cmp dword ptr [edi+18h], 0
	jne jmp_1003715e
	call GifReadByte
	mov dword ptr [edi+14h], eax
	mov dword ptr [edi+18h], 8
jmp_1003715e:
	mov eax, edx
	cmp dword ptr [edi+18h], eax
	jge jmp_10037176
	call GifReadByte
	mov ecx, dword ptr [edi+18h]
	shl eax, cl
	or dword ptr [edi+14h], eax
	add dword ptr [edi+18h], 8
jmp_10037176:
	mov ebx, edx
	movzx eax, byte ptr [g_gifCodeMasks+ebx]
	mov ebx, dword ptr [edi+14h]
	and ebx, eax
	push ebx
	sub dword ptr [edi+18h], edx
	mov ecx, edx
	shr dword ptr [edi+14h], cl
	pop eax
	ret
GifReadCode endp

; Internal assembly helper: installs a code and grows the code table when full.
GifAddCode proc
	push ebx
	mov ebx, dword ptr [edi]
	mov word ptr [edi+ebx*2+302eh], cx
	pop ebx
	push ebx
	mov al, byte ptr [edi+ebx+102eh]
	mov ebx, dword ptr [edi]
	mov byte ptr [edi+ebx+202eh], al
	mov ebx, ecx
	mov al, byte ptr [edi+ebx+102eh]
	mov ebx, dword ptr [edi]
	mov byte ptr [edi+ebx+102eh], al
	pop ebx
	inc dword ptr [edi]
	mov eax, dword ptr [edi]
	cmp eax, dword ptr [edi+4]
	jne jmp_100371d4
	cmp dword ptr [edi+1ch], 0ch
	jge jmp_100371d4
	inc dword ptr [edi+1ch]
	shl dword ptr [edi+4], 1
jmp_100371d4:
	ret
GifAddCode endp

; Internal assembly helper: appends the pixel in AL to the row buffer and, when the row is
; full, draws it and moves to the next row (in GIF interlace order when enabled).
GifPutPixel proc
	mov ebx, dword ptr [edi+8]
	mov byte ptr [g_scanline+ebx], al
	inc dword ptr [edi+8]
	dec dword ptr [edi+20h]
	cmp dword ptr [edi+20h], 0
	jne jmp_10037251
	push dword ptr [edi+24h]
	push offset g_scanline
	push dword ptr [edi+0ch]
	push dword ptr [g_gifView]
	call WriteViewRow
	add esp, 10h
	mov dword ptr [edi+8], 0
	mov eax, dword ptr [edi+24h]
	mov dword ptr [edi+20h], eax
	cmp byte ptr [edi+2ch], 0
	je jmp_1003723f
	movzx ebx, byte ptr [edi+2dh]
	movzx eax, byte ptr [g_gifPassSteps+ebx]
	add dword ptr [edi+0ch], eax
	mov eax, dword ptr [edi+0ch]
	cmp eax, dword ptr [edi+28h]
	jl jmp_1003723d
	inc byte ptr [edi+2dh]
	movzx ebx, byte ptr [edi+2dh]
	movzx eax, byte ptr [g_gifPassStarts+ebx]
	mov dword ptr [edi+0ch], eax
jmp_1003723d:
	jmp jmp_10037251
jmp_1003723f:
	inc dword ptr [edi+0ch]
	mov eax, dword ptr [edi+0ch]
	cmp eax, dword ptr [edi+28h]
	jl jmp_10037251
	mov dword ptr [edi+0ch], 0
jmp_10037251:
	ret
GifPutPixel endp

; Decodes the LZW image data of a GIF into the view, using p_state as the decoder state.
; Returns the background color index.
BlitGif proc uses ebx esi edi, p_view:dword, p_data:dword, p_state:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	push es
	cld
	push ds
	pop es
	mov edi, dword ptr p_view
	mov dword ptr [g_gifView], edi
	mov edi, dword ptr p_state
	mov eax, 0
	mov ecx, 2eh
	mov edx, ecx
	and ecx, 3
	rep stosb
	mov ecx, edx
	shr ecx, 2
	rep stosd
	mov edi, dword ptr p_state
	mov esi, dword ptr p_data
	movzx eax, byte ptr [esi+0bh]
	mov dword ptr l_unk0x18, eax
	mov al, byte ptr [esi+0ah]
	mov cl, al
	and cl, 7
	inc cl
	mov ebx, 1
	shl ebx, cl
	add esi, 0dh
	test al, 80h
	je jmp_100372ad
	imul ebx, ebx, 3
	add esi, ebx
jmp_100372ad:
	movzx eax, word ptr [esi+5]
	mov dword ptr [edi+24h], eax
	movzx eax, word ptr [esi+7]
	mov dword ptr [edi+28h], eax
	movzx eax, byte ptr [esi+9]
	mov byte ptr [edi+2ch], al
	and byte ptr [edi+2ch], 40h
	add esi, 0ah
	test al, 80h
	je jmp_100372e0
	mov cl, al
	and cl, 7
	inc cl
	mov ebx, 1
	shl ebx, cl
	imul ebx, ebx, 3
	add esi, ebx
jmp_100372e0:
	mov dword ptr [edi+10h], 0
	lodsb
	movzx ecx, al
	mov edx, 8
	push ecx
	push edx
	mov eax, 1
	shl eax, cl
	mov dword ptr l_unk0x04, eax
	inc eax
	mov dword ptr l_unk0x08, eax
	inc ecx
	mov dword ptr [edi+1ch], ecx
	mov ecx, dword ptr l_unk0x04
	call GifInitCodes
	mov dword ptr l_unk0x10, 0ffffh
	mov dword ptr l_unk0x14, 0
	mov byte ptr [edi+2dh], 0
	mov eax, dword ptr [edi+24h]
	mov dword ptr [edi+20h], eax
	mov dword ptr [edi+8], 0
	mov dword ptr [edi+0ch], 0
	pop edx
	pop ecx
jmp_10037334:
	push ecx
	push edx
	mov edx, dword ptr [edi+1ch]
	cmp edx, 8
	jg jmp_10037347
	push edx
	call GifReadCode
	pop edx
	jmp jmp_10037364
jmp_10037347:
	push edx
	mov edx, 8
	call GifReadCode
	pop edx
	push eax
	push edx
	sub edx, 8
	call GifReadCode
	pop edx
	shl eax, 8
	pop ebx
	or eax, ebx
jmp_10037364:
	mov dword ptr l_unk0x0c, eax
	pop edx
	pop ecx
	cmp eax, dword ptr l_unk0x04
	jne jmp_1003738c
	push ecx
	push edx
	mov ecx, dword ptr l_unk0x04
	call GifInitCodes
	pop edx
	pop ecx
	mov eax, ecx
	inc eax
	mov dword ptr [edi+1ch], eax
	mov dword ptr l_unk0x10, 0ffffh
	jmp jmp_10037457
jmp_1003738c:
	cmp eax, dword ptr l_unk0x08
	jne jmp_100373b8
jmp_10037391:
	cmp dword ptr [edi+10h], 0
	je jmp_1003739d
	lodsb
	dec dword ptr [edi+10h]
	jmp jmp_10037391
jmp_1003739d:
	lodsb
	and eax, 0ffh
	mov dword ptr [edi+10h], eax
	cmp dword ptr [edi+10h], 0
	jne jmp_10037391
	mov dword ptr l_unk0x14, 0ffffh
	jmp jmp_10037457
jmp_100373b8:
	mov ebx, dword ptr l_unk0x0c
	cmp word ptr [edi+ebx*2+302eh], -2
	je jmp_100373dd
	cmp dword ptr l_unk0x10, 0ffffh
	je jmp_100373db
	push ecx
	push edx
	mov ecx, dword ptr l_unk0x10
	call GifAddCode
	pop edx
	pop ecx
jmp_100373db:
	jmp jmp_100373ec
jmp_100373dd:
	push ecx
	push edx
	mov ebx, dword ptr l_unk0x10
	mov ecx, dword ptr l_unk0x10
	call GifAddCode
	pop edx
	pop ecx
jmp_100373ec:
	push ecx
	push edx
	mov ebx, dword ptr l_unk0x0c
	push esi
	mov ecx, 0
	mov esi, edi
	add esi, 2eh
jmp_100373fc:
	mov al, byte ptr [edi+ebx+202eh]
	mov byte ptr [esi], al
	inc esi
	inc ecx
	movzx ebx, word ptr [edi+ebx*2+302eh]
	cmp ebx, 0ffffh
	jne jmp_100373fc
	cmp edx, 1
	jne jmp_1003743d
jmp_1003741c:
	dec esi
	mov al, byte ptr [esi]
	and eax, 1
	push ecx
	call GifPutPixel
	pop ecx
	mov al, byte ptr [esi]
	and eax, 0ffh
	shr eax, 1
	push ecx
	call GifPutPixel
	pop ecx
	loop jmp_1003741c
	jmp jmp_1003744e
jmp_1003743d:
	dec esi
	mov al, byte ptr [esi]
	and eax, 0ffh
	push ecx
	call GifPutPixel
	pop ecx
	loop jmp_1003743d
jmp_1003744e:
	pop esi
	pop edx
	pop ecx
	mov eax, dword ptr l_unk0x0c
	mov dword ptr l_unk0x10, eax
jmp_10037457:
	cmp dword ptr l_unk0x14, 0
	jne jmp_10037462
	jmp jmp_10037334
jmp_10037462:
	mov eax, dword ptr l_unk0x18
	pop es
	ret
BlitGif endp

; Copies the GIF's global color table, and then its local one, into p_palette as 6-bit
; components.
ReadGifPalette proc uses ebx esi edi, p_data:dword, p_palette:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_data
	mov al, byte ptr [esi+0ah]
	add esi, 0dh
	test al, 80h
	je jmp_1003749f
	mov cl, al
	and cl, 7
	inc cl
	mov ebx, 1
	shl ebx, cl
	imul ebx, ebx, 3
	mov edi, dword ptr p_palette
	mov ecx, ebx
jmp_10037498:
	lodsb
	shr al, 2
	stosb
	loop jmp_10037498
jmp_1003749f:
	mov al, byte ptr [esi+9]
	test al, 80h
	je jmp_100374c6
	mov cl, al
	and cl, 7
	inc cl
	mov ebx, 1
	shl ebx, cl
	imul ebx, ebx, 3
	add esi, 0ah
	mov edi, dword ptr p_palette
	mov ecx, ebx
jmp_100374bf:
	lodsb
	shr al, 2
	stosb
	loop jmp_100374bf
jmp_100374c6:
	pop es
	ret
ReadGifPalette endp

; Returns the width and height of a GIF's first image, packed as width << 16 | height.
GetGifSize proc uses ebx esi edi, p_data:dword
	push es
	mov esi, dword ptr p_data
	mov al, byte ptr [esi+0ah]
	mov cl, al
	and cl, 7
	inc cl
	mov ebx, 1
	shl ebx, cl
	add esi, 0dh
	test al, 80h
	je jmp_100374f3
	imul ebx, ebx, 3
	add esi, ebx
jmp_100374f3:
	mov ax, word ptr [esi+5]
	shl eax, 10h
	mov ax, word ptr [esi+7]
	pop es
	ret
GetGifSize endp

; Returns the size of an SHP animation's frame p_index, (width - 1) << 16 | (height - 1): the
; entry selected from the offset table at data + 8 starts with it.
GetShpFrameSize proc uses ebx esi edi, p_data:dword, p_index:dword
	push es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	mov esi, dword ptr [esi]
	add esi, dword ptr p_data
	mov eax, dword ptr [esi]
	pop es
	ret
GetShpFrameSize endp

; Returns the second dword in an entry selected from the offset table at data + 8. What that
; dword means isn't known, so it keeps its placeholder.
FUN_10037526 proc uses ebx esi edi, p_data:dword, p_index:dword
	push es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	mov esi, dword ptr [esi]
	add esi, dword ptr p_data
	mov eax, dword ptr [esi+4]
	pop es
	ret
FUN_10037526 endp

; Returns the entry's inclusive horizontal and vertical spans packed into a dword.
GetShpFrameExtent proc uses ebx esi edi, p_data:dword, p_index:dword
	push es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	mov esi, dword ptr [esi]
	add esi, dword ptr p_data
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [esi+8]
	inc eax
	mov ebx, dword ptr [esi+14h]
	sub ebx, dword ptr [esi+0ch]
	inc ebx
	shl eax, 10h
	mov ax, bx
	pop es
	ret
GetShpFrameExtent endp

; Returns the entry's two coordinate fields at +8 and +c packed into a dword.
GetShpFrameOrigin proc uses ebx esi edi, p_data:dword, p_index:dword
	push es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	mov esi, dword ptr [esi]
	add esi, dword ptr p_data
	mov eax, dword ptr [esi+8]
	shl eax, 10h
	mov ax, word ptr [esi+0ch]
	pop es
	ret
GetShpFrameOrigin endp

; Applies a list of three-byte palette updates from an entry. The entry's format isn't pinned
; down, so this and the two routines after it keep their placeholders.
FUN_100375a7 proc uses ebx esi edi, p_data:dword, p_index:dword, p_palette:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	add esi, 4
	mov esi, dword ptr [esi]
	cmp esi, 0
	je jmp_100375ec
	add esi, dword ptr p_data
	lodsd
	mov ecx, eax
	mov edi, dword ptr p_palette
jmp_100375d2:
	lodsb
	and eax, 0ffh
	mov ebx, eax
	shl ebx, 1
	add ebx, eax
	lodsb
	mov byte ptr [edi+ebx], al
	inc ebx
	lodsw
	mov word ptr [edi+ebx], ax
	dec ecx
	jne jmp_100375d2
jmp_100375ec:
	pop es
	ret
FUN_100375a7 endp

; Copies the entry's dword array into the supplied destination, if present.
FUN_100375f2 proc uses ebx esi edi, p_data:dword, p_index:dword, p_destination:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	add esi, 4
	mov esi, dword ptr [esi]
	cmp esi, 0
	jne jmp_1003761b
	mov eax, 0
	jmp jmp_10037634
jmp_1003761b:
	add esi, dword ptr p_data
	lodsd
	mov ebx, eax
	mov edi, dword ptr p_destination
	or edi, edi
	je jmp_10037634
	mov ecx, ebx
	mov eax, 0
jmp_1003762f:
	movsd
	loop jmp_1003762f
	mov eax, ebx
jmp_10037634:
	pop es
	ret
FUN_100375f2 endp

; Writes the entry's dword array back from the supplied source, if present.
FUN_1003763a proc uses ebx esi edi, p_data:dword, p_index:dword, p_source:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_data
	add esi, 8
	mov eax, dword ptr p_index
	shl eax, 3
	add esi, eax
	add esi, 4
	mov esi, dword ptr [esi]
	cmp esi, 0
	jne jmp_10037663
	mov eax, 0
	jmp jmp_1003767e
jmp_10037663:
	add esi, dword ptr p_data
	lodsd
	mov ebx, eax
	mov edi, esi
	mov esi, dword ptr p_source
	or esi, esi
	je jmp_1003767e
	mov ecx, ebx
	mov eax, 0
jmp_10037679:
	movsd
	loop jmp_10037679
	mov eax, ebx
jmp_1003767e:
	pop es
	ret
FUN_1003763a endp

; Returns an SHP animation's frame count, the dword at +4 in its header.
GetShpFrameCount proc uses ebx esi edi, p_data:dword
	push es
	mov esi, dword ptr p_data
	mov eax, dword ptr [esi+4]
	pop es
	ret
GetShpFrameCount endp

; Counts the distinct entries of the data's offset table, storing the index of each first
; occurrence in p_indices when given.
CountShpUniqueFrames proc uses ebx esi edi, p_data:dword, p_indices:dword
	local l_unk0x04:dword
	push es
	mov esi, dword ptr p_data
	mov ecx, dword ptr [esi+4]
	dec ecx
	add esi, 8
	mov dword ptr l_unk0x04, esi
	mov ebx, 1
	mov edx, dword ptr p_indices
	cmp edx, 0
	je jmp_100376c4
	mov dword ptr [edx], 0
	add edx, 4
jmp_100376c4:
	cmp ecx, 0
	je jmp_100376f1
jmp_100376c9:
	add esi, 8
	mov eax, dword ptr [esi]
	mov edi, dword ptr l_unk0x04
jmp_100376d1:
	cmp eax, dword ptr [edi]
	je jmp_100376ef
	add edi, 8
	cmp edi, esi
	jl jmp_100376d1
	cmp edx, 0
	je jmp_100376ee
	mov eax, esi
	sub eax, dword ptr l_unk0x04
	shr eax, 3
	mov dword ptr [edx], eax
	add edx, 4
jmp_100376ee:
	inc ebx
jmp_100376ef:
	loop jmp_100376c9
jmp_100376f1:
	mov eax, ebx
	pop es
	ret
CountShpUniqueFrames endp

; Counts the distinct first dwords among the data's 8-byte entries (the count at +4, the
; entries from +0xc). When p_out isn't NULL, stores the index of each entry that starts a new
; value there. Returns the count. The container it reads isn't identified, so it keeps its
; placeholder.
FUN_100376f9 proc uses ebx esi edi, p_data:dword, p_out:dword
	local l_unk0x04:dword
	push es
	mov esi, dword ptr p_data
	mov ecx, dword ptr [esi+4]
	dec ecx
	add esi, 0ch
	mov dword ptr l_unk0x04, esi
	mov ebx, 1
	mov edx, dword ptr p_out
	cmp edx, 0
	je jmp_10037726
	mov dword ptr [edx], 0
	add edx, 4
jmp_10037726:
	cmp ecx, 0
	je jmp_10037753
jmp_1003772b:
	add esi, 8
	mov eax, dword ptr [esi]
	mov edi, dword ptr l_unk0x04
jmp_10037733:
	cmp eax, dword ptr [edi]
	je jmp_10037751
	add edi, 8
	cmp edi, esi
	jl jmp_10037733
	cmp edx, 0
	je jmp_10037750
	mov eax, esi
	sub eax, dword ptr l_unk0x04
	shr eax, 3
	mov dword ptr [edx], eax
	add edx, 4
jmp_10037750:
	inc ebx
jmp_10037751:
	loop jmp_1003772b
jmp_10037753:
	mov eax, ebx
	pop es
	ret
FUN_100376f9 endp

; The tap masks of maximal-length LFSRs, for 2 to 32 bits. The original keeps the table in
; .text, between FUN_100376f9 and DissolveView.
	public g_dissolveTaps
g_dissolveTaps_t struct
m_data dd 3, 6, 0ch, 14h, 30h, 60h, 0b8h, 110h, 240h, 500h, 0ca0h, 1b00h, 3500h, 6000h, 0b400h
	dd 12000h, 20400h, 72000h, 90000h, 140000h, 300000h, 420000h, 0d80000h, 1200000h, 3880000h
	dd 7200000h, 9000000h, 14000000h, 32800000h, 48000000h, 0a3000000h
g_dissolveTaps_t ends
g_dissolveTaps g_dissolveTaps_t <>

; Dissolves p_src into p_dest: copies up to p_count pixels, in the order of a maximal-length
; LFSR over the pixel indices (taps from g_dissolveTaps), starting from p_state (0 starts a
; new dissolve). Returns the LFSR state to continue from.
; Not 100%: the tap lookup indexes from g_dissolveTaps - 8 (the bit count starts at 2). In the
; original that address is inside FUN_100376f9 and has no symbol; here it falls in whichever
; global precedes the table, so reccmp names the two sides differently.
DissolveView proc p_src:dword, p_dest:dword, p_count:dword, p_state:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword, l_unk0x24:dword, l_unk0x28:dword, l_unk0x2c:dword, l_unk0x30:dword
	local l_unk0x34:dword, l_unk0x38:dword, l_unk0x3c:dword, l_unk0x40:dword, l_unk0x44:dword, l_unk0x48:dword
	local l_unk0x4c:dword, l_unk0x50:dword, l_unk0x54:dword, l_unk0x58:dword, l_unk0x5c:dword, l_unk0x60:dword
	local l_unk0x64:dword, l_unk0x68:dword, l_unk0x6c:dword, l_unk0x70:dword
	push es
	push ebx
	push esi
	push edi
	mov esi, dword ptr p_src
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x44, eax
	jle jmp_10037853
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10037853
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x4c, eax
	cmp eax, 0
	jg jmp_10037807
	mov eax, 0
jmp_10037807:
	mov dword ptr l_unk0x34, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x50, eax
	cmp eax, 0
	jg jmp_1003781a
	mov eax, 0
jmp_1003781a:
	mov dword ptr l_unk0x38, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x44
	dec edx
	cmp eax, edx
	jl jmp_1003782a
	mov eax, edx
jmp_1003782a:
	mov dword ptr l_unk0x3c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_10037839
	mov eax, edx
jmp_10037839:
	mov dword ptr l_unk0x40, eax
	mov eax, dword ptr l_unk0x3c
	cmp eax, dword ptr l_unk0x34
	jl jmp_1003785e
	mov eax, dword ptr l_unk0x40
	cmp eax, dword ptr l_unk0x38
	jl jmp_1003785e
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x48, eax
	jmp jmp_10037869
jmp_10037853:
	mov eax, 0ffffffffh
	pop edi
	pop esi
	pop ebx
	pop es
	ret
jmp_1003785e:
	mov eax, 0fffffffeh
	pop edi
	pop esi
	pop ebx
	pop es
	ret
jmp_10037869:
	mov esi, dword ptr p_src
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [esi+4]
	inc eax
	mov dword ptr l_unk0x24, eax
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [esi+8]
	inc eax
	mov dword ptr l_unk0x28, eax
	mov eax, dword ptr l_unk0x50
	imul dword ptr l_unk0x44
	add eax, dword ptr l_unk0x48
	add eax, dword ptr l_unk0x4c
	mov edi, eax
	xor ebx, ebx
	mov ecx, dword ptr l_unk0x28
jmp_10037893:
	mov dword ptr [g_unk0x10068845+ebx*4], edi
	inc ebx
	add edi, dword ptr l_unk0x44
	loop jmp_10037893
	mov esi, dword ptr p_dest
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [ebx+4]
	inc eax
	mov dword ptr l_unk0x68, eax
	jle jmp_10037912
	mov eax, dword ptr [ebx+8]
	inc eax
	mov ecx, eax
	jle jmp_10037912
	mov eax, dword ptr [esi+4]
	mov dword ptr l_unk0x6c, eax
	cmp eax, 0
	jg jmp_100378c6
	mov eax, 0
jmp_100378c6:
	mov dword ptr l_unk0x54, eax
	mov eax, dword ptr [esi+8]
	mov dword ptr l_unk0x70, eax
	cmp eax, 0
	jg jmp_100378d9
	mov eax, 0
jmp_100378d9:
	mov dword ptr l_unk0x58, eax
	mov eax, dword ptr [esi+0ch]
	mov edx, dword ptr l_unk0x68
	dec edx
	cmp eax, edx
	jl jmp_100378e9
	mov eax, edx
jmp_100378e9:
	mov dword ptr l_unk0x5c, eax
	mov eax, dword ptr [esi+10h]
	mov edx, ecx
	dec edx
	cmp eax, edx
	jl jmp_100378f8
	mov eax, edx
jmp_100378f8:
	mov dword ptr l_unk0x60, eax
	mov eax, dword ptr l_unk0x5c
	cmp eax, dword ptr l_unk0x54
	jl jmp_1003791d
	mov eax, dword ptr l_unk0x60
	cmp eax, dword ptr l_unk0x58
	jl jmp_1003791d
	mov eax, dword ptr [ebx]
	mov dword ptr l_unk0x64, eax
	jmp jmp_10037928
jmp_10037912:
	mov eax, 0ffffffffh
	pop edi
	pop esi
	pop ebx
	pop es
	ret
jmp_1003791d:
	mov eax, 0fffffffeh
	pop edi
	pop esi
	pop ebx
	pop es
	ret
jmp_10037928:
	mov esi, dword ptr p_dest
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [esi+4]
	inc eax
	mov dword ptr l_unk0x2c, eax
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [esi+8]
	inc eax
	mov dword ptr l_unk0x30, eax
	mov eax, dword ptr l_unk0x70
	imul dword ptr l_unk0x68
	add eax, dword ptr l_unk0x64
	add eax, dword ptr l_unk0x6c
	mov edi, eax
	xor ebx, ebx
	mov ecx, dword ptr l_unk0x30
jmp_10037952:
	mov dword ptr [g_unk0x10069445+ebx*4], edi
	inc ebx
	add edi, dword ptr l_unk0x68
	loop jmp_10037952
	mov eax, dword ptr l_unk0x28
	cmp eax, dword ptr l_unk0x30
	jl jmp_1003796a
	mov eax, dword ptr l_unk0x30
jmp_1003796a:
	mov dword ptr l_unk0x04, eax
	mov eax, dword ptr l_unk0x24
	mov ebx, dword ptr l_unk0x2c
	cmp ebx, eax
	jg jmp_10037979
	mov eax, ebx
jmp_10037979:
	mov dword ptr l_unk0x08, eax
	xor ebx, ebx
	mov dword ptr l_unk0x20, ebx
	mov eax, dword ptr l_unk0x04
jmp_10037984:
	inc ebx
	stc
	rcl dword ptr l_unk0x20, 1
	shr eax, 1
	jne jmp_10037984
	mov dword ptr l_unk0x1c, ebx
	mov eax, dword ptr l_unk0x08
jmp_10037993:
	inc ebx
	shr eax, 1
	jne jmp_10037993
	mov eax, dword ptr [jmp_10037753+ebx*4]
	mov dword ptr l_unk0x10, eax
	cmp dword ptr p_state, 0
	jne jmp_100379bb
	mov dword ptr p_state, 1
	mov ebx, 0
	mov esi, 0
	jmp jmp_100379e9
jmp_100379bb:
	mov esi, dword ptr p_state
	shr esi, 1
	jae jmp_100379c5
	xor esi, dword ptr l_unk0x10
jmp_100379c5:
	mov dword ptr p_state, esi
	cmp esi, 1
	jne jmp_100379d4
	mov dword ptr p_count, 0
jmp_100379d4:
	mov ecx, dword ptr l_unk0x1c
	shr esi, cl
	cmp esi, dword ptr l_unk0x08
	jae jmp_100379bb
	mov ebx, dword ptr p_state
	and ebx, dword ptr l_unk0x20
	cmp ebx, dword ptr l_unk0x04
	jae jmp_100379bb
jmp_100379e9:
	dec dword ptr p_count
	mov eax, esi
	add eax, dword ptr l_unk0x6c
	cmp eax, dword ptr l_unk0x54
	jl jmp_100379bb
	cmp eax, dword ptr l_unk0x5c
	jg jmp_100379bb
	mov eax, esi
	add eax, dword ptr l_unk0x4c
	cmp eax, dword ptr l_unk0x34
	jl jmp_100379bb
	cmp eax, dword ptr l_unk0x3c
	jg jmp_100379bb
	mov eax, ebx
	add eax, dword ptr l_unk0x70
	cmp eax, dword ptr l_unk0x58
	jl jmp_100379bb
	cmp eax, dword ptr l_unk0x60
	jg jmp_100379bb
	mov eax, ebx
	add eax, dword ptr l_unk0x50
	cmp eax, dword ptr l_unk0x38
	jl jmp_100379bb
	cmp eax, dword ptr l_unk0x40
	jg jmp_100379bb
	mov edi, dword ptr [g_unk0x10069445+ebx*4]
	add edi, esi
	mov eax, dword ptr [g_unk0x10068845+ebx*4]
	add esi, eax
	movsb
	cmp dword ptr p_count, 0
	jge jmp_100379bb
	mov eax, dword ptr p_state
	pop edi
	pop esi
	pop ebx
	pop es
	ret
DissolveView endp

; Fades the colors used in the view toward p_palette in p_steps steps, through the
; callbacks in g_displayDriver.
FadeViewColors proc uses ebx esi edi, p_view:dword, p_palette:dword, p_steps:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword
	push es
	cld
	push ds
	pop es
	mov edi, offset g_unk0x10069a45
	mov eax, 0
	mov ecx, 40h
	rep stosd
	mov esi, dword ptr p_view
	mov eax, dword ptr [esi+4]
	inc eax
	mov ecx, dword ptr [esi+8]
	inc ecx
	imul ecx, eax
	mov esi, dword ptr [esi]
	mov edi, offset g_fadeColors
	mov dword ptr l_unk0x04, 0ffffffffh
jmp_10037a88:
	mov eax, 0
	lodsb
	cmp byte ptr [g_unk0x10069a45+eax], 0
	jne jmp_10037abb
	or byte ptr [g_unk0x10069a45+eax], 1
	stosb
	inc dword ptr l_unk0x04
	mov ebx, eax
	shl ebx, 1
	add ebx, eax
	add ebx, offset g_scanline
	push ecx
	push ebx
	push eax
	call dword ptr [g_displayDriver+20h]
	add esp, 8
	pop ecx
jmp_10037abb:
	dec ecx
	jne jmp_10037a88
	mov dword ptr l_unk0x08, 0
	mov ecx, dword ptr l_unk0x04
	mov ebx, 0
	mov esi, dword ptr p_palette
jmp_10037ad0:
	movzx eax, byte ptr [g_fadeColors+ecx]
	mov ebx, eax
	shl ebx, 1
	add ebx, eax
	mov edx, 2
jmp_10037ae2:
	mov ah, 0ffh
	mov al, byte ptr [g_scanline+ebx]
	sub al, byte ptr [esi+ebx]
	jge jmp_10037af3
	mov ah, 1
	neg al
jmp_10037af3:
	mov byte ptr [g_unk0x10069a45+ebx], ah
	mov byte ptr [g_fadeDistances+ebx], al
	cmp al, byte ptr l_unk0x08
	jle jmp_10037b07
	mov byte ptr l_unk0x08, al
jmp_10037b07:
	inc ebx
	dec edx
	jns jmp_10037ae2
	dec ecx
	jns jmp_10037ad0
	mov ecx, dword ptr l_unk0x04
	mov edi, offset g_fadeErrors
	mov al, byte ptr l_unk0x08
	shr al, 1
	mov ah, al
	shr ecx, 1
	rep stosw
	adc ecx, 0
	rep stosb
	movzx esi, byte ptr l_unk0x08
	cmp esi, 0
	je jmp_10037bcc
	mov eax, dword ptr p_steps
	mov edx, 0
	shld edx, eax, 10h
	shl eax, 10h
	div esi
	mov dword ptr l_unk0x0c, eax
	mov dword ptr l_unk0x10, 8000h
jmp_10037b4f:
	mov ecx, dword ptr l_unk0x04
jmp_10037b52:
	movzx eax, byte ptr [g_fadeColors+ecx]
	mov ebx, eax
	shl ebx, 1
	add ebx, eax
	mov edx, 2
jmp_10037b64:
	mov al, byte ptr [g_fadeErrors+ebx+edx]
	add al, byte ptr [g_fadeDistances+ebx+edx]
	cmp al, byte ptr l_unk0x08
	jl jmp_10037b88
	sub al, byte ptr l_unk0x08
	mov ah, byte ptr [g_unk0x10069a45+ebx+edx]
	add byte ptr [g_scanline+ebx+edx], ah
jmp_10037b88:
	mov byte ptr [g_fadeErrors+ebx+edx], al
	dec edx
	jns jmp_10037b64
	push ecx
	mov eax, ebx
	add eax, offset g_scanline
	push eax
	movzx eax, byte ptr [g_fadeColors+ecx]
	push eax
	call dword ptr [g_displayDriver+24h]
	add esp, 8
	pop ecx
	dec ecx
	jns jmp_10037b52
	mov eax, dword ptr l_unk0x0c
	add dword ptr l_unk0x10, eax
	cmp word ptr l_unk0x10+2, 1
	jl jmp_10037bc9
jmp_10037bbd:
	call dword ptr [g_displayDriver+14h]
	dec word ptr l_unk0x10+2
	jne jmp_10037bbd
jmp_10037bc9:
	dec esi
	jne jmp_10037b4f
jmp_10037bcc:
	pop es
	ret
FadeViewColors endp

; Counts the distinct colors in the view, storing each one in p_colors when given.
CountViewColors proc uses ebx esi edi, p_view:dword, p_colors:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword
	push es
	cld
	push ds
	pop es
	mov edi, offset g_unk0x10069a45
	mov eax, 0
	mov ecx, 40h
	rep stosd
	mov esi, dword ptr p_view
	mov ecx, dword ptr [esi+0ch]
	sub ecx, dword ptr [esi+4]
	mov dword ptr l_unk0x08, ecx
	mov ebx, dword ptr [esi+10h]
	sub ebx, dword ptr [esi+8]
	mov esi, dword ptr [esi]
	mov eax, dword ptr [esi+4]
	inc eax
	mov dword ptr l_unk0x0c, eax
	mov esi, dword ptr p_view
	mov eax, dword ptr [esi+8]
	imul dword ptr l_unk0x0c
	add eax, dword ptr [esi+4]
	mov esi, dword ptr [esi]
	mov esi, dword ptr [esi]
	add esi, eax
	mov edi, dword ptr p_colors
	mov dword ptr l_unk0x04, 0ffffffffh
jmp_10037c27:
	mov eax, 0
	movzx eax, byte ptr [esi+ecx]
	cmp byte ptr [g_unk0x10069a45+eax], 0
	jne jmp_10037c49
	or byte ptr [g_unk0x10069a45+eax], 1
	inc dword ptr l_unk0x04
	cmp edi, 0
	je jmp_10037c49
	stosd
jmp_10037c49:
	dec ecx
	jns jmp_10037c27
	mov ecx, dword ptr l_unk0x08
	add esi, dword ptr l_unk0x0c
	dec ebx
	jns jmp_10037c27
	mov eax, dword ptr l_unk0x04
	inc eax
	pop es
	ret
CountViewColors endp

	end
