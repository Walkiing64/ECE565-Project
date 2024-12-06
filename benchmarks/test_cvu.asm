	.arch armv8-a
	.file	"test_cvu.c"
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"sum = %d"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB6:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	mov	x29, sp
	mov	x0, 8
	str	x0, [sp, 32]
	ldr	x0, [sp, 32]
	lsl	x0, x0, 2
	bl	malloc
	str	x0, [sp, 40]
	ldr	x0, [sp, 40]
	cmp	x0, 0
	bne	.L2
	mov	w0, 1
	b	.L3
.L2:
	str	wzr, [sp, 20]
	b	.L4
.L5:
	ldrsw	x0, [sp, 20]
	lsl	x0, x0, 2
	ldr	x1, [sp, 40]
	add	x0, x1, x0
	ldr	w1, [sp, 20]
	str	w1, [x0]
	ldr	w0, [sp, 20]
	add	w0, w0, 1
	str	w0, [sp, 20]
.L4:
	ldrsw	x0, [sp, 20]
	ldr	x1, [sp, 32]
	cmp	x1, x0
	bhi	.L5
	str	wzr, [sp, 24]
	str	wzr, [sp, 28]
	b	.L6
.L7:
	ldr	x0, [sp, 40]
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 4
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 8
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 12
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 16
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 20
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 24
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	x0, [sp, 40]
	add	x0, x0, 28
	ldr	w0, [x0]
	ldr	w1, [sp, 24]
	add	w0, w1, w0
	str	w0, [sp, 24]
	ldr	w0, [sp, 28]
	add	w0, w0, 1
	str	w0, [sp, 28]
.L6:
	ldr	w0, [sp, 28]
	cmp	w0, 31
	ble	.L7
	ldr	w1, [sp, 24]
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	printf
	mov	w0, 0
.L3:
	ldp	x29, x30, [sp], 48
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE6:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
