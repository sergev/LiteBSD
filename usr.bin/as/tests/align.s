	.set	noreorder
	.text

        .p2align 2

        sw $fp, ($sp)
        sw $fp, 4($sp)
foo:
#        .extern foo
