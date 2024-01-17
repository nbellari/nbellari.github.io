---
layout: post
title: The Many Flavors of Fibonacci
categories: programming
date:  2024-01-17 21:42:05 +0530
---

Fibonacci is a sequence. `fib(n)` refers to nth element in the sequence. Since this is a mathematical sequence, there is no `fib(0)`. That is, the sequence starts from 1.

A basic iterator version of the sequence is like this:

```python
def fib_iter(n):
    prev, curr = 1, 0
    for _ in range(n-1):
        prev, curr = curr, prev + curr

    return curr
```

It is a very simple and elegant version.

At any point of time in the function `curr` points to the `i`th element. `prev` starts with an initial value of 1. Before the loop starts, `prev` points to 1 (not any element in the sequence), and `curr` points to the first element. As the loop executes, `prev` and `curr` advance one step at a time. So, `fib(1)` - just returns `curr` which is `0`. `fib(2)` makes the loop go once, which means `prev` and `curr` point to the previous and current elements (i.e. first and second element) and so `curr` returns `fib(2)`

