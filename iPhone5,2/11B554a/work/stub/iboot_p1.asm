
iboot_p1.o:     file format elf32-littlearm


Disassembly of section .text:

00000000 <_entry>:
       0:	eafffffe 	b	0 <_entry>
	...

00021064 <_get_current_task>:
   21064:	4770      	bx	lr
	...

000227ec <_sub_9FF227EC>:
   227ec:	4770      	bx	lr
	...

000257e0 <_decompress_lzss>:
   257e0:	4770      	bx	lr
	...

000341d8 <_bcopy>:
   341d8:	e12fff1e 	bx	lr
	...

00034c2c <_disable_interrupts>:
   34c2c:	4770      	bx	lr
	...

00047bb0 <_payload>:
   47bb0:	f8df d050 	ldr.w	sp, [pc, #80]	; 47c04 <next+0xa>
   47bb4:	f7ed f83a 	bl	34c2c <_disable_interrupts>
   47bb8:	4c13      	ldr	r4, [pc, #76]	; (47c08 <next+0xe>)
   47bba:	4814      	ldr	r0, [pc, #80]	; (47c0c <next+0x12>)
   47bbc:	4621      	mov	r1, r4
   47bbe:	4a14      	ldr	r2, [pc, #80]	; (47c10 <next+0x16>)
   47bc0:	f7ec eb0a 	blx	341d8 <_bcopy>
   47bc4:	f44f 5110 	mov.w	r1, #9216	; 0x2400
   47bc8:	f8a4 1e54 	strh.w	r1, [r4, #3668]	; 0xe54
   47bcc:	4811      	ldr	r0, [pc, #68]	; (47c14 <next+0x1a>)
   47bce:	f04f 4100 	mov.w	r1, #2147483648	; 0x80000000
   47bd2:	5021      	str	r1, [r4, r0]
   47bd4:	4810      	ldr	r0, [pc, #64]	; (47c18 <next+0x1e>)
   47bd6:	4911      	ldr	r1, [pc, #68]	; (47c1c <next+0x22>)
   47bd8:	5021      	str	r1, [r4, r0]
   47bda:	f7d9 fa43 	bl	21064 <_get_current_task>
   47bde:	2100      	movs	r1, #0
   47be0:	6441      	str	r1, [r0, #68]	; 0x44
   47be2:	480f      	ldr	r0, [pc, #60]	; (47c20 <next+0x26>)
   47be4:	21fc      	movs	r1, #252	; 0xfc
   47be6:	4a0f      	ldr	r2, [pc, #60]	; (47c24 <next+0x2a>)
   47be8:	23e6      	movs	r3, #230	; 0xe6
   47bea:	4605      	mov	r5, r0
   47bec:	f7dd fdf8 	bl	257e0 <_decompress_lzss>
   47bf0:	480d      	ldr	r0, [pc, #52]	; (47c28 <next+0x2e>)
   47bf2:	e002      	b.n	47bfa <next>
   47bf4:	e7ffdef0 			; <UNDEFINED> instruction: 0xe7ffdef0
   47bf8:	4780def0 			; <UNDEFINED> instruction: 0x4780def0

00047bfa <next>:
   47bfa:	4780      	blx	r0
   47bfc:	f7da fdf6 	bl	227ec <_sub_9FF227EC>
   47c00:	47a8      	blx	r5
   47c02:	4720      	bx	r4
   47c04:	bfff8000 	svclt	0x00ff8000
   47c08:	84000000 	strhi	r0, [r0], #-0
   47c0c:	bff00000 	svclt	0x00f00000	; IMB
   47c10:	000446c0 	andeq	r4, r4, r0, asr #13
   47c14:	00041e88 	andeq	r1, r4, r8, lsl #29
   47c18:	0001ad14 	andeq	sl, r1, r4, lsl sp
   47c1c:	60182000 	andsvs	r2, r8, r0
   47c20:	bff48000 	svclt	0x00f48000
   47c24:	bff47a7c 	svclt	0x00f47a7c
   47c28:	bff01769 	svclt	0x00f01769

Disassembly of section .ARM.attributes:

00000000 <.ARM.attributes>:
   0:	00001541 	andeq	r1, r0, r1, asr #10
   4:	61656100 	cmnvs	r5, r0, lsl #2
   8:	01006962 	tsteq	r0, r2, ror #18
   c:	0000000b 	andeq	r0, r0, fp
  10:	01080806 	tsteq	r8, r6, lsl #16
  14:	Address 0x00000014 is out of bounds.

