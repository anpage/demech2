; Hand-written assembly: a MASM object assembled with MASM 6.11 (ML), linked into both DLLs:
; MW2SHELL's starts at 0x10017a7c, right after debugout.c, MW2's at 0x10021cf4, and the two are
; byte for byte the same. It holds a 0x2000-byte routine table, a large block of code that patches
; its own operands at run time (CodeBlock; MW2SHELL 0x10019a7c-0x100286a5) and the routines after
; it. ML aligns its .text to 4, as both starts show.
;
; MW2 draws textured polygons through CallCodeBlockRoutineClipped (FUN_1006dd50). In MW2SHELL the
; object is believed to be dead: the original's relocation table holds no absolute reference into
; 0x10017a7c-0x1002900f from outside it, Ghidra finds no call or jump into it from any other
; function, and .text is read-only with no VirtualProtect import, so the block could not patch
; itself in a Win32 process anyway.
;
; Annotated by name in each DLL's codeblock.h; COMPAT_MODE builds take each DLL's codeblock.c
; stubs. Names follow the MW2SHELL addresses where they are placeholders.

	.386
	.model flat, c
	option noscoped
	option casemap:none

CODE typedef proto
CODEPTR typedef ptr CODE
DATAPTR typedef ptr byte

	.data

; The pair FUN_100286a6 sets. Only the untranscribed code block reads them, so they stay unnamed
; until it is understood.
	public g_unk0x10064cd8
g_unk0x10064cd8 dd 8000h

	public g_unk0x10064cdc
g_unk0x10064cdc dd 0

; The code block's working variables. CallCodeBlockRoutine sets the view (+0x8, +0x10), the vertex
; list's bounds (+0x14, +0x18) and its arguments (+0x98, +0x100 to +0x108).
	public g_codeBlockVars
g_codeBlockVars_t struct
m_data dd 43h dup (0)
g_codeBlockVars_t ends
g_codeBlockVars g_codeBlockVars_t <>

; CallCodeBlockRoutineClipped's clipped polygon: up to 0x100 vertices of six dwords. The clip
; passes alternate between it and the caller's vertex list.
	public g_codeBlockClipVertices
g_codeBlockClipVertices_t struct
m_data dd 600h dup (0)
g_codeBlockClipVertices_t ends
g_codeBlockClipVertices g_codeBlockClipVertices_t <>

; Flags CallCodeBlockRoutineClipped derives from the routine index before clipping, read by its
; edge intersections: 0x400 set; both 0x400 and 0x200 set; a mode among 0x40, 0xc0 and 0x100 in
; bits 6-8, or 4 or 7 in bits 0-2.
	public g_unk0x100665ec
g_unk0x100665ec dd 0

	public g_unk0x100665f0
g_unk0x100665f0 dd 0

	public g_unk0x100665f4
g_unk0x100665f4 dd 0

	.code

; The table of routines CallCodeBlockRoutine dispatches to: 0x800 entries, mostly null, pointing
; into the code block. It lives in .text in the original.
	public g_codeBlockRoutines
g_codeBlockRoutines_t struct
m_unk0x00 dd 40h dup (0)
m_unk0x100 CODEPTR jmp_10026fc9
m_unk0x104 dd 3fh dup (0)
m_unk0x200 CODEPTR jmp_10026947
m_unk0x204 dd 1bfh dup (0)
m_unk0x900 CODEPTR jmp_10028018
m_unk0x904 dd 3fh dup (0)
m_unk0xa00 CODEPTR jmp_10026cd6
m_unk0xa04 dd 3fh dup (0)
m_unk0xb00 CODEPTR jmp_10027aa6
m_unk0xb04 dd 3fh dup (0)
m_unk0xc00 CODEPTR jmp_1002763d
m_unk0xc04 dd 0ffh dup (0)
m_unk0x1000 CODEPTR jmp_10024386
m_unk0x1004 dd 7 dup (0)
m_unk0x1020 CODEPTR jmp_100252b1
m_unk0x1024 dd 7 dup (0)
m_unk0x1040 CODEPTR jmp_10024a82
m_unk0x1044 CODEPTR jmp_1001bafb
m_unk0x1048 dd 3eh dup (0)
m_unk0x1140 CODEPTR jmp_10025ce7
m_unk0x1144 dd 3fh dup (0)
m_unk0x1240 CODEPTR jmp_10023a55
m_unk0x1244 CODEPTR jmp_1001b18e
m_unk0x1248 dd 5 dup (0)
m_unk0x125c CODEPTR jmp_10022ec2
m_unk0x1260 dd 168h dup (0)
m_unk0x1800 CODEPTR jmp_10020b5c
m_unk0x1804 dd 0fh dup (0)
m_unk0x1840 CODEPTR jmp_100214f5
m_unk0x1844 CODEPTR jmp_1001a686
m_unk0x1848 dd 3eh dup (0)
m_unk0x1940 CODEPTR jmp_10021fc1
m_unk0x1944 dd 2fh dup (0)
m_unk0x1a00 CODEPTR jmp_1001cd7c
m_unk0x1a04 dd 8 dup (0)
m_unk0x1a24 CODEPTR jmp_1001e687
m_unk0x1a28 dd 6 dup (0)
m_unk0x1a40 CODEPTR jmp_1001ff8e
m_unk0x1a44 CODEPTR CodeBlock
m_unk0x1a48 dd 5 dup (0)
m_unk0x1a5c CODEPTR jmp_1001d853
m_unk0x1a60 dd 8 dup (0)
m_unk0x1a80 CODEPTR jmp_1001c366
m_unk0x1a84 dd 10h dup (0)
m_unk0x1ac4 CODEPTR jmp_1001f491
m_unk0x1ac8 dd 14eh dup (0)
g_codeBlockRoutines_t ends
g_codeBlockRoutines g_codeBlockRoutines_t <>

; The self-modifying code block, entered through g_codeBlockRoutines (CallCodeBlockRoutine and
; CallCodeBlockRoutineClipped). Its routines haven't been told apart, so it is one procedure.
CodeBlock proc
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10019f40+3], eax
	je jmp_10019ae8
	mov dword ptr es:[jmp_10019f40+3], eax
	mov dword ptr es:[jmp_10019f8e+3], eax
	mov dword ptr es:[jmp_10019fd0+3], eax
	mov dword ptr es:[jmp_1001a012+3], eax
	mov dword ptr es:[jmp_1001a054+3], eax
	mov dword ptr es:[jmp_1001a096+3], eax
	mov dword ptr es:[jmp_1001a0d8+3], eax
	mov dword ptr es:[jmp_1001a11a+3], eax
	mov dword ptr es:[jmp_1001a15c+3], eax
	mov dword ptr es:[jmp_1001a1c6+3], eax
	mov dword ptr es:[jmp_1001a214+3], eax
	mov dword ptr es:[jmp_1001a262+3], eax
	mov dword ptr es:[jmp_1001a2b0+3], eax
	mov dword ptr es:[jmp_1001a2fe+3], eax
	mov dword ptr es:[jmp_1001a348+3], eax
jmp_10019ae8:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_10019f66+2], eax
	je jmp_10019b62
	mov dword ptr es:[jmp_10019f66+2], eax
	mov dword ptr es:[jmp_10019fa7+2], eax
	mov dword ptr es:[jmp_10019fe9+2], eax
	mov dword ptr es:[jmp_1001a02b+2], eax
	mov dword ptr es:[jmp_1001a06d+2], eax
	mov dword ptr es:[jmp_1001a0af+2], eax
	mov dword ptr es:[jmp_1001a0f1+2], eax
	mov dword ptr es:[jmp_1001a133+2], eax
	mov dword ptr es:[jmp_1001a192+2], eax
	mov dword ptr es:[jmp_1001a1df+2], eax
	mov dword ptr es:[jmp_1001a22d+2], eax
	mov dword ptr es:[jmp_1001a27b+2], eax
	mov dword ptr es:[jmp_1001a2c9+2], eax
	mov dword ptr es:[jmp_1001a317+2], eax
	mov dword ptr es:[jmp_1001a361+2], eax
jmp_10019b62:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10019b72:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10019b7d
	mov esi, eax
	mov ecx, ebx
jmp_10019b7d:
	cmp eax, edi
	jl jmp_10019b83
	mov edi, eax
jmp_10019b83:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10019b72
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001a684
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10019bae:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10019bca
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10019bca:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10019bae
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_10019c65:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10019c7e
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10019c7e:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10019c65
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10019d2a:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_10019d6e
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_10019d86
jmp_10019d6e:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_10019d86:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001a3a5
jmp_10019e66:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_10019eed:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10019f40:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001a180
jmp_10019f54:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10019f66:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10019f72
	mov byte ptr [edi], al
jmp_10019f72:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10019f8e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10019fa7:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10019fb4
	mov byte ptr [edi+1], al
jmp_10019fb4:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10019fd0:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10019fe9:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10019ff6
	mov byte ptr [edi+2], al
jmp_10019ff6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a012:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a02b:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a038
	mov byte ptr [edi+3], al
jmp_1001a038:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a054:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a06d:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a07a
	mov byte ptr [edi+4], al
jmp_1001a07a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a096:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a0af:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a0bc
	mov byte ptr [edi+5], al
jmp_1001a0bc:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a0d8:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a0f1:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a0fe
	mov byte ptr [edi+6], al
jmp_1001a0fe:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a11a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a133:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a140
	mov byte ptr [edi+7], al
jmp_1001a140:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a15c:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001a37a
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10019f54
jmp_1001a180:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a192:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a19e
	mov byte ptr [edi], al
jmp_1001a19e:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001a37a
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a1c6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a1df:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a1ec
	mov byte ptr [edi+1], al
jmp_1001a1ec:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001a37a
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a214:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a22d:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a23a
	mov byte ptr [edi+2], al
jmp_1001a23a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001a37a
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a262:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a27b:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a288
	mov byte ptr [edi+3], al
jmp_1001a288:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001a37a
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a2b0:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a2c9:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a2d6
	mov byte ptr [edi+4], al
jmp_1001a2d6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001a37a
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a2fe:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a317:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a324
	mov byte ptr [edi+5], al
jmp_1001a324:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001a37a
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001a348:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001a361:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001a36e
	mov byte ptr [edi+6], al
jmp_1001a36e:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001a37a:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001a41c
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_10019e66
jmp_1001a3a5:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001a41c
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_10019eed
jmp_1001a41c:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001a684
	je jmp_1001a4aa
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001a507
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001a46d:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001a5c7
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_10019d2a
jmp_1001a4aa:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_10019d2a
jmp_1001a507:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001a523
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001a523:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001a46d
jmp_1001a5c7:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001a5e0
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001a5e0:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_10019d2a
jmp_1001a684:
	pop ebp
	ret
jmp_1001a686:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001aae2+3], eax
	je jmp_1001a6f2
	mov dword ptr es:[jmp_1001aae2+3], eax
	mov dword ptr es:[jmp_1001ab26+3], eax
	mov dword ptr es:[jmp_1001ab5e+3], eax
	mov dword ptr es:[jmp_1001ab96+3], eax
	mov dword ptr es:[jmp_1001abce+3], eax
	mov dword ptr es:[jmp_1001ac06+3], eax
	mov dword ptr es:[jmp_1001ac3e+3], eax
	mov dword ptr es:[jmp_1001ac76+3], eax
	mov dword ptr es:[jmp_1001acae+3], eax
	mov dword ptr es:[jmp_1001ad0e+3], eax
	mov dword ptr es:[jmp_1001ad52+3], eax
	mov dword ptr es:[jmp_1001ad96+3], eax
	mov dword ptr es:[jmp_1001adda+3], eax
	mov dword ptr es:[jmp_1001ae1a+3], eax
	mov dword ptr es:[jmp_1001ae5a+3], eax
jmp_1001a6f2:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001a714:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001a71f
	mov esi, eax
	mov ecx, ebx
jmp_1001a71f:
	cmp eax, edi
	jl jmp_1001a725
	mov edi, eax
jmp_1001a725:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001a714
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001b18c
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001a750:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001a76c
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001a76c:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001a750
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_1001a807:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001a820
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001a820:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001a807
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001a8cc:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1001a910
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_1001a928
jmp_1001a910:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_1001a928:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001aead
jmp_1001aa08:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1001aa8f:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001aae2:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001acd2
jmp_1001aaf6:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ab0a
	mov byte ptr [edi], al
jmp_1001ab0a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ab26:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ab42
	mov byte ptr [edi+1], al
jmp_1001ab42:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ab5e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ab7a
	mov byte ptr [edi+2], al
jmp_1001ab7a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ab96:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001abb2
	mov byte ptr [edi+3], al
jmp_1001abb2:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001abce:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001abea
	mov byte ptr [edi+4], al
jmp_1001abea:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ac06:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ac22
	mov byte ptr [edi+5], al
jmp_1001ac22:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ac3e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ac5a
	mov byte ptr [edi+6], al
jmp_1001ac5a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ac76:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ac92
	mov byte ptr [edi+7], al
jmp_1001ac92:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001acae:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001ae82
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001aaf6
jmp_1001acd2:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ace6
	mov byte ptr [edi], al
jmp_1001ace6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ae82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ad0e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ad2a
	mov byte ptr [edi+1], al
jmp_1001ad2a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ae82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ad52:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ad6e
	mov byte ptr [edi+2], al
jmp_1001ad6e:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ae82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ad96:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001adb2
	mov byte ptr [edi+3], al
jmp_1001adb2:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ae82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001adda:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001adf6
	mov byte ptr [edi+4], al
jmp_1001adf6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ae82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ae1a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ae36
	mov byte ptr [edi+5], al
jmp_1001ae36:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ae82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ae5a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001ae76
	mov byte ptr [edi+6], al
jmp_1001ae76:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001ae82:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001af24
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_1001aa08
jmp_1001aead:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001af24
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1001aa8f
jmp_1001af24:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001b18c
	je jmp_1001afb2
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001b00f
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001af75:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001b0cf
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001a8cc
jmp_1001afb2:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001a8cc
jmp_1001b00f:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001b02b
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001b02b:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001af75
jmp_1001b0cf:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001b0e8
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001b0e8:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1001a8cc
jmp_1001b18c:
	pop ebp
	ret
jmp_1001b18e:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001b4bd+3], eax
	je jmp_1001b1fa
	mov dword ptr es:[jmp_1001b4bd+3], eax
	mov dword ptr es:[jmp_1001b50b+3], eax
	mov dword ptr es:[jmp_1001b54d+3], eax
	mov dword ptr es:[jmp_1001b58f+3], eax
	mov dword ptr es:[jmp_1001b5d1+3], eax
	mov dword ptr es:[jmp_1001b613+3], eax
	mov dword ptr es:[jmp_1001b655+3], eax
	mov dword ptr es:[jmp_1001b697+3], eax
	mov dword ptr es:[jmp_1001b6d9+3], eax
	mov dword ptr es:[jmp_1001b743+3], eax
	mov dword ptr es:[jmp_1001b791+3], eax
	mov dword ptr es:[jmp_1001b7df+3], eax
	mov dword ptr es:[jmp_1001b82d+3], eax
	mov dword ptr es:[jmp_1001b87b+3], eax
	mov dword ptr es:[jmp_1001b8c5+3], eax
jmp_1001b1fa:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_1001b4e3+2], eax
	je jmp_1001b274
	mov dword ptr es:[jmp_1001b4e3+2], eax
	mov dword ptr es:[jmp_1001b524+2], eax
	mov dword ptr es:[jmp_1001b566+2], eax
	mov dword ptr es:[jmp_1001b5a8+2], eax
	mov dword ptr es:[jmp_1001b5ea+2], eax
	mov dword ptr es:[jmp_1001b62c+2], eax
	mov dword ptr es:[jmp_1001b66e+2], eax
	mov dword ptr es:[jmp_1001b6b0+2], eax
	mov dword ptr es:[jmp_1001b70f+2], eax
	mov dword ptr es:[jmp_1001b75c+2], eax
	mov dword ptr es:[jmp_1001b7aa+2], eax
	mov dword ptr es:[jmp_1001b7f8+2], eax
	mov dword ptr es:[jmp_1001b846+2], eax
	mov dword ptr es:[jmp_1001b894+2], eax
	mov dword ptr es:[jmp_1001b8de+2], eax
jmp_1001b274:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001b284:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001b28f
	mov esi, eax
	mov ecx, ebx
jmp_1001b28f:
	cmp eax, edi
	jl jmp_1001b295
	mov edi, eax
jmp_1001b295:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001b284
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001baf9
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001b2c0:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001b2dc
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001b2dc:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001b2c0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_1001b359:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001b372
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001b372:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001b359
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001b400:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_1001b42c
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_1001b42c:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b4bd:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001b6fd
jmp_1001b4d1:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b4e3:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b4ef
	mov byte ptr [edi], al
jmp_1001b4ef:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b50b:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b524:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b531
	mov byte ptr [edi+1], al
