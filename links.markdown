---
layout: page
title: "Links"
permalink: /links/
---

### On identities

David Crawshaw [explains nicely](https://tailscale.com/blog/asymmetry-identity/) how **identity** is defined/represented/used in the Internet. I especially liked the terminology used to depict the identity model - that there are **people**, who use the service, **brand**s which provide the required service. That **people** can communicate only with the **brand** or **people** can communicate with other **people** via the **brand** (which acts as a platform). How the two entities **people** and **brand** establish each others' identity and what are the different means to do that. Interesting read. It also highlights that even today **people** talking to **people** without the need for a **brand** is messy!


### On Dynamic Memory Allocation

This set of [slides](https://my.eng.utah.edu/~cs4400/malloc.pdf) has a decent in-depth coverage of different ways of doing dynamic memory allocation. What if you do only mmap for dynamic memory? How does it compare with malloc? What are the things to consider when one wants to design a memory allocator? Very nice read!

Other than talking about the dynamic memory management, this set of slides also highlight a methodology. A design methodology. There are a set of design questions that are posed. Then all the alternatives are considered and the design questions are answered for each alternative - so as to clearly see how each design alternative fares.
