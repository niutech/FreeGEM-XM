------------------------------------------------------------------------
FreeGEM/XM                                              Release 3.0beta4
Source distribution                                       Build 20000406
README.TXT - Information on the FreeGEM/XM GDOS source code.
------------------------------------------------------------------------
Copyright (C) Digital Research, Inc. 1985 - 1986.
          (C) Caldera Thin Clients, Inc. 1999.
          (C) FreeGEM Programmers, 2000.
          This software is released under the GNU General Public License
------------------------------------------------------------------------

This directory contains the source to the FreeGEM/XM GDOS/GEMVDI.

The GDOS in XM contains, in addition to the actual graphics code, the
task switching code and keyboard handler for DOS applications.  The
main modifications I made to the GDOS are in the INTS.A86 file, under
the 'gd_int16' section.  The IRET after an unintercepted function has
been replaced with a far JMP to the BIOS's keyboard code, which fixed
the problem with DOS applications not recognising the keyboard, while
a small section has been optimised out because it was redundant (only
added a few bytes, but it was still redundant).

I also added a FreeGEM/XM copyright banner to the code.  Please note
that the GDOS.OBJ and LOAD.OBJ files (the graphics code and the loader
code) do not appear to have source, so have been copied out to the
OBJWOSRC directory.  This also means that, until someone can coax the
GEM/3, ViewMAX or FreeGEM VDI equivalents into a form usable by this
code, we're stuck with the GEM/2 VDI and ASSIGN.SYS.

Regards,
Ben A L Jemmett
6th April, 2000.
