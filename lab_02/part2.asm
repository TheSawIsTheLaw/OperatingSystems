.386p

seg_descr struc    
    lim 	dw 0	
    base_l 	dw 0	
    base_m 	db 0	
    attr_1	db 0	
    attr_2	db 0	
    base_h 	db 0	
seg_descr ends


int_descr struc 
    offs_l 	dw 0  
    sel		dw 0  
    cntr    db 0  
    attr	db 0  
    offs_h 	dw 0  
int_descr ends


stack_seg segment  para stack 'STACK'
    stack_start	db	100h dup(?)
    stack_size = $-stack_start
stack_seg 	ENDS


data_seg segment para 'DATA'
    
    gdt_null  seg_descr <>
    
    gdt_CS_16bit seg_descr <rm_code_size-1, 0, 0, 10011000b, 00000000b, 0>
    
    gdt_DS_16bit seg_descr <0FFFFh, 0, 0, 10010010b, 10001111b, 0>  
    
    gdt_CS_32bit seg_descr <pm_code_size-1, 0, 0, 10011000b, 01000000b, 0>    
    
    gdt_DS_32bit seg_descr <data_size-1, 0, 0, 10010010b, 01000000b, 0>  
    
    gdt_SS_32bit seg_descr <stack_size-1, 0, 0, 10010110b, 01000000b, 0>
    
    gdt_VB_32bit seg_descr <3999, 8000h, 0Bh, 10010010b, 01000000b, 0>

    gdt_size = $-gdt_null 
    gdtr	df 0	      
    	
    sel_CS_16bit    equ    8   
    sel_DS_16bit    equ   16   
    sel_CS_32bit    equ   24
    sel_DS_32bit    equ   32
    sel_SS_32bit    equ   40
    sel_videobuffer equ   48
    
    IDT	label byte    
	
	trap_f int_descr 12 dup (<0, sel_CS_32bit, 0, 10001111b, 0>) 
	trap_13 int_descr <0, sel_CS_32bit, 0, 10001111b, 0>  
	trap_s int_descr 19 dup (<0, sel_CS_32bit, 0, 10001111b, 0>) 
    
    int08 int_descr <0, sel_CS_32bit, 0, 10001110b, 0> 
    
    int09 int_descr	<0, sel_CS_32bit, 0, 10001110b, 0> 

    idt_size = $-IDT 

    idtr df 0                       
	
    idtr_backup dw	3FFh, 0, 0      

    mask_master	db 0		
    mask_slave	db 0		
	
	ascii	db 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 0, 0
			db 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 0, 0, 65, 83
			db 68, 70, 71, 72, 74, 75, 76, 59, 39, 96, 0, 92, 90, 88, 67
			db 86, 66, 78, 77, 44, 46, 47

    flag_enter_pr	    db 0				 
    cnt_time	        dd 0		    
    syml_pos     dd 2 * (80 * 10)   
    
    msg_in_rm   db 27, '[32;40mIn real mode. ', 27, '[0m$'
    msg_move_pm db 27, '[32;40mPress any key...', 27, '[0m$'
    msg_out_pm  db 27, '[32;40mBack in real mode. ', 27, '[0m$'


    data_size = $-gdt_null 
data_seg ends

