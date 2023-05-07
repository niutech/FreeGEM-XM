/*	GEMDOS.C	4/18/84 - 03/07/85	Lee Lorenzen		*/


/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 1.1
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1985			Digital Research Inc.
*	-------------------------------------------------------------
*/

#include <portab.h>
#include <machine.h>
#include <dos.h>

#define DESKTOP 0
						/* in DOSIF.A86		*/
EXTERN WORD	__DOS();
EXTERN WORD	__EXEC();

GLOBAL UWORD	DOS_AX;
GLOBAL UWORD	DOS_BX;
GLOBAL UWORD	DOS_CX;
GLOBAL UWORD	DOS_DX;
GLOBAL UWORD	DOS_DS;
GLOBAL UWORD	DOS_ES;
GLOBAL UWORD	DOS_SI;
GLOBAL UWORD	DOS_DI;
GLOBAL UWORD	DOS_ERR;

	VOID
dos_func(ax, lodsdx, hidsdx)
	UWORD		ax;
	UWORD		lodsdx;
	UWORD		hidsdx;
{
	DOS_AX = ax;
	DOS_DX = lodsdx;
	DOS_DS = hidsdx;

	__DOS();
}

dos_gcdir(drive, lobuf, hibuf)
	BYTE	drive;
	UWORD	lobuf;
	UWORD	hibuf;
{
	DOS_AX = 0x4700;
	DOS_DX = drive;
	DOS_SI = lobuf;
	DOS_DS = hibuf;

	__DOS();

	return(DOS_AX);
}

dos_scdir(pbuffer)
	LONG	pbuffer;
{
	dos_func(0x3b00, pbuffer);

	return(DOS_AX);
}

	WORD
dos_gdrv()
{
	DOS_AX = 0x1900;

	__DOS();
	return(DOS_AX & 0x00ff);
}
	WORD
dos_sdrv(newdrv)
	WORD		newdrv;
{
	DOS_AX = 0x0e00;
	DOS_DX = newdrv;

	__DOS();

	return(DOS_AX & 0x00ff);
}
	WORD
dos_sdta(ldta)
	LONG		ldta;
{
	dos_func(0x1a00, ldta);
}
	LONG
dos_gdta()
{
	dos_func(0x2f00, LLDS() );
	return( HW(DOS_ES) | LW(DOS_BX) );
}
	WORD
dos_gpsp()
{
	DOS_AX = 0x5100;

	__DOS();
	return(DOS_BX);
}
	WORD
dos_spsp(newpsp)
	WORD		newpsp;
{
	DOS_AX = 0x5000;
	DOS_BX = newpsp;

	__DOS();

	return(DOS_AX);
}
	LONG
dos_alloc(nbytes)
	LONG		nbytes;
{
	LONG		maddr;

	DOS_AX = 0x4800;
	if (nbytes == 0xFFFFFFFFL)
	  DOS_BX = 0xffff;
	else
	  DOS_BX = (nbytes + 15L) >> 4L;

	__DOS();

	if (DOS_ERR)
	  maddr = 0x0L;
	else
	  maddr = HW(DOS_AX) & 0xFFFF0000L;

	return(maddr);
}
	LONG
dos_avail()
{
	LONG		mlen;

	DOS_AX = 0x4800;
	DOS_BX = 0xffff;

	__DOS();

	mlen = ((LONG) DOS_BX) << 4;
	return(mlen);
}
	WORD
dos_free(maddr)
	LONG		maddr;
{
	DOS_AX = 0x4900;
	DOS_ES = LHIWD(maddr);

	__DOS();

	return(DOS_AX);
}
	WORD
dos_create(pname, attr)
	LONG		pname;
	WORD		attr;
{
	DOS_CX = attr;
	dos_func(0x3c00, pname);

	return(DOS_AX);
}
	WORD
dos_delete(pname)
	LONG		pname;
{
	dos_func(0x4100, pname);
	return(DOS_AX);
}
	WORD
dos_open(pname, access)
	LONG		pname;
	WORD		access;
{
	dos_func(0x3d00 + access, pname);

	return(DOS_AX);
}
	WORD
dos_close(handle)
	WORD		handle;
{
	DOS_AX = 0x3e00;
	DOS_BX = handle;

	__DOS();

	return(!DOS_ERR);
}
	UWORD
dos_write(handle, cnt, pbuffer)
	WORD		handle;
	UWORD		cnt;
	LONG		pbuffer;
{
	DOS_CX = cnt;
	DOS_BX = handle;
	dos_func(0x4000, pbuffer);
	return(DOS_AX);
}
	UWORD
dos_read(handle, cnt, pbuffer)
	WORD		handle;
	UWORD		cnt;
	LONG		pbuffer;
{
	DOS_CX = cnt;
	DOS_BX = handle;
	dos_func(0x3f00, pbuffer);
	return(DOS_AX);
}
	LONG
dos_lseek(handle, smode, sofst)
	WORD		handle;
	WORD		smode;
	LONG		sofst;
{
	DOS_AX = 0x4200;
	DOS_AX += smode;
	DOS_BX = handle;
	DOS_CX = LHIWD(sofst);
	DOS_DX = LLOWD(sofst);

	__DOS();

	return(DOS_AX + HW(DOS_DX) );
}
