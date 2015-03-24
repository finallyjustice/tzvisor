#include "param.h"
#include "types.h"
#include "defs.h"
#include "arm.h"
#include "memlayout.h"
#include "mmu.h"
#include "elf.h"
#include "proc.h"

extern char data[];  // defined by kernel.ld
pde_t *kpgdir;  // for use in scheduler()

// Xv6 can only allocate memory in 4KB blocks. This is fine
// for x86. ARM's page table and page directory (for 28-bit
// user address) have a size of 1KB. kpt_alloc/free is used
// as a wrapper to support allocating page tables during boot
// (use the initial kernel map, and during runtime, use buddy
// memory allocator. 
struct run {
	struct run *next;
};

struct {
	struct run *freelist;
} kpt_mem;

void init_vmm (void)
{
    kpt_mem.freelist = NULL;
}

static void _kpt_free (char *v)
{
    struct run *r;

    r = (struct run*) v;
    r->next = kpt_mem.freelist;
    kpt_mem.freelist = r;
}


static void kpt_free (char *v)
{
    if (v >= (char*)INIT_KERNMAP) {
		cprintf("panic!");	
		while(1);
        return;
    }
    
    _kpt_free (v);
}

// add some memory used for page tables (initialization code)
void kpt_freerange (uint32 low, uint32 hi)
{
    while (low < hi) {
        _kpt_free ((char*)low);
        low += PT_SZ;
    }
}

void* kpt_alloc (void)
{
    struct run *r;
    
    if ((r = kpt_mem.freelist) != NULL ) {
        kpt_mem.freelist = r->next;
    }

    // Allocate a PT page if no inital pages is available
    //if ((r == NULL) && ((r = kmalloc (PT_ORDER)) == NULL)) {
    //    panic("oom: kpt_alloc");
    //}

    memset(r, 0, PT_SZ);
    return (char*) r;
}

// Return the address of the PTE in page directory that corresponds to
// virtual address va.  If alloc!=0, create any required page table pages.
static pte_t* walkpgdir (pde_t *pgdir, const void *va, int alloc)
{
    pde_t *pde;
    pte_t *pgtab;

    // pgdir points to the page directory, get the page direcotry entry (pde)
    pde = &pgdir[PDE_IDX(va)];

    if (*pde & PE_TYPES) {
        pgtab = (pte_t*) p2v(PT_ADDR(*pde));

    } else {
        if (!alloc || (pgtab = (pte_t*) kpt_alloc()) == 0) {
            return 0;
        }

        // Make sure all those PTE_P bits are zero.
        memset(pgtab, 0, PT_SZ);

        // The permissions here are overly generous, but they can
        // be further restricted by the permissions in the page table
        // entries, if necessary.
        *pde = v2p(pgtab) | UPDE_TYPE;
    }

    return &pgtab[PTE_IDX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int mappages (pde_t *pgdir, void *va, uint size, uint pa, int ap)
{
    char *a, *last;
    pte_t *pte;

    a = (char*) align_dn(va, PTE_SZ);
    last = (char*) align_dn((uint)va + size - 1, PTE_SZ);

    for (;;) {
        if ((pte = walkpgdir(pgdir, a, 1)) == 0) {
            return -1;
        }

        if (*pte & PE_TYPES) {
            panic("remap");
        }

        *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;

        if (a == last) {
            break;
        }

        a += PTE_SZ;
        pa += PTE_SZ;
    }

    return 0;
}

// flush all TLB
static void flush_tlb (void)
{
    uint val = 0;
    asm("MCR p15, 0, %[r], c8, c7, 0" : :[r]"r" (val):);

    // invalid entire data and instruction cache
    asm ("MCR p15,0,%[r],c7,c10,0": :[r]"r" (val):);
    asm ("MCR p15,0,%[r],c7,c11,0": :[r]"r" (val):);
}

// Switch to the user page table (TTBR0)
void switchuvm (struct proc *p)
{
    uint val;

    pushcli();

    if (p->pgdir == 0) {
        panic("switchuvm: no pgdir");
    }

    val = (uint) V2P(p->pgdir) | 0x00;

    asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);
    flush_tlb();

    popcli();
}

// Load the initcode into address 0 of pgdir. sz must be less than a page.
void inituvm (pde_t *pgdir, char *init, uint sz)
{
    char *mem;

    if (sz >= PTE_SZ) {
        panic("inituvm: more than a page");
    }

    mem = alloc_page();
    memset(mem, 0, PTE_SZ);
    mappages(pgdir, 0, PTE_SZ, v2p(mem), AP_KU);
    memmove(mem, init, sz);
}
