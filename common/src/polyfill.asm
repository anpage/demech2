; Hand-written assembly: the polygon fillers, a MASM object assembled with MASM 6.11 (ML), which
; both DLLs link byte for byte the same. ML aligns its .text to 4, which puts it right after the
; compiled code before it: at 0x1002a968 in MW2SHELL, 0x10036918 in MW2. The routines fill polygons
; given as arrays of six-dword vertices (x and y first) and share the working variables in
; g_polyVars. Annotated by name in each DLL's polyfill.h; COMPAT_MODE builds take each DLL's
; polyfill.c stubs. Placeholder names take the MW2SHELL addresses.

	.386
	.model flat, c
	option noscoped
	option casemap:none

CODE typedef proto
CODEPTR typedef ptr CODE
DATAPTR typedef ptr byte

	.data

; The routines' working variables: the clipped view, the vertex list and its edges' walks.
; SetLumaTable fills the 0x40 dwords at +0xd0.
	public g_polyVars
g_polyVars_t struct
m_data dd 74h dup (0)
g_polyVars_t ends
g_polyVars g_polyVars_t <>

	.code

; Fills a polygon in one color (the third dword of the first vertex, 16.16), clipped to the view.
FillPolygonFlat proc uses ebx esi edi, p_view:dword, p_count:dword, p_vertices:dword
	push es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+0ch], ecx
	mov eax, dword ptr [esi+0ch]
	mov ecx, dword ptr [ebx+4]
	cmp ecx, eax
	jl jmp_1002a98a
	mov ecx, eax
jmp_1002a98a:
	mov eax, dword ptr [esi+4]
	mov edi, 0
	cmp edi, eax
	jg jmp_1002a998
	mov edi, eax
jmp_1002a998:
	sub ecx, edi
	jl jmp_1002ac9b
	mov dword ptr [g_polyVars], ecx
	mov eax, dword ptr [esi+10h]
	mov ecx, dword ptr [ebx+8]
	cmp ecx, eax
	jl jmp_1002a9b2
	mov ecx, eax
jmp_1002a9b2:
	mov edx, dword ptr [esi+8]
	mov eax, 0
	cmp eax, edx
	jg jmp_1002a9c0
	mov eax, edx
jmp_1002a9c0:
	sub ecx, eax
	jl jmp_1002ac9b
	mov dword ptr [g_polyVars+4], ecx
	mul dword ptr [g_polyVars+0ch]
	add eax, edi
	add eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+8], eax
	push ds
	pop es
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_polyVars+24h], ebx
	mov dword ptr [g_polyVars+28h], eax
	mov eax, dword ptr [ebx+8]
	add eax, 8000h
	shr eax, 10h
	mov ah, al
	mov edx, eax
	shl eax, 10h
	or eax, edx
	mov dword ptr [g_polyVars+0a8h], eax
	mov dword ptr [g_polyVars+20h], 0
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_1002aa2d:
	mov edx, 0
	mov eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	or dword ptr [g_polyVars+20h], edx
	mov eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002aa6b
	mov esi, eax
	mov dword ptr [g_polyVars+2ch], ebx
jmp_1002aa6b:
	cmp eax, edi
	jl jmp_1002aa71
	mov edi, eax
jmp_1002aa71:
	and ecx, edx
	add ebx, 18h
	cmp ebx, dword ptr [g_polyVars+28h]
	jne jmp_1002aa2d
	or ecx, ecx
	jne jmp_1002ac9b
	mov eax, dword ptr [g_polyVars+2ch]
	mov dword ptr [g_polyVars+38h], eax
	mov dword ptr [g_polyVars+3ch], eax
	mov dword ptr [g_polyVars+4ch], esi
	cmp edi, esi
	je jmp_1002ac9b
jmp_1002aaa3:
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002aac5
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002aac5:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002aadb
	cmp ecx, 0
	jle jmp_1002aaa3
jmp_1002aadb:
	sub ecx, edx
	je jmp_1002aaa3
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+50h], edx
jmp_1002ab10:
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002ab2f
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002ab2f:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002ab45
	cmp ecx, 0
	jle jmp_1002ab10
jmp_1002ab45:
	sub ecx, edx
	je jmp_1002ab10
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+54h], edx
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [g_polyVars+4ch]
	sub edi, dword ptr [g_polyVars+4]
	jg jmp_1002ab8f
	add eax, edi
jmp_1002ab8f:
	mov dword ptr [g_polyVars+48h], eax
	mov eax, 0
	sub eax, dword ptr [g_polyVars+4ch]
	jle jmp_1002abfd
	sub dword ptr [g_polyVars+48h], eax
	mov ecx, 0
	mov dword ptr [g_polyVars+4ch], ecx
	mov ebx, dword ptr [g_polyVars+30h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+40h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+70h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+50h], eax
	mov ecx, 0
	mov ebx, dword ptr [g_polyVars+34h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+44h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+74h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+54h], eax
jmp_1002abfd:
	mov eax, dword ptr [g_polyVars+4ch]
	mul dword ptr [g_polyVars+0ch]
	mov edi, dword ptr [g_polyVars+8]
	add edi, eax
	mov eax, dword ptr [g_polyVars+50h]
	mov ebx, dword ptr [g_polyVars+54h]
	cmp dword ptr [g_polyVars+20h], 0
	jne jmp_1002acb4
jmp_1002ac28:
	push eax
	push edi
	mov edx, ebx
	cmp edx, eax
	jg jmp_1002ac31
	xchg edx, eax
jmp_1002ac31:
	sar eax, 10h
	sar edx, 10h
	mov ecx, edx
	sub ecx, eax
	inc ecx
	add edi, eax
	mov eax, dword ptr [g_polyVars+0a8h]
	cmp ecx, 4
	jle jmp_1002ac61
	mov edx, ecx
	mov ecx, edi
	neg ecx
	and ecx, 3
	sub edx, ecx
	rep stosb
	mov ecx, edx
	shr ecx, 2
	rep stosd
	and edx, 3
	mov ecx, edx
jmp_1002ac61:
	rep stosb
	pop edi
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002ac9b
	je jmp_1002aca1
	dec dword ptr [g_polyVars+40h]
	je jmp_1002ad5b
	add eax, dword ptr [g_polyVars+70h]
jmp_1002ac87:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002adcf
	add ebx, dword ptr [g_polyVars+74h]
	jmp jmp_1002ac28
jmp_1002ac9b:
	pop es
	ret
jmp_1002aca1:
	add eax, dword ptr [g_polyVars+70h]
	add ebx, dword ptr [g_polyVars+74h]
	jmp jmp_1002ac28
	align 4
jmp_1002acb4:
	push eax
	push edi
	mov edx, ebx
	cmp edx, eax
	jg jmp_1002acbd
	xchg edx, eax
jmp_1002acbd:
	sar eax, 10h
	sar edx, 10h
	cmp eax, dword ptr [g_polyVars]
	jg jmp_1002ad09
	cmp edx, 0
	jl jmp_1002ad09
	mov ecx, edx
	sub ecx, eax
	inc ecx
	add edi, eax
	sub eax, 0
	jl jmp_1002ad51
jmp_1002acdc:
	sub edx, dword ptr [g_polyVars]
	jg jmp_1002ad57
jmp_1002ace4:
	mov eax, dword ptr [g_polyVars+0a8h]
	cmp ecx, 4
	jle jmp_1002ad07
	mov edx, ecx
	mov ecx, edi
	neg ecx
	and ecx, 3
	sub edx, ecx
	rep stosb
	mov ecx, edx
	shr ecx, 2
	rep stosd
	and edx, 3
	mov ecx, edx
jmp_1002ad07:
	rep stosb
jmp_1002ad09:
	pop edi
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002ac9b
	je jmp_1002ad40
	dec dword ptr [g_polyVars+40h]
	je jmp_1002ad5b
	add eax, dword ptr [g_polyVars+70h]
jmp_1002ad29:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002adcf
	add ebx, dword ptr [g_polyVars+74h]
	jmp jmp_1002acb4
jmp_1002ad40:
	add eax, dword ptr [g_polyVars+70h]
	add ebx, dword ptr [g_polyVars+74h]
	jmp jmp_1002acb4
jmp_1002ad51:
	sub edi, eax
	add ecx, eax
	jmp jmp_1002acdc
jmp_1002ad57:
	sub ecx, edx
	jmp jmp_1002ace4
jmp_1002ad5b:
	push ebx
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002ad7e
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002ad7e:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	pop ebx
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002ac87
	jmp jmp_1002ad29
