.386P						 
 
descr	struc					 
	limit	dw	0			 
	base_l	dw	0			 
	base_m	db	0			 
	attr_1	db	0			 
	attr_2	db	0			 
	base_h	db	0			 
descr	ends					 

data	segment	use16				 
	gdt_null descr<0,0,0,0,0,0>			 
	gdt_data descr<data_size-1,,,92h,,>	 
	gdt_code descr<code_size-1,,,98h,,>	 
	gdt_stack descr<255,,,92h,,>		 
	gdt_screen descr<4095,8000h,0Bh,92h,,>	 
	gdt_data32 descr<0FFFFh,,,92h,0CFh>
gdt_size=$-gdt_null				 
 
	pdescr	df	0	

	messageInReal	db	'In real mode'
	messageInReal_len=$-messageInReal
	msgInProtected	db	'In protected mode'
	msgInProtected_len=$-msgInProtected
data_size=$-gdt_null				 
data	ends					 

text	segment	'code' use16			 
	assume	cs:text,ds:data			 
main	proc					 
	mov	ax,data
	mov	ds,ax
	mov	ax,0B800h
	mov	es,ax

	mov	di,22*160
	mov	bx,offset messageInReal
	mov	cx,messageInReal_len
	mov ah, 2
screen1:	mov	al,byte ptr [bx]
	inc	bx
	stosw
	loop	screen1

	xor	eax,eax				 
	mov	ax,data
 
	shl	eax,4				 
	mov	ebp,eax				 
	mov	bx,offset gdt_data 
	mov	[bx].base_l,ax			 
	rol	eax,16				 
	mov	[bx].base_m,al			 
 
	xor	eax,eax				 
	mov	ax,cs				 
	shl	eax,4				 
	mov	bx,offset gdt_code 
	mov	[bx].base_l,ax			 
	rol	eax,16				 
	mov	[bx].base_m,al			 
 
	xor	eax,eax				 
	mov	ax,ss				 
	shl	eax,4				 
	mov	bx,offset gdt_stack		 
	mov	[bx].base_l,ax			 
	rol	eax,16				 
	mov	[bx].base_m,al			 
 
	mov	dword ptr pdescr+2,ebp		 
	mov	word ptr pdescr,gdt_size-1	 
	lgdt	pdescr				 
 
	cli			 
 
	mov	eax,cr0				 
	or	eax,1				 
	mov	cr0,eax	
 
	db	0EAh				 
	dw	offset continue			 
	dw	16				 
continue:					 
 
	mov	ax,8				 
	mov	ds,ax				 
 
	mov	ax,24				 
	mov	ss,ax				 
 
	mov	ax,32				 
	mov	es,ax				 

	mov	ax,40
	mov	gs,ax

	mov	di,23*160
	mov	bx,offset msgInProtected
	mov	cx,msgInProtected_len
	mov ah, 3
screen2:	mov	al,byte ptr [bx]
	inc	bx
	stosw
	loop	screen2	

	;mov gdt_data.limit, 0FFFFh
    ;mov gdt_code.limit, 0FFFFh
    ;mov gdt_stack.limit, 0FFFFh
    ;mov gdt_screen.limit, 0FFFFh

    ;push ds
    ;pop  ds
    ;push es
    ;pop  es
    ;push ss
    ;pop  ss			 
 
	db	0EAh				 
	dw	offset next			 
	dw	16				 
 
next:	mov	eax,cr0				 
	and	eax,0FFFFFFFEh			 
	mov	cr0,eax				 
	db	0EAh				 
	dw	offset return			 
	dw	text				 
 
 
return:	mov	ax,data				 
	mov	ds,ax				 
	mov	ax,stk				 
	mov	ss,ax				 
 
	sti					 
	mov	al,0				 
	out	70h,al				 

	mov	di,24*160
	mov	bx,offset messageInReal
	mov	cx,messageInReal_len
	mov ah, 2
screen3:	mov	al,byte ptr [bx]
	inc	bx
	stosw
	loop	screen3

	mov	ax,4C00h			 
	int	21h				 
main	endp					 
code_size=$-main				 
text	ends					 

stk	segment	stack 'stack'			 
	db	256 dup('^')			 
stk	ends					 
	end	main				 