jmp_1001b531:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b54d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b566:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b573
	mov byte ptr [edi+2], al
jmp_1001b573:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b58f:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b5a8:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b5b5
	mov byte ptr [edi+3], al
jmp_1001b5b5:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b5d1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b5ea:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b5f7
	mov byte ptr [edi+4], al
jmp_1001b5f7:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b613:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b62c:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b639
	mov byte ptr [edi+5], al
jmp_1001b639:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b655:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b66e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b67b
	mov byte ptr [edi+6], al
jmp_1001b67b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b697:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b6b0:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b6bd
	mov byte ptr [edi+7], al
jmp_1001b6bd:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b6d9:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001b8f9
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001b4d1
jmp_1001b6fd:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b70f:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b71b
	mov byte ptr [edi], al
jmp_1001b71b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001b8f9
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b743:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b75c:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b769
	mov byte ptr [edi+1], al
jmp_1001b769:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001b8f9
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b791:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b7aa:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b7b7
	mov byte ptr [edi+2], al
jmp_1001b7b7:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001b8f9
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b7df:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b7f8:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b805
	mov byte ptr [edi+3], al
jmp_1001b805:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001b8f9
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b82d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b846:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b853
	mov byte ptr [edi+4], al
jmp_1001b853:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001b8f9
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b87b:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b894:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b8a1
	mov byte ptr [edi+5], al
jmp_1001b8a1:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001b8f9
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001b8c5:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001b8de:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001b8eb
	mov byte ptr [edi+6], al
jmp_1001b8eb:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_1001b8f9
jmp_1001b8f9:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001baf9
	je jmp_1001b971
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001b9b8
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_1001b93f:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001ba5a
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_1001b400
jmp_1001b971:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_1001b400
jmp_1001b9b8:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001b9d4
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001b9d4:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_1001b93f
jmp_1001ba5a:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001ba73
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001ba73:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_1001b400
jmp_1001baf9:
	pop ebp
	ret
jmp_1001bafb:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001bdc2+3], eax
	je jmp_1001bb67
	mov dword ptr es:[jmp_1001bdc2+3], eax
	mov dword ptr es:[jmp_1001be06+3], eax
	mov dword ptr es:[jmp_1001be3e+3], eax
	mov dword ptr es:[jmp_1001be76+3], eax
	mov dword ptr es:[jmp_1001beae+3], eax
	mov dword ptr es:[jmp_1001bee6+3], eax
	mov dword ptr es:[jmp_1001bf1e+3], eax
	mov dword ptr es:[jmp_1001bf56+3], eax
	mov dword ptr es:[jmp_1001bf8e+3], eax
	mov dword ptr es:[jmp_1001bfee+3], eax
	mov dword ptr es:[jmp_1001c032+3], eax
	mov dword ptr es:[jmp_1001c076+3], eax
	mov dword ptr es:[jmp_1001c0ba+3], eax
	mov dword ptr es:[jmp_1001c0fa+3], eax
	mov dword ptr es:[jmp_1001c13a+3], eax
jmp_1001bb67:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001bb89:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001bb94
	mov esi, eax
	mov ecx, ebx
jmp_1001bb94:
	cmp eax, edi
	jl jmp_1001bb9a
	mov edi, eax
jmp_1001bb9a:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001bb89
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001c364
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001bbc5:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001bbe1
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001bbe1:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001bbc5
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_1001bc5e:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001bc77
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001bc77:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001bc5e
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001bd05:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_1001bd31
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_1001bd31:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001bdc2:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001bfb2
jmp_1001bdd6:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001bdea
	mov byte ptr [edi], al
jmp_1001bdea:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001be06:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001be22
	mov byte ptr [edi+1], al
jmp_1001be22:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001be3e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001be5a
	mov byte ptr [edi+2], al
jmp_1001be5a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001be76:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001be92
	mov byte ptr [edi+3], al
jmp_1001be92:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001beae:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001beca
	mov byte ptr [edi+4], al
jmp_1001beca:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001bee6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001bf02
	mov byte ptr [edi+5], al
jmp_1001bf02:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001bf1e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001bf3a
	mov byte ptr [edi+6], al
jmp_1001bf3a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001bf56:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001bf72
	mov byte ptr [edi+7], al
jmp_1001bf72:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001bf8e:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001c164
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001bdd6
jmp_1001bfb2:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001bfc6
	mov byte ptr [edi], al
jmp_1001bfc6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001c164
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001bfee:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001c00a
	mov byte ptr [edi+1], al
jmp_1001c00a:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001c164
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001c032:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001c04e
	mov byte ptr [edi+2], al
jmp_1001c04e:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001c164
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001c076:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001c092
	mov byte ptr [edi+3], al
jmp_1001c092:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001c164
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001c0ba:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001c0d6
	mov byte ptr [edi+4], al
jmp_1001c0d6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001c164
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001c0fa:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001c116
	mov byte ptr [edi+5], al
jmp_1001c116:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001c164
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001c13a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	cmp al, 0ffh
	je jmp_1001c156
	mov byte ptr [edi+6], al
jmp_1001c156:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_1001c164
jmp_1001c164:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001c364
	je jmp_1001c1dc
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001c223
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_1001c1aa:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001c2c5
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_1001bd05
jmp_1001c1dc:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_1001bd05
jmp_1001c223:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001c23f
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001c23f:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_1001c1aa
jmp_1001c2c5:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001c2de
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001c2de:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_1001bd05
jmp_1001c364:
	pop ebp
	ret
jmp_1001c366:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001c7f7+3], eax
	je jmp_1001c3ba
	mov dword ptr es:[jmp_1001c7f7+3], eax
	mov dword ptr es:[jmp_1001c835+3], eax
	mov dword ptr es:[jmp_1001c867+3], eax
	mov dword ptr es:[jmp_1001c899+3], eax
	mov dword ptr es:[jmp_1001c8cb+3], eax
	mov dword ptr es:[jmp_1001c922+3], eax
	mov dword ptr es:[jmp_1001c95d+3], eax
	mov dword ptr es:[jmp_1001c998+3], eax
	mov dword ptr es:[jmp_1001c9d3+3], eax
	mov dword ptr es:[jmp_1001ca0a+3], eax
	mov dword ptr es:[jmp_1001ca41+3], eax
jmp_1001c3ba:
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_1001c81e+2], eax
	je jmp_1001c40a
	mov dword ptr es:[jmp_1001c81e+2], eax
	mov dword ptr es:[jmp_1001c84f+2], eax
	mov dword ptr es:[jmp_1001c881+2], eax
	mov dword ptr es:[jmp_1001c8b3+2], eax
	mov dword ptr es:[jmp_1001c902+2], eax
	mov dword ptr es:[jmp_1001c93c+2], eax
	mov dword ptr es:[jmp_1001c977+2], eax
	mov dword ptr es:[jmp_1001c9b2+2], eax
	mov dword ptr es:[jmp_1001c9ed+2], eax
	mov dword ptr es:[jmp_1001ca24+2], eax
	mov dword ptr es:[jmp_1001ca5b+2], eax
jmp_1001c40a:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001c41a:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001c425
	mov esi, eax
	mov ecx, ebx
jmp_1001c425:
	cmp eax, edi
	jl jmp_1001c42b
	mov edi, eax
jmp_1001c42b:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001c41a
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001cd7a
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001c456:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001c472
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001c472:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001c456
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_1001c50d:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001c526
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001c526:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001c50d
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001c5d2:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1001c616
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_1001c62e
jmp_1001c616:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_1001c62e:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001ca9b
jmp_1001c70e:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1001c795:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	shl ecx, 1
	rcl edx, 1
	shl ebx, 1
	rcl eax, 1
	mov dword ptr [g_codeBlockVars+0d8h], ecx
	mov dword ptr [g_codeBlockVars+0dch], edx
	mov dword ptr [g_codeBlockVars+0e0h], ebx
	mov dword ptr [g_codeBlockVars+0e4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
jmp_1001c7f7:
	mov eax, dword ptr [ecx*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001c8ef
jmp_1001c80b:
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c81e:
	mov al, byte ptr [ebx-8000h]
	mov ah, al
	mov word ptr [edi], ax
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
jmp_1001c835:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c84f:
	mov al, byte ptr [ebx-8000h]
	mov ah, al
	mov word ptr [edi+2], ax
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
jmp_1001c867:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c881:
	mov al, byte ptr [ebx-8000h]
	mov ah, al
	mov word ptr [edi+4], ax
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
jmp_1001c899:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c8b3:
	mov al, byte ptr [ebx-8000h]
	mov ah, al
	mov word ptr [edi+6], ax
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
jmp_1001c8cb:
	mov eax, dword ptr [ecx*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001ca70
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001c80b
jmp_1001c8ef:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c902:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ca70
jmp_1001c922:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c93c:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ca70
jmp_1001c95d:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c977:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ca70
jmp_1001c998:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c9b2:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ca70
jmp_1001c9d3:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001c9ed:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ca70
jmp_1001ca0a:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ca24:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001ca70
jmp_1001ca41:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ca5b:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001ca70:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001cb12
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_1001c70e
jmp_1001ca9b:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001cb12
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1001c795
jmp_1001cb12:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001cd7a
	je jmp_1001cba0
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001cbfd
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001cb63:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001ccbd
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001c5d2
jmp_1001cba0:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001c5d2
jmp_1001cbfd:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001cc19
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001cc19:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001cb63
jmp_1001ccbd:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001ccd6
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001ccd6:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1001c5d2
jmp_1001cd7a:
	pop ebp
	ret
jmp_1001cd7c:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001d21e+3], eax
	je jmp_1001cde8
	mov dword ptr es:[jmp_1001d21e+3], eax
	mov dword ptr es:[jmp_1001d259+3], eax
	mov dword ptr es:[jmp_1001d288+3], eax
	mov dword ptr es:[jmp_1001d2b7+3], eax
	mov dword ptr es:[jmp_1001d2e6+3], eax
	mov dword ptr es:[jmp_1001d315+3], eax
	mov dword ptr es:[jmp_1001d344+3], eax
	mov dword ptr es:[jmp_1001d373+3], eax
	mov dword ptr es:[jmp_1001d3a2+3], eax
	mov dword ptr es:[jmp_1001d3f9+3], eax
	mov dword ptr es:[jmp_1001d434+3], eax
	mov dword ptr es:[jmp_1001d46f+3], eax
	mov dword ptr es:[jmp_1001d4aa+3], eax
	mov dword ptr es:[jmp_1001d4e1+3], eax
	mov dword ptr es:[jmp_1001d518+3], eax
jmp_1001cde8:
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_1001d245+2], eax
	je jmp_1001ce50
	mov dword ptr es:[jmp_1001d245+2], eax
	mov dword ptr es:[jmp_1001d273+2], eax
	mov dword ptr es:[jmp_1001d2a2+2], eax
	mov dword ptr es:[jmp_1001d2d1+2], eax
	mov dword ptr es:[jmp_1001d300+2], eax
	mov dword ptr es:[jmp_1001d32f+2], eax
	mov dword ptr es:[jmp_1001d35e+2], eax
	mov dword ptr es:[jmp_1001d38d+2], eax
	mov dword ptr es:[jmp_1001d3d9+2], eax
	mov dword ptr es:[jmp_1001d413+2], eax
	mov dword ptr es:[jmp_1001d44e+2], eax
	mov dword ptr es:[jmp_1001d489+2], eax
	mov dword ptr es:[jmp_1001d4c4+2], eax
	mov dword ptr es:[jmp_1001d4fb+2], eax
	mov dword ptr es:[jmp_1001d532+2], eax
jmp_1001ce50:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001ce60:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001ce6b
	mov esi, eax
	mov ecx, ebx
jmp_1001ce6b:
	cmp eax, edi
	jl jmp_1001ce71
	mov edi, eax
jmp_1001ce71:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001ce60
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001d851
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001ce9c:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001ceb8
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001ceb8:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001ce9c
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_1001cf53:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001cf6c
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001cf6c:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001cf53
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001d018:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1001d05c
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_1001d074
jmp_1001d05c:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_1001d074:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001d572
jmp_1001d154:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1001d1db:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
jmp_1001d21e:
	mov eax, dword ptr [ecx*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001d3c6
jmp_1001d232:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d245:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d259:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d273:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d288:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d2a2:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d2b7:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d2d1:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d2e6:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d300:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d315:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d32f:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d344:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d35e:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d373:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d38d:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d3a2:
	mov eax, dword ptr [ecx*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001d547
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001d232
jmp_1001d3c6:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d3d9:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001d547
jmp_1001d3f9:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d413:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001d547
jmp_1001d434:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d44e:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001d547
jmp_1001d46f:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d489:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001d547
jmp_1001d4aa:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d4c4:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001d547
jmp_1001d4e1:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d4fb:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001d547
jmp_1001d518:
	mov eax, dword ptr [ecx*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001d532:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001d547:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001d5e9
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_1001d154
jmp_1001d572:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001d5e9
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1001d1db
jmp_1001d5e9:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001d851
	je jmp_1001d677
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001d6d4
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001d63a:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001d794
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001d018
jmp_1001d677:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001d018
jmp_1001d6d4:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001d6f0
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001d6f0:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001d63a
jmp_1001d794:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001d7ad
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001d7ad:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1001d018
jmp_1001d851:
	pop ebp
	ret
jmp_1001d853:
	push ebp
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001ded5+3], eax
	je jmp_1001d8d3
	mov dword ptr es:[jmp_1001ded5+3], eax
	mov dword ptr es:[jmp_1001df23+3], eax
	mov dword ptr es:[jmp_1001df65+3], eax
	mov dword ptr es:[jmp_1001dfa7+3], eax
	mov dword ptr es:[jmp_1001dfe9+3], eax
	mov dword ptr es:[jmp_1001e02b+3], eax
	mov dword ptr es:[jmp_1001e06d+3], eax
	mov dword ptr es:[jmp_1001e0af+3], eax
	mov dword ptr es:[jmp_1001e0f1+3], eax
	mov dword ptr es:[jmp_1001e15b+3], eax
	mov dword ptr es:[jmp_1001e1a9+3], eax
	mov dword ptr es:[jmp_1001e1f7+3], eax
	mov dword ptr es:[jmp_1001e245+3], eax
	mov dword ptr es:[jmp_1001e293+3], eax
	mov dword ptr es:[jmp_1001e2dd+3], eax
jmp_1001d8d3:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_1001defb+2], eax
	je jmp_1001d94d
	mov dword ptr es:[jmp_1001defb+2], eax
	mov dword ptr es:[jmp_1001df3c+2], eax
	mov dword ptr es:[jmp_1001df7e+2], eax
	mov dword ptr es:[jmp_1001dfc0+2], eax
	mov dword ptr es:[jmp_1001e002+2], eax
	mov dword ptr es:[jmp_1001e044+2], eax
	mov dword ptr es:[jmp_1001e086+2], eax
	mov dword ptr es:[jmp_1001e0c8+2], eax
	mov dword ptr es:[jmp_1001e127+2], eax
	mov dword ptr es:[jmp_1001e174+2], eax
	mov dword ptr es:[jmp_1001e1c2+2], eax
	mov dword ptr es:[jmp_1001e210+2], eax
	mov dword ptr es:[jmp_1001e25e+2], eax
	mov dword ptr es:[jmp_1001e2ac+2], eax
	mov dword ptr es:[jmp_1001e2f6+2], eax
jmp_1001d94d:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001d95d:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001d968
	mov esi, eax
	mov ecx, ebx
jmp_1001d968:
	cmp eax, edi
	jl jmp_1001d96e
	mov edi, eax
jmp_1001d96e:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001d95d
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001e685
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001d999:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001d9b5
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001d9b5:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001d999
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_1001da6e:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001da87
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001da87:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001da6e
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001db51:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_1001db6f
	xchg ebx, eax
	xchg ecx, edx
jmp_1001db6f:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_1001dba1
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
jmp_1001dba1:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test edi, 1
	je jmp_1001dbeb
	mov byte ptr [edi], cl
	dec esi
	je jmp_1001dca9
	inc edi
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1001dbeb:
	cmp esi, 1
	je jmp_1001dca1
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_1001dc51
jmp_1001dbfd:
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
	js jmp_1001dc98
	cmp esi, 5
	jge jmp_1001dbfd
jmp_1001dc51:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1001dc98
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1001dc98
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1001dc98
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_1001dc98
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_1001dc98:
	pop esi
	test esi, 1
	je jmp_1001dca9
jmp_1001dca1:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	mov byte ptr [edi], cl
jmp_1001dca9:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1001dd03
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_1001dd1b
jmp_1001dd03:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_1001dd1b:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001e33a
jmp_1001ddfb:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1001de82:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001ded5:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001e115
jmp_1001dee9:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001defb:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001df07
	mov byte ptr [edi], al
jmp_1001df07:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001df23:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001df3c:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001df49
	mov byte ptr [edi+1], al
jmp_1001df49:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001df65:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001df7e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001df8b
	mov byte ptr [edi+2], al
jmp_1001df8b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001dfa7:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001dfc0:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001dfcd
	mov byte ptr [edi+3], al
jmp_1001dfcd:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001dfe9:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e002:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e00f
	mov byte ptr [edi+4], al
jmp_1001e00f:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e02b:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e044:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e051
	mov byte ptr [edi+5], al
jmp_1001e051:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e06d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e086:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e093
	mov byte ptr [edi+6], al
jmp_1001e093:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e0af:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e0c8:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e0d5
	mov byte ptr [edi+7], al
jmp_1001e0d5:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e0f1:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001e30f
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001dee9
jmp_1001e115:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e127:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e133
	mov byte ptr [edi], al
jmp_1001e133:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001e30f
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e15b:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e174:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e181
	mov byte ptr [edi+1], al
jmp_1001e181:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001e30f
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e1a9:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e1c2:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e1cf
	mov byte ptr [edi+2], al
jmp_1001e1cf:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001e30f
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e1f7:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e210:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e21d
	mov byte ptr [edi+3], al
jmp_1001e21d:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001e30f
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e245:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e25e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e26b
	mov byte ptr [edi+4], al
jmp_1001e26b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001e30f
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e293:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e2ac:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e2b9
	mov byte ptr [edi+5], al
jmp_1001e2b9:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001e30f
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001e2dd:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001e2f6:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001e303
	mov byte ptr [edi+6], al
jmp_1001e303:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001e30f:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001e3b1
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_1001ddfb
jmp_1001e33a:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001e3b1
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1001de82
jmp_1001e3b1:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001e685
	je jmp_1001e459
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001e4cc
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001e411:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001e5aa
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001db51
jmp_1001e459:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001db51
jmp_1001e4cc:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001e4e8
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001e4e8:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001e411
jmp_1001e5aa:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001e5c3
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001e5c3:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1001db51
jmp_1001e685:
	pop ebp
	ret
jmp_1001e687:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001eb55+3], eax
	je jmp_1001e6f3
	mov dword ptr es:[jmp_1001eb55+3], eax
	mov dword ptr es:[jmp_1001ebaf+3], eax
	mov dword ptr es:[jmp_1001ebfd+3], eax
	mov dword ptr es:[jmp_1001ec4b+3], eax
	mov dword ptr es:[jmp_1001ec99+3], eax
	mov dword ptr es:[jmp_1001ece7+3], eax
	mov dword ptr es:[jmp_1001ed35+3], eax
	mov dword ptr es:[jmp_1001ed83+3], eax
	mov dword ptr es:[jmp_1001edd1+3], eax
	mov dword ptr es:[jmp_1001ee47+3], eax
	mov dword ptr es:[jmp_1001eea1+3], eax
	mov dword ptr es:[jmp_1001eefb+3], eax
	mov dword ptr es:[jmp_1001ef55+3], eax
	mov dword ptr es:[jmp_1001efaf+3], eax
	mov dword ptr es:[jmp_1001f005+3], eax
jmp_1001e6f3:
	mov eax, dword ptr [ebx+4]
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_1001eb7b+2], eax
	je jmp_1001e76b
	mov dword ptr es:[jmp_1001eb7b+2], eax
	mov dword ptr es:[jmp_1001ebc8+2], eax
	mov dword ptr es:[jmp_1001ec16+2], eax
	mov dword ptr es:[jmp_1001ec64+2], eax
	mov dword ptr es:[jmp_1001ecb2+2], eax
	mov dword ptr es:[jmp_1001ed00+2], eax
	mov dword ptr es:[jmp_1001ed4e+2], eax
	mov dword ptr es:[jmp_1001ed9c+2], eax
	mov dword ptr es:[jmp_1001ee07+2], eax
	mov dword ptr es:[jmp_1001ee60+2], eax
	mov dword ptr es:[jmp_1001eeba+2], eax
	mov dword ptr es:[jmp_1001ef14+2], eax
	mov dword ptr es:[jmp_1001ef6e+2], eax
	mov dword ptr es:[jmp_1001efc8+2], eax
	mov dword ptr es:[jmp_1001f01e+2], eax
