;; code by jaromil
;; if this is used on sizes which are not multiply of 8bytes
;; bad things happen, i tell you my friend

SEGMENT CODE USE32 ALIGN=16

global asm_clearscr

	extern asmdst
	extern asmnum1

asm_clearscr:
	;; EAX screen
	;; ECX size

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
	mov eax,[asmdst]
	mov ecx,[asmnum1]
	shr ecx,3		; size=size/3 (loop decrementa di 1)
	pxor mm1,mm1

.MainLoop
	movq [eax],mm1
	add eax,8
	loop .MainLoop		; loop finche' ecx == 0

.TheEnd
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