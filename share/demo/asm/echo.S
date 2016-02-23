/*
 * This is a standard /bin/echo utility, rewritten in MIPS assembler.
 *
 * To compile this program, type:
 *  cc -c echo.S
 *  ld echo.o -o echo
 *
 * Run as:
 *  ./echo Make love not war
 */
#include <sys/syscall.h>

        .data                           // begin data segment
eoln:   .ascii  "\n"
space:  .ascii  " "

        .text                           // begin code segment
start:  .globl  start                   // entry point for ld

        //
        // Program gets three arguments:
        // argc in $a0 - number of words in command line
        // argv in $a1 - address of list of pointers to words
        // env  in $a2 - address of list of pointers to environment variables
        //
        addi    $s0, $a0, -1            // argc - 1
        beq     $s0, $zero, done        // if (argc == 0) goto gone
        addi    $s1, $a1, 4             // argv + 4
loop:
        jal     print                   // print string
        lw      $a0, 0($s1)             // arg 1: *argv

        addi    $s0, $s0, -1            // --argc
        la      $a1, eoln               // arg2: newline
        beq     $s0, $zero, last        // if (argc == 0) goto last
        li      $a0, 1                  // arg1: stdout
        la      $a1, space              // arg2: space
last:
        li      $a2, 1                  // arg3: length
        li      $v0, SYS_write          // syscall index: write()
        syscall                         // call the kernel
        nop                             // ignore errors
        nop

        bne     $s0, $zero, loop        // if (argc == 0) goto gone
        addi    $s1, $s1, 4             // ++argv
done:
        li      $a0, 0                  // arg1: exit status
        li      $v0, SYS_exit           // syscall index: exit()
        syscall SYS_                    // call the kernel
        // no return

print:  move    $a1, $a0                // arg2: string

	addi	$a2, $a0, 1             // compute length
strlen:
	lb	$v0, 0($a0)		// get byte from string
	bne	$v0, $zero, strlen	// continue if not end
	addi	$a0, $a0, 1		// increment pointer

	subu	$a2, $a0, $a2		// arg3: length

        li      $a0, 1                  // arg1: stdout
        li      $v0, SYS_write          // syscall index: write()
        syscall                         // call the kernel
        nop                             // ignore errors
        nop

        jr      $ra
        nop
