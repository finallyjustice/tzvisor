#include "types.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "arm.h"

// load a user program for execution
int exec (char *base)
{
    struct elfhdr elf;  // elf file header
    struct proghdr ph;  // elf program header
    
	pde_t *pgdir; 
	char *s;
    char *last;
    int i;
    int off;
    
	uint argc;
    uint sz;    // allocated memory virtual address space util now
    uint sp;
    uint ustack[3 + MAXARG + 1];

	// copy the elf header to secure memory
	memcpy(&elf, base, sizeof(struct elfhdr));

    if (elf.magic != ELF_MAGIC) {
		panic("[TZV] file is not elf!");
    }	

	// alloc memory for page directory
    pgdir = 0;
    if ((pgdir = kpt_alloc()) == 0) {
		panic("[TZV] alloc pgd failed!");
    }

    // Load program into memory.
    sz = 0;
	
    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) 
	{
		// copy the program header to secure memory
		memcpy(&ph, base+off, sizeof(struct proghdr));

        if (ph.type != ELF_PROG_LOAD) {
            continue;
        }

        if (ph.memsz < ph.filesz) {
            panic("[TZV] ph.memsz error!");
        }

		// alloc address space for segment
        if ((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0) {
            panic("[TZV] allocuvm error");
        }

		// load segment into address space
        if (loaduvm(pgdir, (char*) ph.vaddr, base, ph.off, ph.filesz) < 0) {
            panic("[TZV] loaduvm error");
        }
    }

    // Allocate two pages at the next page boundary.
    // Make the first inaccessible.  Use the second as the user stack.
    sz = align_up (sz, PTE_SZ);

    if ((sz = allocuvm(pgdir, sz, sz + 2 * PTE_SZ)) == 0) {
		panic("[TZV] allocuvm error");
    }

    clearpteu(pgdir, (char*) (sz - 2 * PTE_SZ));

    sp = sz;
    
	struct proc *p = allocproc();
	proc = p;

	memset(p->tf, 0, sizeof(*p->tf));

	p->tf->r14_svc = (uint)error_init;
	p->tf->spsr = spsr_usr ();
	p->tf->lr_usr = 0;

	safestrcpy(p->name, "initproc", sizeof(p->name));

	p->state = RUNNABLE;	

    p->pgdir = pgdir;
    p->sz = sz;
    p->tf->pc = elf.entry;
    p->tf->sp_usr = sp;

	cprintf("[TZV] prepare....\n");
	
	switchuvm(p);

	p->state = RUNNING;
	swtch(&cpu->scheduler, proc->context);	

	cprintf("[TZV] job is finished\n");
	// continue the execution of secure world function
	sec_cont();

    return 0;
}
