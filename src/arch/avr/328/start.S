;
; Boot code
;

#define SPH   0x3E
#define SPL   0x3D

#ifndef RAM
#define RAM 2048
#endif

.macro	vector name
	.weak \name
	.set \name, __bad_interrupt
	jmp \name
.endm

.global __vectors
.section .vectors
__vectors:
.org 0x0
	jmp __ctors_end
	vector	__vector_1
	vector	__vector_2
	vector	__vector_3
	vector	__vector_4
	vector	__vector_5
	vector	__vector_6
	vector	__vector_7
	vector	__vector_8
	vector	__vector_9
	vector	__vector_10
	vector	__vector_11
	vector	__vector_12
	vector	__vector_13
	vector	__vector_14
	vector	__vector_15
	vector	__vector_16
	vector	__vector_17
	vector	__vector_18
	vector	__vector_19
	vector	__vector_20
	vector	__vector_21
	vector	__vector_22
	vector	__vector_23
	vector	__vector_24
	vector	__vector_25

.section .init9
__final_boot:
	call BermudaInit ; call main program

__bad_interrupt:
	reti
_exit:
	jmp __vectors
