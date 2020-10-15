.386p

segdesc	struc
	limit	dw 0
	base_l	dw 0
	base_m	db 0
	attr_1	db 0
	arrt_2	db 0
	base_h	db 0
segdesc	ends

intdesc	struc
	offs_l	dw 0
	sel	dw 0
	rsrv	db 0
	attr	db 0
	offs_h	dw 0
intdesc	ends

pseg	segment	'code' use32
	assume	cs:pseg

gdt	label	byte
	gdt_null   segdesc<>
	gdt_4gb    segdesc<0ffffh,,,92h,0cfh>
	gdt_code16 segdesc<rseg_size-1,,,98h>
	gdt_code32 segdesc<pseg_size-1,,,98h,0cfh>
	gdt_data   segdesc<pseg_size-1,,,92h,0cfh>
	gdt_stack  segdesc<sseg_size-1,,,92h,0cfh>
gdt_size = $-gdt

	gdtr	dw gdt_size-1	 
		dd ?		 

	sel_4gb    equ 8
	sel_code16 equ 16
	sel_code32 equ 24
	sel_data   equ 32
	sel_stack  equ 40
					 
idt	label	byte			 
	trap1	intdesc	13 dup (<,sel_code32,,8fh>)	 
	trap13	intdesc	<0, sel_code32,,8fh>		 
	trap2	intdesc	18 dup (<,sel_code32,,8fh>)	 
	int08	intdesc	<,sel_code32,,8eh>		 
	int09	intdesc	<,sel_code32,,8eh>		 
idt_size = $-idt

	idtr	dw idt_size-1	 
		dd ?		 

	idtr_r	dw 3ffh,0,0	 

	msgp	db 'Protected mode'
msgp_size = $-msgp
	msgr	db 'Real mode', '$'
msgr_size = $-msgr-1
	msgt	db 'Timer:'
msgt_size = $-msgt
	msgm	db 'AvaliableMemory:'
msgm_size = $-msgm

	scan2ascii db 0,1bh,'1','2','3','4','5','6','7','8','9','0','-','=',8
		   db ' ','q','w','e','r','t','y','u','i','o','p','[',']','$'
		   db ' ','a','s','d','f','g','h','j','k','l',
		   db '\','z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0,0
		   db 0,0,0,0,0,0,0,0,0,0,0,0

	screen	dd 4*160	 
	timer	dd 0		 

	master	db 0		 
	slave	db 0		 

p_entry:
	 
	mov	ax,sel_4gb
	mov	ds,ax
	mov	es,ax
	mov	ax,sel_stack
	mov	ebx,sseg_size
	mov	ss,ax
	mov	esp,ebx

	in	al,70h
	and	al,7fh
	out	70h,al

	sti

push	ebp
	xor	eax,eax
	mov	ebp,1*160
	add	ebp,0b8000h
	mov	ecx,msgp_size
	xor	esi,esi

screen1:
	mov	al,byte ptr msgp[esi]
	mov	es:[ebp],al
	add	ebp,2
	inc	esi
	loop	screen1

	pop	ebp

	push	ebp
	xor	eax,eax
	mov	ebp, 3*160
	add	ebp, 0b8000h
	mov	ecx,msgm_size
	xor	esi,esi

screen2:
	mov	al,byte ptr msgm[esi]
	mov	es:[ebp],al
	add	ebp,2
	inc	esi
	loop	screen2

	pop	ebp


	push	ebp
	xor	eax,eax
	mov	ebp,40*2
	add	ebp,0b8000h
	mov	ecx,msgt_size
	xor	esi,esi

screen3:
	mov	al,byte ptr msgt[esi]
	mov	es:[ebp],al
	add	ebp,2
	inc	esi
	loop	screen3

	pop	ebp

	call	compute_memory
	 
	jmp	short $

dummy_exc proc
	iretd
dummy_exc endp

exc13	proc
	pop	eax
	iretd
exc13	endp

int08_handler:
	push	eax
	push	ecx
	push	edx

	push	ebp
	mov	eax,timer
	mov	ebp,+55*2
	mov	ecx,8
	add	ebp,0b8000h
cycle:
	mov	dl,al
	and	dl,0fh
	cmp	dl,10
	jl	number
	add	dl,'a'-10
	jmp	print
number:
	add	dl,'0'
print:
	mov	es:[ebp],dl
	ror	eax,4
	sub	ebp,2
	loop	cycle

	pop	ebp

	inc	eax
	mov	timer,eax

 
	mov	al,20h
	out	20h,al

	pop	edx
	pop	ecx
	pop	eax
	iretd

int09_handler:
	push	eax
	push	ebx
	push	es
	push	ds

	in	al,60h			 
	cmp	al,01h			 
	je	esc_pressed		 
	cmp	al,39h			 
	ja	skip_translate		 
	mov	bx,sel_data		 
	mov	ds,bx			 
	mov	ebx,offset scan2ascii	 
	xlatb				 
	mov	bx,sel_4gb
	mov	es,bx			 
	mov	ebx,screen		 
	cmp	al,8			 
	je	bs_pressed
	mov	es:[ebx+0b8000h],al	 
	add	dword ptr screen,2	 
	jmp	short skip_translate
bs_pressed:				 
	mov	al,' '			 
	sub	ebx,2			 
	mov	es:[ebx+0b8000h],al	 
	mov	screen,ebx		 
skip_translate:
 
	in	al,61h
	or	al,80h
	out	61h,al
 
	mov	al,20h
	out	20h,al

	pop	ds
	pop	es
	pop	ebx
	pop	eax
	iretd

