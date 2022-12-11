---
layout: post
title: "Compact Python Code"
date:  2022-12-11 18:42:00 +0530
categories: programming
---

I was working on a piece of code which says "given a set of integers, print `True` if all the integers are positive AND if at least one of them is a palindrome number (same as palindrome string)

Being a C programmer that I was, I came up with the following piece of code:

```python
def checkNumbers(strElems, elems):
    for i in elems:
        if i < 0:
            print(False)
            return
    for i in strElems:
        if (i == i[::-1]):
            print(True)
            return
    print(False)

if __name__ == "__main__":
    nElems = int(input().strip())
    strElems = input().strip().split()
    elems = map(int, strElems)
    checkNumbers(strElems, elems)
```

Turns out, this whole thing can be written in just 3 lines of python code:

```python
if __name__ == "__main__":
    n, elems = int(input()), input().split()
    result = all(map(lambda x: int(x)>0, elems)) and any(map(lambda x: x == x[::-1], elems))
    print(result)
```

How elegant is python programming! It is worthwhile reading other people's code to learn how to code!
