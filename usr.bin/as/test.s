	.set	noreorder

        .data
	.globl  x
x:      .word   123

        .bss
	.globl  y
y:      .space  1

#
# We need to test the following relocation types:
#   R_MIPS_32
#   R_MIPS_26
#   R_MIPS_PC16
#   R_MIPS_GPREL16
#   R_MIPS_HI16
#   R_MIPS_LO16
#
	.text
	.globl  start
        .type   start, @function
start:
        j       start - 4           # R_MIPS_26 backward
        j       _end + 4            # R_MIPS_26 forward

        b       start - 4           # R_MIPS_PC16 backward
        b       _end + 4            # R_MIPS_PC16 forward

	lw	$a0, %gp_rel(y - 4)($gp)    # R_MIPS_GPREL16 backward
	lw	$a0, %gp_rel(_end + 4)($gp) # R_MIPS_GPREL16 forward

        lui     $a0, %hi(y - 4)             # R_MIPS_HI16 backward
        addiu   $a0, %lo(y - 4)             # R_MIPS_LO16 backward

        lui     $a0, %hi(_end + 4)          # R_MIPS_HI16 forward
        addiu   $a0, %lo(_end + 4)          # R_MIPS_LO16 forward

        .type   _val, @object
_val:
        .word   start - 4           # R_MIPS_32 backward
        .word   _end + 4            # R_MIPS_32 forward
