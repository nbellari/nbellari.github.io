---
layout: post
title: Platform Independent File Access in Python
categories: programming
date:  2023-08-15 15:07:05 +0530
---

I have been looking at working with files in a platform independent way for python. Turns out, I found a [resource](https://pymotw.com/2/ospath/). Here are some points to note:

* `os.path` is a module
* `os.sep` is the separator between elements of a path. It can be `/` for Linux and `\` for Windows
* `os.extsep` is the file name extension separator. Mostly it will be `.`
* `os.pardir` is the parent directory. Mostly it will be `..`
* `os.curdir` is the current directory. Mostly it will be `.`
* `os.path.split()` takes a path and returns the directory path and the last component. This is not string `split()` function..This returns a tuple
* `os.path.dirname()` and `os.path.basename()` return the same values as the above `split()` returns
* `os.path.splitext()` works like `split()` except that it splits at the file name and extension
* `os.path.commonprefix()` returns the common string among all the paths in the list
* `os.path.join()` takes a sequence of strings and joins them with os specific path
* `os.path.expanduser()` takes a `~` in the given path and expands it with user home directory. One can give `~` or `~user` and it will expand both.
* `os.path.expandvars()` also takes a path and replace any shell variables with its values
* `os.path.normpath()` takes a path and normalizes it by removing redundant stuff
* `os.path.abspath()` takes any path and converts it to absolute path.
* `os.path.isabs()` to check if a path is absolute
* `os.path.isfile()` to check if it is a file
* `os.path.isdir()` to check if it is a directory
* `os.path.islink()` to check if a file is a link
* `os.path.ismount()` to check if the given path is a mount point
* `os.path.exists()` to check if the path exists
* `os.path.walk(path, callback, arg)` calls the callback which is of signature `callback(arg, dirname, names)`. Basically it starts walking the `path` and gives each directory along with its contents. 
* `os.mkdir()` creates a directory
