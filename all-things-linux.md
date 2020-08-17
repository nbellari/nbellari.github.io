# All Things Linux

This page covers most of the stuff that is related to Linux - commands, shortcuts, tidits etc. Mostly for my reference.

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

Add quotes around a line

`sed 's/^/"/;s/$/"/'`

## Networking

### PCI Device Handling

To see the driver associated with a PCI device:

`lspci -Dk`

To bind/unbind a device to/from a driver:

`echo -n <pci_device_id> > /sys/bus/pci/drivers/e100/[un]bind`
