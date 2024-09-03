---
layout: post
title: Golang Notes
categories: programming
date:  2024-08-19 12:17:05 +0530
---

Here are some golang notes for my reference, taken from the book "The Go Programming Language".

# Chapter 2: Program Structure

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
* There can be many `init` functions in a file and all of them will be called before a program executes - convenient to initialize complex tables etc.
* Apparently, there are explicit blocks that define scope and there are also implicit blocks. For example, `for i, _ := range arr` defines an implicit scope. The scope of the variable `i` for example, in this case is the condition, post statement and the body of the for loop. One interesting thing to note here is that `i` in the for statement can refer to another `i` in the encompassing scope when it is getting defined.
* One has to be careful with short variable declarations as it can inadvertently override the global ones and they never get updated! 

# Chapter 3: Basic Types

* No implicit type conversions are allowed even for the basic integer types
* Integers, floating point numbers and strings are ordered by comparison operators.
* No other types are ordered
* Right shift operator fills the vacated bits not with zero, but with the value of the sign bit (MSB)
* `len(aggregate)` - always returns a signed integer so that statements like `i := len(arr); i >=0; i--` terminate correctly. 
* Signed integers are beneficial in places even though unsigned integers may seem more intuitive
* format specifiers are called `verbs`
* Generally, the number of parameters (after the format string) is same as the number of verbs in the format string, but we can specify adverbs to work around that.
* Some adverbs: `[1]` - after `%` tells Printf to use the first parameter only. `#` - after `%` tells Printf to print prefix (`0` or `0x` or `0X`) appropriately - adverbs precede verbs. The decorator adverb precedes the arg adverb.
* `%q` - prints with quotes
* `float32` provides 6 digits of precision, `float64` provides 15
* Floating numbers can be written as `.80` or `12.`
* Three verbs for floating point - `%g`: chooses the most compact form. `%e` (with exponent) or `%f` (without exponent) is used for tabulating - with column alignment - we can use standard `%8.3f` etc.
* Comparison with `NaN` always yields `false`
* Be it import, var, const etc. They all can be defined in a group like `import (...)`, `const (...)`
* Complex number is a native type - multiplication/addition/subtraction are natively calculated
* `complex(real, img)` is used to construct a complex number. `real(x)`, `imag(x)` - return real and imaginary components.
* Complex numbers can be initialized like mathematically also: `x := 2 - 3i`
* UTF-8 encoded sequence of code points are called as runes
* `ith` byte is not same as `ith` character in the string. `len` returns the number of bytes.
* For strings, when we say `t := s` and do `s += 'abc'`, then `s` will be a new string, `t` will be pointing to old `s`
* Because strings are immutable, substringing is fast because they share the same memory beneath
* Go source files are always encoded in UTF-8.
* A raw string is written in backticks. Except carriage returns, which are deleted, nothing else is interpreted
* Some applications of raw strings: regular expressions, HTML, JSON, multi-line messages etc.
* A `rune` is a unicode code point (i.e. a number)
* Unicode has about 120000 code points
* UTF-32 is a fixed length encoding of unicode code points. UTF-8 is a variable length encoding of unicode code points

| UTF-8 encoding | What does it mean | rune bytes |
| 0xxxxxxx | ASCII | 1 |
| 110xxxxx 10xxxxxx | 128 - 2047 (<128 are not used) | 2 |
| 1110xxxx 10xxxxxx 10xxxxxx | 2048 - 65535 (<2048 are not used) | 3 |
| 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx | 65535 - 0x10ffff | 4 |

* Runes are self synchronizing - we can always find the beginning of a character by backing up no more than 3 bytes (worst case)
* Runes are prefix coded, so we can get to characters without any lookahead
* Apparently string literals and rune literals are different. String literals can be encoded in `\x, `\u` or `\U`. However, rune literals allow `\x` only if it is a single byte unicode, `\u` only if it is a 2-byte unicode and `\U` if it isa 4-byte unicode
* `utf8.RuneCountInString` returns the number of runes in a string. `utf8.DecodeRuneInString` returns the rune and its size
* To step through characters in a string, one does with the help of the `utf8.DecodeRuneInString` and jumping `size` bytes in the string
* `range` does the above job implicitly. If we say `i, r := range <str>`, then it will increment to point to the next character (not next byte)
* Number of runes in a string can be easily done as `for _,_ := range <s> { n++ }`
* Because of this, it is always recommended to always loop over a string with `range`
* Each time `range` or `utf8.DecodeRuneInString` encounters an invalid input in the sequence, it returns the `\uFFFD`which is a question mark
* When we convert integer as rune, the integer is treated as a rune value.
* Because strings are immutable, building them incrementally is costly
* Four main packages for strings: `bytes`, `strings`, `strconv` and `unicode`
* `basename` removes both directory components and suffix
* `path` package works on any platform, for URl paths. `path/filepath` should be used for filenames as it is platform specific
* A string is immutable, but a byte array is not.
* 