jmp_1001e76b:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001e77b:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001e786
	mov esi, eax
	mov ecx, ebx
jmp_1001e786:
	cmp eax, edi
	jl jmp_1001e78c
	mov edi, eax
jmp_1001e78c:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001e77b
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001f48f
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001e7b7:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001e7d3
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001e7d3:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001e7b7
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_1001e86e:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001e887
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001e887:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001e86e
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001e933:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1001e977
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_1001e98f
jmp_1001e977:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_1001e98f:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001f062
jmp_1001ea6f:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1001eaf6:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
jmp_1001eb3d:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f0dd
jmp_1001eb49:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f0e8
jmp_1001eb55:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001edf5
jmp_1001eb69:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001eb7b:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001eb87
	mov byte ptr [edi], al
jmp_1001eb87:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001eb97:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f0f3
jmp_1001eba3:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f0fe
jmp_1001ebaf:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ebc8:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ebd5
	mov byte ptr [edi+1], al
jmp_1001ebd5:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001ebe5:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f109
jmp_1001ebf1:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f114
jmp_1001ebfd:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ec16:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ec23
	mov byte ptr [edi+2], al
jmp_1001ec23:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001ec33:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f11f
jmp_1001ec3f:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f12a
jmp_1001ec4b:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ec64:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ec71
	mov byte ptr [edi+3], al
jmp_1001ec71:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001ec81:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f135
jmp_1001ec8d:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f140
jmp_1001ec99:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ecb2:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ecbf
	mov byte ptr [edi+4], al
jmp_1001ecbf:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001eccf:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f14b
jmp_1001ecdb:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f156
jmp_1001ece7:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ed00:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ed0d
	mov byte ptr [edi+5], al
jmp_1001ed0d:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001ed1d:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f161
jmp_1001ed29:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f16c
jmp_1001ed35:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ed4e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ed5b
	mov byte ptr [edi+6], al
jmp_1001ed5b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001ed6b:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f177
jmp_1001ed77:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f182
jmp_1001ed83:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ed9c:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001eda9
	mov byte ptr [edi+7], al
jmp_1001eda9:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_1001edb9:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f18d
jmp_1001edc5:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f198
jmp_1001edd1:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001f037
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001eb69
jmp_1001edf5:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ee07:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ee13
	mov byte ptr [edi], al
jmp_1001ee13:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001f037
	mov eax, ecx
	mov ebx, edx
jmp_1001ee2f:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f1a3
jmp_1001ee3b:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f1ae
jmp_1001ee47:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ee60:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ee6d
	mov byte ptr [edi+1], al
jmp_1001ee6d:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001f037
	mov eax, ecx
	mov ebx, edx
jmp_1001ee89:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f1b9
jmp_1001ee95:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f1c4
jmp_1001eea1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001eeba:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001eec7
	mov byte ptr [edi+2], al
jmp_1001eec7:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001f037
	mov eax, ecx
	mov ebx, edx
jmp_1001eee3:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f1cf
jmp_1001eeef:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f1da
jmp_1001eefb:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ef14:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ef21
	mov byte ptr [edi+3], al
jmp_1001ef21:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001f037
	mov eax, ecx
	mov ebx, edx
jmp_1001ef3d:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f1e5
jmp_1001ef49:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f1f0
jmp_1001ef55:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001ef6e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001ef7b
	mov byte ptr [edi+4], al
jmp_1001ef7b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001f037
	mov eax, ecx
	mov ebx, edx
jmp_1001ef97:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f1fb
jmp_1001efa3:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f206
jmp_1001efaf:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001efc8:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001efd5
	mov byte ptr [edi+5], al
jmp_1001efd5:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001f037
	mov eax, ecx
	mov ebx, edx
jmp_1001efed:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1001f211
jmp_1001eff9:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_1001f21c
jmp_1001f005:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001f01e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001f02b
	mov byte ptr [edi+6], al
jmp_1001f02b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001f037:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001f227
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_1001ea6f
jmp_1001f062:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001f227
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1001eaf6
jmp_1001f0dd:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001eb3d
jmp_1001f0e8:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001eb49
jmp_1001f0f3:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001eb97
jmp_1001f0fe:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001eba3
jmp_1001f109:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ebe5
jmp_1001f114:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ebf1
jmp_1001f11f:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ec33
jmp_1001f12a:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ec3f
jmp_1001f135:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ec81
jmp_1001f140:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ec8d
jmp_1001f14b:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001eccf
jmp_1001f156:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ecdb
jmp_1001f161:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ed1d
jmp_1001f16c:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ed29
jmp_1001f177:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ed6b
jmp_1001f182:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ed77
jmp_1001f18d:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001edb9
jmp_1001f198:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001edc5
jmp_1001f1a3:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ee2f
jmp_1001f1ae:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ee3b
jmp_1001f1b9:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ee89
jmp_1001f1c4:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ee95
jmp_1001f1cf:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001eee3
jmp_1001f1da:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001eeef
jmp_1001f1e5:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ef3d
jmp_1001f1f0:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001ef49
jmp_1001f1fb:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001ef97
jmp_1001f206:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001efa3
jmp_1001f211:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1001efed
jmp_1001f21c:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1001eff9
jmp_1001f227:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001f48f
	je jmp_1001f2b5
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001f312
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001f278:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001f3d2
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001e933
jmp_1001f2b5:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001e933
jmp_1001f312:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001f32e
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001f32e:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001f278
jmp_1001f3d2:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001f3eb
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001f3eb:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1001e933
jmp_1001f48f:
	pop ebp
	ret
jmp_1001f491:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1001f944+3], eax
	je jmp_1001f4e5
	mov dword ptr es:[jmp_1001f944+3], eax
	mov dword ptr es:[jmp_1001f995+3], eax
	mov dword ptr es:[jmp_1001f9da+3], eax
	mov dword ptr es:[jmp_1001fa1f+3], eax
	mov dword ptr es:[jmp_1001fa64+3], eax
	mov dword ptr es:[jmp_1001face+3], eax
	mov dword ptr es:[jmp_1001fb1c+3], eax
	mov dword ptr es:[jmp_1001fb6a+3], eax
	mov dword ptr es:[jmp_1001fbb8+3], eax
	mov dword ptr es:[jmp_1001fc06+3], eax
	mov dword ptr es:[jmp_1001fc50+3], eax
jmp_1001f4e5:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_1001f96a+2], eax
	je jmp_1001f547
	mov dword ptr es:[jmp_1001f96a+2], eax
	mov dword ptr es:[jmp_1001f9ae+2], eax
	mov dword ptr es:[jmp_1001f9f3+2], eax
	mov dword ptr es:[jmp_1001fa38+2], eax
	mov dword ptr es:[jmp_1001fa9a+2], eax
	mov dword ptr es:[jmp_1001fae7+2], eax
	mov dword ptr es:[jmp_1001fb35+2], eax
	mov dword ptr es:[jmp_1001fb83+2], eax
	mov dword ptr es:[jmp_1001fbd1+2], eax
	mov dword ptr es:[jmp_1001fc1f+2], eax
	mov dword ptr es:[jmp_1001fc69+2], eax
jmp_1001f547:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1001f557:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1001f562
	mov esi, eax
	mov ecx, ebx
jmp_1001f562:
	cmp eax, edi
	jl jmp_1001f568
	mov edi, eax
jmp_1001f568:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1001f557
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1001ff8c
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1001f593:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001f5af
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001f5af:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001f593
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_1001f64a:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001f663
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001f663:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1001f64a
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1001f70f:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1001f753
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_1001f76b
jmp_1001f753:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_1001f76b:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1001fcad
jmp_1001f84b:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1001f8d2:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	shl ecx, 1
	rcl edx, 1
	shl ebx, 1
	rcl eax, 1
	mov dword ptr [g_codeBlockVars+0d8h], ecx
	mov dword ptr [g_codeBlockVars+0dch], edx
	mov dword ptr [g_codeBlockVars+0e0h], ebx
	mov dword ptr [g_codeBlockVars+0e4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001f944:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1001fa88
jmp_1001f958:
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001f96a:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001f979
	mov ah, al
	mov word ptr [edi], ax
jmp_1001f979:
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001f995:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001f9ae:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001f9be
	mov ah, al
	mov word ptr [edi+2], ax
jmp_1001f9be:
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001f9da:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001f9f3:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fa03
	mov ah, al
	mov word ptr [edi+4], ax
jmp_1001fa03:
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fa1f:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0e0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0e4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fa38:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fa48
	mov ah, al
	mov word ptr [edi+6], ax
jmp_1001fa48:
	add esi, dword ptr [g_codeBlockVars+0d8h]
	adc edx, dword ptr [g_codeBlockVars+0dch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fa64:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_1001fc82
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1001f958
jmp_1001fa88:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fa9a:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001faa6
	mov byte ptr [edi], al
jmp_1001faa6:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001fc82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001face:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fae7:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001faf4
	mov byte ptr [edi+1], al
jmp_1001faf4:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001fc82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fb1c:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fb35:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fb42
	mov byte ptr [edi+2], al
jmp_1001fb42:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001fc82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fb6a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fb83:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fb90
	mov byte ptr [edi+3], al
jmp_1001fb90:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001fc82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fbb8:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fbd1:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fbde
	mov byte ptr [edi+4], al
jmp_1001fbde:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001fc82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fc06:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fc1f:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fc2c
	mov byte ptr [edi+5], al
jmp_1001fc2c:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_1001fc82
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1001fc50:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1001fc69:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1001fc76
	mov byte ptr [edi+6], al
jmp_1001fc76:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1001fc82:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1001fd24
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_1001f84b
jmp_1001fcad:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1001fd24
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1001f8d2
jmp_1001fd24:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1001ff8c
	je jmp_1001fdb2
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1001fe0f
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_1001fd75:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_1001fecf
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001f70f
jmp_1001fdb2:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1001f70f
jmp_1001fe0f:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1001fe2b
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1001fe2b:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_1001fd75
jmp_1001fecf:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1001fee8
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1001fee8:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1001f70f
jmp_1001ff8c:
	pop ebp
	ret
jmp_1001ff8e:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10020452+3], eax
	je jmp_1001fffa
	mov dword ptr es:[jmp_10020452+3], eax
	mov dword ptr es:[jmp_1002049c+3], eax
	mov dword ptr es:[jmp_100204da+3], eax
	mov dword ptr es:[jmp_10020518+3], eax
	mov dword ptr es:[jmp_10020556+3], eax
	mov dword ptr es:[jmp_10020594+3], eax
	mov dword ptr es:[jmp_100205d2+3], eax
	mov dword ptr es:[jmp_10020610+3], eax
	mov dword ptr es:[jmp_1002064e+3], eax
	mov dword ptr es:[jmp_100206b4+3], eax
	mov dword ptr es:[jmp_100206fe+3], eax
	mov dword ptr es:[jmp_10020748+3], eax
	mov dword ptr es:[jmp_10020792+3], eax
	mov dword ptr es:[jmp_100207dc+3], eax
	mov dword ptr es:[jmp_10020822+3], eax
jmp_1001fffa:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_10020478+2], eax
	je jmp_10020074
	mov dword ptr es:[jmp_10020478+2], eax
	mov dword ptr es:[jmp_100204b5+2], eax
	mov dword ptr es:[jmp_100204f3+2], eax
	mov dword ptr es:[jmp_10020531+2], eax
	mov dword ptr es:[jmp_1002056f+2], eax
	mov dword ptr es:[jmp_100205ad+2], eax
	mov dword ptr es:[jmp_100205eb+2], eax
	mov dword ptr es:[jmp_10020629+2], eax
	mov dword ptr es:[jmp_10020684+2], eax
	mov dword ptr es:[jmp_100206cd+2], eax
	mov dword ptr es:[jmp_10020717+2], eax
	mov dword ptr es:[jmp_10020761+2], eax
	mov dword ptr es:[jmp_100207ab+2], eax
	mov dword ptr es:[jmp_100207f5+2], eax
	mov dword ptr es:[jmp_1002083b+2], eax
jmp_10020074:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10020084:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002008f
	mov esi, eax
	mov ecx, ebx
jmp_1002008f:
	cmp eax, edi
	jl jmp_10020095
	mov edi, eax
jmp_10020095:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10020084
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10020b5a
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_100200c0:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100200dc
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100200dc:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100200c0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_10020177:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10020190
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10020190:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10020177
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1002023c:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_10020280
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_10020298
jmp_10020280:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_10020298:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_1002087b
jmp_10020378:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_100203ff:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020452:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_10020672
jmp_10020466:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10020478:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002049c:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100204b5:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100204da:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100204f3:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020518:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10020531:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020556:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002056f:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020594:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100205ad:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100205d2:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100205eb:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020610:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10020629:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002064e:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_10020850
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10020466
jmp_10020672:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10020684:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10020850
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100206b4:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100206cd:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10020850
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100206fe:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10020717:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10020850
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020748:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10020761:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10020850
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020792:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100207ab:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10020850
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100207dc:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100207f5:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10020850
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10020822:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002083b:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10020850:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_100208f2
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_10020378
jmp_1002087b:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_100208f2
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_100203ff
jmp_100208f2:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10020b5a
	je jmp_10020980
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_100209dd
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_10020943:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10020a9d
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1002023c
jmp_10020980:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1002023c
jmp_100209dd:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100209f9
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100209f9:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_10020943
jmp_10020a9d:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10020ab6
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10020ab6:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1002023c
jmp_10020b5a:
	pop ebp
	ret
