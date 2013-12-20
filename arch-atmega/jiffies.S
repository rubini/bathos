/* We need an interrupt handler to actually use "unsigned long _jiffies" */

.global do_jiffies_irq
.global _jiffies

.comm _jiffies,4,1

do_jiffies_irq:
	push r1
	in r1, 0x3f
	push r1
	eor r1, r1
	push r24
	push r25
	push r26
	push r27
	/* end of prologue */

	lds r24, _jiffies
	lds r25, _jiffies + 1
	lds r26, _jiffies + 2
	lds r27, _jiffies + 3
	adiw r24, 1
	adc r26, r1
	adc r27, r1
	sts _jiffies, r24
	sts _jiffies + 1, r25
	sts _jiffies + 2, r26
	sts _jiffies + 3, r27

	/* epilogue */
	pop r27
	pop r26
	pop r25
	pop r24
	pop r1
	out 0x3f, r1
	pop r1
	reti

	
