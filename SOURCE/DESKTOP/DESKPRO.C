/*	DESKPRO.C	4/18/84 - 03/19/85	Lee Lorenzen		*/
/*	for 3.0		3/11/86 - 10/23/86	MDF			*/

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*	-------------------------------------------------------------
*	GEM Desktop					  Version 2.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1985			Digital Research Inc.
*	-------------------------------------------------------------
*/

#include <portab.h>
#include <machine.h>
#include <obdefs.h>
#include <deskapp.h>
#include <deskfpd.h>
#include <deskwin.h>
#include <dos.h>
#include <infodef.h>
#include <desktop.h>
#include <gembind.h>
#include <deskbind.h>

#if MULTIAPP
GLOBAL WORD	pr_kbytes;
GLOBAL LONG	pr_beggem;		/* first paragraph of AES	*/
GLOBAL LONG	pr_begacc;		/* first paragraph of acces.	*/
GLOBAL LONG	pr_begdsk;		/* first paragraph of desktop	*/
GLOBAL LONG	pr_topdsk;		/* first paragraph above desktop  */
GLOBAL LONG	pr_topmem;		/* next paragraph above free area */
GLOBAL LONG	pr_ssize;		/* size of channel system overhead */
GLOBAL LONG	pr_itbl;		/* base of GDOS default int table  */
EXTERN WORD	gl_fmemflg;
#endif

EXTERN WORD	do_wopen();
EXTERN WORD	dos_gdrv();
EXTERN WORD	dos_sdrv();
EXTERN WORD	dos_chdir();
EXTERN WORD	shel_envrn();
EXTERN WORD	strlen();
EXTERN WORD	graf_mouse();
EXTERN WORD	shel_write();

EXTERN WORD	gl_stdrv;
EXTERN WORD	DOS_ERR;
EXTERN GLOBES	G;


#if MULTIAPP

#define LMIN(x,y) ((x)<(y)?(x):(y))


	/* in pro_chcalc long addresses are flattened out with no segment */

	VOID
pro_chcalc(appsize, begaddr, chsize)
	LONG		appsize;
	LONG		*begaddr;
	LONG		*chsize;
	
{
	static LONG	begfree = 0l;
	LONG		maxmem;

	if (appsize == -1)			/* full step	*/
	{
	  *begaddr = LSEGOFF(pr_beggem); 
	  *chsize = pr_topmem - pr_beggem;
	  return;
	}
	if ((begfree >= pr_topmem) || (begfree < pr_topdsk))
	  begfree = pr_topdsk;	
	
	maxmem = pr_topmem - pr_begdsk;
	*chsize = LMIN(appsize+pr_ssize, maxmem);

	if ((begfree + *chsize) < pr_topmem)
	  *begaddr = LSEGOFF(begfree);
	else
	  *begaddr = LSEGOFF(pr_topmem - *chsize);
	begfree += *chsize;	
}
#endif


	WORD
pro_chdir(drv, ppath)
	WORD		drv;
	BYTE		*ppath;
{
	WORD		tmpdrv;
						/* change to directory	*/
						/*   that application	*/
						/*   is in		*/
	if (!drv)
	  return( (DOS_ERR = TRUE) );

	if ( drv != '@' ) 
	{
	  tmpdrv = dos_gdrv();
	  dos_sdrv(drv - 'A');
	  if (DOS_ERR)
	  {
	    dos_sdrv(tmpdrv);
	    return(FALSE);
	  }
	  G.g_srcpth[0] = drv;
	  G.g_srcpth[1] = ':';
	  G.g_srcpth[2] = '\\';
	  strcpy(ppath, &G.g_srcpth[3]);
	  dos_chdir(ADDR(&G.g_srcpth[0]));
	}
	else
	  dos_sdrv(gl_stdrv);		/* don't leave closed drive hot	*/
	return(TRUE);
} /* pro_chdir */

	WORD
