README
------

For installation instructions see INSTALL.

vcp copies files and directories in a curses interface, with text only
output available. its options and output are similar to BSD's cp while
adding some new features.
It provides information on:
- files copied and left to copy
- data written and total data size
- data being written every second
- two status bars, one showing current file status, the other total status
  (except with 1 file, both show current), and percentage
when output is sent to the console:
- a status bar
- size copied and speed

The config file (vcp.conf.sample) supports a few options,
- color
- screen state
- default flags
- read bufer size
vcp checks the following files in order:
1. /etc/vcp.conf
2. ~/.vcp
options are overwritten as they are read.

## Original
Fork from Daniel<sisko@bsdmail.com>'s http://members.iinet.net.au/~lynx/vcp
