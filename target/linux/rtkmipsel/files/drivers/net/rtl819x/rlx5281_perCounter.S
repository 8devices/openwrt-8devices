#include <asm/rtl865x/regdef.h>
/* CP3 Performance Counter Event list */
#define INST_FEATCH                (0x01)
#define ICACHE_MISS                (0x02)
#define ICACHE_MISS_CYCLE          (0x03)
#define STORE_INST                 (0x04)
#define LOAD_INST                  (0x05)
#define LORDSTROE_INST             (0x06)
#define COMPLETED_INST             (0x07)
#define CYCLES                     (0x08)
#define DCACHE_MISSE               (0x0A)
#define DCACHE_MISS_CYCLE          (0x0B)
#define L2_CACHE_HIT               (0x0C)
#define L2_CACHE_HIT_CYCLE         (0x0D)
#define L2_CACHE_MISS              (0x0E)
#define L2_CACHE_MISS_CYCLE        (0x0F)
#define BRANCEH_PREDICTION         (0x10)
#define BRANCEH_PREDICTION_MISS    (0x11)
#define BTB_MISS                   (0x12)
#define BTB_TARGET_MISS            (0x13)
#define CBP_MISS                   (0x14)
#define RS_POP                     (0x15)
#define RS_MISS                    (0x16)
#define IBUF_FULL_CYCLE            (0x18)
#define IBUF_EMPTY_CYCLE           (0x19)
#define ZERO_INST_COMMIT_CYCLE     (0x20)
#define PIPE_A_INST_COMMIT_CYCLE   (0x21)
#define PIPE_B_INST_COMMIT_CYCLE   (0x22)
#define DUAL_INST_COMMIT_CYCLE     (0x23)
#define OCP_BUS_READS              (0x24)
#define OCP_BUS_WRITES             (0x25)
#define OCP_BUS_UC_READ            (0x26)
#define OCP_BUS_UC_WRITE           (0x27)
#define OCP_BUS_DATA_READ          (0x28)
#define OCP_BUS_DATA_WRITE         (0x29)
#define OCP_BUS_INS_READ           (0x2A)
#define CP3_COUTER3(event)         (event << 24)
#define CP3_COUTER2(event)         (event << 16)
#define CP3_COUTER1(event)         (event << 8 )
#define CP3_COUTER0(event)         (event << 0 )

//void clear_CP3_counter(void);
//void set_and_start_counter(uint32 counter_control);
//void stop_counter(void);
//void save_counter(uint32 *addr0, uint32 *addr1,uint32 *addr2,uint32 *addr3);


	.align  5
//	.section .iram-gen

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

/* set counter controls:  0   	 1 	2 	3
** counter0 cycles	  (0x08) (0x08) (0x08) (0x08)
** counter1 ifetches 	  (0x01) (0x01) (0x01)	
** counter1 dmiss busy			       (0x0b)
** counter2 ld+stores 	  (0x06)	       (0x06)
** counter2 icache misses	 (0x02) (0x02)
** counter3 dcache misses (0x0a) (0x0a)        (0x0a)
** counter3 imiss busy	 		(0x03)
*/
	/*a0: counter mode 0 ~ 3*/
	.ent set_and_start_counter
	.globl set_and_start_counter
set_and_start_counter:
	addiu sp, sp, -8
	sw t1, 0(sp)
	sw t3, 4(sp)

	move t3, a0
        la t1, 0x0a060108
        beq t3, zero, 1f #cntmode == 0
        addiu t3, -1
        la t1, 0x0a020108
        beq t3, zero, 1f #cntmode == 1
        addiu t3, -1
        la t1, 0x03020108
        beq t3, zero, 1f #cntmode == 2
        addiu t3, -1
        la t1, 0x0a060b08
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
	
