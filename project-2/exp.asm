;;; ----------------------------------------------------------------------------------------
;;; Calculate 
;;; the console, and then exit.
;;;
;;; To assemble and run:
;;;
;;;     $ nasm -felf64 -g exp.asm
;;;     $ gcc -o exp exp.o
;;;     $ ./exp
;;; ----------------------------------------------------------------------------------------

;;; ----------------------------------------------------------------------------------------
        global  main
	global  exp
	extern  printf
;;; ----------------------------------------------------------------------------------------

;;; ----------------------------------------------------------------------------------------
        section	.text	              ; The start of the code portion of the program.
	
main:                                 ; Called by the C library stub code.

	;; Call exp to calculate x^y.
	mov	rdi, [x]	      ; arg[0] = base
	mov	rsi, [y]	      ; arg[1] = exponent
	sub	rsp, 8		      ; Align rsp to double-world boundary.
	call	exp		      ; Calculate rax = x^y
	add	rsp, 8		      ; Restore rsp.

	;; Print the result of x^y.
	mov	rdi, int_format_string ; arg[0] = formatting string.
	mov	rsi, rax	       ; arg[1] = x^y (supra)
	mov	rax, 0		       ; arg[2] = 0 (end of arguments marker for varargs)
	sub	rsp, 8		       ; Align rsp.
	call	printf
	add	rsp, 8		       ; Restore rsp.

	mov	rax, 0		       ; Exit status code = "all normal".
	ret			       ; End of main.

exp:

	;; YOUR CODE GOES HERE.  Calculate x^y, where:
	;;   rdi = x (the base)
	;;   rsi = y (the exponent)
	;;   rax = x^y (the return value carried back to the caller)

	;; base case.
	cmp rsi, 0
	je end
	;; it is not 0, so we need to recur on x^y-1
	dec rsi
	call exp
	;; returns a new rax that we can multiply, and return
	imul rax, rdi
	ret



end:				; Base case has been reached
	mov rax, 1		; make sure that 1 is returned (this would be x^0 which will always be 1
	ret
;;; ----------------------------------------------------------------------------------------

;;; ----------------------------------------------------------------------------------------
        section   .data		         ; The start of the data portion of the program.

x:	dq	4
y:	dq	20
	
int_format_string:	
	db        "%lu", 10, 0	         ; Formatting string for an int, newline, null-terminator.
	;; quick note: I had to change this from %d to %lu because when you use the exp on large integers,
	;; the %d would expect a 32 bit integer. But since the registers we are using that can go to 64-bit unsigned longs,
	;; I had to change the format specifier to hold such a large number. I found this while debugging, because all of
	;; the exp calculations would be correct up until we crossed the max size of a 32-bit integer, which is the max
	;; value when using %d
;;; ----------------------------------------------------------------------------------------