esc_pressed:
 
	in	al,61h
	or	al,80h
	out	61h,al
	mov	al,20h
	out	20h,al
	pop	ds
	pop	es
	pop	ebx
	pop	eax

	cli

	in	al,70h
	or	al,80h
	out	70h,al
	 
	db	0eah
	dd	r_return
	dw	sel_code16

compute_memory proc
	push	ds
	mov	ax,sel_4gb	 
	mov	ds,ax
	mov	ebx,100001h	 
				 
	mov	dl,11101011b	 
	mov	ecx,0FFEFFFFFh	 
				 
check:
	mov	dh,ds:[ebx]	 
	mov	ds:[ebx],dl	 
	cmp	ds:[ebx],dl	 
	jnz	end_of_memory	 
	mov	ds:[ebx],dh	 
	inc	ebx		 
	loop	check

end_of_memory:
	pop	ds
	xor	edx,edx
	mov	eax,ebx
	mov	ebx,100000h	 
	div	ebx

	push	ecx
	push	edx

	push	ebp
	mov	ebp,3*160+25*2
	mov	ecx,8
	add	ebp,0b8000h
cycle1:
	mov	dl,al
	and	dl,0fh
	cmp	dl,10
	jl	number1
	add	dl,'a'-10
	jmp	print1
number1:
	add	dl,'0'
print1:
	mov	es:[ebp],dl
	ror	eax,4
	sub	ebp,2
	loop	cycle1

	pop	ebp

	pop	edx
	pop	ecx

	ret
compute_memory endp

	pseg_size = $-gdt
pseg	ends

rseg	segment para public 'CODE' use16
	assume cs:rseg, ds:pseg, ss:sseg
r_start:
	mov	ax,3
	int	10h

	push	pseg
	pop	ds

	mov ah, 09h
	mov edx, offset msgr
	int 21h
 
	xor	eax,eax
	mov	ax,rseg
	shl	eax,4
	mov	word ptr gdt_code16+2,ax	 
	shr	eax,16
	mov	byte ptr gdt_code16+4,al	 
	mov	ax,pseg
	shl	eax,4
	push	eax				 
	push	eax				 
	mov	word ptr gdt_code32+2,ax	 
	mov	word ptr gdt_stack+2,ax		 
	mov	word ptr gdt_data+2,ax		 
	shr	eax,16
	mov	byte ptr gdt_code32+4,al	 
	mov	byte ptr gdt_stack+4,al		 
	mov	byte ptr gdt_data+4,al
 
	pop	eax
	add	eax,offset gdt			 
	mov	dword ptr gdtr+2,eax		 
	mov	word ptr gdtr,gdt_size-1	 					 
 
	lgdt	fword ptr gdtr
 
	pop	eax
	add	eax,offset idt
	mov	dword ptr idtr+2,eax
	mov	word ptr idtr,idt_size-1
 
	mov	eax,offset dummy_exc
	mov	trap1.offs_l,ax
	shr	eax,16
	mov	trap1.offs_h,ax

	mov	eax,offset exc13
	mov	trap13.offs_l,ax
	shr	eax,16
	mov	trap13.offs_h,ax
	
	mov	eax,offset dummy_exc
	mov	trap2.offs_l,ax
	shr	eax,16
	mov	trap2.offs_h,ax

	mov	eax,offset int08_handler
	mov	int08.offs_l,ax
	shr	eax,16
	mov	int08.offs_h,ax

	mov	eax,offset int09_handler
	mov	int09.offs_l,ax
	shr	eax,16
	mov	int09.offs_h,ax
 
	in	al,21h		 
	mov	master,al	
	in	al,0a1h		 
	mov	slave,al
 
	mov	al,11h
	out	20h,al
	mov	al,20h	 
	out	21h,al	 
	mov	al,4
	out	21h,al
	mov	al,1
	out	21h,al
 
	mov	al,0FCh	 
	out	dx,al
 
	mov	dx,0A1h
	mov	al,0FFh
	out	dx,al
 
	lidt	fword ptr idtr

	mov	al,0d1h
	out	64h,al
	mov	al,0dfh
	out	60h,al
	cli

	in	al,70h
	or	al,80h
	out	70h,al
 
	mov	eax,cr0
	or	al,1
	mov	cr0,eax
 
	db	66h
	db	0eah
	dd	offset p_entry
	dw	sel_code32

r_return:
 
	mov	al,0d1h
	out	64h,al
	mov	al,0ddh
	out	60h,al

	mov	eax,cr0
	and	al,0feh	 
	mov	cr0,eax

 
	db	0eah
	dw	$+4
	dw	rseg
 
	mov	ax,pseg	 
	mov	ds,ax
	mov	es,ax
	mov	ax,sseg
	mov	bx,sseg_size
	mov	ss,ax
	mov	sp,bx
 
	mov	al,11h
	out	20h,al
	mov	al,8
	out	21h,al
	mov	al,4
	out	21h,al
	mov	al,1
	out	21h,al
 
	mov	al,master
	out	21h,al
	mov	al,slave
	out	0a1h,al
 
	lidt	fword ptr idtr_r

	in	al,70h
	and	al,7fh
	out	70h,al

	sti

	mov	ah,2
	xor	bx,bx
	mov	dx,200h
	int	10h

	mov ah,9
	mov edx,offset msgr
	int 21h
	mov	ah,8
	int	21h
	mov	ax,3
	int	10h

	mov	ah,4ch
	int	21h
rseg_size = $-r_start
rseg	ends

sseg	segment para stack 'stack'
	stack_start	db 100h dup(?)
sseg_size = $-stack_start
sseg	ends

end r_start