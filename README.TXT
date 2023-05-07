------------------------------------------------------------------------
FreeGEM/XM                                              Release 3.0beta4
Source distribution                                       Build 20000406
README.TXT - General information on the modifications made to FreeGEM/XM
------------------------------------------------------------------------
Copyright (C) Digital Research, Inc. 1985 - 1986.
          (C) Caldera Thin Clients, Inc. 1999.
          (C) FreeGEM Programmers, 2000.
          This software is released under the GNU General Public License
------------------------------------------------------------------------

This archive contains the latest modifications to DRI's illfated GEM/XM
project.

Partial sources to the code were found in the collection Gene Buckle
obtained from Caldera Thin Clients, Inc. in 1999.  A magazine or DRI
leaflet was also found by Alec Beanse, and sent to the gem-dev list
along with a scanned image (see the files XMGEM.TXT and GEMXM.JPG in
the ORIG directory of this distribution).  The binaries were quickly
installed and tested, and although the system was perfectly capable
of running multiple GEM applications, the support for DOS applications
was somewhat lacking - the majority of test systems refused to let DOS
see the keyboard, rendering the system unusable once a DOS app had been
started.

The files you find in this archive are almost identical to the original
DRI binaries, except for the DOS keyboard handler - the change was so
ridiculously simple I'm surprised no-one had noticed it already!  (See
the INTS.A86 file in the SOURCE\GDOS directory - grep on 'gd_int16').
I've included the following:

\               - Root of this archive
+ GEMXM         - A full FreeGEM/XM installation
|  + GEMAPPS    - Filter table constructor
|  + GEMBOOT    - AES and Desk Accessories
|  + GEMDESK    - FreeGEM/XM Desktop
|  + GEMSCRAP   - Clipboard directory
|  + GEMSYS     - Fixed GDOS, VGA display driver (using MOUSE.COM)
+ ORIG          - Original information on GEM/XM
+ SOURCE        - All available sources for FreeGEM/XM
|  + AES        - FreeGEM/XM Application Environment Services
|  + DESKTOP    - FreeGEM/XM Desktop
|  + GDOS       - FreeGEM/XM Graphical Device Operating System
+ MISC          - Some files that may be useful

The FreeGEM/XM setup in the GEMXM directory is verified to work fine
on this machine (P2-400, 64Mb, Win98), my other development machine
(P133, 40Mb, OpenDOS7.02b2), and my first GEM machine (Amstrad PC1512).
The directory structure is the same as that of GEM/1 and 2, and the VDI
is very similar to those - so don't mess around with it or things may
get confused.  As the VDI reads ASSIGN.SYS using FCBs, it may not work
with FreeDOS or MS-DOS 7 (unless the Win9x GUI is running).

Also, please note that the GEMXM directory should ideally be SUBSTed
to another drive before FreeGEM/XM is started:

C:\XMDIST>SUBST F: GEMXM
C:\XMDIST>F:
F:\>GEM

This is because XM expects things to be in the root.  Also, FreeGEM/XM's
swap support is configured in this distribution to use the highest drive
letter found as swap space, so you may want to do something like:

C:\XMDIST>MKDIR XMSWAP
C:\XMDIST>SUBST Z: XMSWAP

The two SUBSTed drives can be later removed:
C:\>SUBST /D F:
C:\>SUBST /D Z:

You may also find that DOS Copy/Paste support is nonfunctional unless a
C:\GEMSYS directory exists.  On my system, the Copy operation generates
Write error writing to device AUX:
Abort, Retry, Fail?
while trying to copy the screen data to my AUX: port (my modem did not
enjoy discovering this :) ).  Creating C:\GEMSYS fixed this problem, and
the Copy/Paste options work as expected.

The binaries in this distribution are configured to display the DOS menu
when the '+' key on the numeric keypad is hit.  From there, you can switch
to any other running application, or Copy/Paste data between DOS apps (I
don't think it's possible to copy to a GEM application - I can't seem to
make it work).  Just use the arrow keys and Enter.  To Copy text, select
the COPY item, and then select the area on screen as follows:
*  Use the cursors to move to the top left corner of the block.
*  Press 'End'.
*  Use the cursors to move the inverse rectangle to cover the entire block.
*  Press 'Home'.
*  Select 'COPY MARKED AREA'.

You may find some applications refuse to display the popup menu on '+'.
This is because they have taken control of the keyboard for themselves
(I noticed this problem with any Win9x utility - I think the GUI's new
INT 21h handler refuses to be intercepted or something - I wasted many
hours trying to make XM recognise the keystroke, while all along I just
needed to test the real code with DEBUG.  Gah).  Any application which
behaves properly should work fine - the Pacific C PPD.EXE and Infocom's
HitchHiker's Guide to the Galaxy worked here.

For details of the modifications I've made to each component, please see
the README.TXT files under each SOURCE subdirectory.

Happy hacking!

Regards,
Ben A L Jemmett
6th April, 2000.
