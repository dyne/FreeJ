;; code by jaromil
;; if this is used on sizes which are not multiply of 8bytes
;; bad things happen, i tell you my friend

SEGMENT CODE USE32 ALIGN=16

global mmx_blit_add

	extern asmsrc1
	extern asmdst
	extern asmnum1
	extern asmnum2
	extern asmnum3

mmx_blit_add:
	;; EAX source
	;; EDX dest
	;; ECX height
	;; [asmnum2] pitch
	;; [asmnum3] screen->pitch

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
	mov eax,[asmsrc1]	; SRC
	mov edx,[asmdst]	; DST
 	mov ecx,[asmnum1]
	push edx		; puntatore a inizio linea dst

.OuterLoop
	push ecx		; altezza
	mov ecx,[asmnum2]	; src pitch
	shr ecx,3		; pitch=pitch/8 (il loop decrementa di 1)
.InnerLoop
	movq mm1,[eax]		; mette il src in mm1 (64bit) (MMX!)
	movq mm2,[edx]
	paddusb mm1,mm2
	movq [edx],mm1		; mette mm1 in dst (64bit)    (MMX!)
	add eax,8		; incrementa il puntatore *src
	add edx,8		; incrementa il puntatore *dst
	loop .InnerLoop		; se ha blittato tutta la linea va a capo

.PitchReached			; ANDIAMO A CAPO
	pop ecx			; prende il contatore delle linee
	pop edx			; prende l'inizio della linea di *dst
	add edx,[asmnum3]	; va a capo col *dst
	push edx		; rimette tutto apposto nello stack
	loop .OuterLoop		; riluppa

.TheEnd
	pop eax
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