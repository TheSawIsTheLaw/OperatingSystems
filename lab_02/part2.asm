.386p
descr struc
    limit   dw 0
    base_l  dw 0
    base_m  db 0
    attr_1  db 0
    attr_2  db 0
    base_h  db 0
descr ends


idescr struc
    offs_l  dw 0
    sel     dw 0
    cntr    db 0
    attr    db 0
    offs_h  dw 0
idescr ends


stack32 segment  para stack 'STACK'
    stack_start db  100h dup(?)
    stack_size = $-stack_start
stack32 ends


data32 segment para 'data'
    gdt_null  descr <>
    gdt_code16 descr <code16_size-1,0,0,98h>
    gdt_data4gb descr <0FFFFh,0,0,92h,0CFh>
    gdt_code32 descr <code32_size-1,0,0,98h,40h>
    gdt_data32 descr <data_size-1,0,0,92h,40h>
    gdt_stack32 descr <stack_size-1,0,0,92h,40h>
    gdt_video16 descr <3999,8000h,0Bh,92h>

    gdt_size = $-gdt_null
    pdescr    df 0

    code16s  equ 8
    data4gbs equ 16
    code32s  equ 24
    data32s  equ 32
    stack32s equ 40
    video16s equ 48

    idt label byte

    idescr_0_12 idescr 13 dup (<0,code32s,0,8Fh,0>)
    idescr_13 idescr <0,code32s,0,8Fh,0>
    idescr_14_31 idescr 18 dup (<0,code32s,0,8Fh,0>)

    int08 idescr <0,code32s,0,10001110b,0> 
    int09 idescr <0,code32s,0,10001110b,0>

    idt_size = $-idt 

    ipdescr df 0
    ipdescr16 dw 3FFh, 0, 0

    mask_master db 0        
    mask_slave  db 0        
    
    asciimap   db 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 0, 0
    db 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 0, 0, 65, 83
    db 68, 70, 71, 72, 74, 75, 76, 59, 39, 96, 0, 92, 90, 88, 67
    db 86, 66, 78, 77, 44, 46, 47

    flag_enter_pr db 0
    cnt_time      dd 0            
    syml_pos      dd 2 * (80 * 10)   

    
    rm_msg      db 27, '[22;40mIn real mode. ', 27, '[0m$'
    pm_msg_wait db 27, '[22;40mPress any key...', 27, '[0m$'
    pm_msg_out  db 27, '[22;40mBack to real mode. ', 27, '[0m$'

    data_size = $-gdt_null 
data32 ends


code32 segment para public 'code' use32
    assume cs:code32, ds:data32, ss:stack32

pm_start:
    mov ax, data32s
    mov ds, ax
    mov ax, video16s
    mov es, ax
    mov ax, stack32s
    mov ss, ax
    mov eax, stack_size
    mov esp, eax

    sti 
    
    
    mov di, 0
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
        jz  proccess
    
    cli 

    db  0EAh
    dd  offset return_rm
    dw  code16s


    except_1 proc
        iret
    except_1 endp


    except_13 proc uses eax
        pop eax
        iret
    except_13 endp


    new_int08 proc uses eax
        mov edi, 80 * 2
        test cnt_time, 05
        je sym
        test cnt_time, 09
        jne skip

        mov al, ' '
        jmp pr
    sym:
        mov al, '='
    pr:
        mov ah, 7
        stosw

    skip:
        mov  eax, cnt_time
        inc eax
        mov cnt_time, eax
        
        mov al, 20h
        out 20h, al

        iretd
    new_int08 endp

    new_int09 proc uses eax ebx edx
        in  al, 60h
        cmp al, 1Ch

        jne print_value         
        or flag_enter_pr, 1
        jmp allow_handle_keyboard

    print_value:
        cmp al, 80h  
        ja allow_handle_keyboard     

        xor ah, ah   

        xor ebx, ebx
        mov bx, ax

        mov dl, asciimap[ebx]   
        mov ebx, syml_pos   
        mov es:[ebx], dl

        add ebx, 2          
        mov syml_pos, ebx

    allow_handle_keyboard:
        in  al, 61h 
        or  al, 80h 
        out 61h, al 
        and al, 7Fh 
        out 61h, al

        mov al, 20h 
        out 20h, al

        iretd
    new_int09 endp


    count_memory proc uses ds eax ebx
        mov ax, data4gbs
        mov ds, ax

        mov ebx,  100001h
        mov dl,   0AEh

        mov ecx, 0FFEFFFFEh

        iterate_through_memory:
            mov dh, ds:[ebx] 

            mov ds:[ebx], dl        
            cmp ds:[ebx], dl        
        
            jnz print_memory_counter        

            mov ds:[ebx], dh 
            inc ebx
        loop iterate_through_memory

    print_memory_counter:
        mov eax, ebx 
        xor edx, edx

        mov ebx, 100000h
        div ebx

        mov ebx, 2 * 7
        call print_eax

        mov ebx, 2 * 17
        mov al, 'M'
        mov es:[ebx], al

        mov ebx, 2 * 17 + 2
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

    code32_size = $-pm_start
