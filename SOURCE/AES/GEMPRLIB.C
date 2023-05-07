/*	GEMPRLIB.C	9/16/85 - 11/11/85	Lowell Webster	*/
/*			7/2/86			MDF		*/

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
#include <obdefs.h>
#include <struct.h>
#include <basepage.h>
#include <gemlib.h>
#include <crysbind.h>

EXTERN WORD		contrl[];
EXTERN WORD		intin[];
EXTERN WORD		ptsin[];
EXTERN WORD		intout[];
EXTERN WORD		ptsout[];
EXTERN WORD		nulp_msg[];

EXTERN LONG		ad_scmd;
EXTERN LONG		ad_stail;
EXTERN LONG		ad_envrn;

EXTERN VOID		gsx_ncode();
EXTERN VOID		sh_fixtail();
EXTERN VOID		ap_sendmsg();
EXTERN WORD		sh_find();
EXTERN PD		*fpdnm();
EXTERN VOID		ap_accexit();	
EXTERN SHELL		sh[];
EXTERN PD		*gl_mnppd;
EXTERN GRECT		gl_rscreen;

EXTERN WORD		gsx_ncode();	/*code, # ptsin, length intin	*/
EXTERN WORD		gl_ldpid;

GLOBAL WORD		gl_pids;	/* bit vector of pids used	*/
GLOBAL WORD		gl_okalloc = 0;

/*
	ARENA
	0 byte	status	'M'	used
			'Z'	last one in chain
	1 word	segment of owner memory block
		  if 0 then this block is free
	3 word	# of paragraphs
*/

/*	NOTES
*
*	whenever a program terminates, control returns to the
*	desktop. All context switching is done by the AES.
*/

/*
*	if prsize is 0 then maximum channel size will be created
*	if prstart is at the beginning of the aes then this
*/

/* function 10	*/
	VOID
pr_create(pid, start, size, isswap, isgem)
	WORD		pid;
	LONG		start;		/* intin[1 and 2]	*/
	LONG		size;		/* intin[3 and 4]	*/
	WORD		isswap;		/* intin[5]		*/
	WORD		isgem;		/* intin[6]		*/
{	
	intin[0] = pid;
	LLSET(ADDR(&intin[1]), start);
	LLSET(ADDR(&intin[3]), size);
	intin[5] = isswap;
	intin[6] = isgem;
	contrl[5] = 10;
	gsx_ncode(-1, 0, 7);
}

/*	shrink memory, only used with desktop and screen manager
*	function 11
*/
	WORD
pr_shrink(pid, fcreate, pstart, pend)
	WORD		pid;		/* intin[0]		*/
	WORD		fcreate;	/* intin[1]		*/
	LONG		*pstart;	/* intout[0 and 1]	*/
	LONG		*pend;		/* intout[2 and 3]	*/
{
	intin[0] = pid;
	intin[1] = fcreate;
	contrl[5] = 11;
	gsx_ncode(-1, 4, 2);

	*pstart = HW(intout[1]) + LW(intout[0]);
	*pend  = HW(intout[3]) + LW(intout[2]);

	dbg("pr_shrink, pstart=%lx, pend=%lx, ret=%x\r\n",
					*pstart, *pend, intout[4]);
	return(intout[4]);
}

/*
*	function 12
*	Used only from dispatcher
*/
	WORD
pr_load(pid)
	WORD		pid;
{
	if ( !sh[pid].sh_loadable )
	  return(0);
	if (pid == gl_ldpid)
	  return(0);
dbg("pr_load, pid=%x\r\n", pid);
	intin[0] = pid;
	contrl[5] = 12;
	gsx_ncode(-1, 0, 1);
	gl_ldpid = intout[2];
	return(intout[0]);
}

/*
*	Returns menu id of user selection in character menu
*/
	WORD
pr_retid()
{
	return(intout[1]);
}

/*
*	function 15
*	Send  menu parameters. One time only
*/
	WORD
pr_menu(pnames, pdacnt)
	LONG		pnames;
	LONG		pdacnt;
{
	LLSET(ADDR(&intin[0]), pnames);
	LLSET(ADDR(&intin[2]), pdacnt);
	contrl[5] = 15;
	gsx_ncode(-1, 0, 4);
}

/*
*	used only and for all dos apps. if prstart is at the base of the AES
*	then this is a full stepaside application. When exec returns
*	a code indicates when it has terminated or switched.
*
*	function 13
*/
	VOID
