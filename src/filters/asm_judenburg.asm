;; code by jaromil
;; coded some hours before stage in Judenburg, 2 august 2001
;; this messes up the colors, cutting out some under the threshold
	
SEGMENT CODE USE32 ALIGN=16

global asm_judenburg

	extern asmsrc
	extern asmdst
	extern asmnum1
	extern threshold

asm_judenburg:
	;; EAX src buffer
	;; EBX dest buffer
	;; ECX size of buffer
	;; EDX pitch
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
	mov eax,[asmsrc]
	mov ebx,[asmdst]
	mov ecx,[asmnum1]
	movq mm4,[threshold]
	shr ecx,3		; loop decs by 1

.MainLoop
	movq mm1,[eax]		; prende il src
	movq mm2,[eax]
	pcmpgtd mm1,mm4
	pand mm2,mm4
	movq [ebx],mm2		; salva in dest
	add eax,8		; va avanti sul src
	add ebx,8		; va avanti sul dest
	loop .MainLoop		; luppa finche' ecx>0

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