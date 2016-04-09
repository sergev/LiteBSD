        .section .mdebug.abi32
        .previous
        .text
        .p2align 2
        .align  2
        .globl  main
        .ent    main
main:
        subu    $sp, $sp, 16
        sw      $ra, 4($sp)
        sw      $fp, ($sp)
        move    $fp, $sp
        subu    $sp, $sp, 8
$L391:
$L395:
        la      $4, $L397       # load constant address to reg
        subu    $sp, $sp, 16    # call (args, result in v0) to scon/sname
        jal     printf
        nop
        addiu   $sp, $sp, 16
        sw      $2, -4($fp)     # store (u)int/(u)long
        nop
        move    $2, $zero       # load 0 to reg
        sw      $2, -8($fp)     # store (u)int/(u)long
        nop
        j       $L393           # goto label
        nop
        nop
$L393:
        lw      $2, -8($fp)     # load (u)int/(u)long to reg
        nop
        addiu   $sp, $fp, 16
        lw      $ra, -12($sp)
        lw      $fp, -16($sp)
        jr      $ra
        nop
        .end    main
        .size   main, .-main
        .section .rodata
        .p2align 0
$L397:
        .ascii "Hello, C World!\012\0"
