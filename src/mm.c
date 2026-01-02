#include "mm.h"
#include "arm/mmu.h"
#include "peripherals/base.h"
#include "printf.h"
#include "sched.h"

static unsigned short mem_map[PAGING_PAGES] = {
    0,
};

unsigned long allocate_kernel_page() {
  unsigned long page = get_free_page();
  if (page == 0) {
    return 0;
  }
  return page + VA_START;
}

unsigned long allocate_user_page(struct task_struct *task, unsigned long va) {
  unsigned long page = get_free_page();
  if (page == 0) {
    return 0;
  }
  map_page(task, va, page);
  return page + VA_START;
}

unsigned long get_free_page() {
  for (unsigned long i = 0; i < PAGING_PAGES; i++) {
    if (mem_map[i] == 0) {
      mem_map[i] = 1;
      unsigned long page = LOW_MEMORY + i * PAGE_SIZE;
      memzero(page + VA_START, PAGE_SIZE);
      return page;
    }
  }
  return 0;
}

void free_page(unsigned long p) { mem_map[(p - LOW_MEMORY) / PAGE_SIZE] = 0; }

unsigned long map_table(unsigned long *table, unsigned long shift,
                        unsigned long va, int *new_table) {
  unsigned long index = va >> shift;
  index = index & (PTRS_PER_TABLE - 1);
  if (!table[index]) {
    *new_table = 1;
    unsigned long next_level_table = get_free_page();
    unsigned long entry = next_level_table | MM_TYPE_PAGE_TABLE;
    table[index] = entry;
    return next_level_table;
  } else {
    *new_table = 0;
  }
  return table[index] & PAGE_MASK;
}

void map_table_entry(unsigned long *pte, unsigned long va, unsigned long pa) {
  unsigned long index = va >> PAGE_SHIFT;
  index = index & (PTRS_PER_TABLE - 1);
  unsigned long entry = pa | MMU_PTE_FLAGS;
  pte[index] = entry;
}

void map_table_entry_guard(unsigned long *pte, unsigned long va) {
  unsigned long index = va >> PAGE_SHIFT;
  index = index & (PTRS_PER_TABLE - 1);
  // Map to physical address 0 with no user access permissions (AP=0b00)
  unsigned long entry = 0 | MMU_PTE_FLAGS_GUARD;
  pte[index] = entry;
}

void map_page(struct task_struct *task, unsigned long va, unsigned long page) {
  unsigned long pgd;
  if (!task->mm.pgd) {
    task->mm.pgd = get_free_page();
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = task->mm.pgd;
  }
  pgd = task->mm.pgd;
  int new_table;
  unsigned long pud =
      map_table((unsigned long *)(pgd + VA_START), PGD_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = pud;
  }
  unsigned long pmd =
      map_table((unsigned long *)(pud + VA_START), PUD_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = pmd;
  }
  unsigned long pte =
      map_table((unsigned long *)(pmd + VA_START), PMD_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = pte;
  }
  map_table_entry((unsigned long *)(pte + VA_START), va, page);
  struct user_page p = {page, va};
  task->mm.user_pages[task->mm.user_pages_count++] = p;
}

void map_guard_page(struct task_struct *task, unsigned long va) {
  unsigned long pgd;
  if (!task->mm.pgd) {
    task->mm.pgd = get_free_page();
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = task->mm.pgd;
  }
  pgd = task->mm.pgd;
  int new_table;
  unsigned long pud =
      map_table((unsigned long *)(pgd + VA_START), PGD_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = pud;
  }
  unsigned long pmd =
      map_table((unsigned long *)(pud + VA_START), PUD_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = pmd;
  }
  unsigned long pte =
      map_table((unsigned long *)(pmd + VA_START), PMD_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages[++task->mm.kernel_pages_count] = pte;
  }
  map_table_entry_guard((unsigned long *)(pte + VA_START), va);
}

int copy_virt_memory(struct task_struct *dst) {
  struct task_struct *src = current;
  for (int i = 0; i < src->mm.user_pages_count; i++) {
    unsigned long kernel_va =
        allocate_user_page(dst, src->mm.user_pages[i].virt_addr);
    if (kernel_va == 0) {
      return -1;
    }
    // Use physical address + VA_START to access in kernel context, not user
    // virtual address
    unsigned long src_kernel_va = src->mm.user_pages[i].phys_addr + VA_START;
    memcpy(kernel_va, src_kernel_va, PAGE_SIZE);
  }
  return 0;
}

int do_mem_abort(unsigned long addr, unsigned long esr) {
  unsigned long fsc = (esr & 0x3f); // Fault Status Code is bits 5:0

  // Check if this is a translation fault (FSC = 0x04 for level 0, 0x05 for
  // level 1, etc.) or a permission fault (FSC = 0x0c for level 0, 0x0d for
  // level 1, etc.) We handle translation faults (page doesn't exist yet)
  unsigned long fsc_type = fsc & 0x3c; // bits 5:2 indicate fault type

  if (fsc_type == 0x04 || fsc_type == 0x0c) { // Translation or permission fault
    if (current->mm.user_pages_count >= MAX_PROCESS_PAGES) {
      return -1;
    }
    unsigned long page = get_free_page();
    if (page == 0) {
      return -1;
    }
    map_page(current, addr & PAGE_MASK, page);
    return 0;
  }
  return -1;
}