jmp_1002adcf:
	push eax
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002adef
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002adef:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop eax
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002ac28
	jmp jmp_1002acb4
FillPolygonFlat endp

; Another of the polygon fillers, over the same vertex lists. How it colors the spans hasn't been
; worked out from the assembly, so it keeps its placeholder.
FUN_1002ae41 proc uses ebx esi edi, p_view:dword, p_count:dword, p_vertices:dword
	push es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+0ch], ecx
	mov eax, dword ptr [esi+0ch]
	mov ecx, dword ptr [ebx+4]
	cmp ecx, eax
	jl jmp_1002ae63
	mov ecx, eax
jmp_1002ae63:
	mov eax, dword ptr [esi+4]
	mov edi, 0
	cmp edi, eax
	jg jmp_1002ae71
	mov edi, eax
jmp_1002ae71:
	sub ecx, edi
	jl jmp_1002b30a
	mov dword ptr [g_polyVars], ecx
	mov eax, dword ptr [esi+10h]
	mov ecx, dword ptr [ebx+8]
	cmp ecx, eax
	jl jmp_1002ae8b
	mov ecx, eax
jmp_1002ae8b:
	mov edx, dword ptr [esi+8]
	mov eax, 0
	cmp eax, edx
	jg jmp_1002ae99
	mov eax, edx
jmp_1002ae99:
	sub ecx, eax
	jl jmp_1002b30a
	mov dword ptr [g_polyVars+4], ecx
	mul dword ptr [g_polyVars+0ch]
	add eax, edi
	add eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+8], eax
	push ds
	pop es
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_polyVars+24h], ebx
	mov dword ptr [g_polyVars+28h], eax
	mov dword ptr [g_polyVars+20h], 0
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_1002aeed:
	mov edx, 0
	mov eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	or dword ptr [g_polyVars+20h], edx
	mov eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002af2b
	mov esi, eax
	mov dword ptr [g_polyVars+2ch], ebx
jmp_1002af2b:
	cmp eax, edi
	jl jmp_1002af31
	mov edi, eax
jmp_1002af31:
	and ecx, edx
	add ebx, 18h
	cmp ebx, dword ptr [g_polyVars+28h]
	jne jmp_1002aeed
	or ecx, ecx
	jne jmp_1002b30a
	mov eax, dword ptr [g_polyVars+2ch]
	mov dword ptr [g_polyVars+38h], eax
	mov dword ptr [g_polyVars+3ch], eax
	mov dword ptr [g_polyVars+4ch], esi
	cmp edi, esi
	je jmp_1002b30a
jmp_1002af63:
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002af85
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002af85:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002af9b
	cmp ecx, 0
	jle jmp_1002af63
jmp_1002af9b:
	sub ecx, edx
	je jmp_1002af63
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+78h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+50h], edx
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov dword ptr [g_polyVars+58h], edx
jmp_1002affe:
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002b01d
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002b01d:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002b033
	cmp ecx, 0
	jle jmp_1002affe
jmp_1002b033:
	sub ecx, edx
	je jmp_1002affe
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+7ch], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+54h], edx
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov dword ptr [g_polyVars+5ch], edx
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [g_polyVars+4ch]
	sub edi, dword ptr [g_polyVars+4]
	jg jmp_1002b0ab
	add eax, edi
jmp_1002b0ab:
	mov dword ptr [g_polyVars+48h], eax
	mov eax, 0
	sub eax, dword ptr [g_polyVars+4ch]
	jle jmp_1002b13b
	sub dword ptr [g_polyVars+48h], eax
	mov ecx, 0
	mov dword ptr [g_polyVars+4ch], ecx
	mov ebx, dword ptr [g_polyVars+30h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+40h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+70h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+50h], eax
	mov eax, dword ptr [g_polyVars+78h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+58h], eax
	mov ecx, 0
	mov ebx, dword ptr [g_polyVars+34h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+44h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+74h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+54h], eax
	mov eax, dword ptr [g_polyVars+7ch]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+5ch], eax
jmp_1002b13b:
	mov eax, dword ptr [g_polyVars+4ch]
	mul dword ptr [g_polyVars+0ch]
	mov edi, dword ptr [g_polyVars+8]
	add edi, eax
	mov eax, dword ptr [g_polyVars+50h]
	mov ebx, dword ptr [g_polyVars+54h]
	mov ecx, dword ptr [g_polyVars+58h]
	mov edx, dword ptr [g_polyVars+5ch]
	cmp dword ptr [g_polyVars+20h], 0
	jne jmp_1002b330
	align 4
jmp_1002b174:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	cmp ebx, eax
	jg jmp_1002b180
	xchg ebx, eax
	xchg ecx, edx
jmp_1002b180:
	sar eax, 10h
	sar ebx, 10h
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_1002b1a4
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shld edx, eax, 10h
	mov ebx, esi
jmp_1002b1a4:
	add ebx, edi
	mov dword ptr [g_polyVars+1ch], ebx
	inc esi
	mov ebx, ecx
	shr ecx, 10h
	shl ebx, 10h
	shl eax, 10h
	mov ch, cl
	add ebx, eax
	adc ch, dl
	test edi, 1
	je jmp_1002b1d6
	mov byte ptr [edi], cl
	dec esi
	je jmp_1002b2c0
	inc edi
	mov cl, ch
	add ebx, eax
	adc ch, dl
jmp_1002b1d6:
	cmp esi, 1
	je jmp_1002b2b8
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1002b254
jmp_1002b1e8:
	mov word ptr [edi], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+2], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+4], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+6], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+8], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+0ah], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	add edi, 0ch
	sub esi, 6
	js jmp_1002b2af
	cmp esi, 5
	jge jmp_1002b1e8
jmp_1002b254:
	mov word ptr [edi], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b2af
	mov word ptr [edi+2], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b2af
	mov word ptr [edi+4], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b2af
	mov word ptr [edi+6], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b2af
	mov word ptr [edi+8], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
jmp_1002b2af:
	pop esi
	test esi, 1
	je jmp_1002b2c0
jmp_1002b2b8:
	mov edi, dword ptr [g_polyVars+1ch]
	mov byte ptr [edi], cl
jmp_1002b2c0:
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002b30a
	je jmp_1002b310
	dec dword ptr [g_polyVars+40h]
	je jmp_1002b551
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
jmp_1002b2ed:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002b5ef
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002b174
jmp_1002b30a:
	pop es
	ret
jmp_1002b310:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002b174
	align 4
jmp_1002b330:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	cmp ebx, eax
	jg jmp_1002b33c
	xchg ebx, eax
	xchg ecx, edx
jmp_1002b33c:
	sar eax, 10h
	cmp eax, dword ptr [g_polyVars]
	jg jmp_1002b4cb
	sar ebx, 10h
	cmp ebx, 0
	jl jmp_1002b4cb
	mov dword ptr [g_polyVars+90h], eax
	mov dword ptr [g_polyVars+94h], ebx
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	inc esi
	cmp esi, 1
	je jmp_1002b383
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov dword ptr [g_polyVars+98h], eax
jmp_1002b383:
	mov eax, 0
	sub eax, dword ptr [g_polyVars+90h]
	jg jmp_1002b532
jmp_1002b394:
	mov eax, dword ptr [g_polyVars+94h]
	sub eax, dword ptr [g_polyVars]
	jg jmp_1002b54a
jmp_1002b3a5:
	mov ebx, ecx
	shr ecx, 10h
	mov eax, edi
	add eax, esi
	dec eax
	mov dword ptr [g_polyVars+1ch], eax
	mov eax, dword ptr [g_polyVars+98h]
	shld edx, eax, 10h
	shl ebx, 10h
	shl eax, 10h
	mov ch, cl
	add ebx, eax
	adc ch, dl
	test edi, 1
	je jmp_1002b3e1
	mov byte ptr [edi], cl
	dec esi
	je jmp_1002b4cb
	inc edi
	mov cl, ch
	add ebx, eax
	adc ch, dl
jmp_1002b3e1:
	cmp esi, 1
	je jmp_1002b4c3
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1002b45f
jmp_1002b3f3:
	mov word ptr [edi], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+2], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+4], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+6], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+8], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	mov word ptr [edi+0ah], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	add edi, 0ch
	sub esi, 6
	js jmp_1002b4ba
	cmp esi, 5
	jge jmp_1002b3f3
jmp_1002b45f:
	mov word ptr [edi], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b4ba
	mov word ptr [edi+2], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b4ba
	mov word ptr [edi+4], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b4ba
	mov word ptr [edi+6], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
	dec esi
	js jmp_1002b4ba
	mov word ptr [edi+8], cx
	mov cl, ch
	add ebx, eax
	adc cl, dl
	mov ch, cl
	add ebx, eax
	adc ch, dl
