---
layout: post
title: OpenvSwitch Notes
categories: networking programming
date:  2023-05-20 14:13:05 +0530
---

This blog post will be about all the random tidbits about openvswitch that I have encountered so far.

Not a big deal, but in one of the [earlier ovs-dev thread](https://mail.openvswitch.org/pipermail/ovs-dev/2009-August/243630.html), there is a note about openvswitch being a distributed switch - something to note:

> Open vSwitch is not, on its own, a distributed switch.  The distribution of Open vSwitch is implemented through OpenFlow.  An OpenFlow controller, such as NOX, is required.


