;; code by jaromil
;; if this is used on sizes which are not multiply of 64bytes
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
	pushad
	
	;; place data
	mov edi,[asmdst]
	mov ecx,[asmnum1]

	mov eax,ecx
	shr ecx,6
	mov ebx,ecx
	shl ebx,6
	sub eax,ebx
	
	pxor mm0,mm0

.Loop
	movq [edi],mm0
	movq [edi+8],mm0
	movq [edi+16],mm0
	movq [edi+24],mm0
	movq [edi+32],mm0
	movq [edi+40],mm0
	movq [edi+48],mm0
	movq [edi+56],mm0
	add edi,64

	loop .Loop		; loop finche' ecx == 0
	mov ecx,eax
	rep movsb

	emms
	popad
	pop ebp
	ret
	
SEGMENT DATA