jmp_1002b4ba:
	pop esi
	test esi, 1
	je jmp_1002b4cb
jmp_1002b4c3:
	mov edi, dword ptr [g_polyVars+1ch]
	mov byte ptr [edi], cl
jmp_1002b4cb:
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002b30a
	je jmp_1002b515
	dec dword ptr [g_polyVars+40h]
	je jmp_1002b551
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
jmp_1002b4f8:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002b5ef
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002b330
jmp_1002b515:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002b330
jmp_1002b532:
	add edi, eax
	sub esi, eax
	shl eax, 10h
	imul dword ptr [g_polyVars+98h]
	shrd eax, edx, 10h
	add ecx, eax
	jmp jmp_1002b394
jmp_1002b54a:
	sub esi, eax
	jmp jmp_1002b3a5
jmp_1002b551:
	push ebx
	push edx
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002b575
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002b575:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+78h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [ebx+8]
	add ecx, 8000h
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	pop edx
	pop ebx
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002b2ed
	jmp jmp_1002b4f8
jmp_1002b5ef:
	push eax
	push ecx
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002b610
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002b610:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+7ch], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop ecx
	pop eax
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002b174
	jmp jmp_1002b330
FUN_1002ae41 endp

; Another of the polygon fillers, over the same vertex lists. How it colors the spans hasn't been
; worked out from the assembly, so it keeps its placeholder.
FUN_1002b68b proc uses ebx esi edi, p_view:dword, p_unk0x04:dword, p_count:dword, p_vertices:dword
	push es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+0ch], ecx
	mov eax, dword ptr [esi+0ch]
	mov ecx, dword ptr [ebx+4]
	cmp ecx, eax
	jl jmp_1002b6ad
	mov ecx, eax
jmp_1002b6ad:
	mov eax, dword ptr [esi+4]
	mov edi, 0
	cmp edi, eax
	jg jmp_1002b6bb
	mov edi, eax
jmp_1002b6bb:
	sub ecx, edi
	jl jmp_1002bb8a
	mov dword ptr [g_polyVars], ecx
	mov eax, dword ptr [esi+10h]
	mov ecx, dword ptr [ebx+8]
	cmp ecx, eax
	jl jmp_1002b6d5
	mov ecx, eax
jmp_1002b6d5:
	mov edx, dword ptr [esi+8]
	mov eax, 0
	cmp eax, edx
	jg jmp_1002b6e3
	mov eax, edx
jmp_1002b6e3:
	sub ecx, eax
	jl jmp_1002bb8a
	mov dword ptr [g_polyVars+4], ecx
	mul dword ptr [g_polyVars+0ch]
	add eax, edi
	add eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+8], eax
	push ds
	pop es
	mov dword ptr [g_polyVars+0b8h], ebp
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_polyVars+24h], ebx
	mov dword ptr [g_polyVars+28h], eax
	mov dword ptr [g_polyVars+20h], 0
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_1002b73d:
	mov edx, 0
	mov eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	or dword ptr [g_polyVars+20h], edx
	mov eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002b77b
	mov esi, eax
	mov dword ptr [g_polyVars+2ch], ebx
jmp_1002b77b:
	cmp eax, edi
	jl jmp_1002b781
	mov edi, eax
jmp_1002b781:
	and ecx, edx
	add ebx, 18h
	cmp ebx, dword ptr [g_polyVars+28h]
	jne jmp_1002b73d
	or ecx, ecx
	jne jmp_1002bb8a
	mov eax, dword ptr [g_polyVars+2ch]
	mov dword ptr [g_polyVars+38h], eax
	mov dword ptr [g_polyVars+3ch], eax
	mov dword ptr [g_polyVars+4ch], esi
	cmp edi, esi
	je jmp_1002bb8a
jmp_1002b7b3:
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002b7d5
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002b7d5:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002b7eb
	cmp ecx, 0
	jle jmp_1002b7b3
jmp_1002b7eb:
	sub ecx, edx
	je jmp_1002b7b3
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+78h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+50h], edx
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov dword ptr [g_polyVars+58h], edx
jmp_1002b84e:
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002b86d
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002b86d:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002b883
	cmp ecx, 0
	jle jmp_1002b84e
jmp_1002b883:
	sub ecx, edx
	je jmp_1002b84e
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+7ch], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+54h], edx
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov dword ptr [g_polyVars+5ch], edx
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [g_polyVars+4ch]
	sub edi, dword ptr [g_polyVars+4]
	jg jmp_1002b8fb
	add eax, edi
jmp_1002b8fb:
	mov dword ptr [g_polyVars+48h], eax
	mov esi, dword ptr p_unk0x04
	mov edi, 0
	mov eax, 0
	sub eax, dword ptr [g_polyVars+4ch]
	jle jmp_1002b9a0
	sub dword ptr [g_polyVars+48h], eax
	test eax, 1
	je jmp_1002b928
	xchg esi, edi
jmp_1002b928:
	mov ecx, 0
	mov dword ptr [g_polyVars+4ch], ecx
	mov ebx, dword ptr [g_polyVars+30h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+40h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+70h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+50h], eax
	mov eax, dword ptr [g_polyVars+78h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+58h], eax
	mov ecx, 0
	mov ebx, dword ptr [g_polyVars+34h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+44h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+74h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+54h], eax
	mov eax, dword ptr [g_polyVars+7ch]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+5ch], eax
jmp_1002b9a0:
	mov dword ptr [g_polyVars+0b0h], esi
	mov dword ptr [g_polyVars+0b4h], edi
	mov eax, dword ptr [g_polyVars+4ch]
	mul dword ptr [g_polyVars+0ch]
	mov edi, dword ptr [g_polyVars+8]
	add edi, eax
	mov eax, dword ptr [g_polyVars+50h]
	mov ebx, dword ptr [g_polyVars+54h]
	mov ecx, dword ptr [g_polyVars+58h]
	mov edx, dword ptr [g_polyVars+5ch]
	cmp dword ptr [g_polyVars+20h], 0
	jne jmp_1002bbb0
	align 4
jmp_1002b9e4:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	cmp ebx, eax
	jg jmp_1002b9f0
	xchg ebx, eax
	xchg ecx, edx
jmp_1002b9f0:
	sar eax, 10h
	sar ebx, 10h
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_1002ba1c
	sub edx, ecx
	sar ebx, 1
	cmp ebx, 1
	adc ebx, 0
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shld edx, eax, 10h
	mov ebx, esi
jmp_1002ba1c:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_polyVars+1ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_polyVars+0b0h]
	add ebp, dword ptr [g_polyVars+0b4h]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test edi, 1
	je jmp_1002ba66
	mov byte ptr [edi], cl
	dec esi
	je jmp_1002bb24
	inc edi
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1002ba66:
	cmp esi, 1
	je jmp_1002bb1c
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1002bacc
jmp_1002ba78:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+0ah], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add edi, 0ch
	sub esi, 6
	js jmp_1002bb13
	cmp esi, 5
	jge jmp_1002ba78
jmp_1002bacc:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bb13
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bb13
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bb13
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bb13
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_1002bb13:
	pop esi
	test esi, 1
	je jmp_1002bb24
jmp_1002bb1c:
	mov edi, dword ptr [g_polyVars+1ch]
	mov byte ptr [edi], cl
jmp_1002bb24:
	mov eax, dword ptr [g_polyVars+0b0h]
	mov ebx, dword ptr [g_polyVars+0b4h]
	mov dword ptr [g_polyVars+0b4h], eax
	mov dword ptr [g_polyVars+0b0h], ebx
	mov ebp, dword ptr [g_polyVars+0b8h]
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002bb8a
	je jmp_1002bb90
	dec dword ptr [g_polyVars+40h]
	je jmp_1002bdff
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
jmp_1002bb6d:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002be9d
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002b9e4
jmp_1002bb8a:
	pop es
	ret
jmp_1002bb90:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002b9e4
	align 4
jmp_1002bbb0:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	cmp ebx, eax
	jg jmp_1002bbbc
	xchg ebx, eax
	xchg ecx, edx
