;; code by jaromil
	
SEGMENT CODE USE32 ALIGN=16

global mmx_absdiff32

extern absdiff_asmsrc1
extern absdiff_asmsrc2
extern absdiff_asmdst
extern absdiff_asmnum1
			
mmx_absdiff32:
	;; save registers

	push ebp
	mov ebp,esp
	sub esp,0x40
	
	push ebx
	push ecx
	push edx
	push esi
	push edi
	push ebp
	
	;; place data
	movq mm1,[Threshold]	; threshold
	mov eax,[absdiff_asmsrc1]	; surf 1
	mov edx,[absdiff_asmsrc2]	; surf 2
	mov ebx,[absdiff_asmdst]	; dest
	mov ecx,[absdiff_asmnum1]	; len
	pxor mm5,mm5		; azzera mm5
	shr ecx,3		; len = len / 8 (loop decrementa di 1)
	
.DiffLoop

	movq mm2,[eax]		; prende il src1
	movq mm3,[edx]		; prende il src2
	movq mm4,mm2		; salva il src1 in src1_temp
	psubusb mm4,mm3		; src1_temp = src1_temp - src2
	psubusb mm3,mm2		; src2 = src2 - src1
	por mm3,mm4		; or sulle differenze
	movq [edx],mm2		; salva src1 in src2 per il prossimo passaggio
	add eax,8		; avanza SRC1
	add edx,8		; avanza SRC2

	pcmpgtb mm3,mm1		; applica threshold
; 	pcmpeqw mm3,mm5		; check zeroes
	
	movq [ebx],mm3		; salva in dest
	add ebx,8		; incrementa dest

	loop .DiffLoop

.EndLoop
	emms

	;; restore registers
	pop ebp
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx

	leave
	ret

SEGMENT CONST USE32 ALIGN=16
	;; global Threshold
 Threshold db 30,30,30,30,30,30,30,30
