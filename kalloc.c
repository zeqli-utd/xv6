// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define MAXPAGES (PHYSTOP / PGSIZE)

void freerange(void *vstart, void *vend);
void _freerange(void *vstart, void *vend);
void _kfree(char *v);
extern char end[]; // first address after kernel loaded from ELF file

struct run {
  int ref_count;
  struct run *next;
};


struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  // DEP: For COW fork, we can't store the run in the 
  //      physical page, because we need space for the ref
  //      count.  Move to the kmem struct.
  struct run runs[MAXPAGES];
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  _freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  _freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
_freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    _kfree(p);
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}


void
_kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->next = kmem.freelist;
  kmem.freelist = r;
    
  if(r->ref_count != 1)
    panic("kfree ref count err");// Assert the count is one when a page is freed.
  
  if(kmem.use_lock)
    release(&kmem.lock);
}

int
krefc(char *v){
  if(kmem.use_lock)
    acquire(&kmem.lock);
  struct run *r = &kmem.runs[(V2P(v) / PGSIZE)];
  if (kmem.use_lock)
    release(&kmem.lock);
  return r->ref_count;
}

// Increase reference count by 1
void
kincrefc(char *v)
{
  if(kmem.use_lock)
    acquire(&kmem.lock);
  struct run *r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->ref_count++;
  if (kmem.use_lock)
    release(&kmem.lock);
}

// Decrese reference count by 1
void
kdecrefc(char *v)
{
  if(kmem.use_lock)
    acquire(&kmem.lock);
  struct run *r = &kmem.runs[(V2P(v) / PGSIZE)];
  r->ref_count--;
  if (kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  char *rv;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
  {
    r->ref_count = 1; // set the count to 1 when a page is allocated
    kmem.freelist = r->next;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  rv = r ? P2V((r - kmem.runs) * PGSIZE) : r;       
  return rv;
}

