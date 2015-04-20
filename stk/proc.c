#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "proc.h"

//
// Process initialization:
// process initialize is somewhat tricky.
//  1. We need to fake the kernel stack of a new process as if the process
//     has been interrupt (a trapframe on the stack), this would allow us
//     to "return" to the correct user instruction.
//  2. We also need to fake the kernel execution for this new process. When
//     swtch switches to this (new) process, it will switch to its stack,
//     and reload registers with the saved context. We use forkret as the
//     return address (in lr register). (In x86, it will be the return address
//     pushed on the stack by the process.)
//
// The design of context switch in xv6 is interesting: after initialization,
// each CPU executes in the scheduler() function. The context switch is not
// between two processes, but instead, between the scheduler. Think of scheduler
// as the idle process.
//
struct {
	struct proc proc[NPROC];
} ptable;

static struct proc *initproc;
struct proc *proc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
//static struct proc* allocproc(void)
struct proc* allocproc(void)
{
    struct proc *p;
    char *sp;

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->state == UNUSED) {
            goto found;
        }

    }

    return 0;

    found:
    p->state = EMBRYO;
    p->pid = nextpid++;

    // Allocate kernel stack.
    if((p->kstack = alloc_page ()) == 0){
        p->state = UNUSED;
        return 0;
    }

    sp = p->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof (*p->tf);
    p->tf = (struct trapframe*)sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint*)sp = (uint)trapret;

    sp -= 4;
    *(uint*)sp = (uint)p->kstack + KSTACKSIZE;

    sp -= sizeof (*p->context);
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof(*p->context));

    // skip the push {fp, lr} instruction in the prologue of forkret.
    // This is different from x86, in which the harderware pushes return
    // address before executing the callee. In ARM, return address is
    // loaded into the lr register, and push to the stack by the callee
    // (if and when necessary). We need to skip that instruction and let
    // it use our implementation.
    p->context->lr = (uint)forkret+4;

    return p;
}

void error_init ()
{
	panic ("failed to craft first process\n");
}

//PAGEBREAK: 32
// hand-craft the first user process. We link initcode.S into the kernel
// as a binary, the linker will generate __binary_initcode_start/_size
void userinit(void)
{
    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    p = allocproc();
    initproc = p;

    if((p->pgdir = kpt_alloc()) == NULL) {
        panic("userinit: out of memory?");
    }


	extern char *param_base;
	char *gbin = kmalloc(PTE_SHIFT);
	memcpy(gbin, param_base, 72);
    inituvm(p->pgdir, gbin, 72);
    //inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
	cprintf("init_code_size: %d\n", _binary_initcode_size);
    p->sz = PTE_SZ;

    // craft the trapframe as if
    memset(p->tf, 0, sizeof(*p->tf));

    p->tf->r14_svc = (uint)error_init;
    p->tf->spsr = spsr_usr ();
    p->tf->sp_usr = PTE_SZ;	// set the user stack
    p->tf->lr_usr = 0;

    // set the user pc. The actual pc loaded into r15_usr is in
    // p->tf, the trapframe.
    p->tf->pc = 0;					// beginning of initcode.S

	cprintf("p->tf should be 0x%x\n", p->tf);

    safestrcpy(p->name, "initcode", sizeof(p->name));
    ///////////////p->cwd = namei("/");

    p->state = RUNNABLE;
}

/*// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n)
{
    uint sz;

    sz = proc->sz;

    if(n > 0){
        if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0) {
            return -1;
        }

    } else if(n < 0){
        if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0) {
            return -1;
        }
    }

    proc->sz = sz;
    switchuvm(proc);

    return 0;
}*/

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void)
{
	cprintf("This is scheduler!\n");
    struct proc *p;

    for(;;){
        // Enable interrupts on this processor.
        sti();

        // Loop over process table looking for process to run.
        //acquire(&ptable.lock);

        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->state != RUNNABLE) {
                continue;
            }

			cprintf("find a task!\n");

            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.
            proc = p;
            switchuvm(p);

            p->state = RUNNING;
            swtch(&cpu->scheduler, proc->context);
            // Process is done running for now.
            // It should have changed its p->state before coming back.
            proc = 0;
        }

        //release(&ptable.lock);
    }
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void)
{
	static int first = 1;

	// Still holding ptable.lock from scheduler.
	//release(&ptable.lock);

	if (first) {
		// Some initialization functions must be run in the context
		// of a regular process (e.g., they call sleep), and thus cannot
		// be run from main().
		first = 0;
		//initlog();
	}

	// Return to "caller", actually trapret (see allocproc).
}

