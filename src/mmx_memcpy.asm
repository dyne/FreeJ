;; mmx_memcpy_jaro code by jaromil
;; mmx_memcpy is now a better memcopy by Alessandro Gatti

SEGMENT CODE USE32 ALIGN=16

global mmx_memcpy
global mmx_memcpy_jaro

extern asmsrc1
extern asmdst
extern asmnum1

mmx_memcpy:
	push ebp
	mov ebp,esp
	pushad

	mov edi,[asmdst]
	mov esi,[asmsrc1]
	mov ecx,[asmnum1]
	
	mov eax,ecx
	shr ecx,6
	mov ebx,ecx
	shl ebx,6
	sub eax,ebx

	.Loop

	movq mm0,[esi]
	movq mm1,[esi+8]
	movq mm2,[esi+16]
	movq mm3,[esi+24]
	movq mm4,[esi+32]
	movq mm5,[esi+40]
	movq mm6,[esi+48]
	movq mm7,[esi+56]
	movq [edi],mm0
	movq [edi+8],mm1
	movq [edi+16],mm2
	movq [edi+24],mm3
        movq [edi+32],mm4
        movq [edi+40],mm5
        movq [edi+48],mm6
        movq [edi+56],mm7
        add esi,64
        add edi,64
        loop .Loop

	mov ecx,eax
	rep movsb

	emms

	popad
    
	pop ebp
	ret
	
mmx_memcpy_jaro:
	;; EAX source
	;; EDX dest
	;; ECX num

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
	mov eax,[asmsrc1]
	mov edx,[asmdst]
	mov ecx,[asmnum1]
	shr ecx,3

.Loop
	movq mm1,[eax]		; mette il src in mm1 (64bit)
	movq [edx],mm1		; mette mm1 in dst (64bit)
	add eax,8		; incrementa i puntatori
	add edx,8
	loop .Loop		; ecx--; luppa se ecx>0 

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
	
SEGMENT DATA

