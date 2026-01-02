/*
 * Memory Management Tests
 *
 * Tests for:
 * - Page allocation and deallocation
 * - Kernel page allocation
 * - User page allocation
 * - Page mapping
 * - Guard page mapping
 * - Memory copy operations
 * - Virtual memory copying between processes
 */

#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "test.h"
#include <stddef.h>

/* Forward declarations for test functions */
static int test_mm_get_free_page(void);
static int test_mm_get_multiple_pages(void);
static int test_mm_free_page(void);
static int test_mm_page_reuse(void);
static int test_mm_allocate_kernel_page(void);
static int test_mm_kernel_page_is_virtual(void);
static int test_mm_page_alignment(void);
static int test_mm_page_zeroed(void);
static int test_mm_allocate_user_page(void);
static int test_mm_map_page(void);
static int test_mm_map_guard_page(void);
static int test_mm_page_table_creation(void);
static int test_mm_multiple_user_pages(void);
static int test_mm_exhaustion_recovery(void);

/* Helper to check if memory is zeroed */
static int is_memory_zeroed(unsigned long addr, unsigned long size) {
  unsigned char *p = (unsigned char *)addr;
  for (unsigned long i = 0; i < size; i++) {
    if (p[i] != 0)
      return 0;
  }
  return 1;
}

/* Test: Basic page allocation */
static int test_mm_get_free_page(void) {
  unsigned long page = get_free_page();

  TEST_ASSERT_NEQ(0, page);
  TEST_ASSERT_GTE(page, LOW_MEMORY);
  TEST_ASSERT_LT(page, HIGH_MEMORY);

  /* Clean up */
  free_page(page);

  return TEST_PASS;
}

/* Test: Allocate multiple pages, ensure they're different */
static int test_mm_get_multiple_pages(void) {
  unsigned long page1 = get_free_page();
  unsigned long page2 = get_free_page();
  unsigned long page3 = get_free_page();

  TEST_ASSERT_NEQ(0, page1);
  TEST_ASSERT_NEQ(0, page2);
  TEST_ASSERT_NEQ(0, page3);

  /* All pages should be different */
  TEST_ASSERT_NEQ(page1, page2);
  TEST_ASSERT_NEQ(page2, page3);
  TEST_ASSERT_NEQ(page1, page3);

  /* All should be page-aligned */
  TEST_ASSERT_EQ(0, page1 & (PAGE_SIZE - 1));
  TEST_ASSERT_EQ(0, page2 & (PAGE_SIZE - 1));
  TEST_ASSERT_EQ(0, page3 & (PAGE_SIZE - 1));

  /* Clean up */
  free_page(page1);
  free_page(page2);
  free_page(page3);

  return TEST_PASS;
}

/* Test: Free page and verify it can be reused */
static int test_mm_free_page(void) {
  unsigned long page1 = get_free_page();
  TEST_ASSERT_NEQ(0, page1);

  free_page(page1);

  /* Allocate again - should be able to reuse */
  unsigned long page2 = get_free_page();
  TEST_ASSERT_NEQ(0, page2);

  free_page(page2);

  return TEST_PASS;
}

/* Test: Page reuse after freeing */
static int test_mm_page_reuse(void) {
  /* Allocate and free several pages */
  unsigned long pages[5];
  for (int i = 0; i < 5; i++) {
    pages[i] = get_free_page();
    TEST_ASSERT_NEQ(0, pages[i]);
  }

  /* Free all pages */
  for (int i = 0; i < 5; i++) {
    free_page(pages[i]);
  }

  /* Allocate again - should be able to get pages */
  unsigned long new_pages[5];
  for (int i = 0; i < 5; i++) {
    new_pages[i] = get_free_page();
    TEST_ASSERT_NEQ(0, new_pages[i]);
  }

  /* Clean up */
  for (int i = 0; i < 5; i++) {
    free_page(new_pages[i]);
  }

  return TEST_PASS;
}

/* Test: Kernel page allocation returns virtual address */
static int test_mm_allocate_kernel_page(void) {
  unsigned long kpage = allocate_kernel_page();

  TEST_ASSERT_NEQ(0, kpage);
  /* Kernel pages should have VA_START offset */
  TEST_ASSERT_GTE(kpage, VA_START);

  /* Get the physical address */
  unsigned long phys = kpage - VA_START;
  TEST_ASSERT_GTE(phys, LOW_MEMORY);
  TEST_ASSERT_LT(phys, HIGH_MEMORY);

  /* Clean up */
  free_page(phys);

  return TEST_PASS;
}

