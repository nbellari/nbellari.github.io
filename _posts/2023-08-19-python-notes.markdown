---
layout: post
title: Python Notes
categories: programming
date:  2023-08-19 16:47:05 +0530
---

Some Python notes:

* One of the most important and frequently occuring computational pattern - `enumerate -> filter -> map -> accumulate`
* Identity is a stronger condition than equality. That is, `is` is stronger than `==`
* Two empty lists `[]` and `[]` are equal, but not identical. While, two `None`s are equal and identical.
* When assignment binds a variable to another variable pointing to a mutable type, then both variables refer to the same instance of the mutable type.
* If `a = [1,2,3]`, then `a` is equal to `[1,2,3]`, but `a` is not `[1,2,3]`
* If `a = [1,2,3]`, and `b = a`, then `a` is equal to `b` and `a` is `b`. Modifying one will show the effect on the other.
* `Class.foo()` is a function, while `obj.foo()` is a bounded method. To invoke the first variant, one has to pass object and the function parameters, while to invoke the latter, object is implicit (i.e. already bound) and only parameters need to be passed. In the case of inheritance, the parent method is called from the child method using the `Class.foo()` notation.
* Class variables are applicable globally to all instance of the objects. They have to be assigned in class definition. To change them, it has to be referred with the class: `Class.var = x`. If we do `obj.var = x`, it will create a new object instance variable and it will oshadow the class version. It wont be available later in the lifetime of obj.
* General convention is that the class variables (i.e. class state or data) is defined in the `__init__` function. However, other member methods of a class can define and initialize new variables that belong to the class. As it turns out, even users of the class, by default, can create new attributes of objects. Users of a class can also create new class attributes which is applicable to all objects by default.
* The statement to access attributes of a class or an instance is called a `dot expression`. It is of the form `<expression> . <name>`. `<expression>` can be anything that evaluates to an object. It can be a object variable, a constant or a different expression (for eg. a list slice)
* In a dot expression, instance attributes are searched first and then class attributes.
* Method resolution in a child class happens from left to right i.e. the order of parent classes given in the child class definition.
* `Class.mro()` will give the method resolution order of methods for a given class
* `str` is supposed to give human readable version of the object, while `repr` is supposed to give machine readable version. That is, `eval(repr(obj)) == obj`. `repr` can be usd to serialize and de-serialize an object, provided we implement it carefully.
* `1j` is a complex numeral. Only `j` is allowed
* A long string over multiple lines can be had something like this, without the hassle of newlines being inserted (as with a multiline string)
```python
s = ("this is a very long"
     " string, which spans"
     " multiple lines")
```
* one difference between `%(specifier)s' and f-strings is that f-strings evaluate immediately while the former does so lazily, so it can be used in arguments as well
* one can have a dictionary and pass it as `<star><star>dict` to a function to expand it as keyword arguments. 


