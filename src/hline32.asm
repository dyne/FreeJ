;; code by jaromil

SEGMENT CODE USE32 ALIGN=16

global asm_hline32

	extern asmdst
	extern asmnum1

asm_hline32:
	;; EAX dest screen (offset on desired coords)
	;; ECX width

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
	mov eax,[asmdst]	; screen dest
	mov ecx,[asmnum1]	; width
	mov edx,0xffff		; 16bit white dot

.MainLoop
	mov [eax],edx		; blit white pixel
	mov [eax+2],edx
	add eax,4		; goes forward 4 bytes (32bit)
	loop .MainLoop		; loop (dec ecx until 0)

.TheEnd
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