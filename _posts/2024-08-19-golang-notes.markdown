---
layout: post
title: Golang Notes
categories: programming
date:  2024-08-19 12:17:05 +0530
---

Here are some golang notes for my reference, taken from the book "The Go Programming Language".

* In the increasing order of abstraction: values (simple and aggregate types), keywords, variables, expressions, statements, control-flow statements, functions, files, packages.
* There is a difference between `keywords` and `predeclared names` - the former cannot be used in declarations, while the latter can be.
* Any name outside the scope of a function (including the function name itself) is visible across all the files with in a package. If the name starts with an uppercase letter, then it is by default exported to be accessed by all the files of the package as well as other pacakges.
* Package names are always in lower case
* The larger the scope of a name, the longer and more meaningful it should be
* Four types of declarations - types, variables, constants and functions
* A file begins with package, import statments, package level declarations (the four in any order)
* Variable declarations are of the form: `var name(s) type = expression`. Either the type can be ommitted or the expression, but not both. If type is ommitted, then the value and type of the variable is the value and type of the expression. If expression is ommitted, then the variable gets the `zero value` corresponding to the type. An aggregate recursively gets the `zero value` of its elements.
* There is no such thing as an uninitialized variable. So, declare is same as declare and initialize
* `foo(a, b int)` - says that both `a` and `b` are integers
* There can be multi variable declarations in the same line. If type is given, then they belong to the same type, it type is ommitted, then the variables can be of different types - based on the initalizer values
* Short variable declarations (:=) are only for use within a function
* Initializers can be simple values or complex expressions (including function calls)
* A multi variable short declaration should not be confused with a tuple assignment
* A short variable declaration should have atleast one new variable declaration, rest can be assignments
* `nil` is the `zero value` of a pointer
* It is perfectly fine for a function to return the address of its local variable
* `*p++` is equivalent to `(*p)++` which is equivalent to `*p = *p + 1`
* pointer aliasing - a new way to refer to the same variable
* `flag.Bool` and `flag.String` return pointers to variables, and after calling `flag.Parse` one can read those argument values by dereferencing the pointer variables
* `new(T)` will create an unnamed variable of type T, initializes to `zero value` and returns an address of type `*T`
* When we call `new` with empty types like `struct{}` or `[0]`, the resulting pointers may have the same address
* `new` is not a keyword, but a predeclared function
* Package level variables have lifetime until the program exits, while function variables have dynamic lifetimes depending on the reachability.
* When a variable is not reachable via any path (from other variables or pointers), it is considered unreachable and is garbage collected.
* `var` or `new` does not decide what gets allocated in heap or stack - its reachability
* Why does `a,b = b,a` work? Because the right hand side is evaluated first and then the assignments are done.
* Assignability and comparability - two things that need more attention
* `type name <underlying-type>` - is used to create a new type
* The main advantage of user defined types is that even though the underlying type is the same, they can never be assigned or compared with each other without explicit conversion
* Named types can have its own methods - which is another advantage over primitive types
* Only one file in each package should have a doc comment

