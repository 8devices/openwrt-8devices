
#if 1//#ifdef PERF_DUMP_1074K

//#include "8192cd_cfg.h"

#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/proc_fs.h> 
#include <asm/uaccess.h> 
#include <linux/ioport.h> 
#include <linux/timer.h> 
#include <linux/smp.h> 
#include <asm/io.h> 
 
static long cp3ctrlswitch=0;
static long cp3mode=0x00000000;

extern unsigned long long int cp3Value[4];
extern unsigned long int cp3Count[4];

#define MODE_EN	0x0000000f

static void start_perf_counter(void)
{
	unsigned long mode0, mode1, mode2, mode3;
	unsigned long flag;

/*
1. CYCLES:
   event 0: cycles

2. INST_FETCH(no exactly one but it's close):
  event 1: instruction graduated

3. ICACHE_MISS:
  counter 1/3 event 6: Instruction cache misses.

4. ICACHE_MISS_CYCLE:
  counter 0/2 event 7: Cycles where no instruction is fetched because we missed in the I-cache.
*/


    if (get_cpu() == 0) {
        cp3mode = 0x01000100;
        //cp3mode = 0x06070607;

    	mode0 = MODE_EN | ((cp3mode & 0x0000003f) << 5);
    	mode1 = MODE_EN | ((cp3mode & 0x00003f00) >> 3);
    	mode2 = MODE_EN | ((cp3mode & 0x003f0000) << 11);
    	mode3 = MODE_EN | ((cp3mode & 0x3f000000) >> 19);

    	//printk("[%d]start counter\n", get_cpu());
       
    	local_irq_save(flag);
    	/* start a timer to counter cp3 values */
    	__asm__ __volatile__ (
    	"	.set push			\n"
    	"	.set noreorder			\n"
    	"	mtc0	$0,	$25, 0		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 2		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 4		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 6		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 1		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 3		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 5		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 7		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	%[mod0],$25, 0		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	%[mod1],$25, 2		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	%[mod2],$25, 4		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	%[mod3],$25, 6		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	.set pop			\n"
    	:
    	: [mod0] "r" (mode0),
    	  [mod1] "r" (mode1),
    	  [mod2] "r" (mode2),
    	  [mod3] "r" (mode3));

    	local_irq_restore(flag);
    }
}

static void stop_perf_counter(int record)
{
	unsigned long perf_cnt0;
	unsigned long perf_cnt1;
	unsigned long perf_cnt2;
	unsigned long perf_cnt3;
	unsigned long flag;

    //static unsigned int pre_jiffies; 

    if (get_cpu() == 0) {

    	local_irq_save(flag);

    	__asm__ __volatile__ (
    	"	.set push			\n"
    	"	.set noreorder			\n"
    	"					\n"
    	"	mtc0	$0,	$25, 0		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 2		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 4		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mtc0	$0,	$25, 6		\n"
    	"	sll     $0,	$0,  3		\n"
    	"	mfc0	%[cnt0],$25, 1		\n"
    	"	mfc0	%[cnt1],$25, 3		\n"
    	"	mfc0	%[cnt2],$25, 5		\n"
    	"	mfc0	%[cnt3],$25, 7		\n"
    	"	.set pop			\n"
    	: [cnt0] "=&r" (perf_cnt0),
    	  [cnt1] "=&r" (perf_cnt1),
    	  [cnt2] "=&r" (perf_cnt2),
    	  [cnt3] "=&r" (perf_cnt3)
    	:);

    	local_irq_restore(flag);
		if(record) {
	        cp3Value[0] += perf_cnt0;
	        cp3Value[1] += perf_cnt1;
	        cp3Value[2] += perf_cnt2;
	        cp3Value[3] += perf_cnt3;
	        cp3Count[0]++;
	        cp3Count[1]++;
	        cp3Count[2]++;
	        cp3Count[3]++;
		}

#if 0
    //if ((jiffies - pre_jiffies) > 100) {
    if (1) {
    	printk("<%8lx> CPU %d: [0]%8lx,[1]%8lx,[2]%8lx,[3]%8lx\n", 
    		jiffies, get_cpu(),
    		perf_cnt0, perf_cnt1, perf_cnt2, perf_cnt3);
            //pre_jiffies = jiffies;        
    }
#endif
    }

}


static __attribute__ ((unused)) void stopCP3Ctrl(int i)
{
    if (cp3ctrlswitch) {
        stop_perf_counter(i);
        cp3ctrlswitch = 0;
    }
}

static __attribute__ ((unused)) void startCP3Ctrl(int i)
{
    if (!cp3ctrlswitch) {
        cp3ctrlswitch = 1;
        start_perf_counter();
    }
}

#endif // PERF_DUMP_1074K
