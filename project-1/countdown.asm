; ----------------------------------------------------------------------------------------
; Counts down from 9 to 0, printing each.
; To assemble and run:
;
;     $ nasm -felf64 -g countdown.asm
;     $ ld -o countdown countdown.o
;     $ ./countdown	
; ----------------------------------------------------------------------------------------

; ----------------------------------------------------------------------------------------
        global    _start
; ----------------------------------------------------------------------------------------

; ----------------------------------------------------------------------------------------
        section   .text

_start:

	;; YOUR CODE GOES HERE.
	;; Make the program print, one line at a time: 9, 8, 7, ..., 1, 0.
top:	mov 	  rax, 1 	; system call for write
	mov 	  rdi, 1 	; file handle 1 is stdout
	mov       rsi,digit	; address of string to output
	mov 	  rdx, 2	; number of bytes
	cmp 	  byte [rsi], "0"
	jl	  end
	syscall
	dec       byte [rsi]
	jmp	  top
end:
	;; Call the kernel to exit.
        mov       rax, 60                 ; System call code for "exit".
        mov       rdi, 0                  ; Exit code for "normal exit".
        syscall                           ; Make it so.
; ----------------------------------------------------------------------------------------

; ----------------------------------------------------------------------------------------
          section   .data
digit:    db        "9",10	; The first digit to print, as a string with the appended newline.
; ----------------------------------------------------------------------------------------