PM_code_seg segment para public 'CODE' use32

    assume cs:PM_code_seg, ds:data_seg, ss:stack_seg

    pm_start:
            mov	ax, sel_DS_32bit 
            mov	ds, ax
            mov	ax, sel_videobuffer
            mov	es, ax
            mov	ax, sel_SS_32bit
            mov	ss, ax
            mov	eax, stack_size
            mov	esp, eax

        sti 
        
        mov dI, 0
    	mov ah, 2h
    	mov al, 'M'
    	stosw
    	mov al, 'e'
    	stosw
    	mov al, 'm'
    	stosw
    	mov al, 'o'
    	stosw
    	mov al, 'r'
    	stosw
    	mov al, 'y'
    	stosw
    	mov al, ':'
    	stosw
        
        call count_memory
        
    proccess:
        test flag_enter_pr, 1
        jz	proccess
      
        cli
		db	0EAh 
    	dd	offset return_rm
    	dw	sel_CS_16bit
		
		except_1 proc
			iret
		except_1 endp
	
		except_13 proc
			pop eax
			iret
		except_13 endp   
		
        new_int08 proc uses eax 
                
                mov  eax, cnt_time
				push eax
                
					mov edi, 80 * 2
					xor eax, eax
					test cnt_time, 05
					jz X
					test cnt_time, 09
					jnz skip
					
					mov al, ' '
					jmp pr
				X:
					mov al, 'X'
				pr:
					mov ah, 7
					stosw		
						
				skip:	
					pop eax
                
					inc eax

                
                mov cnt_time, eax
                
                mov	al, 20h 
                out	20h, al
                
                iretd
        new_int08 endp

        new_int09 proc uses eax ebx edx 
            in	al, 60h      

            cmp	al, 1Ch 	        
            jne	print_value         
            or flag_enter_pr, 1     
            jmp allow_handle_keyboard
            
            print_value:
                cmp al, 80h  
                ja allow_handle_keyboard 	 
                                
                xor ah, ah	 
                
                xor ebx, ebx
                mov bx, ax

                mov dl, ascii[ebx]   
                mov ebx, syml_pos   
                mov es:[ebx], dl

                add ebx, 2          
                mov syml_pos, ebx

            allow_handle_keyboard: 
                in	al, 61h 
                or	al, 80h 
                out	61h, al 
                and al, 7Fh 
                out	61h, al

                mov	al, 20h 
                out	20h, al

                iretd
        new_int09 endp

        count_memory proc uses ds eax ebx 
            mov ax, sel_DS_16bit
            mov ds, ax
            
            mov ebx, 100001h 
            mov dl, 10101110b  
            
            mov	ecx, 0FFEFFFFEh

            iterate_through_memory:
                mov dh, ds:[ebx] 

                mov ds:[ebx], dl        
                cmp ds:[ebx], dl        
                                                
                jnz print_memory_counter        
            
                mov	ds:[ebx], dh 
                inc ebx          
            loop iterate_through_memory

            print_memory_counter:
                mov eax, ebx 
                xor edx, edx

                mov ebx, 100000h 
                div ebx

                mov ebx, 2 * 10
                call print_eax

                mov ebx, 2 * 20
                mov al, 'M'
                mov es:[ebx], al

                mov ebx, 2 * 20 + 2
                mov al, 'b'
                mov es:[ebx], al
            ret
        count_memory endp
        
        print_eax proc uses ecx ebx edx     
                add ebx, 10h 
                mov ecx, 8   
            
            print_symbol: 
                mov dl, al
                and dl, 0Fh      
                
                cmp dl, 10
                jl add_zero_sym
                add dl, 'A' - '0' - 10 

            add_zero_sym:
                add dl, '0' 
                mov es:[ebx], dl 
                ror eax, 4       
                sub ebx, 2       
                loop print_symbol
            ret
        print_eax endp

    pm_code_size = $-pm_start 	
PM_code_seg ends

