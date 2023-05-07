CLS
ECHO Welcome to GEMPREP!  This procedure will install GEM 3.0
ECHO on your computer.
ECHO          
GETYESNO Do you want to put the GEM Desktop on a hard disk?  Y/N?
IFNO GEMFLOPY
CLS
GETYESNO Is your hard disk Drive C? Y/N?
IFYES NOHDPAT
ECHO Type your hard disk drive letter ( A-Z ):  
GETDRV
PATBAT C:
:NOHDPAT
CLS
MD C:\GEMBOOT
MD C:\GEMDESK
MD C:\GEMSYS
MD C:\GEMAPPS
MD C:\GEMSCRAP
SRCMSG Insert your GEM SYSTEM MASTER DISK into Drive A.
DEL C:\GEMBOOT\DESK1.ACC
DEL C:\GEMSYS\*.*
DEL C:\GEMDESK\*.*
COPY A:GEM.EXE C:\GEMBOOT
COPY A:GEM.RSC C:\GEMBOOT
GDOSINST.EXE C:
COPY A:*.ACC C:\GEMBOOT
COPY A:FILTER.* C:\GEMAPPS
CLS
ECHO Remove the GEM SYSTEM MASTER DISK from Drive A.
ECHO Insert the DEVICE DRIVER DISK #1 into Drive A.
ECHO Press any key to continue.
PAUSE
SRCMSG Insert the DEVICE DRIVER DISK #1 into Drive A.
CLS
COPY A:ASSIGN.SYS C:\GEMSYS
COPY A:METAFIL6.SYS C:\GEMSYS
CLS
SCRNSTAL.EXE C:
CLS
IFERR 2 HDMS
GOTO HDOK
:HDMS
CLS
SRCMSG Insert your Microsoft Mouse disk into Drive A.
COMP
COPY A:MOUSE.COM C:\GEMBOOT
CLS
:HDOK
SRCMSG Insert the GEM SYSTEM MASTER DISK into Drive A.
COPY A:desktop.app C:\gemdesk
COPY A:DESKTOP.RSC C:\GEMDESK
COPY A:desktop.inf C:\gemdesk
COPY A:output.app C:\gemsys
COPY A:output.rsc C:\gemsys
COPY A:desklo.icn C:\gemdesk
COPY A:DESKHI.ICN C:\GEMDESK
CLS
SRCMSG Insert a DOS disk containing COMMAND.COM into Drive A.
COMP
COPY A:\COMMAND.COM C:\COMMAND.COM
SRCMSG Insert a DOS disk containing FORMAT.COM into Drive A.
COMP
COPY A:\FORMAT.COM C:\FORMAT.COM
SRCMSG Insert a DOS disk containing MODE.COM into Drive A.
COMP
COPY A:\MODE.COM C:\MODE.COM
CLS
ECHO This concludes GEMPREP.
ECHO
ECHO If you wish to use the GEM Desktop, remove the disk from
ECHO Drive A and restart your system (Ctrl-Alt-Del).  Type GEM
ECHO at the DOS prompt and then press the Enter key.
ECHO
GOTO DONE
:GEMFLOPY
CLS
ECHO GEM 3.0 requires a hard disk.
ECHO Press any key to continue.
PAUSE
:DONE
:OUT
DOS