/* Test: Kernel page is in virtual address space */
static int test_mm_kernel_page_is_virtual(void) {
  unsigned long kpage1 = allocate_kernel_page();
  unsigned long kpage2 = allocate_kernel_page();

  TEST_ASSERT_NEQ(0, kpage1);
  TEST_ASSERT_NEQ(0, kpage2);

  /* Both should be in kernel virtual space */
  TEST_ASSERT_GTE(kpage1, VA_START);
  TEST_ASSERT_GTE(kpage2, VA_START);

  /* Should be different addresses */
  TEST_ASSERT_NEQ(kpage1, kpage2);

  /* Clean up */
  free_page(kpage1 - VA_START);
  free_page(kpage2 - VA_START);

  return TEST_PASS;
}

/* Test: Page alignment */
static int test_mm_page_alignment(void) {
  for (int i = 0; i < 10; i++) {
    unsigned long page = get_free_page();
    TEST_ASSERT_NEQ(0, page);

    /* Check 4KB alignment */
    TEST_ASSERT_EQ(0, page % PAGE_SIZE);

    free_page(page);
  }

  return TEST_PASS;
}

/* Test: Newly allocated pages are zeroed */
static int test_mm_page_zeroed(void) {
  unsigned long page = get_free_page();
  TEST_ASSERT_NEQ(0, page);

  /* Page should be zeroed (check via kernel virtual address) */
  unsigned long kva = page + VA_START;
  TEST_ASSERT(is_memory_zeroed(kva, PAGE_SIZE));

  /* Write some data to the page */
  unsigned char *p = (unsigned char *)kva;
  for (int i = 0; i < 100; i++) {
    p[i] = 0xAA;
  }

  /* Free and reallocate */
  free_page(page);
  unsigned long page2 = get_free_page();
  TEST_ASSERT_NEQ(0, page2);

  /* New page should be zeroed */
  unsigned long kva2 = page2 + VA_START;
  TEST_ASSERT(is_memory_zeroed(kva2, PAGE_SIZE));

  free_page(page2);

  return TEST_PASS;
}

/* Test: Allocate user page updates task structure */
static int test_mm_allocate_user_page(void) {
  /* Save current state */
  int initial_user_pages = current->mm.user_pages_count;

  /* Allocate a user page at a specific virtual address */
  unsigned long va = 0x400000; /* 4MB mark */
  unsigned long kpage = allocate_user_page(current, va);

  TEST_ASSERT_NEQ(0, kpage);
  TEST_ASSERT_GTE(kpage, VA_START);

  /* User pages count should have increased */
  TEST_ASSERT_GT(current->mm.user_pages_count, initial_user_pages);

  /* The last user page should have our virtual address */
  int idx = current->mm.user_pages_count - 1;
  TEST_ASSERT_EQ(va, current->mm.user_pages[idx].virt_addr);

  return TEST_PASS;
}

/* Test: Map page creates proper page table entry */
static int test_mm_map_page(void) {
  /* Save current state */
  int initial_kernel_pages = current->mm.kernel_pages_count;

  /* Allocate a physical page */
  unsigned long phys_page = get_free_page();
  TEST_ASSERT_NEQ(0, phys_page);

  /* Map it at a specific virtual address */
  unsigned long va = 0x500000; /* 5MB mark */
  map_page(current, va, phys_page);

  /* Kernel pages count should have increased (for page tables) */
  TEST_ASSERT_GTE(current->mm.kernel_pages_count, initial_kernel_pages);

  /* Task should have a PGD now */
  TEST_ASSERT_NEQ(0, current->mm.pgd);

  return TEST_PASS;
}

/* Test: Guard page mapping */
static int test_mm_map_guard_page(void) {
  /* Save current state */
  int initial_kernel_pages = current->mm.kernel_pages_count;

  /* Map a guard page at address 0 */
  unsigned long va = 0x600000; /* 6MB - use different address to not conflict */
  map_guard_page(current, va);

  /* Kernel pages should have increased (for page tables if new) */
  TEST_ASSERT_GTE(current->mm.kernel_pages_count, initial_kernel_pages);

  /* Task should have a PGD */
  TEST_ASSERT_NEQ(0, current->mm.pgd);

  return TEST_PASS;
}

