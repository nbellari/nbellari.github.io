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

Then there is the recursive verion of fibonacci which is the most elegant version:

```python
def fib_recursive(n):
    if (n == 1): 
        return 0
    if (n == 2): 
        return 1
    return fib_recursive(n-1) + fib_recursive(n-2)
```

While this looks elegant, each element in the fibonacci is computed many times again and again. This will not scale as n grows. To illustrate that, we can use python decorators to time the function, something like this:

```python
def get_coarse_time(t):
    if (t < 1000):
        return f"{t} nsec"
    elif (t < 1000000):
        return f"{t/1000} usec"
    elif (t < 1000000000):
        return f"{t/1000000} msec"
    elif (t < 1000000000000):
        return f"{t/1000000000} sec"

def time_it(fn):
    def fn_wrapper(f, n): 
        start = time.time_ns()
        val = f(n)
        end = time.time_ns()
        diff = end - start
        print(f"{get_coarse_time(diff)}")
    return fn_wrapper

@time_it
def run_fib(f, n): 
        print(f(n))
```

See the time taken for the iterative version and recursive version for 20 runs:

### Iterative Version

| n | time |
|1| 3.834 usec |
|2| 3.815 usec  |
|3| 2.519 usec  |
|4| 2.315 usec   |
|5| 2.352 usec    |
|6| 2.389 usec   |
|7| 2.537 usec    |
|8| 2.685 usec    |
|9| 2.723 usec   |
|10| 2.759 usec   |
|11| 5.26 usec    |
|12| 3.722 usec    |
|13| 17.852 usec   |
|14| 3.574 usec    |
|15| 7.241 usec |
|16| 4.0 usec |
|17| 4.148 usec |
|18| 4.111 usec |
|19| 4.241 usec |
|20| 205.368 usec |

### Recursive Version

| n | time |
|1| 1.574 usec |
|2| 1.093 usec |
|3| 2.889 usec |
|4| 2.962 usec |
|5| 4.24 usec |
|6| 7.851 usec |
|7| 9.222 usec |
|8| 13.648 usec |
|9| 21.389 usec |
|10| 33.796 usec |
|11| 191.757 usec |
|12| 88.554 usec |
|13| 283.107 usec |
|14| 286.903 usec |
|15| 430.29 usec |
|16| 653.269 usec |
|17| 973.209 usec |
|18| 1.508776 msec |
|19| 2.376171 msec |
|20| 3.813559 msec |

As you can see, the recursive version is an order of magnitude larger than the iterative version as n increases (we are just at 20)! That is because the recursive version does lot of duplicate work. `fib(5)` calls `fib(4)` and `fib(3)`, but `fib(4)` also calls `fib(3)` - this is the duplicate work that gets done.

Memoization aims to cache already computed results so they dont need to be computed again. Here is a memoized version:

```python
def memoized(fn):
    fib_cache = {}
    def fib(n):
        if n not in fib_cache:
            fib_cache[n] = fn(n)
        return fib_cache[n]
    return fib

fib_recursive = memoized(fib_recursive)
```

The last line above makes `fib_recursive` a memoized version. Whenever it is called, it always returns the cached answer if it exists. Only if it does not exist, will it go for the actual version.

The following chart shows how much memoizing the results will improve the performance. Iterative and memoized versions are practically indistinguishable here and recursive version grows exponentially!!

![fib-chart.png]({{site.baseurl}}/assets/images/fib/fib-chart.png)

How better is iterative better over memoized version? or Vice versa? To understand that we need to run the experiment without recursive version - as it takes the y-axis off the charts (literally!) and for larger `n`s. Here is the resulting chart:
