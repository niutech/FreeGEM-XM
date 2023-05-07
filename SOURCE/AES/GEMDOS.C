/*	GEMDOS.C	4/18/84 - 03/07/85	Lee Lorenzen		*/
/*	for 3.0		6/23/86			MDF			*/

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                                                       
*                  Historical Copyright  
*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 3.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1986			Digital Research Inc.
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


	WORD
dos_chdir(pdrvpath)
	LONG		pdrvpath;
{
	dos_func(0x3b00, pdrvpath);
}


	WORD
dos_gdir(drive, pdrvpath)
	WORD		drive;
	LONG		pdrvpath;
{
	DOS_AX = 0x4700;
	DOS_DX = (UWORD) drive;
	DOS_SI = LLOWD(pdrvpath);
	DOS_DS = LHIWD(pdrvpath);

	__DOS();

	return(TRUE);
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
dos_term()
{
	DOS_AX = 0x4c00;
	__DOS();
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


	WORD
dos_sfirst(pspec, attr)
	LONG		pspec;
	WORD		attr;
{
	DOS_CX = attr;

	dos_func(0x4e00, pspec);
	return(!DOS_ERR);
}


	WORD
dos_snext()
{
	DOS_AX = 0x4f00;

	__DOS();

	return(!DOS_ERR);
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

	WORD
dos_delete(ppath)
	LONG		ppath;
{
	dos_func(0x4100, ppath);
	return(!DOS_ERR);
}


	WORD
dos_read(handle, cnt, pbuffer)
	WORD		handle;
	WORD		cnt;
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


	VOID
dos_exec(pcspec, segenv, pcmdln, pfcb1, pfcb2)
	LONG		pcspec;
	WORD		segenv;
	LONG		pcmdln;
	LONG		pfcb1;
	LONG		pfcb2;
{
	EXEC_BLK	exec;

	exec.eb_segenv = segenv;
	exec.eb_pcmdln = pcmdln;
	exec.eb_pfcb1 = pfcb1;
	exec.eb_pfcb2 = pfcb2;

	DOS_AX = 0x4b00;
	DOS_BX = LLOWD(ADDR(&exec));
	DOS_ES = LHIWD(ADDR(&exec));
	DOS_DX = LLOWD(pcspec);
	DOS_DS = LHIWD(pcspec);

	__EXEC();
}


	WORD
dos_wait()
{
	DOS_AX = 0x4d00;
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


/*
*	Returns the amount of memory available in bytes
*/
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

#if MULTIAPP

	WORD
dos_stblk(blockseg, newsize)
	UWORD		blockseg;
	UWORD		newsize;		/* in paragraphs	*/
{
	DOS_AX = 0x4a00;
	DOS_ES = blockseg;
	DOS_BX = newsize;
	
	__DOS();

	return(DOS_AX);
}

#endif
/************************************************************************/
/*	ONLY USED BY THE DESKTOP					*/
/************************************************************************/

#if DESKTOP
	WORD
dos_label(drive, plabel)
	BYTE		drive;
	BYTE		*plabel;
{
	BYTE		label_buf[128];
	BYTE		ex_fcb[40];
	WORD		i;

	dos_sdta(ADDR(&label_buf[0]));
	ex_fcb[0] = 0xff;
	bfill(5, 0, &ex_fcb[1]);
	ex_fcb[6] = 0x08;		/* volume label	*/
	ex_fcb[7] = drive;
	bfill(11, '?', &ex_fcb[8]);
	bfill(21, 0, &ex_fcb[19]);

	dos_func(0x1100, ADDR(&ex_fcb[0]));

	if ( (DOS_AX & 0x00ff) == 0xff )
	  *plabel = NULL;
	else
	{
	  label_buf[19] = 0x0;
	  strcpy(&label_buf[8], plabel);
	}
}

	WORD
dos_space(drv, ptotal, pavail)
	WORD		drv;
	LONG		*ptotal, *pavail;
{
	DOS_AX = 0x3600;
	DOS_DX = drv;
	__DOS();
	
	DOS_AX *= DOS_CX;
	*ptotal = (LONG) DOS_AX * (LONG) DOS_DX;
	*pavail = (LONG) DOS_AX * (LONG) DOS_BX;
}


	WORD
dos_rmdir(ppath)
	LONG		ppath;
{
	dos_func(0x3a00, ppath);
	return(!DOS_ERR);
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
dos_mkdir(ppath, attr)
	LONG		ppath;
	WORD		attr;
{
	dos_func(0x3900, ppath);
	return(!DOS_ERR);
}


	WORD
dos_delete(pname)
	LONG		pname;
{
	dos_func(0x4100, pname);
	return(DOS_AX);
}


	WORD
dos_rename(poname, pnname)
	LONG		poname;
	LONG		pnname;
{
	DOS_DI = LLOWD(pnname);
	DOS_ES = LHIWD(pnname);
	dos_func(0x5600, poname);
	return(DOS_AX);
}


	WORD
dos_write(handle, cnt, pbuffer)
	WORD		handle;
	WORD		cnt;
	LONG		pbuffer;
{
	DOS_CX = cnt;
	DOS_BX = handle;
	dos_func(0x4000, pbuffer);
	return(DOS_AX);
}


	WORD
dos_chmod(pname, func, attr)
	LONG		pname;
	WORD		func;
	WORD		attr;
{
	DOS_CX = attr;
	dos_func(0x4300 + func, pname);
	return(DOS_CX);
}


	WORD
dos_setdt(handle, time, date)
	WORD		handle, time, date;
{
	DOS_AX = 0x5701;
	DOS_BX = handle;
	DOS_CX = time;
	DOS_DX = date;

	__DOS();
}

#endif
