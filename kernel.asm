bits 32
section .boot
	dd 0x1badb002
	dd 0x00
	dd -(0x1badb002 + 0x00)

global start

extern k_setup
extern k_main

start:
	cli
	mov esp, stack_space
	call k_setup
	call k_main
	hlt

section .bss
	resb 8192
	stack_space:
