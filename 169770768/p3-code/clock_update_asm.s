.text                           # IMPORTANT: subsequent stuff is executable
.global  set_tod_from_ports
        
set_tod_from_ports:
        ## %rdi is a pointer to a tod_t struct
        ## %ecx holds TIME_OF_DAY_PORT
        movl    TIME_OF_DAY_PORT(%rip), %ecx    # copy global var to reg ecx
        cmpl    $0, %ecx                        # comparison between TIME_OF_DAY_PORT and 0
        jl      .ERROR                          # if TIME_OF_DAY_PORT < 0, 
        cmpl    $1382400, %ecx                  # comparison between TIME_OF_DAY_PORT and 1382400
        jge      .ERROR                         # if TIME_OF_DAY_PORT > 1382400

        movl    %ecx, %edx                      # %edx = TIMEOFDAYPORT
        andl    $0b1111, %edx                   # (TIME_OF_DAY_PORT & 0b1111)
        movl    %ecx, %esi                      # %esi = TIMEOFDAYPORT
        shrl    $4, %esi                        # TIMEOFDAYPORT >> 4
        movl    %esi, 0(%rdi)                   # set tod->day_secs
        cmpl    $8, %edx                        # (TIME_OF_DAY_PORT & 0b1111) >= 8
        jge     .ROUND                          # jump to round up 1
.SET_STRUCT:
        movl    0(%rdi), %eax

        cqto                                    # prep for division
        movl    $60, %r8d
        idivl   %r8d
        movw    %dx, 4(%rdi)                    # tod->time_secs = remainder

        cqto                                    # prep for division
        movl    $60, %r8d
        idivl   %r8d
        movw    %dx, 6(%rdi)                    # tod->time_mins = remainder

        cmpl    $12, %eax                       # keep track: %eax = time_hours
        movw    %ax, 8(%rdi)
        jl      .SET_AM                         # sets AM
        jge     .SET_PM

.NOON_CHECK:
        cqto
        movl    $12, %r8d                       # hours %= 12
        idivl   %r8d
        movw    %dx, 8(%rdi)                    # setting tod->time_hours
        cmpw    $0, 8(%rdi)                     # hours == 0
        je      .SET_MIDNIGHT
        jmp     .PASS                           # ensures passes if error isn't hit
.ERROR:
        movl    $1, %eax
        ret                                     # return 1
.ROUND:
        addl    $1, 0(%rdi)                     # add 1 to secs
        jmp     .SET_STRUCT

        ## a useful technique for this problem
        ## movX    SOME_GLOBAL_VAR(%rip), %reg  # load global variable into register
                                                # use movl / movq / movw / movb
                                                # and appropriately sized destination register
.SET_AM:
        movb    $1, 10(%rdi)                    
        jmp     .NOON_CHECK
.SET_PM:
        movb    $2, 10(%rdi)
        cmpw    $12, %ax                        # hours == 12
        je      .PASS                           # hours < 12
        jmp     .NOON_CHECK
.SET_MIDNIGHT:
        movw    $12, 8(%rdi)                    # 12:00AM
        jmp     .PASS
.PASS:
        movl    $0,%eax
        ret                                     # return 0

### Data area associated with the next function
.data                                           # IMPORTANT: use .data directive for data section

display_bits:                                   # display_bits[10]
        .int 0b1110111                          # [0]
        .int 0b0100100                          # [1]
        .int 0b1011101                          # [2]
        .int 0b1101101                          # [3]
        .int 0b0101110                          # [4]
        .int 0b1101011                          # [5]
        .int 0b1111011                          # [6]
        .int 0b0100101                          # [7]
        .int 0b1111111                          # [8]
        .int 0b1101111                          # [9]

.text                           # IMPORTANT: switch back to executable code after .data section
.global  set_display_from_tod