jmp_1002bbbc:
	sar eax, 10h
	cmp eax, dword ptr [g_polyVars]
	jg jmp_1002bd55
	sar ebx, 10h
	cmp ebx, 0
	jl jmp_1002bd55
	mov dword ptr [g_polyVars+90h], eax
	mov dword ptr [g_polyVars+94h], ebx
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	inc esi
	cmp esi, 1
	je jmp_1002bc0b
	sub edx, ecx
	sar ebx, 1
	cmp ebx, 1
	adc ebx, 0
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov dword ptr [g_polyVars+98h], eax
jmp_1002bc0b:
	mov eax, 0
	sub eax, dword ptr [g_polyVars+90h]
	jg jmp_1002bdd8
	mov dword ptr [g_polyVars+0bch], 0
jmp_1002bc26:
	mov eax, dword ptr [g_polyVars+94h]
	sub eax, dword ptr [g_polyVars]
	jg jmp_1002bdf8
jmp_1002bc37:
	mov eax, edi
	add eax, esi
	dec eax
	mov dword ptr [g_polyVars+1ch], eax
	mov eax, dword ptr [g_polyVars+98h]
	shld edx, eax, 10h
	shl eax, 10h
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_polyVars+0b0h]
	add ebp, dword ptr [g_polyVars+0b4h]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test dword ptr [g_polyVars+0bch], 1
	jne jmp_1002bc8f
	test edi, 1
	je jmp_1002bc97
	mov byte ptr [edi], cl
	dec esi
	je jmp_1002bd55
	inc edi
jmp_1002bc8f:
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1002bc97:
	cmp esi, 1
	je jmp_1002bd4d
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1002bcfd
jmp_1002bca9:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	mov word ptr [edi+0ah], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add edi, 0ch
	sub esi, 6
	js jmp_1002bd44
	cmp esi, 5
	jge jmp_1002bca9
jmp_1002bcfd:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bd44
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bd44
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bd44
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002bd44
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_1002bd44:
	pop esi
	test esi, 1
	je jmp_1002bd55
jmp_1002bd4d:
	mov edi, dword ptr [g_polyVars+1ch]
	mov byte ptr [edi], cl
jmp_1002bd55:
	mov eax, dword ptr [g_polyVars+0b0h]
	mov ebx, dword ptr [g_polyVars+0b4h]
	mov dword ptr [g_polyVars+0b4h], eax
	mov dword ptr [g_polyVars+0b0h], ebx
	mov ebp, dword ptr [g_polyVars+0b8h]
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002bb8a
	je jmp_1002bdbb
	dec dword ptr [g_polyVars+40h]
	je jmp_1002bdff
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
jmp_1002bd9e:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002be9d
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002bbb0
jmp_1002bdbb:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002bbb0
jmp_1002bdd8:
	mov dword ptr [g_polyVars+0bch], eax
	add edi, eax
	sub esi, eax
	and eax, 0fffffffeh
	shl eax, 0fh
	imul dword ptr [g_polyVars+98h]
	shrd eax, edx, 10h
	add ecx, eax
	jmp jmp_1002bc26
jmp_1002bdf8:
	sub esi, eax
	jmp jmp_1002bc37
jmp_1002bdff:
	push ebx
	push edx
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002be23
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002be23:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+78h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [ebx+8]
	add ecx, 8000h
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	pop edx
	pop ebx
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002bb6d
	jmp jmp_1002bd9e
jmp_1002be9d:
	push eax
	push ecx
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002bebe
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002bebe:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+7ch], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop ecx
	pop eax
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002b9e4
	jmp jmp_1002bbb0
FUN_1002b68b endp

; Another of the polygon fillers, over the same vertex lists. How it colors the spans hasn't been
; worked out from the assembly, so it keeps its placeholder.
FUN_1002bf39 proc uses ebx esi edi, p_view:dword, p_count:dword, p_vertices:dword, p_unk0x0c:dword
	push es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+0ch], ecx
	mov eax, dword ptr [esi+0ch]
	mov ecx, dword ptr [ebx+4]
	cmp ecx, eax
	jl jmp_1002bf5b
	mov ecx, eax
jmp_1002bf5b:
	mov eax, dword ptr [esi+4]
	mov edi, 0
	cmp edi, eax
	jg jmp_1002bf69
	mov edi, eax
jmp_1002bf69:
	sub ecx, edi
	jl jmp_1002c394
	mov dword ptr [g_polyVars], ecx
	mov eax, dword ptr [esi+10h]
	mov ecx, dword ptr [ebx+8]
	cmp ecx, eax
	jl jmp_1002bf83
	mov ecx, eax
jmp_1002bf83:
	mov edx, dword ptr [esi+8]
	mov eax, 0
	cmp eax, edx
	jg jmp_1002bf91
	mov eax, edx
jmp_1002bf91:
	sub ecx, eax
	jl jmp_1002c394
	mov dword ptr [g_polyVars+4], ecx
	mul dword ptr [g_polyVars+0ch]
	add eax, edi
	add eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+8], eax
	push ds
	pop es
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_polyVars+24h], ebx
	mov dword ptr [g_polyVars+28h], eax
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_1002bfdb:
	mov edx, 0
	mov eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002c013
	mov esi, eax
	mov dword ptr [g_polyVars+2ch], ebx
jmp_1002c013:
	cmp eax, edi
	jl jmp_1002c019
	mov edi, eax
jmp_1002c019:
	and ecx, edx
	add ebx, 18h
	cmp ebx, dword ptr [g_polyVars+28h]
	jne jmp_1002bfdb
	or ecx, ecx
	jne jmp_1002c394
	mov eax, dword ptr [g_polyVars+2ch]
	mov dword ptr [g_polyVars+38h], eax
	mov dword ptr [g_polyVars+3ch], eax
	mov dword ptr [g_polyVars+4ch], esi
	cmp edi, esi
	je jmp_1002c394
jmp_1002c04b:
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002c06d
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002c06d:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002c083
	cmp ecx, 0
	jle jmp_1002c04b
jmp_1002c083:
	sub ecx, edx
	je jmp_1002c04b
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+50h], edx
jmp_1002c0b8:
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002c0d7
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002c0d7:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002c0ed
	cmp ecx, 0
	jle jmp_1002c0b8
jmp_1002c0ed:
	sub ecx, edx
	je jmp_1002c0b8
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+54h], edx
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [g_polyVars+4ch]
	sub edi, dword ptr [g_polyVars+4]
	jg jmp_1002c137
	add eax, edi
jmp_1002c137:
	mov dword ptr [g_polyVars+48h], eax
	mov eax, 0
	sub eax, dword ptr [g_polyVars+4ch]
	jle jmp_1002c1a5
	sub dword ptr [g_polyVars+48h], eax
	mov ecx, 0
	mov dword ptr [g_polyVars+4ch], ecx
	mov ebx, dword ptr [g_polyVars+30h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+40h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+70h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+50h], eax
	mov ecx, 0
	mov ebx, dword ptr [g_polyVars+34h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+44h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+74h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+54h], eax
jmp_1002c1a5:
	mov eax, dword ptr [g_polyVars+4ch]
	mul dword ptr [g_polyVars+0ch]
	add eax, dword ptr [g_polyVars+8]
	mov dword ptr [g_polyVars+18h], eax
	mov eax, dword ptr [g_polyVars+50h]
	mov ebx, dword ptr [g_polyVars+54h]
jmp_1002c1c6:
	push eax
	push ebx
	cmp ebx, eax
	jg jmp_1002c1cd
	xchg ebx, eax
jmp_1002c1cd:
	sar eax, 10h
	cmp eax, dword ptr [g_polyVars]
	jg jmp_1002c357
	sar ebx, 10h
	cmp ebx, 0
	jl jmp_1002c357
	mov dword ptr [g_polyVars+90h], eax
	mov dword ptr [g_polyVars+94h], ebx
	sub ebx, eax
	je jmp_1002c219
	mov ecx, 0
	sub ecx, dword ptr [g_polyVars+90h]
	jg jmp_1002c3ab
jmp_1002c208:
	mov eax, dword ptr [g_polyVars+94h]
	sub eax, dword ptr [g_polyVars]
	jg jmp_1002c3b6
jmp_1002c219:
	mov eax, dword ptr [g_polyVars+90h]
	mov edi, dword ptr [g_polyVars+18h]
	add edi, eax
	mov ebx, dword ptr [g_polyVars+94h]
	sub ebx, eax
	mov eax, edi
	add eax, ebx
	mov dword ptr [g_polyVars+1ch], eax
	xor eax, eax
	mov esi, dword ptr p_unk0x0c
	inc ebx
	test edi, 1
	je jmp_1002c254
	mov al, byte ptr [edi]
	mov dl, byte ptr [esi+eax]
	mov byte ptr [edi], dl
	dec ebx
	je jmp_1002c357
	inc edi
