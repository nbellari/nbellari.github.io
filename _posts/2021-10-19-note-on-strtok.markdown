---
layout: post
title: "A Note on strtok"
date:  2021-12-24 19:00:15 +0530
categories: programming
---

The man page of `strtok` describes it as non-reentrant. What does that mean? We deal with strtok, unlike most other functions, not in just one invocation, but in a series of invocations. We initialize the string to be tokenized in the first call and then we keep calling `strtok` until it returns `NULL`

This set of calls to `strtok` is non-reentrant. That means, while we are in the middle of tokenizing a string, we cannot start tokenizing another string. That way, it is non-reentrant. Is it thread safe? Can `strtok` be called from multiple threads simultaneously? From what it sounds, `strtok` is kind of maintaining some local static variable to keep the context and so, it is not thread safe either.

`strtok_r` instead, lets us to save the context in a pointer and pass it everytime we call `strtok_r` so, this is re-entrant and thread safe as well, assuming we are not passing some global variable to store the context across multiple invocations or different threads.

Locks can be kept inside to make sure thread safety is achieved, but locks cannot be used for re-entrancy, it can result in deadlock! A function that is completely stateless is both re-entrant and thread safe!

There is also async-signal-safe! Signals can happen at unusual times and if a function is called in the context of a signal, then it could be potentially called twice in the same context because a signal handler got invoked in the same context. So, if a function is re-entrant, then it could be technically async-signal-safe.

Re-entrancy, then is the strongest check for a function!
