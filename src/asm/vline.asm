;; code by jaromil

SEGMENT CODE USE32 ALIGN=16

global asm_vline16

	extern asmdst
	extern asmnum1
	extern asmnum2

asm_vline16:
	;; EAX dest screen (offset on desired coords)
	;; ECX height
	;; EBX pitch (screen width in bytes)

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
	mov ecx,[asmnum1]	; height
	mov ebx,[asmnum2]	; pitch
	mov edx,0xffff		; 16bit white dot

.MainLoop
	mov [eax],edx		; blit white pixel
	add eax,ebx		; goes down one line
	loop .MainLoop		; loop (dec ecx until 0)

.TheEnd
	;; emms ?
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
	