jmp_10020b5c:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10020f96+3], eax
	je jmp_10020bc8
	mov dword ptr es:[jmp_10020f96+3], eax
	mov dword ptr es:[jmp_10020fc3+3], eax
	mov dword ptr es:[jmp_10020fe4+3], eax
	mov dword ptr es:[jmp_10021005+3], eax
	mov dword ptr es:[jmp_10021026+3], eax
	mov dword ptr es:[jmp_10021047+3], eax
	mov dword ptr es:[jmp_10021068+3], eax
	mov dword ptr es:[jmp_10021089+3], eax
	mov dword ptr es:[jmp_100210aa+3], eax
	mov dword ptr es:[jmp_100210f3+3], eax
	mov dword ptr es:[jmp_10021120+3], eax
	mov dword ptr es:[jmp_1002114d+3], eax
	mov dword ptr es:[jmp_10021176+3], eax
	mov dword ptr es:[jmp_1002119f+3], eax
	mov dword ptr es:[jmp_100211c8+3], eax
jmp_10020bc8:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10020bd8:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10020be3
	mov esi, eax
	mov ecx, ebx
jmp_10020be3:
	cmp eax, edi
	jl jmp_10020be9
	mov edi, eax
jmp_10020be9:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10020bd8
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_100214f3
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10020c14:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10020c30
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10020c30:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10020c14
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_10020ccb:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10020ce4
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10020ce4:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10020ccb
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10020d90:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_10020dd4
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_10020dec
jmp_10020dd4:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_10020dec:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_10021214
jmp_10020ecc:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_10020f53:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
jmp_10020f96:
	mov eax, dword ptr [ecx*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_100210ce
jmp_10020faa:
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10020fc3:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10020fe4:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10021005:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10021026:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10021047:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10021068:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10021089:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_100210aa:
	mov eax, dword ptr [ecx*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_100211e9
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10020faa
jmp_100210ce:
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100211e9
jmp_100210f3:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100211e9
jmp_10021120:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100211e9
jmp_1002114d:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100211e9
jmp_10021176:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100211e9
jmp_1002119f:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100211e9
jmp_100211c8:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_100211e9:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_1002128b
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_10020ecc
jmp_10021214:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_1002128b
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_10020f53
jmp_1002128b:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_100214f3
	je jmp_10021319
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10021376
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_100212dc:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10021436
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_10020d90
jmp_10021319:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_10020d90
jmp_10021376:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10021392
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10021392:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_100212dc
jmp_10021436:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1002144f
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1002144f:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_10020d90
jmp_100214f3:
	pop ebp
	ret
jmp_100214f5:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10021951+3], eax
	je jmp_10021561
	mov dword ptr es:[jmp_10021951+3], eax
	mov dword ptr es:[jmp_10021991+3], eax
	mov dword ptr es:[jmp_100219c5+3], eax
	mov dword ptr es:[jmp_100219f9+3], eax
	mov dword ptr es:[jmp_10021a2d+3], eax
	mov dword ptr es:[jmp_10021a61+3], eax
	mov dword ptr es:[jmp_10021a95+3], eax
	mov dword ptr es:[jmp_10021ac9+3], eax
	mov dword ptr es:[jmp_10021afd+3], eax
	mov dword ptr es:[jmp_10021b59+3], eax
	mov dword ptr es:[jmp_10021b99+3], eax
	mov dword ptr es:[jmp_10021bd9+3], eax
	mov dword ptr es:[jmp_10021c19+3], eax
	mov dword ptr es:[jmp_10021c55+3], eax
	mov dword ptr es:[jmp_10021c91+3], eax
jmp_10021561:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10021583:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002158e
	mov esi, eax
	mov ecx, ebx
jmp_1002158e:
	cmp eax, edi
	jl jmp_10021594
	mov edi, eax
jmp_10021594:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10021583
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10021fbf
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_100215bf:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100215db
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100215db:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100215bf
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_10021676:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1002168f
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1002168f:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10021676
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1002173b:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1002177f
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_10021797
jmp_1002177f:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_10021797:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_10021ce0
jmp_10021877:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_100218fe:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021951:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_10021b21
jmp_10021965:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021991:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100219c5:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100219f9:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021a2d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021a61:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021a95:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021ac9:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021afd:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_10021cb5
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10021965
jmp_10021b21:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10021cb5
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021b59:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10021cb5
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021b99:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10021cb5
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021bd9:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10021cb5
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021c19:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10021cb5
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021c55:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10021cb5
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10021c91:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10021cb5:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_10021d57
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_10021877
jmp_10021ce0:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_10021d57
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_100218fe
jmp_10021d57:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10021fbf
	je jmp_10021de5
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10021e42
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_10021da8:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10021f02
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1002173b
jmp_10021de5:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_1002173b
jmp_10021e42:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10021e5e
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10021e5e:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_10021da8
jmp_10021f02:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10021f1b
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10021f1b:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_1002173b
jmp_10021fbf:
	pop ebp
	ret
jmp_10021fc1:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1002246d+3], eax
	je jmp_1002202d
	mov dword ptr es:[jmp_1002246d+3], eax
	mov dword ptr es:[jmp_100224ad+3], eax
	mov dword ptr es:[jmp_100224e1+3], eax
	mov dword ptr es:[jmp_10022515+3], eax
	mov dword ptr es:[jmp_10022549+3], eax
	mov dword ptr es:[jmp_1002257d+3], eax
	mov dword ptr es:[jmp_100225b1+3], eax
	mov dword ptr es:[jmp_100225e5+3], eax
	mov dword ptr es:[jmp_10022619+3], eax
	mov dword ptr es:[jmp_10022675+3], eax
	mov dword ptr es:[jmp_100226b5+3], eax
	mov dword ptr es:[jmp_100226f5+3], eax
	mov dword ptr es:[jmp_10022735+3], eax
	mov dword ptr es:[jmp_10022771+3], eax
	mov dword ptr es:[jmp_100227ad+3], eax
jmp_1002202d:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10022063:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002206e
	mov esi, eax
	mov ecx, ebx
jmp_1002206e:
	cmp eax, edi
	jl jmp_10022074
	mov edi, eax
jmp_10022074:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10022063
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10022ec0
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1002209f:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100220bb
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100220bb:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1002209f
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
jmp_10022174:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1002218d
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1002218d:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10022174
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10022257:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+54h]
	mov edi, dword ptr [g_codeBlockVars+58h]
	cmp ebx, eax
	jg jmp_1002229b
	xchg ebx, eax
	xchg ecx, edx
	mov dword ptr [g_codeBlockVars+0b8h], edi
	mov dword ptr [g_codeBlockVars+0bch], esi
	mov edi, dword ptr [g_codeBlockVars+4ch]
	mov esi, dword ptr [g_codeBlockVars+50h]
	jmp jmp_100222b3
jmp_1002229b:
	mov dword ptr [g_codeBlockVars+0b8h], esi
	mov dword ptr [g_codeBlockVars+0bch], edi
	mov edi, dword ptr [g_codeBlockVars+50h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
jmp_100222b3:
	sar eax, 10h
	sar ebx, 10h
	mov dword ptr [g_codeBlockVars+84h], eax
	mov dword ptr [g_codeBlockVars+88h], ebx
	mov dword ptr [g_codeBlockVars+0a8h], ecx
	mov dword ptr [g_codeBlockVars+0ach], edx
	mov dword ptr [g_codeBlockVars+0b0h], esi
	mov dword ptr [g_codeBlockVars+0b4h], edi
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0a0h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+90h], eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+94h], eax
	mov edx, dword ptr [g_codeBlockVars+0bch]
	sub edx, dword ptr [g_codeBlockVars+0b8h]
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	shl eax, 4
	mov dword ptr [g_codeBlockVars+0a4h], eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c0h], eax
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	mov dword ptr [g_codeBlockVars+0c4h], eax
	mov eax, dword ptr [g_codeBlockVars+84h]
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	shr ebx, 4
	mov dword ptr [g_codeBlockVars+0f0h], ebx
	je jmp_100227fc
jmp_10022393:
	mov eax, dword ptr [g_codeBlockVars+0a8h]
	add eax, dword ptr [g_codeBlockVars+90h]
	mov dword ptr [g_codeBlockVars+0a8h], eax
	mov eax, dword ptr [g_codeBlockVars+0b0h]
	add eax, dword ptr [g_codeBlockVars+94h]
	mov dword ptr [g_codeBlockVars+0b0h], eax
	mov eax, dword ptr [g_codeBlockVars+0b8h]
	add eax, dword ptr [g_codeBlockVars+0a4h]
	mov dword ptr [g_codeBlockVars+0b8h], eax
	mov ebx, 10h
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0b0h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	sar eax, 4
	mov esi, eax
	mov edx, dword ptr [g_codeBlockVars+0a8h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0b8h]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	sar eax, 4
	mov ebx, esi
	mov ecx, eax
jmp_1002241a:
	mov edx, ecx
	mov eax, ebx
	sar edx, 10h
	sar eax, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], edx
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], eax
	mov esi, dword ptr [g_codeBlockVars+0c0h]
	mov ebp, dword ptr [g_codeBlockVars+0c4h]
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002246d:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_1002263d
jmp_10022481:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100224ad:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100224e1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10022515:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10022549:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002257d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100225b1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100225e5:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10022619:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_100227d1
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10022481
jmp_1002263d:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100227d1
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10022675:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100227d1
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100226b5:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100227d1
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100226f5:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100227d1
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10022735:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100227d1
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10022771:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100227d1
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100227ad:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_100227d1:
	shrd esi, edx, 10h
	shrd ebp, ecx, 10h
	mov dword ptr [g_codeBlockVars+0c0h], esi
	mov dword ptr [g_codeBlockVars+0c4h], ebp
	mov eax, dword ptr [g_codeBlockVars+0f0h]
	dec eax
	js jmp_10022873
	mov dword ptr [g_codeBlockVars+0f0h], eax
	jne jmp_10022393
jmp_100227fc:
	mov ebx, dword ptr [g_codeBlockVars+0a0h]
	and ebx, 0fh
	je jmp_10022873
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	mov edx, dword ptr [g_codeBlockVars+0ach]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c0h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	mov edx, dword ptr [g_codeBlockVars+0b4h]
	xor eax, eax
	shrd eax, edx, 2
	sar edx, 2
	idiv dword ptr [g_codeBlockVars+0bch]
	add eax, 8000h
	sub eax, dword ptr [g_codeBlockVars+0c4h]
	mov edx, eax
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	jmp jmp_1002241a
jmp_10022873:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_10022891
	xchg ebx, eax
	xchg ecx, edx
jmp_10022891:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_100228c3
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
jmp_100228c3:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	cmp esi, 1
	je jmp_10022bc0
	push esi
	shr esi, 1
	dec esi
	mov dword ptr [g_codeBlockVars+0e8h], eax
	mov dword ptr [g_codeBlockVars+0ech], edx
	cmp esi, 5
	jl jmp_10022a83
jmp_10022914:
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+1]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+2]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+3]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+4]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+5]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+6]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+7]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+8]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+9]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+0ah]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+0bh]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+0ah], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	add edi, 0ch
	sub esi, 6
	js jmp_10022bb7
	cmp esi, 5
	jge jmp_10022914
jmp_10022a83:
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+1]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10022bb7
	xor edx, edx
	mov dl, byte ptr [edi+2]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+3]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10022bb7
	xor edx, edx
	mov dl, byte ptr [edi+4]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+5]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10022bb7
	xor edx, edx
	mov dl, byte ptr [edi+6]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+7]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10022bb7
	xor edx, edx
	mov dl, byte ptr [edi+8]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+9]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
jmp_10022bb7:
	pop esi
	test esi, 1
	je jmp_10022bd6
jmp_10022bc0:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov byte ptr [edi], al
jmp_10022bd6:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10022ec0
	je jmp_10022c94
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10022d07
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
jmp_10022c4c:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10022de5
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_10022257
jmp_10022c94:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+7ch]
	add dword ptr [g_codeBlockVars+54h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	mov eax, dword ptr [g_codeBlockVars+80h]
	add dword ptr [g_codeBlockVars+58h], eax
	jmp jmp_10022257
jmp_10022d07:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10022d23
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10022d23:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+7ch], eax
	mov dword ptr [g_codeBlockVars+54h], edi
	jmp jmp_10022c4c
jmp_10022de5:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10022dfe
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10022dfe:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov edi, dword ptr [ebx+14h]
	mov edx, dword ptr [esi+14h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+80h], eax
	mov dword ptr [g_codeBlockVars+58h], edi
	jmp jmp_10022257
jmp_10022ec0:
	pop ebp
	ret
jmp_10022ec2:
	push ebp
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_100233af+3], eax
	je jmp_10022f42
	mov dword ptr es:[jmp_100233af+3], eax
	mov dword ptr es:[jmp_100233fd+3], eax
	mov dword ptr es:[jmp_1002343f+3], eax
	mov dword ptr es:[jmp_10023481+3], eax
	mov dword ptr es:[jmp_100234c3+3], eax
	mov dword ptr es:[jmp_10023505+3], eax
	mov dword ptr es:[jmp_10023547+3], eax
	mov dword ptr es:[jmp_10023589+3], eax
	mov dword ptr es:[jmp_100235cb+3], eax
	mov dword ptr es:[jmp_10023635+3], eax
	mov dword ptr es:[jmp_10023683+3], eax
	mov dword ptr es:[jmp_100236d1+3], eax
	mov dword ptr es:[jmp_1002371f+3], eax
	mov dword ptr es:[jmp_1002376d+3], eax
	mov dword ptr es:[jmp_100237b7+3], eax
jmp_10022f42:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_100233d5+2], eax
	je jmp_10022fbc
	mov dword ptr es:[jmp_100233d5+2], eax
	mov dword ptr es:[jmp_10023416+2], eax
	mov dword ptr es:[jmp_10023458+2], eax
	mov dword ptr es:[jmp_1002349a+2], eax
	mov dword ptr es:[jmp_100234dc+2], eax
	mov dword ptr es:[jmp_1002351e+2], eax
	mov dword ptr es:[jmp_10023560+2], eax
	mov dword ptr es:[jmp_100235a2+2], eax
	mov dword ptr es:[jmp_10023601+2], eax
	mov dword ptr es:[jmp_1002364e+2], eax
	mov dword ptr es:[jmp_1002369c+2], eax
	mov dword ptr es:[jmp_100236ea+2], eax
	mov dword ptr es:[jmp_10023738+2], eax
	mov dword ptr es:[jmp_10023786+2], eax
	mov dword ptr es:[jmp_100237d0+2], eax
jmp_10022fbc:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10022fcc:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10022fd7
	mov esi, eax
	mov ecx, ebx
jmp_10022fd7:
	cmp eax, edi
	jl jmp_10022fdd
	mov edi, eax
jmp_10022fdd:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10022fcc
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10023a53
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10023008:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10023024
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10023024:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10023008
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_100230bf:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100230d8
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100230d8:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100230bf
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10023184:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_100231a2
	xchg ebx, eax
	xchg ecx, edx
jmp_100231a2:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_100231d4
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
jmp_100231d4:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test edi, 1
	je jmp_1002321e
	mov byte ptr [edi], cl
	dec esi
	je jmp_100232dc
	inc edi
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1002321e:
	cmp esi, 1
	je jmp_100232d4
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_10023284
jmp_10023230:
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
	js jmp_100232cb
	cmp esi, 5
	jge jmp_10023230
jmp_10023284:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100232cb
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100232cb
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100232cb
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100232cb
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_100232cb:
	pop esi
	test esi, 1
	je jmp_100232dc
jmp_100232d4:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	mov byte ptr [edi], cl
jmp_100232dc:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_1002331e
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_1002331e:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100233af:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_100235ef
jmp_100233c3:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100233d5:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100233e1
	mov byte ptr [edi], al
jmp_100233e1:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100233fd:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023416:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10023423
	mov byte ptr [edi+1], al
jmp_10023423:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002343f:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023458:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10023465
	mov byte ptr [edi+2], al
jmp_10023465:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023481:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002349a:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100234a7
	mov byte ptr [edi+3], al
jmp_100234a7:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100234c3:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100234dc:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100234e9
	mov byte ptr [edi+4], al
jmp_100234e9:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023505:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002351e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1002352b
	mov byte ptr [edi+5], al
jmp_1002352b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023547:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023560:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1002356d
	mov byte ptr [edi+6], al