jmp_1002c254:
	cmp ebx, 1
	je jmp_1002c34a
	push ebx
	shr ebx, 1
	dec ebx
	cmp ebx, 5
	jl jmp_1002c2dd
jmp_1002c266:
	mov cx, word ptr [edi]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi], dx
	mov cx, word ptr [edi+2]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+2], dx
	mov cx, word ptr [edi+4]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+4], dx
	mov cx, word ptr [edi+6]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+6], dx
	mov cx, word ptr [edi+8]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+8], dx
	mov cx, word ptr [edi+0ah]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+0ah], dx
	add edi, 0ch
	sub ebx, 6
	js jmp_1002c341
	cmp ebx, 5
	jge jmp_1002c266
jmp_1002c2dd:
	mov cx, word ptr [edi]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi], dx
	dec ebx
	js jmp_1002c341
	mov cx, word ptr [edi+2]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+2], dx
	dec ebx
	js jmp_1002c341
	mov cx, word ptr [edi+4]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+4], dx
	dec ebx
	js jmp_1002c341
	mov cx, word ptr [edi+6]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+6], dx
	dec ebx
	js jmp_1002c341
	mov cx, word ptr [edi+8]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+8], dx
jmp_1002c341:
	pop ebx
	test ebx, 1
	je jmp_1002c357
jmp_1002c34a:
	mov edi, dword ptr [g_polyVars+1ch]
	mov al, byte ptr [edi]
	mov dl, byte ptr [esi+eax]
	mov byte ptr [edi], dl
jmp_1002c357:
	mov edi, dword ptr [g_polyVars+0ch]
	add dword ptr [g_polyVars+18h], edi
	pop ebx
	pop eax
	dec dword ptr [g_polyVars+48h]
	js jmp_1002c394
	je jmp_1002c39a
	dec dword ptr [g_polyVars+40h]
	je jmp_1002c3c1
	add eax, dword ptr [g_polyVars+70h]
jmp_1002c37d:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002c428
	add ebx, dword ptr [g_polyVars+74h]
	jmp jmp_1002c1c6
jmp_1002c394:
	pop es
	ret
jmp_1002c39a:
	add eax, dword ptr [g_polyVars+70h]
	add ebx, dword ptr [g_polyVars+74h]
	jmp jmp_1002c1c6
jmp_1002c3ab:
	add dword ptr [g_polyVars+90h], ecx
	jmp jmp_1002c208
jmp_1002c3b6:
	sub dword ptr [g_polyVars+94h], eax
	jmp jmp_1002c219
jmp_1002c3c1:
	push ebx
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002c3e4
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002c3e4:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	pop ebx
	jmp jmp_1002c37d
jmp_1002c428:
	push eax
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov edi, ebx
	add edi, 18h
	cmp edi, dword ptr [g_polyVars+28h]
	jl jmp_1002c448
	mov edi, dword ptr [g_polyVars+24h]
jmp_1002c448:
	mov dword ptr [g_polyVars+3ch], edi
	mov ecx, dword ptr [edi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [edi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop eax
	jmp jmp_1002c1c6
FUN_1002bf39 endp

; Another of the polygon fillers, over the same vertex lists. How it colors the spans hasn't been
; worked out from the assembly, so it keeps its placeholder.
FUN_1002c48d proc uses ebx esi edi, p_view:dword, p_unk0x04:dword, p_count:dword, p_vertices:dword
	push es
	mov esi, dword ptr p_view
	mov ebx, dword ptr [esi]
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+0ch], ecx
	mov eax, dword ptr [esi+0ch]
	mov ecx, dword ptr [ebx+4]
	cmp ecx, eax
	jl jmp_1002c4af
	mov ecx, eax
jmp_1002c4af:
	mov eax, dword ptr [esi+4]
	mov edi, 0
	cmp edi, eax
	jg jmp_1002c4bd
	mov edi, eax
jmp_1002c4bd:
	sub ecx, edi
	jl jmp_1002c98e
	mov dword ptr [g_polyVars], ecx
	mov eax, dword ptr [esi+10h]
	mov ecx, dword ptr [ebx+8]
	cmp ecx, eax
	jl jmp_1002c4d7
	mov ecx, eax
jmp_1002c4d7:
	mov edx, dword ptr [esi+8]
	mov eax, 0
	cmp eax, edx
	jg jmp_1002c4e5
	mov eax, edx
jmp_1002c4e5:
	sub ecx, eax
	jl jmp_1002c98e
	mov dword ptr [g_polyVars+4], ecx
	mul dword ptr [g_polyVars+0ch]
	add eax, edi
	add eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+8], eax
	push ds
	pop es
	mov dword ptr [g_polyVars+0b8h], ebp
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_polyVars+24h], ebx
	mov dword ptr [g_polyVars+28h], eax
	mov dword ptr [g_polyVars+20h], 0
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_1002c53f:
	mov edx, 0
	mov eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	or dword ptr [g_polyVars+20h], edx
	mov eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002c57d
	mov esi, eax
	mov dword ptr [g_polyVars+2ch], ebx
jmp_1002c57d:
	cmp eax, edi
	jl jmp_1002c583
	mov edi, eax
jmp_1002c583:
	and ecx, edx
	add ebx, 18h
	cmp ebx, dword ptr [g_polyVars+28h]
	jne jmp_1002c53f
	or ecx, ecx
	jne jmp_1002c98e
	mov eax, dword ptr [g_polyVars+2ch]
	mov dword ptr [g_polyVars+38h], eax
	mov dword ptr [g_polyVars+3ch], eax
	mov dword ptr [g_polyVars+4ch], esi
	cmp edi, esi
	je jmp_1002c98e
jmp_1002c5b5:
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002c5d7
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002c5d7:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002c5ed
	cmp ecx, 0
	jle jmp_1002c5b5
jmp_1002c5ed:
	sub ecx, edx
	je jmp_1002c5b5
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+78h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+50h], edx
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov dword ptr [g_polyVars+58h], edx
jmp_1002c650:
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002c66f
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002c66f:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002c685
	cmp ecx, 0
	jle jmp_1002c650
jmp_1002c685:
	sub ecx, edx
	je jmp_1002c650
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+7ch], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+54h], edx
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov dword ptr [g_polyVars+5ch], edx
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [g_polyVars+4ch]
	sub edi, dword ptr [g_polyVars+4]
	jg jmp_1002c6fd
	add eax, edi
jmp_1002c6fd:
	mov dword ptr [g_polyVars+48h], eax
	mov esi, dword ptr p_unk0x04
	mov edi, 0
	mov eax, 0
	sub eax, dword ptr [g_polyVars+4ch]
	jle jmp_1002c7a2
	sub dword ptr [g_polyVars+48h], eax
	test eax, 1
	je jmp_1002c72a
	xchg esi, edi
jmp_1002c72a:
	mov ecx, 0
	mov dword ptr [g_polyVars+4ch], ecx
	mov ebx, dword ptr [g_polyVars+30h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+40h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+70h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+50h], eax
	mov eax, dword ptr [g_polyVars+78h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+58h], eax
	mov ecx, 0
	mov ebx, dword ptr [g_polyVars+34h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+44h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+74h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+54h], eax
	mov eax, dword ptr [g_polyVars+7ch]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+5ch], eax
jmp_1002c7a2:
	mov dword ptr [g_polyVars+0b0h], esi
	mov dword ptr [g_polyVars+0b4h], edi
	mov eax, dword ptr [g_polyVars+4ch]
	mul dword ptr [g_polyVars+0ch]
	mov edi, dword ptr [g_polyVars+8]
	add edi, eax
	mov eax, dword ptr [g_polyVars+50h]
	mov ebx, dword ptr [g_polyVars+54h]
	mov ecx, dword ptr [g_polyVars+58h]
	mov edx, dword ptr [g_polyVars+5ch]
	cmp dword ptr [g_polyVars+20h], 0
	jne jmp_1002c9b4
	align 4
jmp_1002c7e8:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	cmp ebx, eax
	jg jmp_1002c7f4
	xchg ebx, eax
	xchg ecx, edx
jmp_1002c7f4:
	sar eax, 10h
	sar ebx, 10h
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_1002c820
	sub edx, ecx
	sar ebx, 1
	cmp ebx, 1
	adc ebx, 0
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shld edx, eax, 10h
	mov ebx, esi
jmp_1002c820:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_polyVars+1ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_polyVars+0b0h]
	add ebp, dword ptr [g_polyVars+0b4h]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test edi, 1
	je jmp_1002c86a
	add byte ptr [edi], cl
	dec esi
	je jmp_1002c928
	inc edi
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1002c86a:
	cmp esi, 1
	je jmp_1002c920
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1002c8d0
jmp_1002c87c:
	add word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+0ah], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add edi, 0ch
	sub esi, 6
	js jmp_1002c917
	cmp esi, 5
	jge jmp_1002c87c
