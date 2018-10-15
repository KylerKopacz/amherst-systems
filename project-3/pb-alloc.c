// =================================================================================================================================
/**
 * pb-alloc.c
 *
 * A pointer-bumping, non-reclaiming memory allocator.
 **/
// =================================================================================================================================



// =================================================================================================================================
// INCLUDES

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
// =================================================================================================================================



// =================================================================================================================================
// CONSTANTS AND MACRO FUNCTIONS.

/** The system's page size. */
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

/** Macros to easily calculate the number of bytes for larger scales (e.g., kilo, mega, gigabytes). */
#define KB(size)  ((size_t)size * 1024)
#define MB(size)  (KB(size) * 1024)
#define GB(size)  (MB(size) * 1024)

/** The virtual address space reserved for the heap. */
#define HEAP_SIZE GB(2)
// =================================================================================================================================



// =================================================================================================================================
// GLOBALS

/** The current beginning of free heap space. */
static void*    free_ptr  = NULL;

/** The beginning of the heap. */
static intptr_t start_ptr;

/** The end of the heap. */
static intptr_t end_ptr;

// The header structure that will be at the head of each memory block
typedef struct header {
  size_t size; //size of the block
  unsigned char allocated; //byte to show if the block is free or not
  struct header* next; //pointer to the next header in the list
} header_t; //this header structure is 17 bytes in size
// =================================================================================================================================



// =================================================================================================================================
/**
 * The initialization method.  If this is the first use of the heap, initialize it.
 */

void init () {

  // Only do anything if the heap has not yet been used.
  if (free_ptr == NULL) {

    // Allocate virtual address space in which the heap will reside; free space will be allocated from where it starts.  Make it
    // un-shared and not backed by any file.  A failure to map this space is fatal.
    free_ptr = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (free_ptr == MAP_FAILED) {
      write(STDOUT_FILENO, "mmap failed!\n", 13);
      exit(1);
    }

    // Hold onto the boundaries of the heap as a whole.
    start_ptr = (intptr_t)free_ptr;
    end_ptr   = start_ptr + HEAP_SIZE;

    // DEBUG: Emit a message to indicate that this allocator is being called.
    write(STDOUT_FILENO, "pb!\n", 4);
    fsync(STDOUT_FILENO);

  }

} // init ()
// =================================================================================================================================



// =================================================================================================================================
/**
 * Allocate and return `size` bytes of heap space.
 *
 * \param size The number of bytes to allocate.
 * \return A pointer to the allocated block, if successful; `NULL` if unsuccessful.
 */
void* malloc (size_t size) {

  init();

  // YOUR CODE STARTS HERE Write a working, pointer-bumping allocator.  Be sure that you understand what has been set up in init().
  // Also note that calloc()/realloc() are fully written and depend on malloc(), so you need not touch them.
  
  //we need to make sure the passed in size is double-word (16 byte) aligned. It's a good idea to add the size of the header
  //to the initial passed in size
  size += 17;

  //now we can run calculations to make sure this is double word aligned
  int remainder = size % 16;
  if(size % 16 != 0) {
    size += (16 - remainder);
  }

  //now that the memory is double word aligned, we need to find the next free space of memory
  if((intptr_t) free_ptr == start_ptr) {//this is the beginning of memory allocation
    //create a new structure
    header_t block_header;
    block_header = (header_t){.size = size, .allocated = 1, .next = NULL};

    //place that header at the start of the heap
    header_t* header_pointer = (header_t*) free_ptr;
    *header_pointer = block_header;

    //now go to the memory location right after the header
    free_ptr = (char*) free_ptr + sizeof(header_t);

    //store that address
    void* last_free = free_ptr;
    
    //increment the free pointer to go to the start of the next free space
    free_ptr = (char*)free_ptr - sizeof(header_t);
    free_ptr = ((char*) free_ptr + block_header.size + 1);
    
    //now return the location of the start of that memory block
    return last_free;
    
  } else { //this is not the first allocation, so we need to check the free list to find a fit

    //create a variable that will hold a pointer to the previous header in the list
    header_t* previous = NULL;

    //pointer to the current header
    header_t* current = (header_t*) start_ptr;

    while(current != NULL) {
      //see if the current header is not allocated and is >= the requested size
      if(current->allocated == 0 && current->size >= size) {//this is the block that we need
	//set allocatioin status of the current header to free
	current->allocated = 1;

	//return the memory pointer right after the header of that block
	void* result = (char*)current + sizeof(header_t);
	return result;
      } else { //the current block is not the right block, so we have to go to the next one
	previous = current;
	current = current->next;
      }
    }

    //if we get to here, then we need to pointer bump if there is enough space
    if((intptr_t)((char*)free_ptr + size) <= end_ptr) {
       //if we get through this loop, then we have to place a new block, pointer bump, and update previous to hold the new 
      //blocks address
      //create a new structure
      header_t block_header;
      block_header = (header_t){.size = size, .allocated = 1, .next = NULL};

      //place that header at the start of the heap
      header_t* header_pointer = (header_t*) free_ptr;
      *header_pointer = block_header;

      //set the previous->next to header_pointer
      previous->next = header_pointer;

      //now go to the memory location right after the header
      free_ptr = (char*) free_ptr + sizeof(header_t);

      //store that address
      void* last_free = free_ptr;
    
      //increment the free pointer to go to the start of the next free space
      free_ptr = (char*)free_ptr - sizeof(header_t);
      free_ptr = ((char*) free_ptr + block_header.size + 1);
    
      //now return the location of the start of that memory block
      return last_free;
	}
  }

  //if we get here, then we return null because none of the other methods of allocation worked
  return NULL;
 
} // malloc()
// =================================================================================================================================