jmp_1002356d:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023589:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100235a2:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100235af
	mov byte ptr [edi+7], al
jmp_100235af:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100235cb:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_100237eb
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_100233c3
jmp_100235ef:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023601:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1002360d
	mov byte ptr [edi], al
jmp_1002360d:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100237eb
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023635:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002364e:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_1002365b
	mov byte ptr [edi+1], al
jmp_1002365b:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100237eb
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023683:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002369c:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100236a9
	mov byte ptr [edi+2], al
jmp_100236a9:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100237eb
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100236d1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100236ea:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100236f7
	mov byte ptr [edi+3], al
jmp_100236f7:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100237eb
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002371f:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023738:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10023745
	mov byte ptr [edi+4], al
jmp_10023745:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100237eb
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002376d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023786:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_10023793
	mov byte ptr [edi+5], al
jmp_10023793:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100237eb
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100237b7:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100237d0:
	mov al, byte ptr [ebx-8000h]
	cmp al, 0ffh
	je jmp_100237dd
	mov byte ptr [edi+6], al
jmp_100237dd:
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_100237eb
jmp_100237eb:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10023a53
	je jmp_10023879
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_100238d6
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_1002383c:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10023996
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10023184
jmp_10023879:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10023184
jmp_100238d6:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100238f2
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100238f2:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_1002383c
jmp_10023996:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100239af
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100239af:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_10023184
jmp_10023a53:
	pop ebp
	ret
jmp_10023a55:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10023d84+3], eax
	je jmp_10023ac1
	mov dword ptr es:[jmp_10023d84+3], eax
	mov dword ptr es:[jmp_10023dce+3], eax
	mov dword ptr es:[jmp_10023e0c+3], eax
	mov dword ptr es:[jmp_10023e4a+3], eax
	mov dword ptr es:[jmp_10023e88+3], eax
	mov dword ptr es:[jmp_10023ec6+3], eax
	mov dword ptr es:[jmp_10023f04+3], eax
	mov dword ptr es:[jmp_10023f42+3], eax
	mov dword ptr es:[jmp_10023f80+3], eax
	mov dword ptr es:[jmp_10023fe6+3], eax
	mov dword ptr es:[jmp_10024030+3], eax
	mov dword ptr es:[jmp_1002407a+3], eax
	mov dword ptr es:[jmp_100240c4+3], eax
	mov dword ptr es:[jmp_1002410e+3], eax
	mov dword ptr es:[jmp_10024154+3], eax
jmp_10023ac1:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_codeBlockVars+104h]
	cmp dword ptr es:[jmp_10023daa+2], eax
	je jmp_10023b3b
	mov dword ptr es:[jmp_10023daa+2], eax
	mov dword ptr es:[jmp_10023de7+2], eax
	mov dword ptr es:[jmp_10023e25+2], eax
	mov dword ptr es:[jmp_10023e63+2], eax
	mov dword ptr es:[jmp_10023ea1+2], eax
	mov dword ptr es:[jmp_10023edf+2], eax
	mov dword ptr es:[jmp_10023f1d+2], eax
	mov dword ptr es:[jmp_10023f5b+2], eax
	mov dword ptr es:[jmp_10023fb6+2], eax
	mov dword ptr es:[jmp_10023fff+2], eax
	mov dword ptr es:[jmp_10024049+2], eax
	mov dword ptr es:[jmp_10024093+2], eax
	mov dword ptr es:[jmp_100240dd+2], eax
	mov dword ptr es:[jmp_10024127+2], eax
	mov dword ptr es:[jmp_1002416d+2], eax
jmp_10023b3b:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10023b4b:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10023b56
	mov esi, eax
	mov ecx, ebx
jmp_10023b56:
	cmp eax, edi
	jl jmp_10023b5c
	mov edi, eax
jmp_10023b5c:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10023b4b
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10024384
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10023b87:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10023ba3
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10023ba3:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10023b87
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_10023c20:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10023c39
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10023c39:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10023c20
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10023cc7:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_10023cf3
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_10023cf3:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023d84:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_10023fa4
jmp_10023d98:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023daa:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023dce:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023de7:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023e0c:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023e25:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023e4a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023e63:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023e88:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023ea1:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023ec6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023edf:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023f04:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023f1d:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023f42:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023f5b:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023f80:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_10024184
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10023d98
jmp_10023fa4:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023fb6:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024184
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10023fe6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10023fff:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024184
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024030:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10024049:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024184
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002407a:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10024093:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024184
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100240c4:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_100240dd:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024184
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002410e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_10024127:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024184
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024154:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	xor ebx, ebx
	mov bl, al
jmp_1002416d:
	mov al, byte ptr [ebx-8000h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_10024184
jmp_10024184:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10024384
	je jmp_100241fc
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10024243
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_100241ca:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_100242e5
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10023cc7
jmp_100241fc:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10023cc7
jmp_10024243:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1002425f
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1002425f:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_100241ca
jmp_100242e5:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100242fe
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100242fe:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_10023cc7
jmp_10024384:
	pop ebp
	ret
jmp_10024386:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_1002462b+3], eax
	je jmp_100243f2
	mov dword ptr es:[jmp_1002462b+3], eax
	mov dword ptr es:[jmp_10024658+3], eax
	mov dword ptr es:[jmp_10024679+3], eax
	mov dword ptr es:[jmp_1002469a+3], eax
	mov dword ptr es:[jmp_100246bb+3], eax
	mov dword ptr es:[jmp_100246dc+3], eax
	mov dword ptr es:[jmp_100246fd+3], eax
	mov dword ptr es:[jmp_1002471e+3], eax
	mov dword ptr es:[jmp_1002473f+3], eax
	mov dword ptr es:[jmp_10024788+3], eax
	mov dword ptr es:[jmp_100247b5+3], eax
	mov dword ptr es:[jmp_100247e2+3], eax
	mov dword ptr es:[jmp_1002480b+3], eax
	mov dword ptr es:[jmp_10024834+3], eax
	mov dword ptr es:[jmp_1002485d+3], eax
jmp_100243f2:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10024402:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002440d
	mov esi, eax
	mov ecx, ebx
jmp_1002440d:
	cmp eax, edi
	jl jmp_10024413
	mov edi, eax
jmp_10024413:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10024402
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10024a80
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1002443e:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1002445a
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1002445a:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1002443e
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_100244d7:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100244f0
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100244f0:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100244d7
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1002457e:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_100245aa
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_100245aa:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
jmp_1002462b:
	mov eax, dword ptr [ecx*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_10024763
jmp_1002463f:
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10024658:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_10024679:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1002469a:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_100246bb:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_100246dc:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_100246fd:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1002471e:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
jmp_1002473f:
	mov eax, dword ptr [ecx*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_10024880
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_1002463f
jmp_10024763:
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024880
jmp_10024788:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024880
jmp_100247b5:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024880
jmp_100247e2:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024880
jmp_1002480b:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024880
jmp_10024834:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10024880
jmp_1002485d:
	mov eax, dword ptr [ecx*4-1]
	add ebp, ebx
	mov al, byte ptr [eax+edx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_10024880
jmp_10024880:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10024a80
	je jmp_100248f8
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1002493f
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_100248c6:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_100249e1
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_1002457e
jmp_100248f8:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_1002457e
jmp_1002493f:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1002495b
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1002495b:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_100248c6
jmp_100249e1:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100249fa
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100249fa:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_1002457e
jmp_10024a80:
	pop ebp
	ret
jmp_10024a82:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10024d49+3], eax
	je jmp_10024aee
	mov dword ptr es:[jmp_10024d49+3], eax
	mov dword ptr es:[jmp_10024d89+3], eax
	mov dword ptr es:[jmp_10024dbd+3], eax
	mov dword ptr es:[jmp_10024df1+3], eax
	mov dword ptr es:[jmp_10024e25+3], eax
	mov dword ptr es:[jmp_10024e59+3], eax
	mov dword ptr es:[jmp_10024e8d+3], eax
	mov dword ptr es:[jmp_10024ec1+3], eax
	mov dword ptr es:[jmp_10024ef5+3], eax
	mov dword ptr es:[jmp_10024f51+3], eax
	mov dword ptr es:[jmp_10024f91+3], eax
	mov dword ptr es:[jmp_10024fd1+3], eax
	mov dword ptr es:[jmp_10025011+3], eax
	mov dword ptr es:[jmp_1002504d+3], eax
	mov dword ptr es:[jmp_10025089+3], eax
jmp_10024aee:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10024b10:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10024b1b
	mov esi, eax
	mov ecx, ebx
jmp_10024b1b:
	cmp eax, edi
	jl jmp_10024b21
	mov edi, eax
jmp_10024b21:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10024b10
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_100252af
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10024b4c:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10024b68
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10024b68:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10024b4c
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_10024be5:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10024bfe
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10024bfe:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10024be5
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10024c8c:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_10024cb8
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_10024cb8:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024d49:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_10024f19
jmp_10024d5d:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024d89:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024dbd:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024df1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024e25:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024e59:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024e8d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024ec1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024ef5:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_100250af
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10024d5d
jmp_10024f19:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100250af
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024f51:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100250af
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024f91:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100250af
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10024fd1:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100250af
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10025011:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100250af
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002504d:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_100250af
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10025089:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_100250af
jmp_100250af:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_100252af
	je jmp_10025127
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1002516e
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_100250f5:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10025210
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10024c8c
jmp_10025127:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10024c8c
jmp_1002516e:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_1002518a
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_1002518a:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_100250f5
jmp_10025210:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10025229
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10025229:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_10024c8c
jmp_100252af:
	pop ebp
	ret
jmp_100252b1:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10025582+3], eax
	je jmp_1002531d
	mov dword ptr es:[jmp_10025582+3], eax
	mov dword ptr es:[jmp_100255ce+3], eax
	mov dword ptr es:[jmp_1002560e+3], eax
	mov dword ptr es:[jmp_1002564e+3], eax
	mov dword ptr es:[jmp_1002568e+3], eax
	mov dword ptr es:[jmp_100256ce+3], eax
	mov dword ptr es:[jmp_1002570e+3], eax
	mov dword ptr es:[jmp_1002574e+3], eax
	mov dword ptr es:[jmp_1002578e+3], eax
	mov dword ptr es:[jmp_100257f6+3], eax
	mov dword ptr es:[jmp_10025842+3], eax
	mov dword ptr es:[jmp_1002588e+3], eax
	mov dword ptr es:[jmp_100258da+3], eax
	mov dword ptr es:[jmp_10025926+3], eax
	mov dword ptr es:[jmp_10025972+3], eax
jmp_1002531d:
	mov eax, dword ptr [ebx+4]
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1002533d:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10025348
	mov esi, eax
	mov ecx, ebx
jmp_10025348:
	cmp eax, edi
	jl jmp_1002534e
	mov edi, eax
jmp_1002534e:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1002533d
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10025ce5
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10025379:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10025395
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10025395:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10025379
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_10025412:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1002542b
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1002542b:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10025412
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_100254b9:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_100254e5
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_100254e5:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
jmp_1002556a:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_1002599b
jmp_10025576:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_100259a6
jmp_10025582:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_100257b2
jmp_10025596:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_100255b6:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_100259b1
jmp_100255c2:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_100259bc
jmp_100255ce:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_100255f6:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_100259c7
jmp_10025602:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_100259d2
jmp_1002560e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_10025636:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_100259dd
jmp_10025642:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_100259e8
jmp_1002564e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_10025676:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_100259f3
jmp_10025682:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_100259fe
jmp_1002568e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_100256b6:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a09
jmp_100256c2:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a14
jmp_100256ce:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_100256f6:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a1f
jmp_10025702:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a2a
jmp_1002570e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_10025736:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a35
jmp_10025742:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a40
jmp_1002574e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
jmp_10025776:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a4b
jmp_10025782:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a56
jmp_1002578e:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_10025ae5
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10025596
jmp_100257b2:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10025ae5
	mov eax, ecx
	mov ebx, edx
jmp_100257de:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a61
jmp_100257ea:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a6c
jmp_100257f6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10025ae5
	mov eax, ecx
	mov ebx, edx
jmp_1002582a:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a77
jmp_10025836:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a82
jmp_10025842:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10025ae5
	mov eax, ecx
	mov ebx, edx
jmp_10025876:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025a8d
jmp_10025882:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025a98
jmp_1002588e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10025ae5
	mov eax, ecx
	mov ebx, edx
jmp_100258c2:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025aa3
jmp_100258ce:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025aae
jmp_100258da:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10025ae5
	mov eax, ecx
	mov ebx, edx
jmp_1002590e:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025ab9
jmp_1002591a:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025ac4
jmp_10025926:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10025ae5
	mov eax, ecx
	mov ebx, edx
jmp_1002595a:
	cmp eax, dword ptr [g_codeBlockVars+0fch]
	jae jmp_10025acf
jmp_10025966:
	cmp ebx, dword ptr [g_codeBlockVars+0f8h]
	jae jmp_10025ada
jmp_10025972:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_10025ae5
jmp_1002599b:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1002556a
jmp_100259a6:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025576
jmp_100259b1:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_100255b6
jmp_100259bc:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_100255c2
jmp_100259c7:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_100255f6
jmp_100259d2:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025602
jmp_100259dd:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_10025636
jmp_100259e8:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025642
jmp_100259f3:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_10025676
jmp_100259fe:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025682
jmp_10025a09:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_100256b6
jmp_10025a14:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_100256c2
jmp_10025a1f:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_100256f6
jmp_10025a2a:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025702
jmp_10025a35:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_10025736
jmp_10025a40:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025742
jmp_10025a4b:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_10025776
jmp_10025a56:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025782
jmp_10025a61:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_100257de
jmp_10025a6c:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_100257ea
jmp_10025a77:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1002582a
jmp_10025a82:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025836
jmp_10025a8d:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_10025876
jmp_10025a98:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025882
jmp_10025aa3:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_100258c2
jmp_10025aae:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_100258ce
jmp_10025ab9:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1002590e
jmp_10025ac4:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_1002591a
jmp_10025acf:
	sub eax, dword ptr [g_codeBlockVars+0fch]
	jmp jmp_1002595a
jmp_10025ada:
	sub ebx, dword ptr [g_codeBlockVars+0f8h]
	jmp jmp_10025966
jmp_10025ae5:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10025ce5
	je jmp_10025b5d
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10025ba4
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_10025b2b:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10025c46
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_100254b9
jmp_10025b5d:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_100254b9
jmp_10025ba4:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10025bc0
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10025bc0:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_10025b2b
jmp_10025c46:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10025c5f
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10025c5f:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_100254b9
jmp_10025ce5:
	pop ebp
	ret
jmp_10025ce7:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+100h]
	mov eax, dword ptr [ebx]
	cmp dword ptr es:[jmp_10025ffe+3], eax
	je jmp_10025d53
	mov dword ptr es:[jmp_10025ffe+3], eax
	mov dword ptr es:[jmp_1002603e+3], eax
	mov dword ptr es:[jmp_10026072+3], eax
	mov dword ptr es:[jmp_100260a6+3], eax
	mov dword ptr es:[jmp_100260da+3], eax
	mov dword ptr es:[jmp_1002610e+3], eax
	mov dword ptr es:[jmp_10026142+3], eax
	mov dword ptr es:[jmp_10026176+3], eax
	mov dword ptr es:[jmp_100261aa+3], eax
	mov dword ptr es:[jmp_10026206+3], eax
	mov dword ptr es:[jmp_10026246+3], eax
	mov dword ptr es:[jmp_10026286+3], eax
	mov dword ptr es:[jmp_100262c6+3], eax
	mov dword ptr es:[jmp_10026302+3], eax
	mov dword ptr es:[jmp_1002633e+3], eax
jmp_10025d53:
	mov eax, dword ptr [ebx+4]
	dec eax
	mov dword ptr [g_codeBlockVars+0f8h], eax
	mov eax, dword ptr [ebx+8]
	dec eax
	mov dword ptr [g_codeBlockVars+0fch], eax
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10025d89:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10025d94
	mov esi, eax
	mov ecx, ebx
jmp_10025d94:
	cmp eax, edi
	jl jmp_10025d9a
	mov edi, eax
jmp_10025d9a:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10025d89
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10026945
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10025dc5:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10025de1
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10025de1:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10025dc5
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
jmp_10025e7c:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10025e95
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10025e95:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10025e7c
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10025f41:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+44h]
	mov edx, dword ptr [g_codeBlockVars+48h]
	mov esi, dword ptr [g_codeBlockVars+4ch]
	mov edi, dword ptr [g_codeBlockVars+50h]
	cmp ebx, eax
	jg jmp_10025f6d
	xchg ebx, eax
	xchg ecx, edx
	xchg esi, edi
