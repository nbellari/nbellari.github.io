---
layout: post
title: "On writing a REPL shell"
date:  2021-12-24 19:03:20 +0530
categories: programming
---

## Introduction

I have finally decided to do some DIY projects so that I regain my level of comfort with coding. Towards that end, I decided to start with [Stephen Brennan](https://brennan.io/)'s famous tutorial on [how to write a shell](https://brennan.io/2015/01/16/write-a-shell-in-c/)

## Details

When I started coding for the shell, I realized there are a few subtleties that needed some consideration. The basic function of a shell can be categorized into the following key elements:

1. read the input from the shell
2. parse the input into command and arguments
3. execute the command


### Read a arbitrarily long command line

In higher level languages, this is not an issue, but in a language like C where memory allocation is explicit, this has to be handled properly. A long global or static array looks to do the trick, but it is not good coding practice. One can always write a test case that exceeds the array size and cause the program to crash.

Then what is the correct way to read the command line into memory? The right approach is to dynamically allocate a chunk of memory, start reading into it and when it is full, grow the chunk by reallocating the same memory. Here is the rough code skeleton:


```c
    int c; /* to store the current character */
    int position; /* current position in the buffer */

    char *buffer = malloc(sizeof(char) * MYSH_RL_BUF_SIZE);
    int bufsize = MYSH_RL_BUF_SIZE;


    position = 0;
    while (1) {
        c = getchar();

        buffer[position++] = c;

        /* Check if the buffer needs expansion */
        if (position >= bufsize) {
            bufsize = bufsize + MYSH_RL_BUF_SIZE;
            buffer = realloc(buffer, bufsize);
        }
    }

```

### Tokenizing a arbitrarily long command line

We encounter the same problem when we begin to tokenize the long command line. We may have a long global or static array of pointers to tokens, but it only helps so much. The right approach is to do the same thing as above. Here is the rough code skeleton:

```c
    int  num_max_tokens = MYSH_MAX_TOKENS;
    char **tokens = malloc(num_max_tokens * sizeof(char *));
    char *token;
    int position = 0;

    token = strtok(line, MYSH_TOKEN_DELIMS);
    while (token != NULL) {
        tokens[position++] = token;

        /* Extend the token stream, if needed */
        if (position >= num_max_tokens) {
            num_max_tokens += MYSH_MAX_TOKENS;
            tokens = realloc(tokens, num_max_tokens * sizeof(char *));
        }
    }

```

### Other stuff

Apart from this, the rest of the stuff in building a REPL shell is not nuanced and is pretty straightforward.
