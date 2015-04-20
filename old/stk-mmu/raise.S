	.syntax unified
	.arch_extension sec	
	.arch_extension virt
	.text

# This raise is to handle the 
# compilation error with libgcc.a
.global raise
raise:
	mov pc, lr
