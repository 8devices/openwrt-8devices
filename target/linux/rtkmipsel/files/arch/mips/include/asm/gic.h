/*
 * Realtek Semiconductor Corp.
 *
 * GIC Register Definitions
 *
 * Copyright (C) 2012  Tony Wu (tonywu@realtek.com)
 */
#ifndef _ASM_GIC_H_
#define _ASM_GIC_H_

/* GIC's Nomenclature for Core Routing Pins */
#define GIC_CPU0			(0) /* Core 0 */
#define GIC_CPU1			(1) /* Core 1 */
#define GIC_CPU2			(2) /* Core 2 */
#define GIC_CPU3			(3) /* Core 3 */

/* GIC's Nomenclature for Core Interrupt Pins */
#define GIC_CPU_INT0			0xdead /* Core SW Interrupt 0 */
#define GIC_CPU_INT1			0xdead /* Core SW Interrupt 1 */
#define GIC_CPU_INT2			0x0 /* Core Interrupt 2 */
#define GIC_CPU_INT3			0x1 /* Core Interrupt 3 */
#define GIC_CPU_INT4			0x2 /* Core Interrupt 4 */
#define GIC_CPU_INT5			0x3 /* Core Interrupt 5 */
#define GIC_CPU_INT6			0x4 /* Core Interrupt 6 */
#define GIC_CPU_INT7			0x5 /* Core Interrupt 7 */
#define GIC_CPU_VEC0			0x6 /* LOPI Interrupt 0 */
#define GIC_CPU_VEC1			0x7 /* LOPI Interrupt 1 */
#define GIC_CPU_VEC2			0x8 /* LOPI Interrupt 2 */
#define GIC_CPU_VEC3			0x9 /* LOPI Interrupt 3 */
#define GIC_CPU_VEC4			0xa /* LOPI Interrupt 4 */
#define GIC_CPU_VEC5			0xb /* LOPI Interrupt 5 */
#define GIC_CPU_VEC6			0xc /* LOPI Interrupt 6 */
#define GIC_CPU_VEC7			0xd /* LOPI Interrupt 7 */

#define GIC_IRQ_BASE			0

#define GIC_EXT_INTR(x)			x
#define GIC_NUM_INTRS			(24 + NR_CPUS * 2)

#define GIC_IPI_RESCHED_BASE		(GIC_NUM_INTRS - NR_CPUS * 2)
#define GIC_IPI_CALL_BASE		(GIC_NUM_INTRS - NR_CPUS)

#define GIC_IPI_RESCHED(cpu)		(GIC_IPI_RESCHED_BASE + (cpu))
#define GIC_IPI_CALL(cpu)		(GIC_IPI_CALL_BASE + (cpu))

/* GIC Accessors */
#define _GMSK(n)		((1 << (n)) - 1)
#define _GR32(addr,offs)	(*(volatile unsigned int *) ((addr) + (offs)))

#define GIC_SHF(reg,bit)	(GIC_##reg##_##bit##_SHF)
#define GIC_MSK(reg,bit)	(_GMSK(GIC_##reg##_##bit##_BIT) << GIC_SHF(reg,bit))

#define GIC_REG32(reg)		_GR32(MMCR_BASE,GIC_##reg##_OFS)
#define GIC_REG32p(reg,p)	_GR32(MMCR_BASE,GIC_##reg##_OFS + (p))

#define GIC_VAL32(reg,bit)	((GIC_REG32(reg) & GIC_MSK(reg,bit)) >> GIC_SHF(reg,bit))
#define GIC_VAL32p(reg,p,bit)	((GIC_REG32p(reg,p) & GIC_MSK(reg,bit)) >> GIC_SHF(reg,bit))

/* Global control register */
#define GIC_GLOBAL_CTRL_OFS		0x1000
#define GIC_GLOBAL_DEBUG_OFS		0x1008
#define GIC_IPI_OFS			0x100c

/*
 * Set Mask (WO) - Enables Interrupt
 * 1-bit per interrupt
 */
#define GIC_SMASK_OFS			0x1020
#define GIC_SMASK0_OFS			0x1020
#define GIC_SMASK1_OFS			0x1024
#define GIC_SMASK2_OFS			0x1028
#define GIC_SMASK3_OFS			0x102c
#define GIC_SMASK4_OFS			0x1030
#define GIC_SMASK5_OFS			0x1034
#define GIC_SMASK6_OFS			0x1038
#define GIC_SMASK7_OFS			0x103c

/*
 * Reset Mask - Disables Interrupt
 * 1-bit per interrupt
 */
#define GIC_RMASK_OFS			0x1040
#define GIC_RMASK0_OFS			0x1040
#define GIC_RMASK1_OFS			0x1044
#define GIC_RMASK2_OFS			0x1048
#define GIC_RMASK3_OFS			0x104c
#define GIC_RMASK4_OFS			0x1050
#define GIC_RMASK5_OFS			0x1054
#define GIC_RMASK6_OFS			0x1058
#define GIC_RMASK7_OFS			0x105c

/*
 * Global Interrupt Mask Register (RO) - Bit Set == Interrupt enabled
 * 1-bit per interrupt
 */
