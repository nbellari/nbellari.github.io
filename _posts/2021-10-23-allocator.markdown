---
layout: post
title: "Writing a Memory Allocator"
date:  2021-12-24 18:48:59 +0530
categories: programming
---

This is another small coding exercise that I took to understand memory allocators. Inspired from [Arjun](https://arjunsreedharan.org)'s [Memory Allocators 101](https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory) post.

## Process Memory Layout

Here's is what a process address space looks like:

![address-space.png]({{site.baseurl}}/assets/images/allocator/address-space.png)

A Process' virtual address space begins from the code section. After the code is laid out, the next section contains initialized global and static data. This region stores all the initialized global and static variables. Uninitialized global and static variables follow next with that mapped to zero pages (as they must be initialized to zero). As soon as the process begins to write to those variables, new zero pages are allocated to the process (lazy allocation).

The stack and heap are the ones that are generally keep changing in size (stack, because we dont know how deep a function call chain can go and heap, because we dont know who much memory is allocated by the process. So, to tackle with this, the stack grows top to bottom and the heap grows bottom to top (with reference to the above figure). The address space in between stack and heap is actually unused (and unallocated). The area where heap ends in the process space is called (for historical reasons) a **break**. The address space literally breaks (stops) there! That said, the stack space is limited to some size (varies from 2MB to 8MB) and it does not dynamically grow in the address space like heap does. Although, a stack limit can be changed dynamically with `setrlimit` call with `RLIMIT_STACK` resource specified.

The system call that changes this break in the address space is `sbrk`. `sbrk(0)` gives the current break in the address space, `sbrk(x)` increases the address space by `x` bytes, while `sbrk(-x)` gives back `x` bytes to the operating system. That said, `sbrk` is not the only way to extend the memory. `mmap` can also be used to extend the memory. However, with `sbrk` memory can only be operated in LIFO order - that is, if you extend it a couple of times, only the latest expanded portion can be contracted first. `sbrk` is also, apparently, not thread safe.

That said, the malloc library does not always use `sbrk` for for allocating dynamic memory. If the requested memory crosses `MMAP_THRESHOLD`, then malloc will fall back to `mmap` to allocate memory (as highlighted in the picture above). The malloc library will manage both segments of memory by itself. We will not cover `mmap` based memory allocation here. Just the `sbrk` way.

Life would have been simpler, if we had requested memory from operating system when needed and gave it back when we are done with it. For various reasons, OS does not choose to do that. For example, how the memory is to be kept and managed with in a process context is not OS business. So, while we happily get more memory from the OS with `sbrk`, because of its LIFO nature (described earlier), we cant give back the memory in any order.

So, then, we have to do some book keeping on our own for the purpose of freeing the memory in arbitrary order. And not just that, since the memory cannot be given back to the OS, it has to be kept back. And because it can be kept back it can be used to serve future malloc requests. Thus, an allocator also acts as a kind of cache between the application and operating system.

So, then, what are we supposed to maintain for book keeping? To understand that, we need to understand the workflow of malloc/free

* `malloc` allocates a memory and returns the pointer to the allocated region
  * It should first search the free list, if such a request can be satisfied with the given free blocks, if so return one
  * If not, use `sbrk` to get more from the OS and then return it
* free releases the memory pointed to by the given pointer
  * Puts the memory back in the free list
  * Occasionally it should try to see if some memory can be given back to the OS
  
There are other things that an allocator can do - like maintaining statistics, adding some memory around the allocated memory and try to detect if the memory is overwritten (both during free as well as periodically), but those are additional features to the allocator, which can be dealt with later.

Since `free` only gives a pointer back to the allocator, to be able to know the size of the allocated memory, there should be some book keeping done. One way to do that would be to maintain a hash table mapping address to a book keeping structure that gives other details like its size etc. However, there is a lookup overhead associated - even with a hash table. The best way is to maintain the book keeping structure just above the memory block that is allocated so that we can retrieve the structure in constant time!

Since this structure is co-located with memory, care should be taken such that the memory address returned to the caller is properly aligned, otherwise there will be cache mis-alignment issues with the returned memory (i.e. it may end up crossing the cache lines). So, such a header definition looks like this:

```c
/* Lets keep it simple, and grow as needed */
struct header {
    int size; /* size of the allocated memory, excluding the header */
    struct header *next; /* Points to the next element in the free list */
};
```

To make it aligned properly, we do this:
```c
/* Lets keep it simple, and grow as needed */
typedef union {
    struct header h;
    char _align[16]; /* This chunk of memory (header) will be aligned to 16 bytes */
} header_t;

#define ALLOCATOR_HEADER_SIZE sizeof(header_t);
```

We also talked about maintaining a free list so that we can keep track of free blocks that can serve future requests. For that, we need to have a list. Lets maintain a singly linked list for now with a head and tail. Basically this is a unsorted linked list where we append free blocks to the tail and search for a free block from head to tail until we find a match:

```c
/* Free list */
header_t *free_list_head, *free_list_tail;
```

Initially `allocator_free_list` will be `NULL`. So, when a first malloc request comes, we have to `sbrk`. Any free block, if found in the list, must be removed from the list before returning it. Here is the function `get_free_block`

```c
/* Function to get a free block from the free list, if one is available */
void *
get_free_block(size_t size)
{
    header_t *cur = free_list_head, *prev = free_list_head;
    header_t *free = NULL;

    while (cur) {
        /* Exact match may not be found */
        if (cur->h.size >= size) {
            free = cur;
            if (cur == free_list_head) {
                free_list_head = free_list_head->h.next;
            } else {
                prev->h.next = cur->h.next;
            }
            break;
        } else {
            prev = cur;
            cur = cur->h.next;
        }
    }

    return free;
}
```

We are looking for a match here that is large enough to serve the current request. When a match is found, the block is removed from the list (the classic "remove a node from a singly linked list" interview question). This may not be a good way to find the best match, but it works! If we search till the end of the list, then we may find a more perfect block that serves the current request. In that case, the complexity of finding a free block is constant and proportional to the size of the free list. But we digress.

Now that we have `get_free_block`, its pretty easy to write `malloc2` (not to confuse with classic `malloc`)

```c
void *
malloc2(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;

    if (size == 0) {
        return NULL;
    }

    /* Try to find from the free list */
    header = (header_t *)get_free_block(size);
    if (header != NULL) {
        block = (void *)(header + 1);
        return block;
    }

    /* Get from OS */
    total_size = size + sizeof(header_t);
    header = sbrk(total_size);

    if (header == (void *)-1) {
        /* Cannot grow the process, serious error!
         * However, we simply return NULL, taking action
         * is caller's responsibility
         */
        return NULL;
    }

    /* record the size */
    header->h.size = size;
    header->h.next = NULL;
    block =  (void *)(header + 1);
    return block;
}
```

Please note, the newly allocated block is not kept in any list. All the allocated blocks are with user and user will free them. One disadvantage with this approach is that if the user loses a reference to the allocated block, then the block is eternally lost. Well, malloc essentially does the same, so this is not such a bad implementation. If the allocator need to provide debug information, then it can have another allocated list which it can walk through to list the blocks that are not freed. So, it is very important to keep the end goal in mind and design the software such that it is easily extensible.

Now comes the `free2` function. It tries to locate the header, and then checks if the block that is going to be freed is at the top of the heap i.e. it is at the *break*. If so, this can be released to the operating system, if not, then the only thing that can be done is to put it in the free list and hope for it to be used again. Here is the `free2` function:

```c
void
free2(void *block)
{
    header_t *header = NULL;
    size_t total_size;

    if (block == NULL) {
        return NULL;
    }

    header = (header_t *)block - 1;
    total_size = header->h.size + sizeof (header_t);

    if ((char *)header + total_size == allocator_break) {
        /* This can be given back to the operating system */
        sbrk(-total_size);
    } else {
        /* put it in the free list */
        put_free_block(header);
    }

    return;
}
```

And a very simple function to put the block in the free list, if it cannot be released to OS.

```c
void
put_free_block(header_t *header)
{
    /* Insert it at the head of the list */
    header->next = free_list_head;
    free_list_head = header;
}
```

We insert the block at the head so as to honor MRU policy. There are two more functions to be implemented `calloc` and `realloc`. `calloc` zeroes the memory before returning:

```c
void *
calloc2(size_t n, size_t size)
{
    size_t total_size;
    void *block;

    if (!n || !size) {
        return NULL;
    }

    total_size = n * size;

    if (size != total_size/n) {
        return NULL;
    }

    block = malloc(total_size);

    if (block) {
        memset(block, 0, total_size);
    }

    return block;
}
```

One thing to note here is that after multiplying `size` with `n` we check if the resultant multiplication has not overflown. These are some of the things that one needs to take care of in C. Other than that, there is nothing special about the function.

`realloc` has to copy the contents and free the old memory before returning the new one.

```c
void *
realloc2(void *block, size_t size)
{
    header_t *header;
    size_t orig_size;
    void *new_block;

    if (!block || !size) {
        return NULL;
    }

    header = (header_t *)block - 1;
    orig_size = header->h.size;

    /* Since the block allocated need not be of exact same size,
     * it makes sense to check if the original block size can satisfy
     * the requested size
     */
    if (orig_size >= size) {
        return block;
    }
    
    new_block = malloc(size);

    if (!new_block) {
        return NULL;
    }

    memcpy(new_block, block, orig_size);
    free(block);

    return new_block;

}
```

One nuance to be noted here is that since the block allocated previously is not necessarily of the exact same size, we should check if it can satisfy the requested size. If so, there is no need to anything, we simply return the same old block!
