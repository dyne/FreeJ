;; code by jaromil
;; if this is used on sizes which are not multiply of 8
;; bad things happen, i tell you my friend

SEGMENT CODE USE32 ALIGN=16

global mmx_memcpy

	extern asmsrc1
	extern asmdst
	extern asmnum1

mmx_memcpy:
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

