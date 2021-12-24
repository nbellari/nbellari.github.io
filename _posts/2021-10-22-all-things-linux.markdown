---
layout: post
date:  2021-12-24 18:56:49 +0530
title: "All Things Linux"
categories: linux
---

This page covers most of the stuff that is related to Linux - commands, shortcuts, tidits etc. Mostly for my reference.

## Processes

Monitor a process and its children in one dashboard

`top -H -p <pid>`

## RPM Stuff

Find, to which RPM, a file belongs to:

`rpm -qf <file>`

## Byobu

Set scrollback buffer size (in .byobu/.tmux.conf):

`set-option -g history-limit 3000`

Disable renaming of windows by shell:

`set-option -g allow-rename off`

## Sed

Remove empty lines

`sed -i '/^$/d' <input-file>`

Append to the end of the line

`sed 's/$/ xyz/' filename`

## Files

Truncate a file

`truncate -s 0 <file>`

Add quotes around a line

`sed 's/^/"/;s/$/"/' filename`

Append a line to the beginning of the file

`sed '1i <text>' filename`

## Networking

### PCI Device Handling

To see the driver associated with a PCI device:

`lspci -Dk`

To bind/unbind a device to/from a driver:

`echo -n <pci_device_id> > /sys/bus/pci/drivers/e100/[un]bind`

### Sockets

To see all the stats - tcp/udp/raw etc.

`ss -a (or ip netns exec <ns> ss -a for the corresponding namespace)`

### Bonding

To remove a bond interface from the system:

`echo "-bond0" > /sys/class/net/bonding_masters # bond0 is the device name`

To get bonding mode information:

`cat /proc/net/bonding/bondX`

To set bonding mode to static, add the following in `/etc/sysconfig/network-scripts/ifcfg-bondX`:

`BONDING_OPTS="mode=0"`

## Systemd

Get a plot of the dependency of the services

`systemd-analyze plot > boot-analysis.svg`