// =================================================================================================================================
/**
 * Deallocate a given block on the heap.  This function currently does nothing, leaving freed blocks unused.
 *
 * \param ptr A pointer to the block to be deallocated.
 */
void free (void* ptr) {
  //if the passed in pointer is null, then do nothing
  if(ptr == NULL) 
    return;

  //the pointer passed in is a pointer to the data of the block
  ptr = (char*) ptr - sizeof(header_t);
  header_t* header_pointer = (header_t*) ptr;
  
  header_pointer->allocated = 0;
  
} // free()
// =================================================================================================================================



// =================================================================================================================================
/**
 * Allocate a block of `nmemb * size` bytes on the heap, zeroing its contents.
 *
 * \param nmemb The number of elements in the new block.
 * \param size The size, in bytes, of each of the `nmemb` elements.
 * \return A pointer to the newly allocated and zeroed block, if successful; `NULL` if unsuccessful.
 */
void* calloc (size_t nmemb, size_t size) {

  // Allocate a block of the requested size.
  size_t block_size    = nmemb * size;
  void*  new_block_ptr = malloc(block_size);

  // If the allocation succeeded, clear the entire block.
  if (new_block_ptr != NULL) {
    memset(new_block_ptr, 0, block_size);
  }

  return new_block_ptr;
  
} // calloc ()
// =================================================================================================================================



// =================================================================================================================================
/**
 * Update the given block at `ptr` to take on the given `size`.  Here, if `size` is a reduction for the block, then the block is
 * returned unchanged.  If the `size` is an increase for the block, then a new and larger block is allocated, and the data from the
 * old block is copied, the old block freed, and the new block returned.
 *
 * \param ptr The block to be assigned a new size.
 * \param size The new size that the block should assume.
 * \return A pointer to the resultant block, which may be `ptr` itself, or may be a newly allocated block.
 */
void* realloc (void* ptr, size_t size) {

  // Special case:  If there is no original block, then just allocate the new one of the given size.
  if (ptr == NULL) {
    return malloc(size);
  }

  // Special case: If the new size is 0, that's tantamount to freeing the block.
  if (size == 0) {
    free(ptr);
    return NULL;
  }

  // Get the current block size from its header.
  size_t* header_ptr = (size_t*)((intptr_t)ptr - sizeof(size_t));
  size_t  block_size = *header_ptr;

  // If the new size isn't an increase, then just return the original block as-is.
  if (size <= block_size) {
    return ptr;
  }

  // The new size is an increase.  Allocate the new, larger block, copy the contents of the old into it, and free the old.
  void* new_block_ptr = malloc(size);
  if (new_block_ptr != NULL) {
    memcpy(new_block_ptr, ptr, block_size);
    free(ptr);
  }
    
  return new_block_ptr;
  
} // realloc()
// =================================================================================================================================



#if !defined (PB_NO_MAIN)
// =================================================================================================================================
/**
 * The entry point if this code is compiled as a standalone program for testing purposes.
 */
void main () {

  // Allocate an array of 100 ints.
  int  size = 100;
  int* x    = (int*)malloc(size * sizeof(int));
  assert(x != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    x[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", x[size / 2]);


  //free y
  free(x);

   // Allocate an array of 100 ints.
   size = 100;
   int* y = (int*)malloc(size * sizeof(int));
   assert(y != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    y[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", y[size / 2]);


  // Allocate an array of 100 ints.
  size = MB(200);
   int* z    = (int*)malloc(size * sizeof(int));
   assert(z != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    z[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", z[size / 2]);


  //free z
  free(z);

   // Allocate an array of 100 ints.
  size = KB(200);
   int* a    = (int*)malloc(size * sizeof(int));
   assert(a != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    a[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", a[size / 2]);


  // Allocate an array of 100 ints.
  size = MB(300);
   int* b    = (int*)malloc(size * sizeof(int));
   assert(b != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    b[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", b[size / 2]);


  // Allocate an array of 100 ints.
  size = KB(100);
   int* c    = (int*)malloc(size * sizeof(int));
   assert(c != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    c[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", c[size / 2]);



  // Allocate an array of 100 ints.
  size = MB(11);
   int* d    = (int*)malloc(size * sizeof(int));
   assert(d != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    d[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", d[size / 2]);


  // Allocate an array of 100 ints.
  size = KB(500);
   int* e    = (int*)malloc(size * sizeof(int));
   assert(e != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    e[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", e[size / 2]);



  // Allocate an array of 100 ints.
  size = KB(423);
   int* f    = (int*)malloc(size * sizeof(int));
   assert(f != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    f[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", f[size / 2]);


  //free f
  free(f);

  // Allocate an array of 100 ints.
  size = KB(423);
   int* g    = (int*)malloc(size * sizeof(int));
   assert(g != NULL);

  // Assign some values.
  for (int i = 0; i < size; i += 1) {
    g[i] = i * 2;
  }

  // Print one of the values from the middle, just to show that it worked.
  printf("%d\n", g[size / 2]);

  free(a);
  free(b);
  free(c);



  
} // main()
// =================================================================================================================================
#endif