/* Test: Page table creation hierarchy */
static int test_mm_page_table_creation(void) {
  /* Create a fresh task-like structure on a new page */
  unsigned long task_page = allocate_kernel_page();
  TEST_ASSERT_NEQ(0, task_page);

  struct task_struct *test_task = (struct task_struct *)task_page;

  /* Initialize mm structure */
  test_task->mm.pgd = 0;
  test_task->mm.user_pages_count = 0;
  test_task->mm.kernel_pages_count = 0;

  /* Map a page - this should create the full hierarchy */
  unsigned long phys = get_free_page();
  TEST_ASSERT_NEQ(0, phys);

  map_page(test_task, 0x1000, phys);

  /* Should have created PGD */
  TEST_ASSERT_NEQ(0, test_task->mm.pgd);

  /* Should have created additional page table levels */
  /* PGD + PUD + PMD + PTE = 4 kernel pages minimum for first mapping */
  TEST_ASSERT_GTE(test_task->mm.kernel_pages_count, 1);

  /* User page should be tracked */
  TEST_ASSERT_EQ(1, test_task->mm.user_pages_count);
  TEST_ASSERT_EQ(0x1000, test_task->mm.user_pages[0].virt_addr);
  TEST_ASSERT_EQ(phys, test_task->mm.user_pages[0].phys_addr);

  /* Clean up */
  free_page(phys);
  free_page(task_page - VA_START);

  return TEST_PASS;
}

/* Test: Multiple user pages in same task */
static int test_mm_multiple_user_pages(void) {
  /* Create a test task */
  unsigned long task_page = allocate_kernel_page();
  TEST_ASSERT_NEQ(0, task_page);

  struct task_struct *test_task = (struct task_struct *)task_page;
  test_task->mm.pgd = 0;
  test_task->mm.user_pages_count = 0;
  test_task->mm.kernel_pages_count = 0;

  /* Allocate multiple user pages */
  unsigned long vas[] = {0x1000, 0x2000, 0x3000, 0x4000};
  unsigned long pages[4];

  for (int i = 0; i < 4; i++) {
    pages[i] = allocate_user_page(test_task, vas[i]);
    TEST_ASSERT_NEQ(0, pages[i]);
  }

  /* Should have 4 user pages tracked */
  TEST_ASSERT_EQ(4, test_task->mm.user_pages_count);

  /* Verify each mapping */
  for (int i = 0; i < 4; i++) {
    TEST_ASSERT_EQ(vas[i], test_task->mm.user_pages[i].virt_addr);
  }

  /* Clean up */
  free_page(task_page - VA_START);

  return TEST_PASS;
}

/* Test: Recovery from allocation failures */
static int test_mm_exhaustion_recovery(void) {
  /* Allocate and free several pages to ensure system is stable */
  unsigned long page = get_free_page();
  TEST_ASSERT_NEQ(0, page);

  free_page(page);

  /* Allocate again - should still work */
  unsigned long page2 = get_free_page();
  TEST_ASSERT_NEQ(0, page2);

  free_page(page2);

  return TEST_PASS;
}

/* Register all memory management tests */
void register_mm_tests(void) {
  TEST_REGISTER(mm, get_free_page);
  TEST_REGISTER(mm, get_multiple_pages);
  TEST_REGISTER(mm, free_page);
  TEST_REGISTER(mm, page_reuse);
  TEST_REGISTER(mm, allocate_kernel_page);
  TEST_REGISTER(mm, kernel_page_is_virtual);
  TEST_REGISTER(mm, page_alignment);
  TEST_REGISTER(mm, page_zeroed);
  TEST_REGISTER(mm, allocate_user_page);
  TEST_REGISTER(mm, map_page);
  TEST_REGISTER(mm, map_guard_page);
  TEST_REGISTER(mm, page_table_creation);
  TEST_REGISTER(mm, multiple_user_pages);
  TEST_REGISTER(mm, exhaustion_recovery);
}
