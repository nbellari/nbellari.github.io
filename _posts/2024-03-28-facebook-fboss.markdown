---
layout: post
title: FBOSS - Building Software Switch at Scale
categories: programming
date:  2024-03-28 15:01:05 +0530
---

Another paper I finished today is Facebook's [FBOSS - Building Software Switch at Scale](https://research.facebook.com/file/579370826398894/FBOSS-Building-Switch-Software-at-Scale.pdf).

In the era of disaggregation - where network boxes (i.e switches/routers) is moving from a closed/proprietary environment to an open environment - where we can now buy a box with a merchant silicon and specification of our choice - how Facebook is also keeping software of their choice which helps them scale in their environment. That's the gist of the paper.

Facebook aimed to streamline software lifecycle process on network bases like that of servers - quick/fast/iterative upgrades etc. It also talks about how vendor software is closed, tightly coupled and not very fast to respond on requirements unless many customers ask for them. With scale, Facebook saw better prospects in writing their own switch software so that it seamlessly integrates with their environment, leverages the existing infrastructure at various levels (logging/monitoring etc.) and also in automating the whole process. One thing I especially liked is their comparison of switch software vs general software (i.e. software for servers) on many metrics/attributes:

![fb-pet-vs-cattle.png]({{site.baseurl}}/assets/images/fboss/fb-pet-vs-cattle.png)

Then the paper goes on to talk about a typical switch system. I really liked the way a typical switch platform is summarized. I tried to capture the whole description in the form of a diagram here:

![fb-switch-system.png]({{site.baseurl}}/assets/images/fboss/fb-switch-system.png)

One thing that was not clear to me in the paper was about dynamic lane mapping of QSFPs to port virtual IDs. It claims it can change port mappings without restarting anything with this, but I am not exactly sure how it works. Need to check that out. In addition to the hardware platform, it also talks about event handlers such as link event handlers (in which many components are interested) and slow path packet handler. While we normally call slow path as host path or control path to process network control related stuff (protocols/exceptions etc.), the paper looks at it as a way to extend switch functionality by processing the packet in the software! Clearly, it is not as fast as the fast path, but still for some cases serves the purpose.

Then the paper talks at a high level about the fboss software architecture. FBOSS integrates with SDK to program the ASIC and also with openBMC to control the board. There is a `HwSwitch` which is an abstraction of switch hardware that provides an interface to configure various stuff on ASIC. This interface is implemented by a hardware abstraction layer that makes calls to the real SDK. Then there is a `SwSwitch` that implements basic L2/L3/ACLs/State management etc. and runs various control protocols. FBOSS implements state as a verisoned CoW (Copy-on-Write) tree that helps it enable high concurrency fast reads. This is similar to a `cmap` in OVS. The states are only created/destroyed - but never modified. That is what makes it immutable. However, it needs more CPU to achieve this.

There is some thrift interface with which a central management system interacts with each and every fboss agent. The paper also talks about the ToR switch being a single point of failure (not sure why). A ToR switch is not taken from the service unless all of the services under ToR switch are also taken. When a switch is to be taken, it receives some specific config (called drained config) from central system and applies the config and restarts BGP. Looks like this is how the ToR goes out of service from the routing system before getting drained!

After this, the paper talks about how they learnt from past mistakes. It makes for a light read. One interesting lesson is that - when Facebook got excited about the fact that a network switch can be brought up just like a server and it can use all the existing infrastructure - it missed the fact that the existing infrastructure depends on the network to bootstrap itself. When such an infrastructure is being brought up on a network switch it wont work because the network itself is down. They finally had to figure other ways to solve this.

A nice paper to read.