pr_exec(pid,  pcmd, ptail, penvrn, p1fcb, p2fcb)
	WORD	pid;		/* intin[0]	*/
	LONG	pcmd;		/* intin[1]	*/
	LONG	ptail;		/* intin[5]	*/
	LONG	penvrn;		/* intin[3]	*/
	LONG	p1fcb;		/* intin[7]	*/
	LONG	p2fcb;		/* intin[9]	*/
{
	intin[0] = pid;
	LLSET(ADDR(&intin[1]), pcmd);
	LLSET(ADDR(&intin[3]), penvrn);
	LLSET(ADDR(&intin[5]), ptail);
	LLSET(ADDR(&intin[7]), p1fcb);
	LLSET(ADDR(&intin[9]), p2fcb);
	contrl[5] = 13;
	gsx_ncode(-1, 0, 7);
}


gsx_prcall(cnum)
	WORD		cnum;
{
	intin[0] = rlr->p_pid;
	contrl[5] = cnum;
	gsx_ncode(-1, 0, 2);
}

GLOBAL BYTE gl_zyxg[] = {'z','y','x','g'};
GLOBAL BYTE gl_allwins = 0; 

	WORD
pr_run(prnum, isgraf, isover, pcmd, ptail)
	WORD		prnum;
	WORD		isgraf, isover;
	LONG		pcmd, ptail;
{
	PD		*nulpd;
	SHELL		*psh;
	WORD		ii;

dbg("isgraf=%x, isover=%x\r\n", isgraf, isover);
	if (gl_allwins && isgraf && (isover == -1))
	  isover = 4;				/* if gl_all wins is patched */
						/* then make all gemapps     */
						/* CONTINUOUS		     */
	LBCOPY(ad_scmd, pcmd, 128);
	LBCOPY(ad_stail, ptail, 128);

	nulpd = fpdnm(NULLPTR, prnum);
	if ( !nulpd)
	  return(FALSE);

	psh = &sh[nulpd->p_pid];
	psh->sh_isgem = (isgraf != FALSE);
	psh->sh_dodef = FALSE;
					/* only acc's need to be shrunk	*/
					/*  others are "already shrunk"	*/
	if (isover == 4)
	  psh->sh_state |= CONTINUOUS; 
	else
	if (isover == 3)		/* gem acc			*/
	{
	  psh->sh_isacc = TRUE;
	  psh->sh_state |= SHRINK;
	}
	else
	{
	  psh->sh_isacc = FALSE;	/* normal app			*/

	  mn_bar(0x0L, FALSE, 0);	/* disable menu bar		*/
	  dsptch();
	  dsptch();
	}

	ap_sendmsg(&nulp_msg[0], NP_RUNIT, 
		   nulpd, 0, 0, 0, 0, 0);
		   				/* only acc's can be	*/
						/*  shrunk.		*/
	if (psh->sh_isacc)
	{
	  while(psh->sh_state & SHRINK)
	    dsptch();
	  if (! (psh->sh_state & RUNNING ))
	    return(FALSE);	 	  
	}

	return(TRUE);
}


	WORD
prc_create(caddr, csize, isswap, isgem, pid)
	LONG	caddr;
	LONG	csize;
	WORD	isswap;
	WORD	isgem;
	WORD	*pid;
{
	WORD		id;

	if ( (id = pr_gpid()) == NIL)
	  return(FALSE);
	pr_scpid(id, FALSE);
	pr_create(id, caddr, csize, isswap, isgem);
	*pid = id;			/* set process id	*/
	return(TRUE);
}

/*
*	gdos function 14
*/
	WORD
pr_delete(prnum)
	WORD	prnum;
{
	intin[0] = prnum;
	contrl[5] = 14;
	gsx_ncode(-1, 0, 1);
	gl_ldpid = 1;
	pr_scpid(prnum, TRUE);
	return(TRUE);
}

	WORD
pr_info(prnum, isswap, isgem, caddr, csize, endmem, cssize, intaddr)
	WORD	prnum;			/* pid			*/
	WORD	*isswap;		/* swappable		*/
	WORD	*isgem;			/* gem or dos		*/
	LONG	*caddr;			/* channel address	*/
	LONG	*csize;			/* channel size		*/
	LONG	*endmem;		/* end of mem + 1	*/
	LONG	*cssize;		/* channel system size	*/
	LONG	*intaddr;		/* addr of default interrupts	*/
{
	contrl[5] = 16;
	intin[0] = prnum;
	gsx_ncode(-1, 0, 1);
	*isswap = intout[4];
	*isgem = intout[5];
	*caddr = HW(intout[1]) + LW(intout[0]);
	*csize = HW(intout[3]) + LW(intout[2]);
	*endmem = HW(intout[7]) + LW(intout[6]);
	*cssize = HW(intout[9]) + LW(intout[8]);
	*intaddr = HW(intout[11]) + LW(intout[10]);
	return(TRUE);
}