pro_cmd(psubcmd, psubtail, exitflag)
	BYTE		*psubcmd, *psubtail;
	WORD		exitflag;
{
	LONG		lp;
	WORD		ii, drv;
	BYTE		save_ch;

	shel_envrn(ADDR(&lp), ADDR("COMSPEC="));
	if (lp)
	{
	  LSTCPY(ADDR(&G.g_cmd[0]), lp);
/* BugFix	*/
	  if (!exitflag)
	  {
	    ii = 0;
	    while(G.g_cmd[ii] != '\\')	/* find first backslash		*/
	      ii++;
				/* change to drive specified by COMSPEC	*/
	    drv = G.g_cmd[ii - 2] - 'A';
	    dos_sdrv(drv);
				/* chdir to path specified by COMSPEC	*/
	    ii++;			/* get char past backslash	*/
	    save_ch = G.g_cmd[ii];	/* save it for later		*/
	    G.g_cmd[ii] = NULL;		/* make a null-term. string	*/
	    dos_chdir(ADDR(&G.g_cmd[0]));  /* change to that dir.	*/
	    G.g_cmd[ii] = save_ch;	/* put the char back		*/
	  }
/* */
	  if (exitflag)
	  {
#if PCDOS
	    strcpy("/C ", &G.g_tail[1]);
#endif
	    strcat(psubcmd, &G.g_tail[1]);
	    strcat(" ", &G.g_tail[1]);
	    strcat(psubtail, &G.g_tail[1]);
	  }
	  else
	    G.g_tail[1] = NULL;
	  return(TRUE);
	} /* if lp */
	else
	  return(FALSE);
} /* pro_cmd */

	WORD
pro_run(isgraf, isover, wh, curr)
	WORD		isgraf, isover;
	WORD		wh, curr;
{
	WORD		ret, len, i;

	G.g_tail[0] = len = strlen(&G.g_tail[1]);
	if ( (len) && (!isgraf) )
	{
	  for(i = len; i; i--)
	    G.g_tail[i+1] = G.g_tail[i];
	  G.g_tail[1] = ' ';
	  len++;
	} /* if */
	G.g_tail[0] = len;
	G.g_tail[len+1] = 0x0D;
#if MULTIAPP
	do_wopen(FALSE, wh, curr, G.g_xdesk, G.g_ydesk, G.g_wdesk, G.g_hdesk);
#endif
	ret = pro_exec(isgraf, isover, G.a_cmd, G.a_tail);
	if (isover == -1)
	  ret = FALSE;
	else
	{
	  if (wh != -1)
	    do_wopen(FALSE, wh, curr, G.g_xdesk, G.g_ydesk,
	    	     G.g_wdesk, G.g_hdesk);
	} /* else */
	return(ret);
} /* pro_run */

	WORD
pro_exec(isgraf, isover, pcmd, ptail)
	WORD		isgraf;
	WORD		isover;
	LONG		pcmd, ptail;
{
	WORD		ret;
#if MULTIAPP
	WORD		chnum;
	LONG		begaddr, csize;

	if (isover != 3)
#endif
	graf_mouse(HGLASS, 0x0L);

#if MULTIAPP
	if ((isover == -1) || (isover == 2) || (isover == 3))
	{
	  if (isover == 2)			/* full step	*/
	    pro_chcalc((LONG)-1, &begaddr, &csize);
	  else
	    pro_chcalc((LONG)pr_kbytes << 10, &begaddr, &csize);
/*
dbg("NEW START ADDRESS = %X\r\n", begaddr);
dbg("NEW CHANNEL SIZE  = %X\r\n", csize);
*/
	  ret = proc_create(begaddr, csize, 1, isgraf, &chnum);
	  if (!ret)
	  {
	    fun_alert(1,STNOROOM,NULLPTR);
	    return(FALSE);
	  }
	  if (isover == 2)
	    gl_fmemflg |= (1 << chnum);

/*
dbg("CREATE = %d\r\n", ret);
dbg("CHNUM  = %d\r\n", chnum);
dbg("PCMD   = %s\r\n", pcmd);
dbg("PTAIL  = %s\r\n", ptail);
*/
	  ret = proc_run(chnum, isgraf, isover, pcmd, ptail);
	  if (isover==3)
	    ret = 0;
	}
	else
#endif
	  ret = shel_write(TRUE, isgraf, isover, pcmd, ptail);
	if (!ret)
	  graf_mouse(ARROW, 0x0L);
	return( ret );
} /*  */

	WORD
pro_exit(pcmd, ptail)
	LONG		pcmd, ptail;
{
	WORD		ret;

	ret = shel_write(FALSE, FALSE, 1, pcmd, ptail);
	return( ret );
} /* pro_exit */
