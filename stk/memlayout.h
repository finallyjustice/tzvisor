#ifndef MEMLAYOUT_INCLUDE
#define MEMLAYOUT_INCLUDE

// Memory layout

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE  		0x40000000         // First kernel virtual address 
						// P:0x80000000  (PHY_START) => V:0xc0000000
// HCLIN added for RAM's PA starts from non-zero 
// use this cmd to check and add PHY_START : grep P2V * -R
//#define PHY_START		0x80000000
#define PHY_START		0xf0000000

// #define EXTMEM	  		0x20000
// #define KERNLINK  		(KERNBASE+EXTMEM)  // Address where kernel is linked

// we first map 1MB low memory containing kernel code.
#define INIT_KERN_SZ	0x100000
#define INIT_KERNMAP 	(INIT_KERN_SZ + PHY_START)
// 1MB = 0x10 0000
// 0x100000

#ifndef __ASSEMBLER__

static inline uint v2p_dram(void *a) { return ((uint) (a)); }
static inline void *p2v_dram(uint a) { return (void *) ((a)); }

static inline uint v2p_dev(void *a) { return ((uint) (a))  - KERNBASE; }
static inline void *p2v_dev(uint a) { return (void *) ((a) + KERNBASE); }


#endif

#define V2P_DRAM(a) (((uint) (a)))
#define P2V_DRAM(a) (((void *) (a)))

#define V2P_DEV(a) (((uint) (a)) - KERNBASE)
#define P2V_DEV(a) (((void *) (a)) + KERNBASE)


#define V2P_WO_DRAM(x) ((x))    // same as V2P, but without casts
#define P2V_WO_DRAM(x) ((x))    // same as V2P, but without casts

#define V2P_WO_DEV(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO_DEV(x) ((x) + KERNBASE)    // same as V2P, but without casts


#endif
