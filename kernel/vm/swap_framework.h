#ifndef __VM_SWAP_FRAMEWORK_H
#define __VM_SWAP_FRAMEWORK_H

#include "vm.h"
#include "memory.h"
#include "klibc.h"
#include <stdint.h>
#include "swap_pqueue.h"
#include "swap_fs.h"

// Contributors: Noel Negusse and Jesse Thaden
/* Function: swap_framework
 * Purpose: To present an API for swapping pages to various swap spaces
 * so that pages may be stored independently of other related operations
 *
 * EXAMPLE OF VIRTUAL MEMORY ID/ADDRESS FROM SWAP SPACE
 *	_____________________________________________________________
 *	|| [24 bit] SWAP SPACE ID | [8 bit] SWAP SPACE SPECIFIC ID ||
 *	¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 *	Total: 32 Bit Address
 *	therefore, there are 256 possible swap spaces possible
 *	w/ 2^12 page entries each (Assuming 4kB pages)
 */

#define SWAP_SPACES (1<<8)
#define PAGE_ENTRIES (1<<12) // Assuming 4kB Pages right now
#define PAGE_SIZE (1<<12) // May carry an if statement later...

struct swap_space {
	struct swap_entry *e_head;
        uint16_t pages_free;
	uint8_t lower_bits; // swap space ID [8-bits]
	uint16_t flags; // SWP_USED (1000 or 1), SWP_WRITEOK (0010 or 2) OR BOTH (0011 or 3)
	uint8_t priority; // lower is better 
        uint32_t *store_func;
        uint32_t *retrieve_func;
}; // Total: 18 bytes


struct swap_entry {
	struct swap_entry *next;
	uint32_t higher_bits; // swap entry ID [24-bit assuming 4kB pages]
	uint16_t e_flags; // ENT_USED (1000 or 1), ENT_WRITEOK (0100 or 2) OR BOTH (1100 or 3)
        uint8_t free; //0 - used, 1 - free
        void *page; // virtual address pointer used for resolving page faults
}; // Total: 15 bytes 

struct swap_space *front;
static os_size_t memory_count;


// Swap_init initializes swap framework
void swap_init();
//void swap_init(os_size_t);  To be implemented... [specifies global page size]


/* store_page will store a page to media - not main memory - storage, e.g. HDD
 * from the page* and will change uint32_t *ID -> value to the swap space address
 *	VARIABLES
 * void* page -> data to be paged
 * os_size_t pagesize: The size of a page in bytes, i.e. 4096 b
 * uint32_t* ID -> The page ID or address [not yet implemented]
 * Returns: The stored page's (24-bit) swap space ID/Address 
 */
uint32_t *store_page(void*, uint32_t*);
uint32_t *store_pageLZ(void*, uint32_t*); // Waiting on file systems team to complete
// uint32_t *store_page(void*, os_size_t, uint32_t*); To be implemented...

/* retrieve_page will retrieve the page identified the ID pointer and
 * will store it back into main memory (specified by the void *page pointer)
 * Returns: The retrieved page's (32-bit) new main memory ID/Address (like above example)
 * NOTE: Page size was set by store_page 
 */
uint32_t retrieve_page(void*, uint32_t*);
uint32_t retrieve_pageLZ(void*, uint32_t*); // Waiting on file systems team to complete


// Returns: The total stored memory in bytes by function store_page(void*, uint32_t*)
os_size_t sum_stored();


/* vm_swap_page will store a page from main memory (specified by void *page) 
 * and an auxillary swap spaces (specified by uint32_t *ID)
 * Returns: returns the new ID of where the swap_space was stored and changes the ID pointer 
 */
uint32_t *vm_swap_page(void*, uint32_t*);
uint32_t *vm_unswap_page(void*, uint32_t);


/* vm_register/vm_deregister will activate/deactivate a swap space and set a priority
 * accordingly to it. This allows the swap spaces use as well as increasing it's efficiency
 * NOTE: deregister_swap_space will always deregister the lowest priority swap space first 
 * Returns: 0 or 1 whether the swap space registeration was a failure or success, respectively
 */
uint8_t vm_register_swap_space(uint32_t (*)(void*, uint32_t*), uint32_t (*)
		(void*, uint32_t*), int priority);
uint8_t vm_deregister_swap_space();


/* Resolves the page fault and updates the corresponding page table
 * Returns: The new resolved physical address of the page fault
 * NOTE: May return a 0 if the page_fault could not be resolved
 */
uint32_t vm_page_fault(void*);

/* vm_scan_pages will scan through all pages for the corresponding page
 * to the given page
 * Returns: The address of matching swapped page 
 */
uint32_t *vm_scan_pages(void*);

#endif