jmp_1002c8d0:
	add word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002c917
	add word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002c917
	add word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002c917
	add word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002c917
	add word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_1002c917:
	pop esi
	test esi, 1
	je jmp_1002c928
jmp_1002c920:
	mov edi, dword ptr [g_polyVars+1ch]
	add byte ptr [edi], cl
jmp_1002c928:
	mov eax, dword ptr [g_polyVars+0b0h]
	mov ebx, dword ptr [g_polyVars+0b4h]
	mov dword ptr [g_polyVars+0b4h], eax
	mov dword ptr [g_polyVars+0b0h], ebx
	mov ebp, dword ptr [g_polyVars+0b8h]
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002c98e
	je jmp_1002c994
	dec dword ptr [g_polyVars+40h]
	je jmp_1002cc03
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
jmp_1002c971:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002cca1
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002c7e8
jmp_1002c98e:
	pop es
	ret
jmp_1002c994:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002c7e8
	align 4
jmp_1002c9b4:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	cmp ebx, eax
	jg jmp_1002c9c0
	xchg ebx, eax
	xchg ecx, edx
jmp_1002c9c0:
	sar eax, 10h
	cmp eax, dword ptr [g_polyVars]
	jg jmp_1002cb59
	sar ebx, 10h
	cmp ebx, 0
	jl jmp_1002cb59
	mov dword ptr [g_polyVars+90h], eax
	mov dword ptr [g_polyVars+94h], ebx
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	inc esi
	cmp esi, 1
	je jmp_1002ca0f
	sub edx, ecx
	sar ebx, 1
	cmp ebx, 1
	adc ebx, 0
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov dword ptr [g_polyVars+98h], eax
jmp_1002ca0f:
	mov eax, 0
	sub eax, dword ptr [g_polyVars+90h]
	jg jmp_1002cbdc
	mov dword ptr [g_polyVars+0bch], 0
jmp_1002ca2a:
	mov eax, dword ptr [g_polyVars+94h]
	sub eax, dword ptr [g_polyVars]
	jg jmp_1002cbfc
jmp_1002ca3b:
	mov eax, edi
	add eax, esi
	dec eax
	mov dword ptr [g_polyVars+1ch], eax
	mov eax, dword ptr [g_polyVars+98h]
	shld edx, eax, 10h
	shl eax, 10h
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_polyVars+0b0h]
	add ebp, dword ptr [g_polyVars+0b4h]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test dword ptr [g_polyVars+0bch], 1
	jne jmp_1002ca93
	test edi, 1
	je jmp_1002ca9b
	add byte ptr [edi], cl
	dec esi
	je jmp_1002cb59
	inc edi
jmp_1002ca93:
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1002ca9b:
	cmp esi, 1
	je jmp_1002cb51
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1002cb01
jmp_1002caad:
	add word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add word ptr [edi+0ah], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	add edi, 0ch
	sub esi, 6
	js jmp_1002cb48
	cmp esi, 5
	jge jmp_1002caad
jmp_1002cb01:
	add word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002cb48
	add word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002cb48
	add word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002cb48
	add word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1002cb48
	add word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_1002cb48:
	pop esi
	test esi, 1
	je jmp_1002cb59
jmp_1002cb51:
	mov edi, dword ptr [g_polyVars+1ch]
	add byte ptr [edi], cl
jmp_1002cb59:
	mov eax, dword ptr [g_polyVars+0b0h]
	mov ebx, dword ptr [g_polyVars+0b4h]
	mov dword ptr [g_polyVars+0b4h], eax
	mov dword ptr [g_polyVars+0b0h], ebx
	mov ebp, dword ptr [g_polyVars+0b8h]
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	add edi, dword ptr [g_polyVars+0ch]
	dec dword ptr [g_polyVars+48h]
	js jmp_1002c98e
	je jmp_1002cbbf
	dec dword ptr [g_polyVars+40h]
	je jmp_1002cc03
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
jmp_1002cba2:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002cca1
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002c9b4
jmp_1002cbbf:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+78h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+7ch]
	jmp jmp_1002c9b4
jmp_1002cbdc:
	mov dword ptr [g_polyVars+0bch], eax
	add edi, eax
	sub esi, eax
	and eax, 0fffffffeh
	shl eax, 0fh
	imul dword ptr [g_polyVars+98h]
	shrd eax, edx, 10h
	add ecx, eax
	jmp jmp_1002ca2a
jmp_1002cbfc:
	sub esi, eax
	jmp jmp_1002ca3b
jmp_1002cc03:
	push ebx
	push edx
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002cc27
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002cc27:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+78h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [ebx+8]
	add ecx, 8000h
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	pop edx
	pop ebx
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002c971
	jmp jmp_1002cba2
jmp_1002cca1:
	push eax
	push ecx
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002ccc2
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002ccc2:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi+8]
	sub edx, dword ptr [ebx+8]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+7ch], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov edx, dword ptr [ebx+8]
	add edx, 8000h
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop ecx
	pop eax
	cmp dword ptr [g_polyVars+20h], 0
	je jmp_1002c7e8
	jmp jmp_1002c9b4
FUN_1002c48d endp

; Copies a 0x80-entry luma table (MW2's LUMA resource) into the working variables, for
; FillPolygonTextured.
SetLumaTable proc uses ebx esi edi, p_table:dword
	push es
	cld
	push ds
	pop es
	mov esi, dword ptr p_table
	lea edi, [g_polyVars+0d0h]
	mov ecx, 40h
	rep movsd
	pop es
	ret
SetLumaTable endp

; Fills a textured polygon, clipped to the view: p_unk0x10 points at the texture (its pixels and
; width minus one), and p_mode picks the span routine from g_polySpanRoutines. It ends by jumping
; to that routine; the routines jump back into it for each scan line.
FillPolygonTextured proc
	push ebp
	mov ebp, esp
	push ebx
	push esi
	push edi
	push es
	mov esi, dword ptr [ebp+8]
	mov ebx, dword ptr [esi]
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+0ch], ecx
	mov eax, dword ptr [esi+0ch]
	mov ecx, dword ptr [ebx+4]
	cmp ecx, eax
	jl jmp_1002cd7f
	mov ecx, eax
jmp_1002cd7f:
	mov eax, dword ptr [esi+4]
	mov edi, 0
	cmp edi, eax
	jg jmp_1002cd8d
	mov edi, eax
jmp_1002cd8d:
	sub ecx, edi
	jl jmp_1002d8aa
	mov dword ptr [g_polyVars], ecx
	mov eax, dword ptr [esi+10h]
	mov ecx, dword ptr [ebx+8]
	cmp ecx, eax
	jl jmp_1002cda7
	mov ecx, eax
jmp_1002cda7:
	mov edx, dword ptr [esi+8]
	mov eax, 0
	cmp eax, edx
	jg jmp_1002cdb5
	mov eax, edx
jmp_1002cdb5:
	sub ecx, eax
	jl jmp_1002d8aa
	mov dword ptr [g_polyVars+4], ecx
	mul dword ptr [g_polyVars+0ch]
	add eax, edi
	add eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+8], eax
	mov ebx, dword ptr [ebp+14h]
	mov eax, dword ptr [ebx]
	mov dword ptr [g_polyVars+14h], eax
	mov ecx, dword ptr [ebx+4]
	inc ecx
	mov dword ptr [g_polyVars+10h], ecx
	push ds
	pop es
	mov ebx, dword ptr [ebp+10h]
	mov eax, dword ptr [ebp+0ch]
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_polyVars+24h], ebx
	mov dword ptr [g_polyVars+28h], eax
	mov esi, 7fffh
	mov edi, 0ffff8000h
	mov ecx, 0fh
jmp_1002ce13:
	mov edx, 0
	mov eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars]
	sub eax, dword ptr [ebx]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [ebx+4]
	shld edx, eax, 1
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002ce4b
	mov esi, eax
	mov dword ptr [g_polyVars+2ch], ebx
jmp_1002ce4b:
	cmp eax, edi
	jl jmp_1002ce51
	mov edi, eax
jmp_1002ce51:
	and ecx, edx
	add ebx, 18h
	cmp ebx, dword ptr [g_polyVars+28h]
	jne jmp_1002ce13
	or ecx, ecx
	jne jmp_1002d8aa
	mov eax, dword ptr [g_polyVars+2ch]
	mov dword ptr [g_polyVars+38h], eax
	mov dword ptr [g_polyVars+3ch], eax
	mov dword ptr [g_polyVars+4ch], esi
	cmp edi, esi
	je jmp_1002d8aa
