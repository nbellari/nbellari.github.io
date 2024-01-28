---
layout: post
title: A Simple Array Based Ring Buffer
categories: programming
date:  2024-01-28 13:39:05 +0530
---

The other day, at work, I was trying to design a simple ring buffer which is array based. It had simple requirements - it is an array based buffer with a fixed number of entries (`n`) and keeps overwriting the least recent entry once the array is full. When accessed, it need to visit the entries from the least recent to the most recent item.

All we need is a simple structure and two variables:

```c
typedef struct element_s {
    // your data definition goes here
} element_t;

element_t ring_buffer[MAX_ENTRIES]; //An array of maximum entries
int next_index = -1;
```

Adding entries to the ring buffer is simple:

```c
void
add(element)
{
    next_index = (next_index + 1) % MAX_ENTRIES;
    ring_buffer[next_index] = element;
}
```

Looping through the entries from the least recent to the most recent is also simple:

```c
void
iterate()
{
    if (next_index == -1) {
        return;

    idx = (next_index + 1) % MAX_ENTRIES;
    if (is_zero(ring_buffer[idx])) {
         /* ring buffer is somewhere in the middle
         */
        start = 0;
        count = next_index - start;
    } else {
        /* Every element in the array is written atleast once, so
         * visit every element in the array
         */
        start = (next_index + 1) % MAX_ENTRIES;
        count = MAX_ENTRIES;
    }
}
```

The rationale is that when element next to the current index is uninitialized (i.e. never written), then only the elements from the beginning to the current index is written. We need to visit entries from 0 till current index. However, if the element next to the current index is initialized (already written), then every element in the ring buffer is written atleast once and we need to visit the elements from the first index after current index until the current index (for a total of `MAX_ENTRIES`)

It is a very simple implementation!
