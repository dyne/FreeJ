;; code by jaromil
	
SEGMENT CODE USE32 ALIGN=16

global mmx_absdiff16

extern asmsrc1
extern asmsrc2
extern asmdst
extern asmnum1
			
mmx_absdiff16:

	movq mm1,[Threshold]	; threshold
	mov eax,[asmsrc1]	; surf 1
	mov edx,[asmsrc2]	; surf 2
	mov ebx,[asmdst]	; dest
	mov ecx,[asmnum1]	; len
	pxor mm5,mm5		; azzera mm5
	shr ecx,3		; len = len / 8 (loop decrementa di 1)
	
.DiffLoop

	movq mm2,[eax]		; prende il src1
	movq mm3,[edx]		; prende il src2
	movq mm4,mm2		; salva il src1 in src1_temp
	psubusw mm4,mm3		; src1_temp = src1_temp - src2
	psubusw mm3,mm2		; src2 = src2 - src1
	por mm3,mm4		; or sulle differenze
	movq [edx],mm2		; salva src1 in src2 per il prossimo passaggio
	add eax,8		; avanza SRC1
	add edx,8		; avanza SRC2

	pcmpgtw mm3,mm1		; applica threshold
; 	pcmpeqw mm3,mm5		; check zeroes
	
	movq [ebx],mm3		; salva in dest
	add ebx,8		; incrementa dest

	loop .DiffLoop

.EndLoop
	emms
	ret

SEGMENT CONST USE32 ALIGN=16
	;; global Threshold
 Threshold db 30,30,30,30,30,30,30,30
