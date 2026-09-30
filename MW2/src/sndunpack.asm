; Hand-written assembly: the sound-block decoder, a MASM object assembled with MASM 6.11 (ML). It
; starts at 0x1001a63c, flush against loadres.c's code, and its data at 0x100a2f04. It uses short
; jumps and loop, and FUN_1001a63c has ML's frame (add esp, -N for its locals); its two helpers
; take their arguments in registers. Annotated by name in sndunpack.h; COMPAT_MODE builds take
; sndunpack.c's stubs.

	.386
	.model flat, c
	option noscoped
	option casemap:none

	.data

; FUN_1001a87b's and FUN_1001a8d3's output, copied back over their input.
	public g_unk0x100a2f04
g_unk0x100a2f04_t struct
m_data db 400h dup (0)
g_unk0x100a2f04_t ends
g_unk0x100a2f04 g_unk0x100a2f04_t <>

; One decoded frame.
	public g_unk0x100a3304
g_unk0x100a3304_t struct
m_data db 401h dup (0)
g_unk0x100a3304_t ends
g_unk0x100a3304 g_unk0x100a3304_t <>

; The delta table of the current frame: 2, 4 or 16 dwords.
	public g_unk0x100a3705
g_unk0x100a3705_t struct
m_data db 43h dup (0)
g_unk0x100a3705_t ends
g_unk0x100a3705 g_unk0x100a3705_t <>

	.code

; Decodes p_count frames of p_frameSize 8-bit samples from p_src to p_dst and returns the end of
; p_src. Each frame starts with a byte whose top two bits select the upsampling (1: x2, 2: x4) and
; whose low four the coding: 0 silence, 1 the previous frame again, 2-4 1-, 2- or 4-bit indices
; into a delta table that follows the byte, clamped to -127..127 around the running value in
; *p_state, and 5 raw samples.
FUN_1001a63c proc p_src:dword, p_dst:dword, p_count:dword, p_frameSize:dword, p_state:dword
	local l_unk0x04:dword, l_unk0x08:dword, l_unk0x0c:dword, l_unk0x10:dword, l_unk0x14:dword
	push ds
	push es
	push ebx
	push esi
	push edi
	push ds
	pop es
	cld
	mov edi, dword ptr p_dst
	mov dword ptr l_unk0x14, edi
	mov edi, dword ptr p_src
	mov ecx, dword ptr p_count
jmp_1001a656:
	push ecx
	xor eax, eax
	mov al, byte ptr [edi]
	shr al, 6
	mov dword ptr l_unk0x08, eax
	mov ecx, dword ptr p_frameSize
	mov dword ptr l_unk0x10, ecx
	cmp al, 1
	jne jmp_1001a66f
	shr ecx, 1
	jmp jmp_1001a676
jmp_1001a66f:
	cmp al, 2
	jne jmp_1001a676
	shr ecx, 2
jmp_1001a676:
	mov dword ptr l_unk0x0c, ecx
	mov al, byte ptr [edi]
	and al, 0fh
	inc edi
	mov ebx, dword ptr p_state
	mov ebx, dword ptr [ebx]
	mov dword ptr l_unk0x04, ebx
	cmp al, 0
	jne jmp_1001a6a1
	push edi
	mov edi, offset g_unk0x100a3304
	mov al, 80h
	rep stosb
	mov dword ptr l_unk0x04, 0
	pop edi
	jmp jmp_1001a827
jmp_1001a6a1:
	cmp al, 1
	jne jmp_1001a6aa
	jmp jmp_1001a827
jmp_1001a6aa:
	cmp al, 2
	jne jmp_1001a71f
	xor eax, eax
	mov al, byte ptr [edi]
	inc edi
	shl eax, 1
	sub eax, 80h
	mov dword ptr [g_unk0x100a3705], eax
	xor eax, eax
	mov al, byte ptr [edi]
	inc edi
	shl eax, 1
	sub eax, 80h
	mov dword ptr [g_unk0x100a3705+4], eax
	xor esi, esi
jmp_1001a6d2:
	mov bl, byte ptr [edi]
	inc edi
	mov al, bl
	mov ecx, 8
jmp_1001a6dc:
	and ebx, 1
	mov edx, dword ptr [g_unk0x100a3705+ebx*4]
	shr al, 1
	mov bl, al
	add edx, dword ptr l_unk0x04
	cmp edx, 7fh
	jle jmp_1001a6f9
	mov edx, 7fh
	jmp jmp_1001a703
jmp_1001a6f9:
	cmp edx, -7fh
	jge jmp_1001a703
	mov edx, 0ffffff81h
jmp_1001a703:
	mov dword ptr l_unk0x04, edx
	add edx, 80h
	mov byte ptr [g_unk0x100a3304+esi], dl
	inc esi
	loop jmp_1001a6dc
	cmp esi, dword ptr l_unk0x0c
	jb jmp_1001a6d2
	jmp jmp_1001a827
jmp_1001a71f:
	cmp al, 3
	jne jmp_1001a792
	mov ecx, 4
	xor esi, esi
jmp_1001a72a:
	xor eax, eax
	mov al, byte ptr [edi]
	shl eax, 1
	sub eax, 80h
	mov dword ptr [g_unk0x100a3705+esi*1], eax
	inc edi
	add esi, 4
	loop jmp_1001a72a
	xor esi, esi
jmp_1001a744:
	mov bl, byte ptr [edi]
	inc edi
	mov al, bl
	mov ecx, 4
jmp_1001a74e:
	and ebx, 3
	mov edx, dword ptr [g_unk0x100a3705+ebx*4]
	shr al, 2
	mov bl, al
	add edx, dword ptr l_unk0x04
	cmp edx, 7fh
	jle jmp_1001a76c
	mov edx, 7fh
	jmp jmp_1001a776
