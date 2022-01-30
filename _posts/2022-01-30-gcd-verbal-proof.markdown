---
layout: post
title: Verbal Proof of GCD
categories: programming
date: 2022-01-30 12:29:36 +0530
---

How do you prove that `gcd(m, n)` is equal to `gcd(n, m mod n)`?

In the case when `n` is 0, `m` is the gcd, which is a fairly simple base case. In the case `n` divides `m`, then `n` is the gcd. That is also fairly clear.

However, when `n` does not evenly divide `m`, we can visualize as having two groups - one group is the count where `n` evenly divides `m`, and the other group which is less than `n` (the remainder of `m` after taking out the first group). Now, for the gcd to divide the two numbers `m` and `n` - we can paraphrase the same condition as the gcd dividing the two groups of `m` - one group is a multiple of `n` and the other is the residue after dividing `m` by `n`, since `n` is now in one of the groups.

So any number that divides `m` and `n` will also divide `n` and `m mod n`
