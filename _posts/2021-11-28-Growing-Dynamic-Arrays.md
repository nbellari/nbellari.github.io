---
published: true
---

Normal arrays have constant access and insert time - **O(1)**. Dynamic arrays, however, grow in size to accommodate new elements. Access time is still constant, however, insert time varies depending on the element being inserted.

Let's assume we have a policy of doubling the array size when the array is full. So the array sizes will be 1, 2, 4, 8, 16, 32 etc. When the size of an array is **n** (which is a power of 2), and a new element needs to be inserted, then new memory of double the size (**2n**) is allocated and the old memory is copied to the new one. In essence, when the new array size is m, there is always m/2 copy operations needed to copy the contents and resume operations.

It can be noted that most insert operations are **O(1)**, only a few insert operations are not. An interesting questions is - given an array of size **n**, how many copy operations would have been performed, assuming that the array size starts from 1?

![copies2.png]({{site.baseurl}}/copies2.png)

For example, if n is 10, then number of copies are:

![example1.png]({{site.baseurl}}/example1.png)

In other words, the total number of copy operations done for an array of n elements are:

![example1.1.png]({{site.baseurl}}/example1.1.png)

So, the average number of copy operations per insertion becomes:

![average.png]({{site.baseurl}}/average.png)

This is amortization. Of course, an efficient implementation never copies elements one by one, instead it relies on **memcpy**, which based on platform features (such as **SIMD** etc.), can copy much more efficiently, so the cost of insertion is reduced further.