jmp_1001a76c:
	cmp edx, -7fh
	jge jmp_1001a776
	mov edx, 0ffffff81h
jmp_1001a776:
	mov dword ptr l_unk0x04, edx
	add edx, 80h
	mov byte ptr [g_unk0x100a3304+esi], dl
	inc esi
	loop jmp_1001a74e
	cmp esi, dword ptr l_unk0x0c
	jb jmp_1001a744
	jmp jmp_1001a827
jmp_1001a792:
	cmp al, 4
	jne jmp_1001a801
	mov ecx, 10h
	xor esi, esi
jmp_1001a79d:
	xor eax, eax
	mov al, byte ptr [edi]
	shl eax, 1
	sub eax, 80h
	mov dword ptr [g_unk0x100a3705+esi], eax
	inc edi
	add esi, 4
	loop jmp_1001a79d
	xor esi, esi
jmp_1001a7b6:
	mov bl, byte ptr [edi]
	inc edi
	mov al, bl
	mov ecx, 2
jmp_1001a7c0:
	and ebx, 0fh
	mov edx, dword ptr [g_unk0x100a3705+ebx*4]
	shr al, 4
	mov bl, al
	add edx, dword ptr l_unk0x04
	cmp edx, 7fh
	jle jmp_1001a7de
	mov edx, 7fh
	jmp jmp_1001a7e8
jmp_1001a7de:
	cmp edx, -7fh
	jge jmp_1001a7e8
	mov edx, 0ffffff81h
jmp_1001a7e8:
	mov dword ptr l_unk0x04, edx
	add edx, 80h
	mov byte ptr [g_unk0x100a3304+esi], dl
	inc esi
	loop jmp_1001a7c0
	cmp esi, dword ptr l_unk0x0c
	jb jmp_1001a7b6
	jmp jmp_1001a827
jmp_1001a801:
	cmp al, 5
	jne jmp_1001a825
	xor esi, esi
jmp_1001a807:
	mov al, byte ptr [edi]
	mov byte ptr [g_unk0x100a3304+esi], al
	inc esi
	inc edi
	loop jmp_1001a807
	xor eax, eax
	mov al, byte ptr [g_unk0x100a2f04+3ffh+esi]
	sub eax, 80h
	mov dword ptr l_unk0x04, eax
	jmp jmp_1001a827
jmp_1001a825:
	jmp jmp_1001a86a
jmp_1001a827:
	push edi
	mov edi, offset g_unk0x100a3304
	mov ecx, dword ptr l_unk0x10
	cmp dword ptr l_unk0x08, 1
	jne jmp_1001a83d
	call FUN_1001a8d3
	jmp jmp_1001a848
jmp_1001a83d:
	cmp dword ptr l_unk0x08, 2
	jne jmp_1001a848
	call FUN_1001a87b
jmp_1001a848:
	mov edi, dword ptr l_unk0x04
	mov ebx, dword ptr p_state
	mov dword ptr [ebx], edi
	mov edi, dword ptr l_unk0x14
	mov esi, offset g_unk0x100a3304
	mov ecx, dword ptr l_unk0x10
	rep movsb
	mov dword ptr l_unk0x14, edi
	pop edi
	pop ecx
	dec ecx
	je jmp_1001a872
	jmp jmp_1001a656
jmp_1001a86a:
	pop ecx
	mov eax, 0
	mov edi, eax
jmp_1001a872:
	mov eax, edi
	pop edi
	pop esi
	pop ebx
	pop es
	pop ds
	ret
FUN_1001a63c endp

; Upsamples the ecx samples at edi four times, interpolating linearly.
FUN_1001a87b proc
	push ebp
	dec ecx
	xor esi, esi
	xor ah, ah
jmp_1001a881:
	mov ebx, esi
	shr ebx, 2
	mov al, byte ptr [edi+ebx]
	mov byte ptr [g_unk0x100a2f04+esi], al
	xor edx, edx
	mov dl, byte ptr [edi+ebx+1]
	add edx, eax
	shr edx, 1
	mov ebp, edx
	mov byte ptr [g_unk0x100a2f04+2+esi], dl
	add edx, eax
	shr edx, 1
	mov byte ptr [g_unk0x100a2f04+1+esi], dl
	mov edx, ebp
	mov al, byte ptr [edi+ebx+1]
	add edx, eax
	shr edx, 1
	mov byte ptr [g_unk0x100a2f04+3+esi], dl
	add esi, 4
	cmp esi, ecx
	jb jmp_1001a881
	inc ecx
	xor ebx, ebx
jmp_1001a8c5:
	mov al, byte ptr [g_unk0x100a2f04+ebx]
	mov byte ptr [edi+ebx], al
	inc ebx
	loop jmp_1001a8c5
	pop ebp
	ret
FUN_1001a87b endp

; Upsamples the ecx samples at edi twice, interpolating linearly.
FUN_1001a8d3 proc
	dec ecx
	xor esi, esi
	xor edx, edx
jmp_1001a8d8:
	mov ebx, esi
	shr ebx, 1
	mov al, byte ptr [edi+ebx]
	mov byte ptr [g_unk0x100a2f04+esi], al
	mov dl, byte ptr [edi+ebx+1]
	xor ah, ah
	add eax, edx
	shr eax, 1
	mov byte ptr [g_unk0x100a2f04+1+esi], al
	add esi, 2
	cmp esi, ecx
	jb jmp_1001a8d8
	inc ecx
	xor ebx, ebx
jmp_1001a8ff:
	mov al, byte ptr [g_unk0x100a2f04+ebx]
	mov byte ptr [edi+ebx], al
	inc ebx
	loop jmp_1001a8ff
	ret
FUN_1001a8d3 endp

	end