code32 ends


code16 segment para public 'CODE' use16
assume cs:code16, ds:data32, ss: stack32
start:
    mov ax, data32
    mov ds, ax

    mov ah, 09h
    lea dx, rm_msg
    int 21h

    xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h

    mov ah, 09h
    lea dx, pm_msg_wait
    int 21h
    xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h

    ; Ожидание нажатия клавиши
    mov ah, 10h
    int 16h
    
    ; Очистка
    mov ax, 3
    int 10h


    xor eax, eax

    mov ax, code16
    shl eax, 4                        
    mov word ptr gdt_code16.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code16.base_m, al  
    mov byte ptr gdt_code16.base_h, ah  

    mov ax, code32
    shl eax, 4                        
    mov word ptr gdt_code32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_code32.base_m, al  
    mov byte ptr gdt_code32.base_h, ah  

    mov ax, data32
    shl eax, 4                        
    mov word ptr gdt_data32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_data32.base_m, al  
    mov byte ptr gdt_data32.base_h, ah  

    mov ax, stack32
    shl eax, 4                        
    mov word ptr gdt_stack32.base_l, ax  
    shr eax, 16                       
    mov byte ptr gdt_stack32.base_m, al  
    mov byte ptr gdt_stack32.base_h, ah  

    mov ax, data32
    shl eax, 4
    add eax, offset gdt_null

    mov dword ptr pdescr+2, eax
    mov word ptr  pdescr, gdt_size-1  
    lgdt fword ptr pdescr             

    lea eax, es:except_1
    mov idescr_0_12.offs_l, ax 
    shr eax, 16             
    mov idescr_0_12.offs_h, ax 

    lea eax, es:except_13
    mov idescr_13.offs_l, ax 
    shr eax, 16             
    mov idescr_13.offs_h, ax 

    lea eax, es:except_1
    mov idescr_14_31.offs_l, ax 
    shr eax, 16             
    mov idescr_14_31.offs_h, ax 

    
    lea eax, es:new_int08
    mov int08.offs_l, ax
    shr eax, 16
    mov int08.offs_h, ax

    lea eax, es:new_int09
    mov int09.offs_l, ax 
    shr eax, 16             
    mov int09.offs_h, ax 

    mov ax, data32
    shl eax, 4
    add eax, offset idt

    mov  dword ptr ipdescr + 2, eax 
    mov  word ptr  ipdescr, idt_size-1 
    
    
    ; Сохранение масок
    in  al, 21h                     
    mov mask_master, al             
    in  al, 0A1h                    
    mov mask_slave, al
    
    ; Перепрограммирование ведущего контроллера
    mov al, 11h
    out 20h, al                     
    mov al, 32
    out 21h, al                     
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

    ; Масква ведущего контроллера
    mov al, 0FCh
    out 21h, al

    ; Маска ведомого контроллера (запрет прерываний)
    mov al, 0FFh
    out 0A1h, al
    
    lidt fword ptr ipdescr                                    
    
    ; А20 открывается
    in  al, 92h
    or  al, 2
    out 92h, al

    cli

    mov eax, cr0
    or eax, 1     
    mov cr0, eax

    db  66h 
    db  0EAh
    dd  offset pm_start
    dw  code32s


return_rm:
    mov eax, cr0
    and al, 0FEh                
    mov cr0, eax
    
    db  0EAh    
    dw  $+4     
    dw  code16

    mov ax, data32   
    mov ds, ax
    mov ax, code32
    mov es, ax
    mov ax, stack32   
    mov ss, ax
    mov ax, stack_size
    mov sp, ax
    
    mov al, 11h
    out 20h, al
    mov al, 8
    out 21h, al
    mov al, 4
    out 21h, al
    mov al, 1
    out 21h, al

    mov al, mask_master
    out 21h, al
    mov al, mask_slave
    out 0A1h, al

    lidt    fword ptr ipdescr16

    in  al, 70h 
    and al, 7Fh
    out 70h, al

    sti     
    
    ; очистка
    mov ax, 3
    int 10h

    mov ah, 09h
    lea dx, pm_msg_out
    int 21h
    xor dx, dx
    mov ah, 2
    mov dl, 13
    int 21h
    mov dl, 10
    int 21h

    mov ax, 4C00h
    int 21h

    code16_size = $-start  
code16 ends

end start