jmp_10025f6d:
	sar eax, 10h
	sar ebx, 10h
	push ecx
	push esi
	push eax
	sub ebx, eax
	inc ebx
	mov dword ptr [g_codeBlockVars+0f4h], ebx
	sub edx, ecx
	xor eax, eax
	shrd eax, edx, 10h
	shl ebx, 10h
	sar edx, 10h
	idiv ebx
	mov ecx, eax
	sub edi, esi
	mov edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ebx
	mov ebx, eax
	pop edi
	add edi, dword ptr [g_codeBlockVars+10h]
	mov eax, ecx
	mov edx, ebx
	sar eax, 10h
	sar edx, 10h
	shl ecx, 10h
	shl ebx, 10h
	mov dword ptr [g_codeBlockVars+0c8h], ecx
	mov dword ptr [g_codeBlockVars+0cch], eax
	mov dword ptr [g_codeBlockVars+0d0h], ebx
	mov dword ptr [g_codeBlockVars+0d4h], edx
	pop ebp
	pop esi
	add ebp, 8000h
	add esi, 8000h
	mov edx, esi
	mov ecx, ebp
	shr edx, 10h
	shr ecx, 10h
	shl esi, 10h
	shl ebp, 10h
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10025ffe:
	add ebx, dword ptr [eax*4-1]
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jl jmp_100261ce
jmp_10026012:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002603e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026072:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100260a6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100260da:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002610e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026142:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026176:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+7], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100261aa:
	add ebx, dword ptr [eax*4-1]
	add edi, 8
	sub dword ptr [g_codeBlockVars+0f4h], 8
	je jmp_10026364
	cmp dword ptr [g_codeBlockVars+0f4h], 8
	jge jmp_10026012
jmp_100261ce:
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10026364
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026206:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+1], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10026364
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026246:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+2], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10026364
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026286:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+3], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10026364
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_100262c6:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+4], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10026364
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_10026302:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+5], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	dec dword ptr [g_codeBlockVars+0f4h]
	je jmp_10026364
	mov eax, ecx
	mov ebx, edx
	and eax, dword ptr [g_codeBlockVars+0fch]
	and ebx, dword ptr [g_codeBlockVars+0f8h]
jmp_1002633e:
	add ebx, dword ptr [eax*4-1]
	add ebp, dword ptr [g_codeBlockVars+0d0h]
	mov al, byte ptr [ebx]
	adc ecx, dword ptr [g_codeBlockVars+0d4h]
	mov byte ptr [edi+6], al
	add esi, dword ptr [g_codeBlockVars+0c8h]
	adc edx, dword ptr [g_codeBlockVars+0cch]
	jmp jmp_10026364
jmp_10026364:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_10026382
	xchg ebx, eax
	xchg ecx, edx
jmp_10026382:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_100263b4
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
jmp_100263b4:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	cmp esi, 1
	je jmp_100266b1
	push esi
	shr esi, 1
	dec esi
	mov dword ptr [g_codeBlockVars+0e8h], eax
	mov dword ptr [g_codeBlockVars+0ech], edx
	cmp esi, 5
	jl jmp_10026574
jmp_10026405:
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+1]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+2]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+3]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+4]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+5]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+6]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+7]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+8]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+9]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+0ah]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+0bh]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+0ah], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	add edi, 0ch
	sub esi, 6
	js jmp_100266a8
	cmp esi, 5
	jge jmp_10026405
jmp_10026574:
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+1]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100266a8
	xor edx, edx
	mov dl, byte ptr [edi+2]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+3]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100266a8
	xor edx, edx
	mov dl, byte ptr [edi+4]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+5]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100266a8
	xor edx, edx
	mov dl, byte ptr [edi+6]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+7]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100266a8
	xor edx, edx
	mov dl, byte ptr [edi+8]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+9]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
jmp_100266a8:
	pop esi
	test esi, 1
	je jmp_100266c7
jmp_100266b1:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov byte ptr [edi], al
jmp_100266c7:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10026945
	je jmp_1002676b
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_100267c8
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
jmp_1002672e:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10026888
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10025f41
jmp_1002676b:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+6ch]
	add dword ptr [g_codeBlockVars+44h], eax
	mov eax, dword ptr [g_codeBlockVars+74h]
	add dword ptr [g_codeBlockVars+4ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	mov eax, dword ptr [g_codeBlockVars+70h]
	add dword ptr [g_codeBlockVars+48h], eax
	mov eax, dword ptr [g_codeBlockVars+78h]
	add dword ptr [g_codeBlockVars+50h], eax
	jmp jmp_10025f41
jmp_100267c8:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100267e4
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100267e4:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+6ch], eax
	mov dword ptr [g_codeBlockVars+44h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+74h], eax
	mov dword ptr [g_codeBlockVars+4ch], edi
	jmp jmp_1002672e
jmp_10026888:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100268a1
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100268a1:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov edi, dword ptr [ebx+0ch]
	mov edx, dword ptr [esi+0ch]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+70h], eax
	mov dword ptr [g_codeBlockVars+48h], edi
	mov edi, dword ptr [ebx+10h]
	mov edx, dword ptr [esi+10h]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+78h], eax
	mov dword ptr [g_codeBlockVars+50h], edi
	jmp jmp_10025f41
jmp_10026945:
	pop ebp
	ret
jmp_10026947:
	push ebp
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10026958:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10026963
	mov esi, eax
	mov ecx, ebx
jmp_10026963:
	cmp eax, edi
	jl jmp_10026969
	mov edi, eax
jmp_10026969:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10026958
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10026cd4
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10026994:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100269b0
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100269b0:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10026994
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
jmp_100269f1:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10026a0a
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10026a0a:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100269f1
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10026a5c:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	cmp ebx, eax
	jg jmp_10026a6c
	xchg ebx, eax
jmp_10026a6c:
	sar eax, 10h
	sar ebx, 10h
	sub ebx, eax
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov eax, ebx
	add eax, edi
	mov dword ptr [g_codeBlockVars+0ch], eax
	xor eax, eax
	mov esi, dword ptr [g_codeBlockVars+104h]
	inc ebx
	test edi, 1
	je jmp_10026aa5
	mov al, byte ptr [edi]
	mov dl, byte ptr [esi+eax]
	mov byte ptr [edi], dl
	dec ebx
	je jmp_10026ba8
	inc edi
jmp_10026aa5:
	cmp ebx, 1
	je jmp_10026b9b
	push ebx
	shr ebx, 1
	dec ebx
	cmp ebx, 5
	jl jmp_10026b2e
jmp_10026ab7:
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
	js jmp_10026b92
	cmp ebx, 5
	jge jmp_10026ab7
jmp_10026b2e:
	mov cx, word ptr [edi]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi], dx
	dec ebx
	js jmp_10026b92
	mov cx, word ptr [edi+2]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+2], dx
	dec ebx
	js jmp_10026b92
	mov cx, word ptr [edi+4]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+4], dx
	dec ebx
	js jmp_10026b92
	mov cx, word ptr [edi+6]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+6], dx
	dec ebx
	js jmp_10026b92
	mov cx, word ptr [edi+8]
	mov al, cl
	mov dl, byte ptr [esi+eax]
	mov al, ch
	mov dh, byte ptr [esi+eax]
	mov word ptr [edi+8], dx
jmp_10026b92:
	pop ebx
	test ebx, 1
	je jmp_10026ba8
jmp_10026b9b:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	mov al, byte ptr [edi]
	mov dl, byte ptr [esi+eax]
	mov byte ptr [edi], dl
jmp_10026ba8:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10026cd4
	je jmp_10026bf0
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10026c0b
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
jmp_10026bd4:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10026c71
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	jmp jmp_10026a5c
jmp_10026bf0:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	jmp jmp_10026a5c
jmp_10026c0b:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10026c27
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10026c27:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	jmp jmp_10026bd4
jmp_10026c71:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10026c8a
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10026c8a:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	jmp jmp_10026a5c
jmp_10026cd4:
	pop ebp
	ret
jmp_10026cd6:
	push ebp
	mov eax, dword ptr [g_codeBlockVars+98h]
	shr eax, 10h
	mov ah, al
	mov edx, eax
	shl eax, 10h
	or eax, edx
	mov dword ptr [g_codeBlockVars+98h], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10026cfd:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10026d08
	mov esi, eax
	mov ecx, ebx
jmp_10026d08:
	cmp eax, edi
	jl jmp_10026d0e
	mov edi, eax
jmp_10026d0e:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10026cfd
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10026fc7
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10026d39:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10026d55
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10026d55:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10026d39
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
jmp_10026d96:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10026daf
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10026daf:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10026d96
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10026e01:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov edx, dword ptr [g_codeBlockVars+38h]
	cmp edx, eax
	jg jmp_10026e11
	xchg edx, eax
jmp_10026e11:
	sar eax, 10h
	sar edx, 10h
	mov ecx, edx
	sub ecx, eax
	inc ecx
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	mov eax, dword ptr [g_codeBlockVars+98h]
	cmp ecx, 4
	jl jmp_10026e8a
	test edi, 3
	je jmp_10026e52
	mov byte ptr [edi], al
	inc edi
	dec ecx
	test edi, 3
	je jmp_10026e52
	mov byte ptr [edi], al
	inc edi
	dec ecx
	test edi, 3
	je jmp_10026e52
	mov byte ptr [edi], al
	inc edi
	dec ecx
jmp_10026e52:
	push ecx
	shr ecx, 2
jmp_10026e56:
	cmp ecx, 4
	jl jmp_10026e6e
	mov dword ptr [edi], eax
	mov dword ptr [edi+4], eax
	mov dword ptr [edi+8], eax
	mov dword ptr [edi+0ch], eax
	add edi, 10h
	sub ecx, 4
	jmp jmp_10026e56
jmp_10026e6e:
	dec ecx
	js jmp_10026e86
	mov dword ptr [edi], eax
	add edi, 4
	dec ecx
	js jmp_10026e86
	mov dword ptr [edi], eax
	add edi, 4
	dec ecx
	js jmp_10026e86
	mov dword ptr [edi], eax
	add edi, 4
jmp_10026e86:
	pop ecx
	and ecx, 3
jmp_10026e8a:
	dec ecx
	jl jmp_10026e9b
	mov byte ptr [edi], al
	dec ecx
	jl jmp_10026e9b
	mov byte ptr [edi+1], al
	dec ecx
	jl jmp_10026e9b
	mov byte ptr [edi+2], al
jmp_10026e9b:
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10026fc7
	je jmp_10026ee3
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10026efe
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
jmp_10026ec7:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10026f64
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	jmp jmp_10026e01
jmp_10026ee3:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	jmp jmp_10026e01
jmp_10026efe:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10026f1a
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10026f1a:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	jmp jmp_10026ec7
jmp_10026f64:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10026f7d
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10026f7d:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	jmp jmp_10026e01
jmp_10026fc7:
	pop ebp
	ret
jmp_10026fc9:
	push ebp
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10026fee:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10026ff9
	mov esi, eax
	mov ecx, ebx
jmp_10026ff9:
	cmp eax, edi
	jl jmp_10026fff
	mov edi, eax
jmp_10026fff:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10026fee
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_1002763b
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1002702a:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10027046
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10027046:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1002702a
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
jmp_100270a5:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100270be
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100270be:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100270a5
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1002712e:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_1002714c
	xchg ebx, eax
	xchg ecx, edx
jmp_1002714c:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_1002717e
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
jmp_1002717e:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	cmp esi, 1
	je jmp_1002747b
	push esi
	shr esi, 1
	dec esi
	mov dword ptr [g_codeBlockVars+0e8h], eax
	mov dword ptr [g_codeBlockVars+0ech], edx
	cmp esi, 5
	jl jmp_1002733e
jmp_100271cf:
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+1]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+2]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+3]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+4]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+5]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+6]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+7]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+8]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+9]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	xor edx, edx
	mov dl, byte ptr [edi+0ah]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+0bh]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+0ah], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	add edi, 0ch
	sub esi, 6
	js jmp_10027472
	cmp esi, 5
	jge jmp_100271cf
jmp_1002733e:
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+1]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10027472
	xor edx, edx
	mov dl, byte ptr [edi+2]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+3]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10027472
	xor edx, edx
	mov dl, byte ptr [edi+4]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+5]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10027472
	xor edx, edx
	mov dl, byte ptr [edi+6]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+7]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_10027472
	xor edx, edx
	mov dl, byte ptr [edi+8]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	xor edx, edx
	mov dl, byte ptr [edi+9]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
jmp_10027472:
	pop esi
	test esi, 1
	je jmp_10027491
jmp_1002747b:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	xor edx, edx
	mov dl, byte ptr [edi]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov byte ptr [edi], al
jmp_10027491:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_1002763b
	je jmp_10027505
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10027536
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
jmp_100274de:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_100275ba
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_1002712e
jmp_10027505:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_1002712e
jmp_10027536:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10027552
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10027552:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	jmp jmp_100274de
jmp_100275ba:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_100275d3
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_100275d3:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	jmp jmp_1002712e
jmp_1002763b:
	pop ebp
	ret
jmp_1002763d:
	push ebp
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10027662:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_1002766d
	mov esi, eax
	mov ecx, ebx
jmp_1002766d:
	cmp eax, edi
	jl jmp_10027673
	mov edi, eax
jmp_10027673:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10027662
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10027aa4
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_1002769e:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100276ba
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100276ba:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_1002769e
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
jmp_10027719:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10027732
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10027732:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10027719
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_100277a2:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_100277c0
	xchg ebx, eax
	xchg ecx, edx
jmp_100277c0:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_100277f2
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
jmp_100277f2:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	test edi, 1
	je jmp_1002783c
	mov byte ptr [edi], cl
	dec esi
	je jmp_100278fa
	inc edi
	xchg cl, ch
	xchg ebx, ebp
	add ebp, eax
	adc ch, dl
jmp_1002783c:
	cmp esi, 1
	je jmp_100278f2
	push esi
	shr esi, 1
	dec esi
	cmp esi, 5
	jl jmp_100278a2
jmp_1002784e:
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
	js jmp_100278e9
	cmp esi, 5
	jge jmp_1002784e
jmp_100278a2:
	mov word ptr [edi], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100278e9
	mov word ptr [edi+2], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100278e9
	mov word ptr [edi+4], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100278e9
	mov word ptr [edi+6], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
	dec esi
	js jmp_100278e9
	mov word ptr [edi+8], cx
	add ebx, eax
	adc cl, dl
	add ebp, eax
	adc ch, dl
jmp_100278e9:
	pop esi
	test esi, 1
	je jmp_100278fa
jmp_100278f2:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	mov byte ptr [edi], cl
jmp_100278fa:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10027aa4
	je jmp_1002796e
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1002799f
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
jmp_10027947:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10027a23
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_100277a2
jmp_1002796e:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_100277a2
jmp_1002799f:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100279bb
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100279bb:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	jmp jmp_10027947
jmp_10027a23:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10027a3c
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10027a3c:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	jmp jmp_100277a2
jmp_10027aa4:
	pop ebp
	ret
jmp_10027aa6:
	push ebp
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov eax, dword ptr [g_codeBlockVars+108h]
	cmp dword ptr es:[jmp_10027d50+2], eax
	je jmp_10027b1d
	mov dword ptr es:[jmp_10027d50+2], eax
	mov dword ptr es:[jmp_10027d6f+2], eax
	mov dword ptr es:[jmp_10027d8f+2], eax
	mov dword ptr es:[jmp_10027daf+2], eax
	mov dword ptr es:[jmp_10027de8+2], eax
	mov dword ptr es:[jmp_10027e0f+2], eax
	mov dword ptr es:[jmp_10027e37+2], eax
	mov dword ptr es:[jmp_10027d56+2], eax
	mov dword ptr es:[jmp_10027d75+2], eax
	mov dword ptr es:[jmp_10027d95+2], eax
	mov dword ptr es:[jmp_10027db5+2], eax
	mov dword ptr es:[jmp_10027dee+2], eax
	mov dword ptr es:[jmp_10027e15+2], eax
	mov dword ptr es:[jmp_10027e3d+2], eax
jmp_10027b1d:
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_10027b2d:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10027b38
	mov esi, eax
	mov ecx, ebx
jmp_10027b38:
	cmp eax, edi
	jl jmp_10027b3e
	mov edi, eax
jmp_10027b3e:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_10027b2d
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_10028016
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10027b69:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10027b85
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10027b85:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10027b69
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
jmp_10027be4:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10027bfd
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10027bfd:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10027be4
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_10027c6d:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_10027c8b
	xchg ebx, eax
	xchg ecx, edx
jmp_10027c8b:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_10027cbd
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
jmp_10027cbd:
	shl eax, 10h
	mov dword ptr [g_codeBlockVars+0e8h], eax
	mov dword ptr [g_codeBlockVars+0ech], edx
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov edx, ebp
	shr edx, 10h
	shl ebx, 10h
	shl ebp, 10h
	and ecx, 0ffh
	and edx, 0ffh
	test edi, 1
	je jmp_10027d2a
	mov eax, dword ptr [g_codeBlockVars+108h]
	mov al, byte ptr [eax+ecx]
	mov byte ptr [edi], al
	dec esi
	je jmp_10027e6c
	inc edi
	xchg cl, dl
	xchg ebx, ebp
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc dl, byte ptr [g_codeBlockVars+0ech]
jmp_10027d2a:
	cmp esi, 1
	je jmp_10027e5c
	push esi
	shr esi, 1
	dec esi
	mov dword ptr [g_codeBlockVars+0a0h], esi
	mov esi, dword ptr [g_codeBlockVars+0e8h]
	cmp dword ptr [g_codeBlockVars+0a0h], 3
	jl jmp_10027de8
