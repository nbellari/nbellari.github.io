# Writing a memory allocator

This is another small coding exercise that I took to understand memory allocators. Inspired from [Arjun](https://arjunsreedharan.org)'s [Memory Allocators 101](https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory) post.

## Process Memory Layout

Here's is what a process address space looks like:

<p align="center">
<img align="center" src="images/address-space.png" />
</p>

A Process' virtual address space begins from the code section. After the code is laid out, the next section contains initialized global and static data. This region stores all the initialized global and static variables. Uninitialized global and static variables follow next with that mapped to zero pages (as they must be initialized to zero). As soon as the process begins to write to those variables, new zero pages are allocated to the process (lazy allocation).

The stack and heap are the ones that are generally keep changing in size (stack, because we dont know how deep a function call chain can go and heap, because we dont know who much memory is allocated by the process. So, to tackle with this, the stack grows top to bottom and the heap grows bottom to top (with reference to the above figure). The address space in between stack and heap is actually unused (and unallocated). The area where heap ends in the process space is called (for historical reasons) a **break**. The address space literally breaks (stops) there! That said, the stack space is limited to some size (varies from 2MB to 8MB) and it does not dynamically grow in the address space like heap does. Although, a stack limit can be changed dynamically with `setrlimit` call with `RLIMIT_STACK` resource specified.

The system call that changes this break in the address space is `sbrk`. `sbrk(0)` gives the current break in the address space, `sbrk(x)` increases the address space by `x` bytes, while `sbrk(-x)` gives back `x` bytes to the operating system. That said, `sbrk` is not the only way to extend the memory. `mmap` can also be used to extend the memory. However, with `sbrk` memory can only be operated in LIFO order - that is, if you extend it a couple of times, only the latest expanded portion can be contracted first. `sbrk` is also, apparently, not thread safe.

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