jmp_1002ce83:
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002cea5
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002cea5:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002cebb
	cmp ecx, 0
	jle jmp_1002ce83
jmp_1002cebb:
	sub ecx, edx
	je jmp_1002ce83
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+0ch]
	sub edx, dword ptr [ebx+0ch]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+80h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+10h]
	sub edx, dword ptr [ebx+10h]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+88h], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+50h], edx
	mov edx, dword ptr [ebx+0ch]
	add edx, 8000h
	mov dword ptr [g_polyVars+60h], edx
	mov edx, dword ptr [ebx+10h]
	add edx, 8000h
	mov dword ptr [g_polyVars+68h], edx
jmp_1002cf4c:
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_polyVars+28h]
	jl jmp_1002cf6b
	mov esi, dword ptr [g_polyVars+24h]
jmp_1002cf6b:
	mov dword ptr [g_polyVars+3ch], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp edx, 0
	jge jmp_1002cf81
	cmp ecx, 0
	jle jmp_1002cf4c
jmp_1002cf81:
	sub ecx, edx
	je jmp_1002cf4c
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi+0ch]
	sub edx, dword ptr [ebx+0ch]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+84h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [esi+10h]
	sub edx, dword ptr [ebx+10h]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+8ch], eax
	mov edx, dword ptr [ebx]
	shl edx, 10h
	add edx, 8000h
	mov dword ptr [g_polyVars+54h], edx
	mov edx, dword ptr [ebx+0ch]
	add edx, 8000h
	mov dword ptr [g_polyVars+64h], edx
	mov edx, dword ptr [ebx+10h]
	add edx, 8000h
	mov dword ptr [g_polyVars+6ch], edx
	mov eax, dword ptr [g_polyVars+4]
	sub eax, dword ptr [g_polyVars+4ch]
	sub edi, dword ptr [g_polyVars+4]
	jg jmp_1002d027
	add eax, edi
jmp_1002d027:
	mov dword ptr [g_polyVars+48h], eax
	mov eax, 0
	sub eax, dword ptr [g_polyVars+4ch]
	jle jmp_1002d0dd
	sub dword ptr [g_polyVars+48h], eax
	mov ecx, 0
	mov dword ptr [g_polyVars+4ch], ecx
	mov ebx, dword ptr [g_polyVars+30h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+40h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+70h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+50h], eax
	mov eax, dword ptr [g_polyVars+80h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+60h], eax
	mov eax, dword ptr [g_polyVars+88h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+68h], eax
	mov ecx, 0
	mov ebx, dword ptr [g_polyVars+34h]
	sub ecx, dword ptr [ebx+4]
	sub dword ptr [g_polyVars+44h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+74h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+54h], eax
	mov eax, dword ptr [g_polyVars+84h]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+64h], eax
	mov eax, dword ptr [g_polyVars+8ch]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+6ch], eax
jmp_1002d0dd:
	mov eax, dword ptr [g_polyVars+4ch]
	mul dword ptr [g_polyVars+0ch]
	add eax, dword ptr [g_polyVars+8]
	mov dword ptr [g_polyVars+18h], eax
	mov eax, dword ptr [ebp+18h]
	mov eax, dword ptr [g_polySpanRoutines+eax*4]
	mov dword ptr [g_polyVars+0ach], eax
	mov eax, dword ptr [g_polyVars+50h]
	mov ebx, dword ptr [g_polyVars+54h]
	mov ecx, dword ptr [g_polyVars+60h]
	mov edx, dword ptr [g_polyVars+64h]
	mov esi, dword ptr [g_polyVars+68h]
	mov edi, dword ptr [g_polyVars+6ch]
jmp_1002d125:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	cmp ebx, eax
	jg jmp_1002d134
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_1002d134:
	sar eax, 10h
	cmp eax, dword ptr [g_polyVars]
	jg jmp_1002d84d
	sar ebx, 10h
	cmp ebx, 0
	jl jmp_1002d84d
	mov dword ptr [g_polyVars+90h], eax
	mov dword ptr [g_polyVars+94h], ebx
	mov dword ptr [g_polyVars+0a4h], ecx
	sub ebx, eax
	je jmp_1002d236
	push ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov dword ptr [g_polyVars+9ch], eax
	shld edx, eax, 10h
	pop ebx
	and eax, 0ffffh
	and edx, 0ffffh
	mov ecx, 1
	test edx, 8000h
	je jmp_1002d1a9
	or edx, 0ffff0000h
	neg ecx
	cmp eax, 1
	sbb edx, -1
jmp_1002d1a9:
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
	mov dword ptr [g_polyVars+0a0h], eax
	shld edx, eax, 10h
	and eax, 0ffffh
	and edx, 0ffffh
	mov ecx, dword ptr [g_polyVars+10h]
	test edx, 8000h
	je jmp_1002d1e9
	neg ecx
	cmp eax, 1
	sbb edx, -1
jmp_1002d1e9:
	mov eax, dword ptr [g_polyVars+10h]
	imul dx
	cwde
	pop edx
	pop ebx
	add edx, eax
	mov dword ptr [g_polyVars+0c0h], edx
	add edx, ecx
	mov dword ptr [g_polyVars+0c4h], edx
	add ebx, eax
	mov dword ptr [g_polyVars+0c8h], ebx
	add ebx, ecx
	mov dword ptr [g_polyVars+0cch], ebx
	mov ecx, 0
	sub ecx, dword ptr [g_polyVars+90h]
	jg jmp_1002d8d9
jmp_1002d225:
	mov eax, dword ptr [g_polyVars+94h]
	sub eax, dword ptr [g_polyVars]
	jg jmp_1002d905
jmp_1002d236:
	mov ecx, esi
	shr esi, 10h
	mov eax, esi
	mul dword ptr [g_polyVars+10h]
	add eax, dword ptr [g_polyVars+14h]
	mov esi, dword ptr [g_polyVars+0a4h]
	shr esi, 10h
	add esi, eax
	mov eax, dword ptr [g_polyVars+90h]
	mov edi, dword ptr [g_polyVars+18h]
	add edi, eax
	mov ebx, dword ptr [g_polyVars+94h]
	sub ebx, eax
	push ebp
	mov edx, dword ptr [g_polyVars+0a4h]
	mov eax, dword ptr [g_polyVars+9ch]
	or eax, eax
	jns jmp_1002d27d
	neg eax
	not edx
jmp_1002d27d:
	shl eax, 10h
	shl edx, 10h
	mov ebp, dword ptr [g_polyVars+0a0h]
	or ebp, ebp
	jns jmp_1002d291
	neg ebp
	not ecx
jmp_1002d291:
	shl ebp, 10h
	shl ecx, 10h
	push ebx
	xor ebx, ebx
	jmp dword ptr [g_polyVars+0ach]
FillPolygonTextured endp

; FillPolygonTextured's span routines, one per mode, between the routine and the span routines.
	public g_polySpanRoutines
g_polySpanRoutines_t struct
m_unk0x00 CODEPTR FUN_1002d724
m_unk0x04 CODEPTR FUN_1002d457
m_unk0x08 CODEPTR FUN_1002d5c3
m_unk0x0c CODEPTR FUN_1002d2b0
g_polySpanRoutines_t ends
g_polySpanRoutines g_polySpanRoutines_t <>

; A span routine of FillPolygonTextured (a g_polySpanRoutines entry); register-based, entered by
; jmp. The span modes aren't known, so the four keep their placeholders.
FUN_1002d2b0 proc
	cmp dword ptr [esp], 5
	jl jmp_1002d396
jmp_1002d2ba:
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d2c9
	mov byte ptr [edi], bl
jmp_1002d2c9:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d2ea
	mov byte ptr [edi+1], bl
jmp_1002d2ea:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d30b
	mov byte ptr [edi+2], bl
jmp_1002d30b:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d32c
	mov byte ptr [edi+3], bl
jmp_1002d32c:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d34d
	mov byte ptr [edi+4], bl
jmp_1002d34d:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d36e
	mov byte ptr [edi+5], bl
jmp_1002d36e:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	add edi, 6
	sub dword ptr [esp], 6
	js jmp_1002d452
	cmp dword ptr [esp], 5
	jge jmp_1002d2ba
jmp_1002d396:
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d3a5
	mov byte ptr [edi], bl
jmp_1002d3a5:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d452
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d3cf
	mov byte ptr [edi+1], bl
