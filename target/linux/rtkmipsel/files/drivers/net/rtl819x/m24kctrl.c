
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/proc_fs.h> 
#include <asm/uaccess.h> 
#include <linux/ioport.h> 
#include <linux/timer.h> 
#include <linux/smp.h> 
#include <asm/io.h> 
#include "m24kctrl.h"
 
static long cp3ctrlswitch=0;
static long cp3mode=0x00000000;

unsigned long long int cp3Value[4] = {0};
unsigned long int cp3Count[4] = {0};
rtl8651_romeperf_stat_t cp3romePerfStat[CP3ROMEPERF_INDEX_MAX];
uint64 avrgCycle[CP3ROMEPERF_INDEX_MAX];

#define MODE_EN	0x0000000f

void start_perf_counter(unsigned int i)
{

	unsigned long mode0,mode2; 
	unsigned long flag;

/*
1. CYCLES:
   event 0: cycles

2. INST_FETCH(no exactly one but it's close):
  event 1: instruction graduated

3. ICACHE_MISS:
  counter 1 event 9: Instruction cache misses.

4. ICACHE_MISS_CYCLE:
  counter 0 event 37(0x25): Cycles where no instruction is fetched because we missed in the I-cache.
*/

    if (get_cpu() == 0) {
        //cp3mode = 0x00000025;   // Carl 0x01000100
        //cp3mode = 0x00000100;	// [7:0]= counter0 event, [15:8]= counter1 event
        //cp3mode = 0x00000925;

        cp3mode = i;
        //printk("get cp3mode = 0x%08x\n", (u32)cp3mode);

	mode0 = MODE_EN | ((cp3mode & 0x000000ff) << 5); 
	mode2 = MODE_EN | ((cp3mode & 0x0000ff00) >> 3); 

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
		"	mtc0	$0,	$25, 1		\n"
		"	sll     $0,	$0,  3		\n"
		"	mtc0	$0,	$25, 3		\n"
		"	sll     $0,	$0,  3		\n"
		"	mtc0	%[mod0],$25, 0		\n"
		"	sll     $0,	$0,  3		\n"
		"	mtc0	%[mod2],$25, 2		\n"
		"	sll     $0,	$0,  3		\n"
		"	.set pop			\n"
		:
		: [mod0] "r" (mode0),
		  [mod2] "r" (mode2)); 
	
    	local_irq_restore(flag);
    }
}

int stop_perf_counter(int record, unsigned int index)
{
	unsigned long perf_cnt0;
	unsigned long perf_cnt1;
	unsigned long flag;

    //static unsigned int pre_jiffies; 

	if ( index >= (sizeof(cp3romePerfStat)/sizeof(rtl8651_romeperf_stat_t)) ){
		printk("Invalid index!\n");
		return -1;
	}

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
		"	mfc0	%[cnt0],$25, 1		\n"
		"	mfc0	%[cnt1],$25, 3		\n"
		"	.set pop			\n"
		: [cnt0] "=&r" (perf_cnt0),
		  [cnt1] "=&r" (perf_cnt1)
		:); 

    	local_irq_restore(flag);
		if(record) {
			cp3romePerfStat[index].accCycle[0] += perf_cnt0;
			cp3romePerfStat[index].accCycle[1] += perf_cnt1;
			cp3romePerfStat[index].executedNum++;
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

	return 0;
}


__attribute__ ((unused)) void stopCP3Ctrl(int i, unsigned int index)
{
    if (cp3ctrlswitch) {
        stop_perf_counter(i, index);
        cp3ctrlswitch = 0;
    }
}

__attribute__ ((unused)) void startCP3Ctrl(unsigned int i)
{
	if (!cp3ctrlswitch) {
		cp3ctrlswitch = 1;
		start_perf_counter(i);
	}
}

static int __init cp3romePerfStat_init(void)
{
	memset( &cp3romePerfStat, 0, sizeof( cp3romePerfStat ) );
	memset( &avrgCycle, 0, sizeof(avrgCycle));

	return 1;
}

module_init(cp3romePerfStat_init);

