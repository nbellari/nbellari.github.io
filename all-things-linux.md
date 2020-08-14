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
