;;; ----------------------------------------------------------------------------------------
;;; Given a string to print (see data section), determine its length, write it to
;;; the console, and then exit.
;;;
;;; To assemble and run:
;;;
;;;     $ nasm -felf64 -g neo-hello.asm
;;;     $ gcc -o neo-hello neo-hello.o
;;;     $ ./neo-hello
;;; ----------------------------------------------------------------------------------------

;;; ----------------------------------------------------------------------------------------
        global  main
;;; ----------------------------------------------------------------------------------------

;;; ----------------------------------------------------------------------------------------
        section	.text	              ; The start of the code portion of the program.
	
main:                                 ; Called by the C library stub code.

	;; Call the strlen() to get the length of the message.
	mov	rdi, greetings      ; arg[0] = pointer to string.
	sub	rsp, 8	            ; Make rsp double-word aligned.
	call	string_length       ; Call, leaving rax = length(greetings).
	add	rsp, 8	            ; Restore rsp.

	;; Use the length provided to print the message.
	mov	rdx, rax	    ; rdx gets the number of bytes to write.
	mov     rax, 1              ; rax gets the system call code for "write".
        mov     rdi, 1              ; rdi gets the file handle for stdout (console).
        mov     rsi, greetings      ; rsi gets the address of the string below.
        syscall	                    ; Call kernel, triggering the write.  The registers carry the arguments.

	ret		   	    ; End of main().  Stub code will exit the process.

string_length:

	;; YOUR CODE GOES HERE.  Complete this procedure, where:
	;;   rdi contains the pointer to the null-terminated string.
	;;   rax will carry the return value of the length of the given string.
	mov rax, 0
top:	cmp byte[rdi], 0
	je end
	inc rax
	inc rdi
	jmp top
end:
	sub rdi, rax
	ret
;;; ----------------------------------------------------------------------------------------

;;; ----------------------------------------------------------------------------------------
        section   .data		         ; The start of the data portion of the program.

greetings:			         ; Labels the string of bytes to be written.
	db        "I have tested this program with a variety of different strings. It works!", 10, 0  ; The message, with the byte values for "newline"
                                      	 ; and the null terminator appended.
;;; ----------------------------------------------------------------------------------------