jmp_1002d3cf:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d452
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d3f5
	mov byte ptr [edi+2], bl
jmp_1002d3f5:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d452
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d41b
	mov byte ptr [edi+3], bl
jmp_1002d41b:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d452
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	cmp bl, 0ffh
	je jmp_1002d441
	mov byte ptr [edi+4], bl
jmp_1002d441:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
jmp_1002d452:
	jmp jmp_1002d849
FUN_1002d2b0 endp

; A span routine of FillPolygonTextured (a g_polySpanRoutines entry); register-based, entered by
; jmp. The span modes aren't known, so the four keep their placeholders.
FUN_1002d457 proc
	cmp dword ptr [esp], 5
	jl jmp_1002d51f
jmp_1002d461:
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+1], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+2], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+3], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+4], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+5], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	add edi, 6
	sub dword ptr [esp], 6
	js jmp_1002d5be
	cmp dword ptr [esp], 5
	jge jmp_1002d461
jmp_1002d51f:
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d5be
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+1], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d5be
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+2], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d5be
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+3], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d5be
	mov bl, byte ptr [esi]
	mov bl, byte ptr [g_polyVars+0d0h+ebx]
	mov byte ptr [edi+4], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
jmp_1002d5be:
	jmp jmp_1002d849
FUN_1002d457 endp

; A span routine of FillPolygonTextured (a g_polySpanRoutines entry); register-based, entered by
; jmp. The span modes aren't known, so the four keep their placeholders.
FUN_1002d5c3 proc
	cmp dword ptr [esp], 5
	jl jmp_1002d685
jmp_1002d5cd:
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d5d6
	mov byte ptr [edi], bl
jmp_1002d5d6:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d5f1
	mov byte ptr [edi+1], bl
jmp_1002d5f1:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d60c
	mov byte ptr [edi+2], bl
jmp_1002d60c:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d627
	mov byte ptr [edi+3], bl
jmp_1002d627:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d642
	mov byte ptr [edi+4], bl
jmp_1002d642:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d65d
	mov byte ptr [edi+5], bl
jmp_1002d65d:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	add edi, 6
	sub dword ptr [esp], 6
	js jmp_1002d71f
	cmp dword ptr [esp], 5
	jge jmp_1002d5cd
jmp_1002d685:
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d68e
	mov byte ptr [edi], bl
jmp_1002d68e:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d71f
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d6ae
	mov byte ptr [edi+1], bl
jmp_1002d6ae:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d71f
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d6ce
	mov byte ptr [edi+2], bl
jmp_1002d6ce:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d71f
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d6ee
	mov byte ptr [edi+3], bl
jmp_1002d6ee:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d71f
	mov bl, byte ptr [esi]
	cmp bl, 0ffh
	je jmp_1002d70e
	mov byte ptr [edi+4], bl
jmp_1002d70e:
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
jmp_1002d71f:
	jmp jmp_1002d849
FUN_1002d5c3 endp

; A span routine of FillPolygonTextured (a g_polySpanRoutines entry); register-based, entered by
; jmp. It also holds the scan-line stepping the other span routines jump to. The span modes aren't
; known, so the four keep their placeholders.
FUN_1002d724 proc
	cmp dword ptr [esp], 5
	jl jmp_1002d7c8
jmp_1002d72e:
	mov bl, byte ptr [esi]
	mov byte ptr [edi], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov byte ptr [edi+1], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov byte ptr [edi+2], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov byte ptr [edi+3], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov byte ptr [edi+4], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	mov bl, byte ptr [esi]
	mov byte ptr [edi+5], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	add edi, 6
	sub dword ptr [esp], 6
	js jmp_1002d849
	cmp dword ptr [esp], 5
	jge jmp_1002d72e
jmp_1002d7c8:
	mov bl, byte ptr [esi]
	mov byte ptr [edi], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d849
	mov bl, byte ptr [esi]
	mov byte ptr [edi+1], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d849
	mov bl, byte ptr [esi]
	mov byte ptr [edi+2], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d849
	mov bl, byte ptr [esi]
	mov byte ptr [edi+3], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
	dec dword ptr [esp]
	js jmp_1002d849
	mov bl, byte ptr [esi]
	mov byte ptr [edi+4], bl
	xor ebx, ebx
	add edx, eax
	adc ebx, ebx
	add ecx, ebp
	adc ebx, ebx
	add esi, dword ptr [g_polyVars+0c0h+ebx*4]
jmp_1002d849:
	add esp, 4
	pop ebp
jmp_1002d84d:
	mov edi, dword ptr [g_polyVars+0ch]
	add dword ptr [g_polyVars+18h], edi
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	dec dword ptr [g_polyVars+48h]
	js jmp_1002d8aa
	je jmp_1002d8b0
	dec dword ptr [g_polyVars+40h]
	je jmp_1002d910
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+80h]
	add esi, dword ptr [g_polyVars+88h]
jmp_1002d887:
	dec dword ptr [g_polyVars+44h]
	je jmp_1002d9c9
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+84h]
	add edi, dword ptr [g_polyVars+8ch]
	jmp jmp_1002d125
jmp_1002d8aa:
	pop es
	pop edi
	pop esi
	pop ebx
	leave
	ret
jmp_1002d8b0:
	add eax, dword ptr [g_polyVars+70h]
	add ecx, dword ptr [g_polyVars+80h]
	add esi, dword ptr [g_polyVars+88h]
	add ebx, dword ptr [g_polyVars+74h]
	add edx, dword ptr [g_polyVars+84h]
	add edi, dword ptr [g_polyVars+8ch]
	jmp jmp_1002d125
jmp_1002d8d9:
	add dword ptr [g_polyVars+90h], ecx
	shl ecx, 10h
	mov eax, dword ptr [g_polyVars+9ch]
	imul ecx
	shrd eax, edx, 10h
	add dword ptr [g_polyVars+0a4h], eax
	mov eax, dword ptr [g_polyVars+0a0h]
	imul ecx
	shrd eax, edx, 10h
	add esi, eax
	jmp jmp_1002d225
jmp_1002d905:
	sub dword ptr [g_polyVars+94h], eax
	jmp jmp_1002d236
jmp_1002d910:
	push ebx
	push edx
	mov ebx, dword ptr [g_polyVars+38h]
	mov dword ptr [g_polyVars+30h], ebx
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_polyVars+24h]
	jge jmp_1002d934
	mov esi, dword ptr [g_polyVars+28h]
	sub esi, 18h
jmp_1002d934:
	mov dword ptr [g_polyVars+38h], esi
	mov ecx, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+40h], ecx
	mov edx, dword ptr [esi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+70h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+0ch]
	sub edx, dword ptr [ebx+0ch]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+80h], eax
	mov ecx, dword ptr [g_polyVars+40h]
	mov edx, dword ptr [esi+10h]
	sub edx, dword ptr [ebx+10h]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+88h], eax
	mov eax, dword ptr [ebx]
	shl eax, 10h
	add eax, 8000h
	mov ecx, dword ptr [ebx+0ch]
	add ecx, 8000h
	mov esi, dword ptr [ebx+10h]
	add esi, 8000h
	pop edx
	pop ebx
	jmp jmp_1002d887
jmp_1002d9c9:
	push eax
	push ecx
	mov ebx, dword ptr [g_polyVars+3ch]
	mov dword ptr [g_polyVars+34h], ebx
	mov edi, ebx
	add edi, 18h
	cmp edi, dword ptr [g_polyVars+28h]
	jl jmp_1002d9ea
	mov edi, dword ptr [g_polyVars+24h]
jmp_1002d9ea:
	mov dword ptr [g_polyVars+3ch], edi
	mov ecx, dword ptr [edi+4]
	mov edx, dword ptr [ebx+4]
	sub ecx, edx
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_polyVars+44h], ecx
	mov edx, dword ptr [edi]
	sub edx, dword ptr [ebx]
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+74h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [edi+0ch]
	sub edx, dword ptr [ebx+0ch]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+84h], eax
	mov ecx, dword ptr [g_polyVars+44h]
	mov edx, dword ptr [edi+10h]
	sub edx, dword ptr [ebx+10h]
	xor eax, eax
	shrd eax, edx, 10h
	shl ecx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_polyVars+8ch], eax
	mov edx, dword ptr [ebx+0ch]
	add edx, 8000h
	mov edi, dword ptr [ebx+10h]
	add edi, 8000h
	mov ebx, dword ptr [ebx]
	shl ebx, 10h
	add ebx, 8000h
	pop ecx
	pop eax
	jmp jmp_1002d125
FUN_1002d724 endp

	end