jmp_10027d50:
	mov al, byte ptr [ecx+8000h]
jmp_10027d56:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi], ax
	add ebx, esi
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, esi
	adc dl, byte ptr [g_codeBlockVars+0ech]
jmp_10027d6f:
	mov al, byte ptr [ecx+8000h]
jmp_10027d75:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi+2], ax
	add ebx, esi
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, esi
	adc dl, byte ptr [g_codeBlockVars+0ech]
jmp_10027d8f:
	mov al, byte ptr [ecx+8000h]
jmp_10027d95:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi+4], ax
	add ebx, esi
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, esi
	adc dl, byte ptr [g_codeBlockVars+0ech]
jmp_10027daf:
	mov al, byte ptr [ecx+8000h]
jmp_10027db5:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi+6], ax
	add edi, 8
	sub dword ptr [g_codeBlockVars+0a0h], 4
	js jmp_10027e47
	add ebx, esi
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, esi
	adc dl, byte ptr [g_codeBlockVars+0ech]
	cmp dword ptr [g_codeBlockVars+0a0h], 3
	jge jmp_10027d50
jmp_10027de8:
	mov al, byte ptr [ecx+8000h]
jmp_10027dee:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi], ax
	dec dword ptr [g_codeBlockVars+0a0h]
	js jmp_10027e47
	add ebx, esi
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, esi
	adc dl, byte ptr [g_codeBlockVars+0ech]
jmp_10027e0f:
	mov al, byte ptr [ecx+8000h]
jmp_10027e15:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi+2], ax
	dec dword ptr [g_codeBlockVars+0a0h]
	js jmp_10027e47
	add ebx, esi
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, esi
	adc dl, byte ptr [g_codeBlockVars+0ech]
jmp_10027e37:
	mov al, byte ptr [ecx+8000h]
jmp_10027e3d:
	mov ah, byte ptr [edx+8000h]
	mov word ptr [edi+4], ax
jmp_10027e47:
	pop esi
	test esi, 1
	je jmp_10027e6c
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
jmp_10027e5c:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	mov eax, dword ptr [g_codeBlockVars+108h]
	mov al, byte ptr [eax+ecx]
	mov byte ptr [edi], al
jmp_10027e6c:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_10028016
	je jmp_10027ee0
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_10027f11
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
jmp_10027eb9:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10027f95
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_10027c6d
jmp_10027ee0:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_10027c6d
jmp_10027f11:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10027f2d
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10027f2d:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	jmp jmp_10027eb9
jmp_10027f95:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_10027fae
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_10027fae:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	jmp jmp_10027c6d
jmp_10028016:
	pop ebp
	ret
jmp_10028018:
	push ebp
	mov eax, dword ptr [g_unk0x10064cd8]
	mov dword ptr [g_codeBlockVars], eax
	mov eax, dword ptr [g_unk0x10064cdc]
	mov dword ptr [g_codeBlockVars+4], eax
	mov ebx, dword ptr [g_codeBlockVars+14h]
	mov esi, 7fffffffh
	mov edi, 80000000h
jmp_1002803d:
	mov eax, dword ptr [ebx+4]
	cmp eax, esi
	jg jmp_10028048
	mov esi, eax
	mov ecx, ebx
jmp_10028048:
	cmp eax, edi
	jl jmp_1002804e
	mov edi, eax
jmp_1002804e:
	add ebx, 18h
	cmp ebx, dword ptr [g_codeBlockVars+18h]
	jne jmp_1002803d
	mov dword ptr [g_codeBlockVars+1ch], ecx
	mov dword ptr [g_codeBlockVars+20h], ecx
	mov dword ptr [g_codeBlockVars+30h], esi
	sub edi, esi
	je jmp_100286a4
	mov dword ptr [g_codeBlockVars+2ch], edi
jmp_10028079:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_10028095
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_10028095:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_10028079
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
jmp_100280f4:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1002810d
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1002810d:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	je jmp_100280f4
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	mov eax, dword ptr [g_codeBlockVars+30h]
	mul dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
jmp_1002817d:
	mov eax, dword ptr [g_codeBlockVars+34h]
	mov ebx, dword ptr [g_codeBlockVars+38h]
	mov ecx, dword ptr [g_codeBlockVars+3ch]
	mov edx, dword ptr [g_codeBlockVars+40h]
	cmp ebx, eax
	jg jmp_1002819b
	xchg ebx, eax
	xchg ecx, edx
jmp_1002819b:
	sar eax, 10h
	sar ebx, 10h
	mov edi, dword ptr [g_codeBlockVars+10h]
	add edi, eax
	sub ebx, eax
	mov esi, ebx
	je jmp_100281cd
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
jmp_100281cd:
	shl eax, 10h
	add ebx, edi
	mov dword ptr [g_codeBlockVars+0ch], ebx
	inc esi
	mov ebx, ecx
	mov ebp, ecx
	add ebx, dword ptr [g_codeBlockVars]
	add ebp, dword ptr [g_codeBlockVars+4]
	mov ecx, ebx
	shr ecx, 10h
	mov dh, cl
	mov ecx, ebp
	shr ecx, 8
	mov cl, dh
	shl ebx, 10h
	shl ebp, 10h
	cmp esi, 1
	je jmp_100284e2
	push esi
	shr esi, 1
	dec esi
	mov dword ptr [g_codeBlockVars+0e8h], eax
	mov dword ptr [g_codeBlockVars+0ech], edx
	cmp esi, 5
	jl jmp_1002839a
jmp_1002821e:
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+0ah], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	add edi, 0ch
	sub esi, 6
	js jmp_100284d9
	cmp esi, 5
	jge jmp_1002821e
jmp_1002839a:
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100284d9
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+2], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100284d9
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+4], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100284d9
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+6], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
	dec esi
	js jmp_100284d9
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, ch
	add edx, dword ptr [g_codeBlockVars+104h]
	mov ah, byte ptr [edx]
	mov word ptr [edi+8], ax
	add ebx, dword ptr [g_codeBlockVars+0e8h]
	adc cl, byte ptr [g_codeBlockVars+0ech]
	add ebp, dword ptr [g_codeBlockVars+0e8h]
	adc ch, byte ptr [g_codeBlockVars+0ech]
jmp_100284d9:
	pop esi
	test esi, 1
	je jmp_100284fa
jmp_100284e2:
	mov edi, dword ptr [g_codeBlockVars+0ch]
	mov edx, dword ptr [g_codeBlockVars+98h]
	mov dh, cl
	add edx, dword ptr [g_codeBlockVars+104h]
	mov al, byte ptr [edx]
	mov byte ptr [edi], al
jmp_100284fa:
	mov eax, dword ptr [g_codeBlockVars]
	mov ebx, dword ptr [g_codeBlockVars+4]
	mov dword ptr [g_codeBlockVars+4], eax
	mov dword ptr [g_codeBlockVars], ebx
	mov eax, dword ptr [g_codeBlockVars+8]
	add dword ptr [g_codeBlockVars+10h], eax
	dec dword ptr [g_codeBlockVars+2ch]
	js jmp_100286a4
	je jmp_1002856e
	dec dword ptr [g_codeBlockVars+24h]
	je jmp_1002859f
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
jmp_10028547:
	dec dword ptr [g_codeBlockVars+28h]
	je jmp_10028623
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_1002817d
jmp_1002856e:
	mov eax, dword ptr [g_codeBlockVars+5ch]
	add dword ptr [g_codeBlockVars+34h], eax
	mov eax, dword ptr [g_codeBlockVars+64h]
	add dword ptr [g_codeBlockVars+3ch], eax
	mov eax, dword ptr [g_codeBlockVars+60h]
	add dword ptr [g_codeBlockVars+38h], eax
	mov eax, dword ptr [g_codeBlockVars+68h]
	add dword ptr [g_codeBlockVars+40h], eax
	jmp jmp_1002817d
jmp_1002859f:
	mov ebx, dword ptr [g_codeBlockVars+1ch]
	mov esi, ebx
	sub esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+14h]
	jge jmp_100285bb
	mov esi, dword ptr [g_codeBlockVars+18h]
	sub esi, 18h
jmp_100285bb:
	mov dword ptr [g_codeBlockVars+1ch], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+24h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+5ch], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+34h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+64h], eax
	mov dword ptr [g_codeBlockVars+3ch], edi
	jmp jmp_10028547
jmp_10028623:
	mov ebx, dword ptr [g_codeBlockVars+20h]
	mov esi, ebx
	add esi, 18h
	cmp esi, dword ptr [g_codeBlockVars+18h]
	jl jmp_1002863c
	mov esi, dword ptr [g_codeBlockVars+14h]
jmp_1002863c:
	mov dword ptr [g_codeBlockVars+20h], esi
	mov eax, dword ptr [ebx+4]
	mov ecx, dword ptr [esi+4]
	sub ecx, eax
	cmp ecx, 1
	adc ecx, 0
	mov dword ptr [g_codeBlockVars+28h], ecx
	shl ecx, 10h
	mov edi, dword ptr [ebx]
	mov edx, dword ptr [esi]
	sub edx, edi
	shl edx, 10h
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+60h], eax
	shl edi, 10h
	add edi, 8000h
	mov dword ptr [g_codeBlockVars+38h], edi
	mov edi, dword ptr [ebx+8]
	mov edx, dword ptr [esi+8]
	sub edx, edi
	xor eax, eax
	shrd eax, edx, 10h
	sar edx, 10h
	idiv ecx
	mov dword ptr [g_codeBlockVars+68h], eax
	mov dword ptr [g_codeBlockVars+40h], edi
	jmp jmp_1002817d
jmp_100286a4:
	pop ebp
	ret
CodeBlock endp

; Stores its arguments in g_unk0x10064cd8 and g_unk0x10064cdc. Nothing calls it, and only the
; untranscribed code block reads the pair, so it stays unnamed with them.
FUN_100286a6 proc uses ebx esi edi, p_unk0x00:dword, p_unk0x04:dword
	push es
	mov eax, dword ptr p_unk0x00
	mov dword ptr [g_unk0x10064cd8], eax
	mov eax, dword ptr p_unk0x04
	mov dword ptr [g_unk0x10064cdc], eax
	pop es
	ret
FUN_100286a6 endp

; Stores the self-modifying code block's start and the code segment, and returns the block's
; size (it ends where FUN_100286a6 starts).
GetCodeBlock proc uses ebx esi edi, p_start:dword, p_segment:dword
	push es
	xor ebx, ebx
	mov eax, dword ptr p_segment
	mov bx, cs
	mov dword ptr [eax], ebx
	mov eax, dword ptr p_start
	mov ebx, offset CodeBlock
	mov dword ptr [eax], ebx
	mov eax, offset FUN_100286a6
	sub eax, ebx
	pop es
	ret
GetCodeBlock endp

; Sets up the working variables for a list of six-dword vertices and calls the table's routine
; p_index, if it has one.
CallCodeBlockRoutine proc uses ebx esi edi, p_view:dword, p_vertices:dword, p_count:dword, p_index:dword, p_unk0x18:dword, p_unk0x1c:dword, p_unk0x20:dword, p_unk0x24:dword
	push es
	push ds
	pop es
	mov edi, dword ptr p_view
	mov edi, dword ptr [edi]
	mov eax, dword ptr [edi+4]
	inc eax
	mov dword ptr [g_codeBlockVars+8], eax
	mov eax, dword ptr [edi]
	mov dword ptr [g_codeBlockVars+10h], eax
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_codeBlockVars+14h], ebx
	mov dword ptr [g_codeBlockVars+18h], eax
	mov eax, dword ptr p_unk0x1c
	mov dword ptr [g_codeBlockVars+100h], eax
	mov eax, dword ptr p_unk0x20
	mov dword ptr [g_codeBlockVars+104h], eax
	mov eax, dword ptr p_unk0x24
	mov dword ptr [g_codeBlockVars+108h], eax
	mov eax, dword ptr p_unk0x18
	mov dword ptr [g_codeBlockVars+98h], eax
	mov ebx, dword ptr p_index
	mov eax, dword ptr [g_codeBlockRoutines+ebx*4]
	cmp eax, 0
	je jmp_10028756
	call eax
jmp_10028756:
	pop es
	ret
CallCodeBlockRoutine endp

; Divides two fixed-point values into a 2.30 result, rounded: (p_a << 30) / p_b.
FixedDiv30 proc uses ebx, p_a:dword, p_b:dword
	sub ecx, ecx
	mov eax, dword ptr p_a
	and eax, eax
	jns jmp_1002876c
	inc ecx
	neg eax
jmp_1002876c:
	mov edx, eax
	sar edx, 2
	shl eax, 1eh
	mov ebx, dword ptr p_b
	and ebx, ebx
	jns jmp_1002877e
	dec ecx
	neg ebx
jmp_1002877e:
	div ebx
	shr ebx, 1
	adc ebx, 0
	dec ebx
	cmp ebx, edx
	adc eax, 0
	and ecx, ecx
	je jmp_10028791
	neg eax
jmp_10028791:
	ret
FixedDiv30 endp

; The rounded reciprocal of p_value: 2^46 / p_value.
FixedReciprocal30 proc uses ebx, p_value:dword
	sub ecx, ecx
	mov edx, 4000h
	xor eax, eax
	mov ebx, dword ptr p_value
	and ebx, ebx
	jns jmp_100287ab
	dec ecx
	neg ebx
jmp_100287ab:
	div ebx
	shr ebx, 1
	adc ebx, 0
	dec ebx
	cmp ebx, edx
	adc eax, 0
	and ecx, ecx
	je jmp_100287be
	neg eax
jmp_100287be:
	ret
	align 4
FixedReciprocal30 endp

; Multiplies two 2.30 fixed-point values, rounded. fixedmul30.c's FixedMul30 has the name in MW2.
CodeBlockFixedMul30 proc p_a:dword, p_b:dword
	mov eax, dword ptr p_a
	imul dword ptr p_b
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	ret
	align 4
CodeBlockFixedMul30 endp

; CallCodeBlockRoutine for a polygon that may cross the target's edges: clips the list of
; p_count six-dword vertices against each edge of the target's rectangle it crosses (the passes
; alternate between p_vertices and g_codeBlockClipVertices), then calls the table's routine
; p_index on what is left, if it has at least three vertices.
CallCodeBlockRoutineClipped proc p_view:dword, p_vertices:dword, p_count:dword, p_index:dword, p_unk0x18:dword, p_texture:dword, p_luma:dword, p_unk0x24:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword, l_unk0x18:dword
	local l_unk0x1c:dword, l_unk0x20:dword
	push es
	push esi
	push edi
	push ebx
	push ds
	pop es
	cld
	mov dword ptr l_unk0x1c, 0
	mov esi, dword ptr p_vertices
	mov edi, dword ptr p_view
	mov eax, dword ptr [edi+0ch]
	mov ebx, dword ptr [edi+4]
	mov ecx, dword ptr [edi+10h]
	mov edx, dword ptr [edi+8]
	mov dword ptr l_unk0x04, eax
	mov dword ptr l_unk0x0c, ebx
	mov dword ptr l_unk0x08, ecx
	mov dword ptr l_unk0x10, edx
	mov edi, dword ptr [edi]
	mov eax, dword ptr [edi+4]
	inc eax
	mov dword ptr [g_codeBlockVars+8], eax
	mov eax, dword ptr [edi]
	mov dword ptr [g_codeBlockVars+10h], eax
	mov ecx, dword ptr p_count
jmp_10028827:
	mov edx, 0
	mov eax, dword ptr l_unk0x04
	mov ebx, dword ptr [esi]
	sub eax, ebx
	shld edx, eax, 1
	mov eax, dword ptr l_unk0x0c
	sub ebx, eax
	shld edx, ebx, 1
	mov eax, dword ptr l_unk0x08
	mov ebx, dword ptr [esi+4]
	sub eax, ebx
	shld edx, eax, 1
	mov eax, dword ptr l_unk0x10
	sub ebx, eax
	shld edx, ebx, 1
	or dword ptr l_unk0x1c, edx
	add esi, 18h
	loop jmp_10028827
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, offset g_codeBlockClipVertices
	mov dword ptr l_unk0x18, 0
	cmp dword ptr l_unk0x1c, 0
	je jmp_10028fb0
	mov dword ptr l_unk0x18, 0
	mov dword ptr l_unk0x20, 0
	mov eax, dword ptr p_index
	and eax, 400h
	mov dword ptr [g_unk0x100665ec], eax
	mov ecx, 0
	mov eax, dword ptr p_index
	and eax, 600h
	cmp eax, 600h
	jne jmp_100288ba
	mov ecx, 1
