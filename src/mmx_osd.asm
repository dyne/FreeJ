;; code by jaromil

SEGMENT CODE USE32 ALIGN=16

global mmx_osd_clean

	extern asmdst
	extern asmsrc1
	extern asmnum1
	extern asmnum2
	extern asmnum3
		
mmx_osd_clean:
	;; EAX screen

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
	mov eax,[asmdst]	; SCR
	mov edx,[asmsrc1]	; fill 2pixel pattern
	movq mm1,[edx]		; preparing our hero

	;; upper stripe (18 vertical pixels)
	mov ecx,[asmnum1]
.UpLoop
	movq [eax],mm1
	add eax,8
	dec ecx
	jnz .UpLoop

	;; left and right stripes (20 pixels on both sides)
	mov ecx,[asmnum3]

.HorLoop
	push ecx
	
	mov ecx,20/2
.HorSXLoop
	movq [eax],mm1
	add eax,8
	dec ecx
	jnz .HorSXLoop

	add eax,[asmnum2]

	mov ecx,10
.HorDXLoop
	movq [eax],mm1
	add eax,8
	dec ecx
	jnz .HorDXLoop

	pop ecx
	dec ecx
	jnz .HorLoop

	mov ecx,[asmnum1]
.BottomLoop
	movq [eax],mm1
	add eax,8
	dec ecx
	jnz .BottomLoop
	
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