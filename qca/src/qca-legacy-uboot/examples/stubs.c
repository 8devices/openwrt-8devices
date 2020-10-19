#include <exports.h>

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif /* GCC_VERSION */

#if defined(CONFIG_I386)
/*
 * x86 does not have a dedicated register to store the pointer to
 * the global_data. Thus the jump table address is stored in a
 * global variable, but such approach does not allow for execution
 * from flash memory. The global_data address is passed as argv[-1]
 * to the application program.
 */
static void **jt;
gd_t *global_data;

#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	movl	%0, %%eax\n"		\
"	movl	jt, %%ecx\n"		\
"	jmp	*(%%ecx, %%eax)\n"	\
	: : "i"(XF_ ## x * sizeof(void *)) : "eax", "ecx");
#elif defined(CONFIG_PPC)
/*
 * r29 holds the pointer to the global_data, r11 is a call-clobbered
 * register
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lwz	%%r11, %0(%%r29)\n"	\
"	lwz	%%r11, %1(%%r11)\n"	\
"	mtctr	%%r11\n"		\
"	bctr\n"				\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "r11");
#elif defined(CONFIG_ARM)
/*
 * r8 holds the pointer to the global_data, ip is a call-clobbered
 * register
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	ldr	ip, [r8, %0]\n"		\
"	ldr	pc, [ip, %1]\n"		\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "ip");
#elif defined(CONFIG_MIPS)
/*
 * k0 ($26) holds the pointer to the global_data; t9 ($25) is a call-
 * clobbered register that is also used to set gp ($26). Note that the
 * jr instruction also executes the instruction immediately following
 * it; however, GCC/mips generates an additional `nop' after each asm
 * statement
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lw	$25, %0($26)\n"		\
"	lw	$25, %1($25)\n"		\
"	jr	$25\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "t9");
#elif defined(CONFIG_NIOS)
/*
 * %g7 holds the pointer to the global_data. %g0 is call clobbered.
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	pfx	%%hi(%0)\n"		\
"	movi	%%g0, %%lo(%0)\n"	\
"	add	%%g0, %%g7\n"		\
"	ld	%%g0, [%%g0]\n"		\
"	pfx	%1\n"			\
"	ld	%%g0, [%%g0]\n"		\
"	jmp	%%g0\n"			\
"	nop	\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x) : "r0");
#elif defined(CONFIG_NIOS2)
/*
 * r15 holds the pointer to the global_data, r8 is call-clobbered
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	movhi	r8, %%hi(%0)\n"		\
"	ori	r8, r0, %%lo(%0)\n"	\
"	add	r8, r0, r15\n"		\
"	ldw	r8, 0(r8)\n"		\
"	ldw	r8, %1(r8)\n"		\
"	jmp	r8\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "r15");
#elif defined(CONFIG_M68K)
/*
 * d7 holds the pointer to the global_data, a0 is a call-clobbered
 * register
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	move.l	%%d7, %%a0\n"		\
"	adda.l	%0, %%a0\n"		\
"	move.l	(%%a0), %%a0\n"		\
"	adda.l	%1, %%a0\n"		\
"	move.l	(%%a0), %%a0\n"		\
"	jmp	(%%a0)\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "a0");
#elif defined(CONFIG_MICROBLAZE)
/*
 * r31 holds the pointer to the global_data. r5 is a call-clobbered.
 */
#define EXPORT_FUNC(x)				\
	asm volatile (				\
"	.globl " #x "\n"			\
#x ":\n"					\
"	lwi	r5, r31, %0\n"			\
"	lwi	r5, r5, %1\n"			\
"	bra	r5\n"				\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "r5");
#elif defined(CONFIG_BLACKFIN)
/*
 * P5 holds the pointer to the global_data, P0 is a call-clobbered
 * register
 */
#define EXPORT_FUNC(x)			\
	asm volatile (			\
"       .globl " #x "\n"		\
#x ":\n"				\
"	P0 = [P5 + %0]\n"		\
"	P0 = [P0 + %1]\n"		\
"	JUMP (P0)\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "P0");
#else
#error stubs definition missing for this architecture
#endif

/* This function is necessary to prevent the compiler from
 * generating prologue/epilogue, preparing stack frame etc.
 * The stub functions are special, they do not use the stack
 * frame passed to them, but pass it intact to the actual
 * implementation. On the other hand, asm() statements with
 * arguments can be used only inside the functions (gcc limitation)
 */
#if GCC_VERSION < 3004
static
#endif /* GCC_VERSION */
void __attribute__((unused)) dummy(void)
{
#include <_exports.h>
}

extern unsigned long __bss_start, _end;

void app_startup(char **argv)
{
	unsigned long * cp = &__bss_start;

	/* Zero out BSS */
	while (cp < &_end) {
		*cp++ = 0;
	}

#if defined(CONFIG_I386)
	/* x86 does not have a dedicated register for passing global_data */
	global_data = (gd_t *)argv[-1];
	jt = global_data->jt;
#endif
}

#undef EXPORT_FUNC
