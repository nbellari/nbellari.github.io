---
layout: post
title: Python Standard Library
categories: programming
date:  2023-09-30 20:06:05 +0530
---


## File Handling

* `os.path.split()` - splits a path into basename and filename and returns a tuple
* `os.path.basename()` and `os.path.dirname()` - return the above separately
* `os.path.splitext()` - splits it not on `os.sep`, but on `os.extsep` and the second element in tuple will contain the `os.extsep` that is the `.`
* `os.path.commonprefix()` - is more like a common substring matching without regards to path boundaries
* `os.path.commonpath()` - honors path boundaries, however either it can't take mix of relative and absolute paths.
* `os.path.join()` - takes a sequence (list or tuple) and builds a path out of it with the help of `os.sep`. If any argument starts with `os.sep`, then all the previous arguments are discarded and the joining process starts from the current one
* `os.path.expanduser()` - expands `~`
* `os.path.expandvars()` - expands shell variables as well
* `os.path.normpath()` - cleans up the path by removing spurious `os.sep`s, `os.curdir`s and `os.pardir`s
* `os.path.abspath()` - returns the absolute path
* `__file__` - returns the current script file name
* `os.path.getatime()`, `os.path.getmtime()`, `os.path.getctime()`, `os.path.getsize()` - returns the access time, modification time, creation time and the size of the file respectively
* `os.path.isabs()` - checks if a path is absolute
* `os.path.isfile()` - check if the path is a regular file
* `os.path.isdir()` - check if the path is a directory
* `os.path.islink()` - check if the path is a link
* `os.path.ismount()` - check if the path is a mount point
* `os.path.exists()` - check if the path exists in the file system (file or dir or sym link etc.)
* `glob.glob()` - takes standard shell conventions for globbing the files (`*`, `?`, ranges, escape characters etc.)
* `tempfile.mkstemp()` - returns a tuple of file handle and name. We can close the handle, though, if not needed
* `os.unlink()` - delete a file
* `os.open()` - open a file and return a handle
* `linecache.getline(file, no)` - opens the file, caches the lines and returns them.
* `tempfile.TemporaryFile()` - returns an unnamed temporary file and unlinks it immediately if possible
* `for line in f` - a simple expression to read all the lines in a file with the handle `f`
* If a file is opened in `r+` or `w+` mode, after writing, one has to `seek()` to read the contents again
* `shutil.copyfile(file1, file2)` - copies file1 to file2. `file2` cannot be a directory
* `shutil.copy()` - copies like unix cp, where if destination is a directory, then it is copied into it
* `shutil.copy2()` - is like `copy()`, but preserves atime and mtime
* file open modes:

| mode | description | file pointer |
|------|-------------|--------------|
| `r` | opens file in read only mode | start of the file |
| `r+` | opens fie in read and write mode | start of the file |
| `w` | opens file in write mode (truncates existing one) | start of the file |
| `w+` | opens file in read and write mode (no truncate) | start of the file |
| `a` | opens file for writing | end of the file |
| `a+` | opens file in append and read mode | end of the file |

* `f.read()` - reads the entire file as one string and returns it
* `f.readline()` - reads the next line from the file
* `f.readlines()` - returns a list of lines from the file
* `f.write()` - writes a string to the file
* `f.writelines()` - writes a list of lines to the file
* `f.append()` - appends to the file (that is, no need to seek to the end of the file, file has to be opened in either `a` or `a+` mode)
