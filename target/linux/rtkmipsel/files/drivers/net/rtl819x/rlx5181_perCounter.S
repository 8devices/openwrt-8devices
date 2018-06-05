#include "asm/rtl865x/regdef.h"


	.align  5
	.section .iram-gen

	.ent init_CP3_counter
	.globl init_CP3_counter
init_CP3_counter:
        mfc0 	t0, $12
        la 	t1, 0x80000000
        or 	t0, t1
        mtc0 	t0, $12
	nop
	jr ra
	nop
	.end init_CP3_counter

	.ent clear_CP3_counter
	.globl clear_CP3_counter
clear_CP3_counter:
/* stop counters */
        ctc3 zero, $0
/* clear counters */
        mtc3 zero, $8
        mtc3 zero, $9
        mtc3 zero, $10
        mtc3 zero, $11
        mtc3 zero, $12
        mtc3 zero, $13
        mtc3 zero, $14
        mtc3 zero, $15
	nop
	jr ra
	nop
	.end clear_CP3_counter


/* set counter controls:  0	 1	 2	 3
** counter0 cycles 	  (0x10) (0x10) (0x10) (0x10)
** counter1 ifetches   	  (0x11) (0x11) (0x11)
** counter1 dmiss busy    		       (0x1b)
** counter2 ld+stores	  (0x16) 	       (0x16)
** counter2 icache misses	 (0x12) (0x12)
** counter3 dcache misses (0x1a) (0x1a)        (0x1a)
** counter3 imiss busy   	        (0x13)
*/
	/*a0: counter mode 0 ~ 3*/
	.ent set_and_start_counter
	.globl set_and_start_counter
set_and_start_counter:
	addiu sp, sp, -8
	sw t1, 0(sp)
	sw t3, 4(sp)

	move t3, a0
        la t1, 0x1a161110
        beq t3, zero, 1f #cntmode == 0
        addiu t3, -1
        la t1, 0x1a121110
        beq t3, zero, 1f #cntmode == 1
        addiu t3, -1
        la t1, 0x13121110
        beq t3, zero, 1f #cntmode == 2
        addiu t3, -1
        la t1, 0x1a161b10
        beq t3, zero, 1f #cntmode == 3
        nop
1:
	ctc3 t1, $0
	nop

	lw t1, 0(sp)
	lw t3, 4(sp)
	addiu sp, sp, 8
	jr ra
	nop
	.end set_and_start_counter

	.ent stop_counter
	.globl stop_counter
stop_counter:
	ctc3 zero, $0
	nop
	jr ra
	nop
	.end stop_counter

/*a0: the address for saving counter 0 */
/*a1: the address for saving counter 1 */
/*a2: the address for saving counter 2 */
/*a3: the address for saving counter 3 */
	.ent save_counter
	.globl save_counter
save_counter:
	addiu sp, sp, -4
	sw t0, 0(sp)

        mfc3 t0, $9 #counter0[48:32]
        nop
        sw t0, 0(a0)
        mfc3 t0, $8 #counter0[31:0]
        nop
        nop
        sw t0, 4(a0)
        mfc3 t0, $11 #counter1[48:32]
        nop
        sw t0, 8(a0)
        mfc3 t0, $10 #counter1[31:0]
        nop
        nop
        sw t0, 12(a0)
        mfc3 t0, $13 #counter2[48:32]
        nop
        sw t0, 16(a0)
        mfc3 t0, $12 #counter2[31:0]
        nop
        nop
        sw t0, 20(a0)
        mfc3 t0, $15 #counter3[48:32]
        nop
        sw t0, 24(a0)
        mfc3 t0, $14 #counter3[31:0]
        nop
        nop
        sw t0, 28(a0)
	nop


	lw t0, 0(sp)
	addiu sp, sp, 4
	jr ra
	nop
	.end save_counter
	
