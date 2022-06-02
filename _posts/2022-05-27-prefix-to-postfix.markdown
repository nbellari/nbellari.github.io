---
layout: post
title: Converting Prefix Expression to Postfix Expression
categories: programming python algorithms
date:  2022-05-27 22:33:55 +0530
---

How do you convert a prefix expression to a postfix expression?

For example, if the given expression is `+AB`, after conversion it is `AB+`. Incidentally, this was one of the challenges given during a course study at a reputed site. If we look at the sample input and write naive code to solve the problem (like assuming that there is only one such expression in a string), then it is our stupidity. :-)

After thinking through this, I came up with the following python code:

```python
from collections import deque

def postfix(prefix):
    stack = deque()

    postfix = ""
    for c in prefix:
        if not c.isalpha():
            stack.append([c, 0])
        else:
            postfix += c
            stack[-1][1] += 1
            while (stack and stack[-1][1] == 2):
                # Two operands have been printed
                postfix += stack.pop()[0]
                if (stack):
                    stack[-1][1] += 1
    while (stack):
        postfix += stack.pop()[0]

    return postfix
```

Well, what I am trying to do is push an operator to the stack and keep track of how many operands have been visited. Once two are visited, then the operator is also popped and the current top element of the stack is examined for the same. With this code, all the given test cases passed, however there is still a lingering feeling that this may not be the elegant code. When I looked at the actual solution, it was indeed elegant:

```python
from collections import deque

def postfix(prefix):
    stack = deque()
    prefix = ''.join((reversed(prefix)))

    for symbol in prefix:

        if symbol != '+' and symbol != '-' and symbol != '*' and symbol != '/':
            stack.append(symbol)

        else:
            operand1 = stack.pop()
            operand2 = stack.pop()
            stack.append(operand1+operand2+symbol)

    return(stack.pop())
```

The main idea here is that the given expression is reversed. So, `+AB` becomes `BA+`. When we push `B` and `A` into the stack and encounter an operator, we know that we have already pushed its operands into the stack and only need to pop them - and we get them in the correct order. So, there is no need to explicitly track the number of operands visited for an operator here. That is, according to me, the key idea which simplifies and makes the solution elegant.

The same logic would apply to the postfix to prefix conversion. Here is the code for it:

```python
from collections import deque

def prefix(postfix):
    stack = deque()

    for s in postfix:
        if s.isalpha():
            stack.append(s)
        else:
            operand2 = stack.pop()
            operand1 = stack.pop()
            stack.append(s+operand1+operand2)

    return stack.pop()
```

Two things different from the prefix version: a) There is no need to reverse the string to start with and b) the pop order of operands is opposite as we push the first operand before second.
