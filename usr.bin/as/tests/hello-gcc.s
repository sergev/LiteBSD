    .set    reorder
    .globl  main
    .text
    .text
    .align  2
    .ent    main
main:
    .frame  $sp,32,$31
    addu    $sp,$sp,-32
    .mask   0x80000000,-16
    sw      $31,16($sp)
    la      $4,L.2
    jal     printf
    move    $2,$0
L.1:
    lw      $31,16($sp)
    addu    $sp,$sp,32
    j       $31
    .end    main

    .rdata
    .align  0
L.2:
    .byte   72
    .byte   101
    .byte   108
    .byte   108
    .byte   111
    .byte   44
    .byte   32
    .byte   67
    .byte   32
    .byte   87
    .byte   111
    .byte   114
    .byte   108
    .byte   100
    .byte   33
    .byte   10
    .byte   0