jmp_100288ba:
	mov dword ptr [g_unk0x100665f0], ecx
	mov ecx, 1
	mov eax, dword ptr p_index
	and eax, 1c0h
	cmp eax, 40h
	je jmp_100288f5
	cmp eax, 0c0h
	je jmp_100288f5
	cmp eax, 100h
	je jmp_100288f5
	mov eax, dword ptr p_index
	and eax, 7
	cmp eax, 4
	je jmp_100288f5
	cmp eax, 7
	je jmp_100288f5
	mov ecx, 0
jmp_100288f5:
	mov dword ptr [g_unk0x100665f4], ecx
	mov eax, dword ptr l_unk0x1c
	and eax, 8
	je jmp_10028a8b
	mov ecx, dword ptr p_count
jmp_1002890a:
	mov eax, dword ptr [esi]
	mov edx, dword ptr [ebx]
	cmp eax, dword ptr l_unk0x04
	jg jmp_10028947
	push eax
	mov eax, dword ptr [esi]
	mov dword ptr [edi], eax
	mov eax, dword ptr [esi+4]
	mov dword ptr [edi+4], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [edi+8], eax
	mov eax, dword ptr [esi+0ch]
	mov dword ptr [edi+0ch], eax
	mov eax, dword ptr [esi+10h]
	mov dword ptr [edi+10h], eax
	mov eax, dword ptr [esi+14h]
	mov dword ptr [edi+14h], eax
	add edi, 18h
	pop eax
	inc dword ptr l_unk0x18
	cmp edx, dword ptr l_unk0x04
	jg jmp_10028950
	jmp jmp_10028a4e
jmp_10028947:
	cmp edx, dword ptr l_unk0x04
	jg jmp_10028a4e
jmp_10028950:
	sub eax, edx
	neg edx
	add edx, dword ptr l_unk0x04
	push ebx
	push ecx
	push eax
	sub ecx, ecx
	mov eax, edx
	and eax, eax
	jns jmp_10028965
	inc ecx
	neg eax
jmp_10028965:
	mov edx, eax
	sar edx, 2
	shl eax, 1eh
	pop ebx
	and ebx, ebx
	jns jmp_10028975
	dec ecx
	neg ebx
jmp_10028975:
	div ebx
	shr ebx, 1
	adc ebx, 0
	dec ebx
	cmp ebx, edx
	adc eax, 0
	and ecx, ecx
	je jmp_10028988
	neg eax
jmp_10028988:
	pop ecx
	pop ebx
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr l_unk0x04
	mov dword ptr [edi], eax
	mov eax, dword ptr [esi+4]
	sub eax, dword ptr [ebx+4]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+4]
	add edx, eax
	mov dword ptr [edi+4], edx
	cmp dword ptr [g_unk0x100665ec], 0
	je jmp_100289f8
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [ebx+0ch]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+0ch]
	add edx, eax
	mov dword ptr [edi+0ch], edx
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [ebx+10h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+10h]
	add edx, eax
	mov dword ptr [edi+10h], edx
jmp_100289f8:
	cmp dword ptr [g_unk0x100665f0], 0
	je jmp_10028a20
	mov eax, dword ptr [esi+14h]
	sub eax, dword ptr [ebx+14h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+14h]
	add edx, eax
	mov dword ptr [edi+14h], edx
jmp_10028a20:
	cmp dword ptr [g_unk0x100665f4], 0
	je jmp_10028a48
	mov eax, dword ptr [esi+8]
	sub eax, dword ptr [ebx+8]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+8]
	add edx, eax
	mov dword ptr [edi+8], edx
jmp_10028a48:
	add edi, 18h
	inc dword ptr l_unk0x18
jmp_10028a4e:
	mov esi, ebx
	add ebx, 18h
	dec ecx
	jne jmp_1002890a
	inc dword ptr l_unk0x20
	mov eax, dword ptr l_unk0x18
	mov dword ptr p_count, eax
	cmp eax, 0
	je jmp_10028fb0
	mov ebx, offset g_codeBlockClipVertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, dword ptr p_vertices
	mov dword ptr l_unk0x18, 0
jmp_10028a8b:
	mov eax, dword ptr l_unk0x1c
	and eax, 1
	je jmp_10028c43
	mov ecx, dword ptr p_count
jmp_10028a9a:
	mov eax, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp eax, dword ptr l_unk0x10
	jl jmp_10028ad9
	push eax
	mov eax, dword ptr [esi]
	mov dword ptr [edi], eax
	mov eax, dword ptr [esi+4]
	mov dword ptr [edi+4], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [edi+8], eax
	mov eax, dword ptr [esi+0ch]
	mov dword ptr [edi+0ch], eax
	mov eax, dword ptr [esi+10h]
	mov dword ptr [edi+10h], eax
	mov eax, dword ptr [esi+14h]
	mov dword ptr [edi+14h], eax
	add edi, 18h
	pop eax
	inc dword ptr l_unk0x18
	cmp edx, dword ptr l_unk0x10
	jl jmp_10028ae2
	jmp jmp_10028bdd
jmp_10028ad9:
	cmp edx, dword ptr l_unk0x10
	jl jmp_10028bdd
jmp_10028ae2:
	sub eax, edx
	neg edx
	add edx, dword ptr l_unk0x10
	push ebx
	push ecx
	push eax
	sub ecx, ecx
	mov eax, edx
	and eax, eax
	jns jmp_10028af7
	inc ecx
	neg eax
jmp_10028af7:
	mov edx, eax
	sar edx, 2
	shl eax, 1eh
	pop ebx
	and ebx, ebx
	jns jmp_10028b07
	dec ecx
	neg ebx
jmp_10028b07:
	div ebx
	shr ebx, 1
	adc ebx, 0
	dec ebx
	cmp ebx, edx
	adc eax, 0
	and ecx, ecx
	je jmp_10028b1a
	neg eax
jmp_10028b1a:
	pop ecx
	pop ebx
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr l_unk0x10
	mov dword ptr [edi+4], eax
	mov eax, dword ptr [esi]
	sub eax, dword ptr [ebx]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx]
	add edx, eax
	mov dword ptr [edi], edx
	cmp dword ptr [g_unk0x100665ec], 0
	je jmp_10028b87
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [ebx+0ch]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+0ch]
	add edx, eax
	mov dword ptr [edi+0ch], edx
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [ebx+10h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+10h]
	add edx, eax
	mov dword ptr [edi+10h], edx
jmp_10028b87:
	cmp dword ptr [g_unk0x100665f0], 0
	je jmp_10028baf
	mov eax, dword ptr [esi+14h]
	sub eax, dword ptr [ebx+14h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+14h]
	add edx, eax
	mov dword ptr [edi+14h], edx
jmp_10028baf:
	cmp dword ptr [g_unk0x100665f4], 0
	je jmp_10028bd7
	mov eax, dword ptr [esi+8]
	sub eax, dword ptr [ebx+8]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+8]
	add edx, eax
	mov dword ptr [edi+8], edx
jmp_10028bd7:
	add edi, 18h
	inc dword ptr l_unk0x18
jmp_10028bdd:
	mov esi, ebx
	add ebx, 18h
	dec ecx
	jne jmp_10028a9a
	mov eax, dword ptr l_unk0x18
	mov dword ptr p_count, eax
	cmp eax, 0
	je jmp_10028fb0
	inc dword ptr l_unk0x20
	mov eax, dword ptr l_unk0x20
	and eax, 1
	je jmp_10028c24
	mov ebx, offset g_codeBlockClipVertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, dword ptr p_vertices
	mov dword ptr l_unk0x18, 0
	jmp jmp_10028c43
jmp_10028c24:
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, offset g_codeBlockClipVertices
	mov dword ptr l_unk0x18, 0
jmp_10028c43:
	mov eax, dword ptr l_unk0x1c
	and eax, 4
	je jmp_10028dfc
	mov ecx, dword ptr p_count
jmp_10028c52:
	mov eax, dword ptr [esi]
	mov edx, dword ptr [ebx]
	cmp eax, dword ptr l_unk0x0c
	jl jmp_10028c8f
	push eax
	mov eax, dword ptr [esi]
	mov dword ptr [edi], eax
	mov eax, dword ptr [esi+4]
	mov dword ptr [edi+4], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [edi+8], eax
	mov eax, dword ptr [esi+0ch]
	mov dword ptr [edi+0ch], eax
	mov eax, dword ptr [esi+10h]
	mov dword ptr [edi+10h], eax
	mov eax, dword ptr [esi+14h]
	mov dword ptr [edi+14h], eax
	add edi, 18h
	pop eax
	inc dword ptr l_unk0x18
	cmp edx, dword ptr l_unk0x0c
	jl jmp_10028c98
	jmp jmp_10028d96
jmp_10028c8f:
	cmp edx, dword ptr l_unk0x0c
	jl jmp_10028d96
jmp_10028c98:
	sub eax, edx
	neg edx
	add edx, dword ptr l_unk0x0c
	push ebx
	push ecx
	push eax
	sub ecx, ecx
	mov eax, edx
	and eax, eax
	jns jmp_10028cad
	inc ecx
	neg eax
jmp_10028cad:
	mov edx, eax
	sar edx, 2
	shl eax, 1eh
	pop ebx
	and ebx, ebx
	jns jmp_10028cbd
	dec ecx
	neg ebx
jmp_10028cbd:
	div ebx
	shr ebx, 1
	adc ebx, 0
	dec ebx
	cmp ebx, edx
	adc eax, 0
	and ecx, ecx
	je jmp_10028cd0
	neg eax
jmp_10028cd0:
	pop ecx
	pop ebx
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr l_unk0x0c
	mov dword ptr [edi], eax
	mov eax, dword ptr [esi+4]
	sub eax, dword ptr [ebx+4]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+4]
	add edx, eax
	mov dword ptr [edi+4], edx
	cmp dword ptr [g_unk0x100665ec], 0
	je jmp_10028d40
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [ebx+0ch]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+0ch]
	add edx, eax
	mov dword ptr [edi+0ch], edx
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [ebx+10h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+10h]
	add edx, eax
	mov dword ptr [edi+10h], edx
jmp_10028d40:
	cmp dword ptr [g_unk0x100665f0], 0
	je jmp_10028d68
	mov eax, dword ptr [esi+14h]
	sub eax, dword ptr [ebx+14h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+14h]
	add edx, eax
	mov dword ptr [edi+14h], edx
jmp_10028d68:
	cmp dword ptr [g_unk0x100665f4], 0
	je jmp_10028d90
	mov eax, dword ptr [esi+8]
	sub eax, dword ptr [ebx+8]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+8]
	add edx, eax
	mov dword ptr [edi+8], edx
jmp_10028d90:
	add edi, 18h
	inc dword ptr l_unk0x18
jmp_10028d96:
	mov esi, ebx
	add ebx, 18h
	dec ecx
	jne jmp_10028c52
	mov eax, dword ptr l_unk0x18
	mov dword ptr p_count, eax
	cmp eax, 0
	je jmp_10028fb0
	inc dword ptr l_unk0x20
	mov eax, dword ptr l_unk0x20
	and eax, 1
	je jmp_10028ddd
	mov ebx, offset g_codeBlockClipVertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, dword ptr p_vertices
	mov dword ptr l_unk0x18, 0
	jmp jmp_10028dfc
jmp_10028ddd:
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, offset g_codeBlockClipVertices
	mov dword ptr l_unk0x18, 0
jmp_10028dfc:
	mov eax, dword ptr l_unk0x1c
	and eax, 2
	je jmp_10028fb0
	mov ecx, dword ptr p_count
jmp_10028e0b:
	mov eax, dword ptr [esi+4]
	mov edx, dword ptr [ebx+4]
	cmp eax, dword ptr l_unk0x08
	jg jmp_10028e4a
	push eax
	mov eax, dword ptr [esi]
	mov dword ptr [edi], eax
	mov eax, dword ptr [esi+4]
	mov dword ptr [edi+4], eax
	mov eax, dword ptr [esi+8]
	mov dword ptr [edi+8], eax
	mov eax, dword ptr [esi+0ch]
	mov dword ptr [edi+0ch], eax
	mov eax, dword ptr [esi+10h]
	mov dword ptr [edi+10h], eax
	mov eax, dword ptr [esi+14h]
	mov dword ptr [edi+14h], eax
	add edi, 18h
	pop eax
	inc dword ptr l_unk0x18
	cmp edx, dword ptr l_unk0x08
	jg jmp_10028e53
	jmp jmp_10028f4e
jmp_10028e4a:
	cmp edx, dword ptr l_unk0x08
	jg jmp_10028f4e
jmp_10028e53:
	sub eax, edx
	neg edx
	add edx, dword ptr l_unk0x08
	push ebx
	push ecx
	push eax
	sub ecx, ecx
	mov eax, edx
	and eax, eax
	jns jmp_10028e68
	inc ecx
	neg eax
jmp_10028e68:
	mov edx, eax
	sar edx, 2
	shl eax, 1eh
	pop ebx
	and ebx, ebx
	jns jmp_10028e78
	dec ecx
	neg ebx
jmp_10028e78:
	div ebx
	shr ebx, 1
	adc ebx, 0
	dec ebx
	cmp ebx, edx
	adc eax, 0
	and ecx, ecx
	je jmp_10028e8b
	neg eax
jmp_10028e8b:
	pop ecx
	pop ebx
	mov dword ptr l_unk0x14, eax
	mov eax, dword ptr l_unk0x08
	mov dword ptr [edi+4], eax
	mov eax, dword ptr [esi]
	sub eax, dword ptr [ebx]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx]
	add edx, eax
	mov dword ptr [edi], edx
	cmp dword ptr [g_unk0x100665ec], 0
	je jmp_10028ef8
	mov eax, dword ptr [esi+0ch]
	sub eax, dword ptr [ebx+0ch]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+0ch]
	add edx, eax
	mov dword ptr [edi+0ch], edx
	mov eax, dword ptr [esi+10h]
	sub eax, dword ptr [ebx+10h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+10h]
	add edx, eax
	mov dword ptr [edi+10h], edx
jmp_10028ef8:
	cmp dword ptr [g_unk0x100665f0], 0
	je jmp_10028f20
	mov eax, dword ptr [esi+14h]
	sub eax, dword ptr [ebx+14h]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+14h]
	add edx, eax
	mov dword ptr [edi+14h], edx
jmp_10028f20:
	cmp dword ptr [g_unk0x100665f4], 0
	je jmp_10028f48
	mov eax, dword ptr [esi+8]
	sub eax, dword ptr [ebx+8]
	imul dword ptr l_unk0x14
	add eax, 20000000h
	adc edx, 0
	shld edx, eax, 2
	mov eax, edx
	mov edx, dword ptr [ebx+8]
	add edx, eax
	mov dword ptr [edi+8], edx
jmp_10028f48:
	add edi, 18h
	inc dword ptr l_unk0x18
jmp_10028f4e:
	mov esi, ebx
	add ebx, 18h
	dec ecx
	jne jmp_10028e0b
	mov eax, dword ptr l_unk0x18
	mov dword ptr p_count, eax
	cmp eax, 0
	je jmp_10028fb0
	inc dword ptr l_unk0x20
	mov eax, dword ptr l_unk0x20
	and eax, 1
	je jmp_10028f91
	mov ebx, offset g_codeBlockClipVertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, dword ptr p_vertices
	mov dword ptr l_unk0x18, 0
	jmp jmp_10028fb0
jmp_10028f91:
	mov ebx, dword ptr p_vertices
	mov eax, dword ptr p_count
	dec eax
	mov esi, eax
	shl eax, 4
	shl esi, 3
	add esi, eax
	add esi, ebx
	mov edi, offset g_codeBlockClipVertices
	mov dword ptr l_unk0x18, 0
jmp_10028fb0:
	mov eax, dword ptr p_count
	cmp eax, 3
	jl jmp_10028fff
	shl eax, 3
	mov edx, eax
	shl eax, 1
	add eax, edx
	add eax, ebx
	mov dword ptr [g_codeBlockVars+14h], ebx
	mov dword ptr [g_codeBlockVars+18h], eax
	mov eax, dword ptr p_texture
	mov dword ptr [g_codeBlockVars+100h], eax
	mov eax, dword ptr p_luma
	mov dword ptr [g_codeBlockVars+104h], eax
	mov eax, dword ptr p_unk0x24
	mov dword ptr [g_codeBlockVars+108h], eax
	mov eax, dword ptr p_unk0x18
	mov dword ptr [g_codeBlockVars+98h], eax
	mov ebx, dword ptr p_index
	mov eax, dword ptr [g_codeBlockRoutines+ebx*4]
	cmp eax, 0
	je jmp_10028fff
	call eax
jmp_10028fff:
	pop ebx
	pop edi
	pop esi
	pop es
	ret
CallCodeBlockRoutineClipped endp

	end
