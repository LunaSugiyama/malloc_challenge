//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
  struct my_metadata_t *prev;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head;
  my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t my_heap;

//
// Helper functions (feel free to add/remove/edit!)
//

void my_add_to_free_list(my_metadata_t *metadata) {
  assert(!metadata->next && !metadata->prev);
  metadata->next = my_heap.free_head;
  if (my_heap.free_head) {
    my_heap.free_head->prev = metadata;
  }
  my_heap.free_head = metadata;
  metadata->prev = NULL;
}

void my_remove_from_free_list(my_metadata_t *metadata) {
  if (metadata->prev) {
    metadata->prev->next = metadata->next;
  } else {
    my_heap.free_head = metadata->next;
  }
  if (metadata->next) {
    metadata->next->prev = metadata->prev;
  }
  metadata->next = metadata->prev = NULL;
}

void my_coalesce(my_metadata_t *metadata) {
  // Coalesce with next block if possible
  my_metadata_t *next_metadata = (my_metadata_t *)((char *)metadata + metadata->size + sizeof(my_metadata_t));
  // Ensure next_metadata is within the valid range and is part of the free list
  my_metadata_t *free_head = my_heap.free_head;
  bool found = false;

  while (free_head) {
    if (free_head == next_metadata) {
      found = true;
      break;
    }
    free_head = free_head->next;
  }

  if (found) {
    my_remove_from_free_list(next_metadata);
    metadata->size += next_metadata->size + sizeof(my_metadata_t);
  }

// // Coalesce with previous block if possible

  metadata->next = NULL;
  metadata->prev = NULL;
  my_add_to_free_list(metadata);
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize() {
  my_heap.free_head = &my_heap.dummy;
  my_heap.dummy.size = 0;
  my_heap.dummy.next = NULL;
  my_heap.dummy.prev = NULL;
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {
  my_metadata_t *metadata = my_heap.free_head;
  my_metadata_t *best_fit = NULL;

  // First-fit: Find the first free slot the object fits.
  // TODO: Update this logic to Best-fit!
  while (metadata) {
    if (metadata->size >= size && (!best_fit || metadata->size < best_fit->size)) {
      best_fit = metadata;
    }
    metadata = metadata->next;
  }
  // now, metadata points to the first free slot
  // and prev is the previous entry.

  metadata = best_fit;

  if (!metadata) {
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t);
    metadata->next = NULL;
    metadata->prev = NULL;
    // Add the memory region to the free list.
    // my_add_to_free_list(metadata);
    my_coalesce(metadata);
    // Now, try my_malloc() again. This should succeed.
    return my_malloc(size);
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = metadata + 1;
  size_t remaining_size = metadata->size - size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(metadata);

  if (remaining_size > sizeof(my_metadata_t)) {
    // Shrink the metadata for the allocated object
    // to separate the rest of the region corresponding to remaining_size.
    // If the remaining_size is not large enough to make a new metadata,
    // this code path will not be taken and the region will be managed
    // as a part of the allocated object.
    metadata->size = size;
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    new_metadata->prev = NULL;
    // Add the remaining free slot to the free list.
    // my_add_to_free_list(new_metadata);
    my_coalesce(new_metadata);
  }
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  metadata->next = NULL;
  metadata->prev = NULL;
  // my_add_to_free_list(metadata);
  my_coalesce(metadata);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
