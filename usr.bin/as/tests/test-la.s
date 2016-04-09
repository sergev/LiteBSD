	.set	noreorder

	.globl  g

        .data
	.globl  da, db
da:     .word   123
db:     .word   567

        .bss
	.globl  vc, vd
vc:     .space  1
vd:     .space  1

#
# Test LA instruction
#
	.text
	.globl  start
        .type   start, @function
start:
        la      $a0, g
        la      $a0, da
        la      $a0, db
        la      $a0, vc
        la      $a0, vd
        la      $a0, start
        la      $a0, te