/*
*	remove a process by doing the following:
*	make the process the active process ( rlr)
*	load the process
*	fake a 4c to get back to sh_ldapp which will take care of the rest
*	BUT we need to get back to the caller!!!
*	
*/
	WORD
pr_abort(prnum)
	WORD		prnum;
{
	PD		*ppd;
	SHELL		*psh;
	WORD		ii;

	ppd = fpdnm(NULLPTR, prnum);
	if ( ppd == NULLPTR )
	  return(FALSE);
	psh = &sh[prnum];

	if (psh->sh_isacc)
	  psh->sh_state |= ABORT;
	else
	  return(FALSE);

	psetup(ppd, &ap_accexit);		/* force a terminate	*/
	
  				/* force app to be ready	*/
	ap_sendmsg(&nulp_msg[0], AC_ABORT,	
		   ppd, 0, 0, 0, 0, 0);

		   			/* only accessories can be	*/
					/*  aborted.			*/
	if (psh->sh_isacc)
	{
	  while(psh->sh_state & ABORT)
	    dsptch();
	}
	return(TRUE);
}

/*
*	set or clear the bit corresponding to the pid in gl_pids
*/
	VOID
pr_scpid(pid, isclear)
	WORD		pid;
	WORD		isclear;
{
	WORD		ii;
	UWORD		bv;

	bv = 0x01;
	for(ii = 0; ii < NUM_PDS; ii++)
	{
	  if (ii == pid)
	  {
	    if (isclear)
	      gl_pids &= ~bv;
	    else
	      gl_pids |= bv;
	  }
	  else
	    bv = bv << 1;
	}
}

/*
*	get next avail pid.
*/
	WORD
pr_gpid()
{
	WORD		ii;
	UWORD		bv;

	bv = 0x01;
	for(ii = 0; ii < NUM_PDS; ii++)
	{
	  if ( !(bv & gl_pids) )
	    return(ii);
	  else
	    bv = bv << 1;
	}
	return(NIL);
}
/*
	WORD
pr_malloc(caddr, csize)
	LONG		caddr, csize;
{
	WORD		id, ret;

	if (gl_okalloc != 0)
	  return(NIL);
	ret = prc_create(caddr, csize, TRUE, TRUE, &id);
	if (ret == TRUE)
	{
	  gl_okalloc++;
	  sh[id].sh_loadable = TRUE;
	  pr_load(id);			/* make sure this memory is ok	*/
	  sh[id].sh_loadable = FALSE;
	  pr_load(rlr->p_pid);		/* load current process back	*/
	}
	else
	  id = NIL;
	return(id);
}
*/

	WORD
pr_malloc(size, paddr, psize)
	LONG	size;
	LONG	*paddr, *psize;
{
dbg("pr_malloc, size=%lx\r\n", size);
	contrl[5] = 17;
	LLSET(ADDR(&intin[0]), size);
	gsx_ncode(-1, 0, 2);			
	*paddr = HW(intout[1]) + LW(intout[0]);
	*psize = HW(intout[3]) + LW(intout[2]);
dbg("pr_malloc, addr=%lx, size=%lx\r\n", *paddr, *psize);
	return(*paddr != 0L); 	
}

	WORD
pr_mfree(pid, caddr, csize)
	WORD		pid;
	LONG		caddr, csize;
{
	pr_delete(pid);
	pr_load(rlr->p_pid);		/* make sure current proc is 	*/
	gl_okalloc--;
	return(TRUE);
}

	WORD
pr_switch(pid)
	WORD		pid;
{
	PD		*ppd;
	WORD		mesg;

	if (pid == rlr->p_pid)
	  return(FALSE);			/* can't switch to self	*/

	ap_sendmsg(&nulp_msg[0], CT_SWITCH, 
				fpdnm(NULLPTR, 1), pid, 0, 0, 0, 0);
	return(TRUE);
}

/*
*	DO a pr_shrink
*/
	WORD
pr_setblock(pid)
	WORD		pid;
{
	intin[0] = pid;
	intin[1] = TRUE;
	contrl[5] = 11;
	gsx_ncode(-1, 4, 2);
	return(TRUE);
}
