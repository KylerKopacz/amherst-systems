// =================================================================================================================================
/**
 * mmu.c
 */
// =================================================================================================================================



// =================================================================================================================================
// INCLUDES

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "mmu.h"
#include "vmsim.h"
// =================================================================================================================================



// =================================================================================================================================
// MACROS AND GLOBALS

/** The (real) address of the upper page table.  Initialized by a call to `mmu_init()`. */
static vmsim_addr_t upper_pt_addr = 0;
// =================================================================================================================================



// =================================================================================================================================
void mmu_init (vmsim_addr_t new_upper_pt_addr) {

  upper_pt_addr = new_upper_pt_addr;
  
}
// =================================================================================================================================



// =================================================================================================================================
vmsim_addr_t mmu_translate (vmsim_addr_t sim_addr) {
  //TIME SPENT ON PROJECT: 15 mins... kidding more like 7 hours
  
  //print the address that is passed in
  printf("address passed: %d\n", sim_addr);

  //upper page table index calculation
  uint32_t upper_pt_index = (sim_addr >> 22);
  printf("= Upper page table index: %d\n", upper_pt_index);
  

  //lower page table index calculation
  uint32_t lower_pt_index = (sim_addr >> 12) & 0x3ff;
  printf("== Lower page table index: %d\n", lower_pt_index);

  //offset calculation
  uint32_t offset = sim_addr & 0xfff;
  printf("=== Offset (in bytes) into lower page: %d\n", offset);

  // STEP 1 =============================
  // access the upper page table, to get the address to the lower page table
  uint32_t lower_pt_addr;
  vmsim_read_real(&lower_pt_addr, upper_pt_addr + (sizeof(lower_pt_addr) * upper_pt_index), sizeof(lower_pt_addr));

  if(lower_pt_addr == 0) {
    vmsim_map_fault(sim_addr);
    vmsim_read_real(&lower_pt_addr, upper_pt_addr + (sizeof(lower_pt_addr) * upper_pt_index), sizeof(lower_pt_addr));
  }

  //debug for this addresss
  printf("==== Lower page table addresss: %d\n", lower_pt_addr);


  // STEP 2 ==============================
  //now that we have the address to the lower page table, we need to find the address of the right page in that table
  uint32_t page_addr;
  vmsim_read_real(&page_addr, lower_pt_addr + (sizeof(page_addr) * lower_pt_index), sizeof(page_addr));
  //debug
  printf("Space that we are checking for a page address: %d\n", lower_pt_addr + (sizeof(page_addr) * lower_pt_index));

  if(page_addr == 0) {
    vmsim_map_fault(sim_addr);
    vmsim_read_real(&page_addr, lower_pt_addr + (sizeof(page_addr) * lower_pt_index), sizeof(page_addr));
  }

  //debug this address to the page
  printf("===== Address of the page in the lower page table: %d\n", page_addr);

  // STEP 3 =============================
  //now that we have the address to the page, all we need to do is add the offset to this address
  uint32_t final_addr = page_addr | offset;
  
  //and return it
  printf("====== Final address returned: %d\n", final_addr);
  return final_addr;  
  
}
// =================================================================================================================================
