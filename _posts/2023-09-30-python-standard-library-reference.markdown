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
* os.getcwd() - get current working directory

## json module

* `json.load(f)` - loads json contents from a file. They will be by default stored as unicode
* `json.loads(str)` - loads a json from string. Still unicode
* `json.dump(f)` - dumps the contents as json into a file
* `json.dumps(type)` - dumps the contents into a string
* Because JSON stores contents as unicode, we need some means to convert it as a normal string for use in the code. This [link](https://stackoverflow.com/questions/956867/how-to-get-string-objects-instead-of-unicode-from-json) from stack overflow gives a very elegant function:
```python
def byteify(input):
    if isinstance(input, dict):
        return {byteify(key): byteify(value)
                for key, value in input.iteritems()}
    elif isinstance(input, list):
        return [byteify(element) for element in input]
    elif isinstance(input, unicode):
        return input.encode('utf-8')
    else:
        return input
```

## argparse

Some terminology

| terminology | description |
| command | the actual command to be executed like `ls`, `ip`, `cat` etc. |
| sub-command | a command with in a command, if the command is complex like `ip addr`, `ip link` etc. |
| argument | the data on which the command is supposed to operate |
| option | something that changes the behavior of the command like `ls -l` |
| parameter | data that is passed to the option itself like `pip -r requirements.txt` |

### Basic steps

* import `argparse`
* Create `ArgumentParser`
* Add options to it with `add_argument`
* call `argument_parser.parse_args` create a **namespace** of arguments. That is, the returned parser has arguments as its attributes

### Details

* the first argument to `add_argument` defines whether it is an argument or an option (which starts with a `-`)
* `ArgumentParser` takes `prog` for displaying custom program name (apart from `sys.argv[0]`), `description` and `epilogue` - something to display at the end (only in the help string!)
* One can segregate arguments with `add_argument_group` first and then `add_argument` into the groups - this will show the help message segrated for those groups
* When the arguments need to be loaded from a file, then `fromfile_prefix_chars=@` needs to be passed to `ArgumentParser` and when the script is invoked with an argument that starts with `@`, it gives a hint to the argument parser to pick that file up for reading the arguments
* By default, the shortest unambiguous prefix of a long argument is enough to specify instead of the full long argument. `allow_abbrev=False` to `ArgumentParser` disables the same
* pass `type` to `add_argument` to convert input to a specific type
* pass `nargs` to specify the number of arguments to be passed
* `default` will take the default value for an argument and `choices` will give a list of allowable values
* we can give `help=` as well to `add_argument` so that help displays description of each argument
* In addition to `add_argument_group`, we can also have `add_mutually_exclusive_group` where options added to it are mutually exclusive
* Any long options like `a-b-c` is automatically converted into `a_b_c` after parsing to be used as an attribute

### Adding Sub Commands

* create sub parsers with `parser.add_subparsers()`
* create a parser on top it for one sub command each with `subparsers.add_parser(cmd)`
* like any other command add arguments to the new parser with `add_argument`
* One can add  handler for that new subcommand with `subparser.set_defaults(func=foo)`
* Once we call `parser.parse_args`, `func` will be set to the default handler. Then we can call `args.func(args)` - `args` will contain other attributes as arguments
* When we have sub commands, we can give `-h` both on the top level as well as at the sub command level

Different action arguments:

| action  | description |
| `store` | stores the given value, default action |
| `store_const` | stores a constant value, mentioned by `const=x` |
| `append` | appends a value to a list each time the option is specified |
| `count` | counts the number of times an option is specified |
| `append_const` | same as `append` with `store_const` |
| `store_true` and `store_false` | stores boolean values |
| version | specifies the version and exits. Must be passed through `version=x.y.z` |


## String Manipulation

### String Module

* `string.capwords(s)` - capitalize words in a sentence
* Different ways of string manipulation

| Method | Description |
| `string.Template` | Instantiate a `string.Template` with a string containing variables mentioned as `$var` or `${var}`. Then have a dictionary of variables and their values and pass them to the template with `t.substitute(dict)`. `safe_substitute()` does not raise any exceptions incase some keys are missing |
| interpolation with `%` | string has `%(var)<num>{s|d|f}` and it is interpolated with a dictionary which contains `var` as one if its keys |
| with `format` method | string has `{var}` in it and `format` is called with keyword arguments for the form `var=val` to replace it in the string |

* template based substitution is not as rich as interpolation or format method when it comes to formatting output.
* `format` method is very rich in specifying the format
* Ways of specifying the variables

| `{foo} has {bar}` | the method takes keyword based arguments where the keywords are `foo` and `bar` - order of arguments is not important |
| `{0} has {1}` | the method takes nameless arguments, first one being `0`. Order of arguments is important |
| `{} has {}` | same as above |

* ways of specifying formatting types

| `:<` | left aligned |
| `:>` | right aligned |
| `:^` | center aligned |
| `:<num>{b|f|o|x|X}` | print the number in binary, float, octal, lower hex, upper hex respectively |
|`{!r}` | user `repr` method to display |
| `{!s}` | use `str` method to display |