RM_code_seg segment para public 'CODE' use16
    assume cs:RM_code_seg, ds:data_seg, ss: stack_seg

    start:
        mov ax, data_seg
        mov ds, ax

        mov ax, PM_code_seg
        mov es, ax

		mov ah, 09h
    	lea dx, msg_in_rm
    	int 21h
		xor dx, dx
    	mov ah, 2
    	mov dl, 13
    	int 21h
    	mov dl, 10
    	int 21h
		
		mov ah, 09h
    	lea dx, msg_move_pm
    	int 21h
		xor dx, dx
    	mov ah, 2
    	mov dl, 13
    	int 21h
    	mov dl, 10
    	int 21h

        push eax
    	mov ah, 10h
    	int 16h
    	pop eax
        mov	ax, 3
    	int	10h 

        xor	eax, eax
		
        mov	ax, RM_code_seg
		shl eax, 4         
    	mov word ptr gdt_CS_16bit.base_l, ax
    	shr eax, 16                       
    	mov byte ptr gdt_CS_16bit.base_m, al
    	mov byte ptr gdt_CS_16bit.base_h, ah

        mov ax, PM_code_seg
		shl eax, 4         
    	mov word ptr gdt_CS_32bit.base_l, ax
    	shr eax, 16                       
    	mov byte ptr gdt_CS_32bit.base_m, al
    	mov byte ptr gdt_CS_32bit.base_h, ah

        mov ax, data_seg
		shl eax, 4         
    	mov word ptr gdt_DS_32bit.base_l, ax
    	shr eax, 16                       
    	mov byte ptr gdt_DS_32bit.base_m, al
    	mov byte ptr gdt_DS_32bit.base_h, ah

        mov ax, stack_seg
		shl eax, 4         
    	mov word ptr gdt_SS_32bit.base_l, ax
    	shr eax, 16                       
    	mov byte ptr gdt_SS_32bit.base_m, al
    	mov byte ptr gdt_SS_32bit.base_h, ah

        mov ax, data_seg  
        shl eax, 4   
        add	eax, offset gdt_null 
        mov	dword ptr gdtr + 2, eax
    	mov word ptr  gdtr, gdt_size-1
    	lgdt fword ptr gdtr
		
		lea eax, es:except_1
		mov	trap_f.offs_l, ax
    	shr	eax, 16
		mov	trap_f.offs_h, ax
			
		lea eax, es:except_1
		mov	trap_s.offs_l, ax
    	shr	eax, 16
		mov	trap_s.offs_h, ax
			
		lea eax, es:except_13
		mov	trap_13.offs_l, ax
    	shr	eax, 16
		mov	trap_13.offs_h, ax
			
        lea eax, es:new_int08    
		mov	int08.offs_l, ax
    	shr	eax, 16
		mov	int08.offs_h, ax

        lea eax, es:new_int09
		mov	int09.offs_l, ax
    	shr	eax, 16
		mov	int09.offs_h, ax
            
        mov ax, data_seg
        shl eax, 4
        add	eax, offset IDT
        mov	 dword ptr idtr + 2, eax
    	mov  word ptr  idtr, idt_size-1
			
        in	al, 21h						
        mov	mask_master, al			    
        in	al, 0A1h					
        mov	mask_slave, al
			
		mov	al, 11h
		out	20h, al
		mov	al, 32			
		out	21h, al						
	
		mov	al, 4						
		out	21h, al                     

		mov	al, 1
		out	21h, al 
            
        mov	al, 0FCh
        out	21h, al
            
        mov	al, 0FFh
        out	0A1h, al
			
        lidt fword ptr idtr                                    
			
        in	al, 92h						
        or	al, 2						
        out	92h, al						

        cli         
        in	al, 70h 
        or	al, 80h
        out	70h, al
			
        mov	eax, cr0
        or eax, 1     
        mov	cr0, eax

        db	66h
		db	0EAh 
    	dd	offset pm_start
    	dw	sel_CS_32bit

    return_rm:
            mov	eax, cr0
            and	al, 0FEh 				
            mov	cr0, eax
            
            db	0EAh	
            dw	$+4	    
            dw	RM_code_seg

            mov	eax, data_seg	
            mov	ds, ax          
            mov eax, PM_code_seg
            mov	es, ax
            mov	ax, stack_seg   
            mov	ss, ax
            mov	ax, stack_size
            mov	sp, ax
			
			mov ax, data_seg
        	shl eax, 4
        	add	eax, offset IDT
        	mov	 dword ptr idtr + 2, eax
    		mov  word ptr  idtr, idt_size-1
			
        	in	al, 21h						
        	mov	mask_master, al			    
        	in	al, 0A1h					
        	mov	mask_slave, al
			
			mov	al, 11h
			out	20h, al
			mov	al, 8			
			out	21h, al						
	
			mov	al, 4						
			out	21h, al                     

			mov	al, 1
			out	21h, al

            mov	al, mask_master 
            out	21h, al
            mov	al, mask_slave
            out	0A1h, al
            
            lidt	fword ptr idtr_backup

            in	al, 70h 
            and	al, 7FH
            out	70h, al
            sti     

        mov	ax, 3
    	int	10h
		mov ah, 09h
    	lea dx, msg_out_pm
    	int 21h
		xor dx, dx
    	mov ah, 2
    	mov dl, 13
    	int 21h
    	mov dl, 10
    	int 21h
        
        mov	ax, 4C00h
        int	21h

    rm_code_size = $-start 	
RM_code_seg	ends
end start