## ENTRY POINT FOR REQUIRED FUNCTION
set_display_from_tod:
        ## assembly instructions here
        ## %rdi, %rsi is tod_t tod
        ## %rdx is int 64 bit display pointer

	## two useful techniques for this problem
        ## movl    my_int(%rip),%eax    # load my_int into register eax
        ## leaq    my_array(%rip),%rdx  # load pointer to beginning of my_array into rdx
        pushq   %r12
        pushq   %r13
        movq    %rdi, %r9                       # %r9 holds copy of %rdi (struct)
        
        # r9 used for tod->time_secs & tod->time_mins
        shrq    $32, %r9                       # shift right to bit number of var
        andq    $0xFFFF, %r9                   # mask to a mask of Fs of the right
        movq    %r9, %r10                      # %r10 = time_secs (2 bytes)

        cmpq    $0, %r10                       # tod.time_secs < 0
        jl      .RESET
        cmpq    $59, %r10                      # tod.time_secs > 59
        jg      .RESET
        
        movq    %rdi, %r9
        shrq    $48, %r9                       # shift right to bit number of var
        andq    $0xFFFF, %r9                   # mask to a mask of Fs of the right
        movq    %r9, %r11                      # %r11 = time_mins (2 bytes)

        cmpq    $0, %r11                       # tod.time_mins < 0
        jl      .RESET
        cmpq    $59, %r11                      # tod.time_mins > 59
        jg      .RESET

        # r9 reused for tod->time_hours & tod->ampm
        movq    %rsi, %r9                       # %r9 holds copy of %rsi (struct)
        andq    $0xFFFF, %r9                    # masks to get hours 
        movq    %r9, %rcx                       # %rcx = time_hours (2 bytes)
        
        cmpq    $0, %rcx                        # tod.time_hours < 0
        jl      .RESET
        cmpq    $12, %rcx                       # tod.time_hours > 12
        jg      .RESET

        movq    %rsi, %r9
        shrq    $16, %r9                        # shift right to bit number of var
        andq    $0xFF, %r9                      # mask to a mask of Fs of the right
        movq    %r9, %r12                       # %r12 = time_ampm (1 byte)
        
        cmpq    $1, %r12
        jl      .RESET
        cmpq    $2, %r12
        jg      .RESET

        movq    %rdx, %r9     # *display        # move *display into %r9 so it doesn't override when we are dividing
        movl    $0, (%r9)                       # go to where r9 is pointing at and 0 out

        
        leaq display_bits(%rip),%rdi            # %rdi = display_bits arr
        movq    %r11, %rax                      # %eax = time_mins

        cqto                                    # prep for division
        movq    $10, %r8                        # tod.time_mins % 10
        idivq   %r8
        movl    (%rdi,%rdx,4),%esi              # (display_bits[min_ones])
        orl     %esi, (%r9)                     # x = x | (display_bits[min_ones]);

        movl    (%rdi,%rax,4),%esi              # (display_bits[min_tens])
        shll    $7, %esi                        # shift 7
        orl     %esi, (%r9)                     # x = x | (display_bits[min_tens]);

        movq    %rcx, %rax                      # tod.time_hours

        cqto                                    # prep for division
        movq    $10, %r8                        # tod.time_hours % 10
        idivq   %r8
        movl    (%rdi,%rdx,4),%esi
        shll    $14, %esi                       # shift 14
        orl     %esi, (%r9)                     # remainder, takes up r10d because we don't need seconds anymore
            # ^array^index^ size of array@index                              # MOVE RDX into new register and keep track
                                                # use orl for x = x | -----

        cmpq    $9, %rcx                        # tod.time_hours > 9
        jg      .DOUBLE_DIGIT_HOUR
.SET_AMPM:  
        cmpq    $1, %r12                        # if(ampm == 1)
        je      .AM                          
                                                # else: pm
        movl    $0b1, %r10d                     # %r9 = 0b1
        shll    $29, %r10d                      # (0b1 << 29)
        orl     %r10d, (%r9)                    # x = x | %ecx // pm
        jmp     .SET_DISPLAY

.DOUBLE_DIGIT_HOUR: # hour = rcx
        movq    $0b1, %r10
        movl    (%rdi, %r10, 4), %esi
        shlq    $21, %rsi
        orl     %esi, (%r9)                     # x = x | 
        jmp     .SET_AMPM
.AM:
        movl    $0b1, %r10d                     # %r9 = 0b1
        shll    $28, %r10d                      # (0b1 << 29)
        orl     %r10d, (%r9)                    # x = x | %ecx // am
        jmp     .SET_DISPLAY
.SET_DISPLAY:
        movq    %rcx, %rdx                      # *display = x
        jmp     .YAY
.RESET:                                         # error in set_display_tod
        movl    $1, %eax
        popq    %r13
        popq    %r12
        ret 
.YAY:                                           # passes in set_display_tod
        movl    $0, %eax
        popq    %r13
        popq    %r12
        ret 
.text
.global clock_update
        
## ENTRY POINT FOR REQUIRED FUNCTION
clock_update:
	subq    $24, %rsp                        # extend stack by 8 bytes
        movq    %rsp, %rdi                        # initializes empty tod struct

        call    set_tod_from_ports              # calls func
        cmpl    $1, %eax
        je     .FAIL
        leaq    CLOCK_DISPLAY_PORT(%rip), %rdx  # loads display pointer to rsi

        movq    (%rsp), %rdi                    # dereferences rdi
        movq    8(%rsp), %rsi
        call    set_display_from_tod            # calls set_display_from_tod
        cmpl    $1, %eax
        je     .FAIL
        addq    $24, %rsp

        movl    $0, %eax
        ret
.FAIL:
        addq    $24, %rsp                        # resets the stack before returning
        movl    $1, %eax
        ret