;
; EffecTV - Realtime Digital Video Effector
; Copyright (C) 2001 FUKUCHI Kentarou
;
; blurzoomcore.nas : blur and zooming module for RadioacTV
; Copyright (C) 2001 FUKUCHI Kentarou
;

BITS 32

GLOBAL blurzoomcore
EXTERN blurzoombuf
EXTERN blurzoomx
EXTERN blurzoomy
EXTERN buf_width
EXTERN buf_height
EXTERN buf_width_blocks
EXTERN buf_area

SECTION .text

blurzoomcore:
	push ebp
	push esi
	push edi
	push ebx

;blur

; eax = 0
; ebx = blurzoombuf + buf_width + 1
; ecx = buf_width - 2
; edx = ebx + buf_area
; esi = -buf_width
; edi = buf_width
; ebp = buf_height - 2
	xor eax, eax
	mov edi, dword [buf_width]
	mov esi, edi
	neg esi
	mov ebp, dword [buf_height]
	sub ebp, 2
	mov ebx, dword [blurzoombuf]
	add ebx, edi
	inc ebx
	mov edx, ebx
	add edx, dword [buf_area]
.byloop:
	mov ecx, edi
	sub ecx, 2
align 4
.bxloop:
	mov al, [ebx+esi]
	add al, [ebx-1]
	add al, [ebx+1]
	add al, [ebx+edi]
	inc ebx
	shr al, 2
	sub al, 1
	adc al, 0 ; increment al if al == -1
	mov [edx], al
	inc edx
	dec ecx
	jnz .bxloop

	add ebx, 2
	add edx, 2
	dec ebp
	jnz .byloop

; zooming

; ebx = buf_height
; ecx = buf_width_blocks
; edx = blurzoomx
; edi = blurzoombuf
; esi = blurzoombuf + buf_area
	mov edi, dword [blurzoombuf]
	mov esi, edi
	add esi, dword [buf_area]
	mov eax, dword [blurzoomy]
	mov dword [zoomyptr], eax
	mov ebx, dword [buf_height]
align 4
.yloop:
	mov eax, dword [zoomyptr]
	mov ecx, dword [buf_width_blocks]
	add esi, [eax]
	add eax, 4
	mov dword [zoomyptr], eax
	mov edx, dword [blurzoomx]
align 4
.xloop:
	mov ebp, [edx]
%macro MOV4PIX 0
	sal ebp ,1
	adc esi, 0
	mov al, [esi]
	sal ebp, 1
	adc esi, 0
	mov ah, [esi]
	rol eax, 16
	sal ebp, 1
	adc esi, 0
	mov al, [esi]
	sal ebp, 1
	adc esi, 0
	mov ah, [esi]
	rol eax, 16
	mov [edi], eax
	add edi, 4
%endmacro

%rep 8
	MOV4PIX
%endrep

	add edx, 4
	dec ecx
	jnz near .xloop

	dec ebx
	jnz near .yloop

.exit:
	pop ebx
	pop edi
	pop esi
	pop ebp
	ret

SECTION .bss

zoomyptr	resd 1

