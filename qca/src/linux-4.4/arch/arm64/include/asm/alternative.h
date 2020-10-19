#ifndef __ASM_ALTERNATIVE_H
#define __ASM_ALTERNATIVE_H

#ifndef __ASSEMBLY__

#include <linux/init.h>
#include <linux/kconfig.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/stringify.h>

struct alt_instr {
	s32 orig_offset;	/* offset to original instruction */
	s32 alt_offset;		/* offset to replacement instruction */
	u16 cpufeature;		/* cpufeature bit set for replacement */
	u8  orig_len;		/* size of original instruction(s) */
	u8  alt_len;		/* size of new instruction(s), <= orig_len */
};

void __init apply_alternatives_all(void);
void apply_alternatives(void *start, size_t length);
void free_alternatives_memory(void);

#define ALTINSTR_ENTRY(feature)						      \
	" .word 661b - .\n"				/* label           */ \
	" .word 663f - .\n"				/* new instruction */ \
	" .hword " __stringify(feature) "\n"		/* feature bit     */ \
	" .byte 662b-661b\n"				/* source len      */ \
	" .byte 664f-663f\n"				/* replacement len */

/*
 * alternative assembly primitive:
 *
 * If any of these .org directive fail, it means that insn1 and insn2
 * don't have the same length. This used to be written as
 *
 * .if ((664b-663b) != (662b-661b))
 * 	.error "Alternatives instruction length mismatch"
 * .endif
 *
 * but most assemblers die if insn1 or insn2 have a .inst. This should
 * be fixed in a binutils release posterior to 2.25.51.0.2 (anything
 * containing commit 4e4d08cf7399b606 or c1baaddf8861).
 */
#define __ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg_enabled)	\
	".if "__stringify(cfg_enabled)" == 1\n"				\
	"661:\n\t"							\
	oldinstr "\n"							\
	"662:\n"							\
	".pushsection .altinstructions,\"a\"\n"				\
	ALTINSTR_ENTRY(feature)						\
	".popsection\n"							\
	".pushsection .altinstr_replacement, \"a\"\n"			\
	"663:\n\t"							\
	newinstr "\n"							\
	"664:\n\t"							\
	".popsection\n\t"						\
	".org	. - (664b-663b) + (662b-661b)\n\t"			\
	".org	. - (662b-661b) + (664b-663b)\n"			\
	".endif\n"

#define _ALTERNATIVE_CFG(oldinstr, newinstr, feature, cfg, ...)	\
	__ALTERNATIVE_CFG(oldinstr, newinstr, feature, IS_ENABLED(cfg))

#else

.macro altinstruction_entry orig_offset alt_offset feature orig_len alt_len
	.word \orig_offset - .
	.word \alt_offset - .
	.hword \feature
	.byte \orig_len
	.byte \alt_len
.endm

.macro alternative_insn insn1, insn2, cap, enable = 1
	.if \enable
661:	\insn1
662:	.pushsection .altinstructions, "a"
	altinstruction_entry 661b, 663f, \cap, 662b-661b, 664f-663f
	.popsection
	.pushsection .altinstr_replacement, "ax"
663:	\insn2
664:	.popsection
	.org	. - (664b-663b) + (662b-661b)
	.org	. - (662b-661b) + (664b-663b)
	.endif
.endm

/*
 * Begin an alternative code sequence.
 *
 * The code that follows this macro will be assembled and linked as
 * normal. There are no restrictions on this code.
 */
.macro alternative_if_not cap, enable = 1
	.if \enable
	.pushsection .altinstructions, "a"
	altinstruction_entry 661f, 663f, \cap, 662f-661f, 664f-663f
	.popsection
661:
	.endif
.endm

/*
 * Provide the alternative code sequence.
 *
 * The code that follows this macro is assembled into a special
 * section to be used for dynamic patching. Code that follows this
 * macro must:
 *
 * 1. Be exactly the same length (in bytes) as the default code
 *    sequence.
 *
 * 2. Not contain a branch target that is used outside of the
 *    alternative sequence it is defined in (branches into an
 *    alternative sequence are not fixed up).
 */
.macro alternative_else, enable = 1
	.if \enable
662:	.pushsection .altinstr_replacement, "ax"
663:
	.endif
.endm

/*
 * Complete an alternative code sequence.
 */
.macro alternative_endif, enable = 1
	.if \enable
664:	.popsection
	.org	. - (664b-663b) + (662b-661b)
	.org	. - (662b-661b) + (664b-663b)
	.endif
.endm

#define _ALTERNATIVE_CFG(insn1, insn2, cap, cfg, ...)	\
	alternative_insn insn1, insn2, cap, IS_ENABLED(cfg)


#endif  /*  __ASSEMBLY__  */

/*
 * Usage: asm(ALTERNATIVE(oldinstr, newinstr, feature));
 *
 * Usage: asm(ALTERNATIVE(oldinstr, newinstr, feature, CONFIG_FOO));
 * N.B. If CONFIG_FOO is specified, but not selected, the whole block
 *      will be omitted, including oldinstr.
 */
#define ALTERNATIVE(oldinstr, newinstr, ...)   \
	_ALTERNATIVE_CFG(oldinstr, newinstr, __VA_ARGS__, 1)

#endif /* __ASM_ALTERNATIVE_H */