#define GIC_MASK_OFS			0x1060
#define GIC_MASK0_OFS			0x1060
#define GIC_MASK1_OFS			0x1064
#define GIC_MASK2_OFS			0x1068
#define GIC_MASK3_OFS			0x106c
#define GIC_MASK4_OFS			0x1070
#define GIC_MASK5_OFS			0x1074
#define GIC_MASK6_OFS			0x1078
#define GIC_MASK7_OFS			0x107c

/*
 * Pending Global Interrupts (RO)
 * 1-bit per interrupt
 */

#define GIC_PEND_OFS			0x1080
#define GIC_PEND0_OFS			0x1080
#define GIC_PEND1_OFS			0x1084
#define GIC_PEND2_OFS			0x1088
#define GIC_PEND3_OFS			0x108c
#define GIC_PEND4_OFS			0x1090
#define GIC_PEND5_OFS			0x1094
#define GIC_PEND6_OFS			0x1098
#define GIC_PEND7_OFS			0x109c

/*
 * R2P and R2C
 * one word per interrupt
 */
#define GIC_R2P_OFS			0x1100
#define GIC_R2C_OFS			0x1500

#define GIC_LOCAL_CONTROL_OFS		0x1900
#define GIC_DBG_GROUP_OFS		0x1908
#define  GIC_DBG_GROUP_INVITE_CORE_SHF	0
#define  GIC_DBG_GROUP_INVITE_CORE_BIT	4
#define  GIC_DBG_GROUP_JOIN_GDINT_SHF	8
#define  GIC_DBG_GROUP_JOIN_GDINT_BIT	1
#define GIC_RPT_INTNUM_OFS		0x190c
#define GIC_SITIMER_R2P_OFS		0x1910
#define  GIC_SITIMER_R2P_SITIMER_SHF	0
#define  GIC_SITIMER_R2P_SITIMER_BIT	8

/* Convert an interrupt number to a byte offset/bit for multi-word registers */
#define GIC_INTR_OFS(intr)		(((intr)/32)*4)
#define GIC_INTR_BIT(intr)		(((intr)%32))

/* Maps Interrupt X to a pin */
#define GIC_ROUTE_TO_PIN(intr,pin)	(GIC_REG32p(R2P, (intr)*4) = (pin))

/*
 * Maps Interrupt X to cores:
 *   the input is one-hot bitmap
 *   cpu = (1 << core)
 *   1 << 0 => core 0
 *   1 << 1 => core 1
 *   1 << n => core n
 *
 * So far, we assume one interrupt per cpu
 */
#define GIC_ROUTE_TO_CORE(intr,cpu)	(GIC_REG32p(R2C, (intr)*4) = (1 << cpu))

/* Mask manipulation */
#define GIC_SET_INTR_MASK(intr) \
	(GIC_REG32p(SMASK,GIC_INTR_OFS(intr)) = 1 << GIC_INTR_BIT(intr))

#define GIC_SET_IPI_MASK(intr) \
	(GIC_REG32(IPI) = 0x80000000 | intr)

#define GIC_CLR_INTR_MASK(intr) \
	(GIC_REG32p(RMASK,GIC_INTR_OFS(intr)) = 1 << GIC_INTR_BIT(intr))

#define GIC_CLR_IPI_MASK(intr) \
	(GIC_REG32(IPI) = intr)

struct gic_pcpu_mask {
	DECLARE_BITMAP(pcpu_mask, GIC_NUM_INTRS);
};

struct gic_pending_regs {
	DECLARE_BITMAP(pending, GIC_NUM_INTRS);
};

struct gic_intrmask_regs {
	DECLARE_BITMAP(intrmask, GIC_NUM_INTRS);
};

/*
 * Interrupt Meta-data specification. The ipiflag helps
 * in building ipi_map.
 */
#define GIC_FLAG_SHARED_MASK	0x00ff
#define GIC_FLAG_LOCAL_MASK	0xff00

#define GIC_UNUSED		0xdead		/* Dummy data */
#define GIC_FLAG_IPI		0x0001
#define GIC_FLAG_TRANSPARENT	0x0002
#define GIC_FLAG_SWINT0		0x0100		/* core local flags */
#define GIC_FLAG_SWINT1		0x0200
#define GIC_FLAG_TIMER		0x0400

struct gic_intr_map {
	unsigned int cpunum;	/* Directed to this CPU */
	unsigned int pin;	/* Directed to this Pin */
	unsigned int polarity;	/* Polarity : +/-	*/
	unsigned int trigtype;	/* Trigger  : Edge/Levl */
	unsigned int flags;	/* Misc flags	*/
};

extern void gic_init(struct gic_intr_map *intrmap, unsigned int irqbase);
extern void gic_setup_ipi(unsigned int *ipimap, unsigned int resched,
			  unsigned int call);

extern unsigned int gic_get_resched_int(unsigned int);
extern unsigned int gic_get_call_int(unsigned int);
extern unsigned int gic_get_int(void);
extern void gic_send_ipi(unsigned int intr);
extern unsigned int gic_get_timer_pending(void);
extern unsigned int gic_get_int(void);
extern void gic_enable_interrupt(int irq_vec);
extern void gic_disable_interrupt(int irq_vec);
extern void gic_irq_ack(struct irq_data *d);
extern void gic_finish_irq(struct irq_data *d);
extern void gic_platform_init(int irqs, struct irq_chip *irq_controller);

#endif /* _ASM_GIC_H_ */
