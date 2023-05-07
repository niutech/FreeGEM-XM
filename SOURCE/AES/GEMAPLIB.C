/*	GEMAPLIB.C	03/15/84 - 08/21/85	Lee Lorenzen		*/
/*	3.0		6/23/86			MDF			*/

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
#include <struct.h>
#include <basepage.h>
#include <obdefs.h>
#include <gemlib.h>

#define TCHNG 0
#define BCHNG 1
#define MCHNG 2
#define KCHNG 3

						/* in PD88.C		*/
EXTERN PD		*fpdnm();

EXTERN WORD		tchange();
EXTERN WORD		bchange();
EXTERN WORD		mchange();
EXTERN WORD		kchange();

EXTERN WORD		gl_ticktime;

EXTERN LONG		ad_valstr;
EXTERN LONG		ad_armice;
EXTERN LONG		menu_tree[NUM_PDS];

GLOBAL WORD		gl_play;
GLOBAL WORD		gl_recd;
GLOBAL WORD		gl_rlen;
GLOBAL LONG		gl_rbuf;

GLOBAL WORD		gl_mnpds[NUM_PDS];

#if MULTIAPP
EXTERN LONG		gl_efnorm, gl_efsave;
EXTERN PD		*gl_mnppd;
EXTERN SHELL		sh[];
EXTERN UWORD		dos_gpsp();
#endif

/*
*	Routine to initialize the application
*/
	WORD
ap_init(isgem)
	WORD		isgem;
{
	WORD		pid;
#if MULTIAPP
	SHELL		*psh;
	UWORD		chseg;
#endif

	pid = rlr->p_pid;
#if MULTIAPP
dbg("ap_init, pid = %d\r\n", pid);
	psh = &sh[pid];
	if ( psh->sh_state & SHRINK )
	{
	  chseg = dos_gpsp();			/* get seg of acc's PSP	*/
dbg("calling dos_stblk, seg = %x, size = %x\r\n", chseg, psh->sh_shrsz);
	  dos_stblk(chseg, psh->sh_shrsz);	/* size down acc	*/
	}
#endif
	return( pid );
}


/*
*	APplication READ or WRITE
*/
	VOID
ap_rdwr(code, p, length, pbuff)
	WORD		code;
	REG PD		*p;
	WORD		length;
	LONG		pbuff;
{
	QPB		m;
						/* do quick version if	*/
						/*   it is standard 16	*/
						/*   byte read and the	*/
						/*   pipe has only 16	*/
						/*   bytes inside it	*/
	if ( (code == MU_MESAG) &&
	     (p->p_qindex == length) &&
	     (length == 16) )
	{
	  LBCOPY(pbuff, p->p_qaddr, p->p_qindex);
	  p->p_qindex = 0;
	}
	else
	{
	  m.qpb_ppd = p;
	  m.qpb_cnt = length;
	  m.qpb_buf = pbuff;
	  ev_block(code, ADDR(&m));
	}
}

/*
*	APplication FIND
*/
	WORD
ap_find(pname)
	LONG		pname;
{
	REG PD		*p;
	BYTE		temp[9];

	LSTCPY(ADDR(&temp[0]), pname);
 
	p = fpdnm(&temp[0], 0x0);
	return( ((p) ? (p->p_pid) : (-1)) );
}

/*
*	APplication Tape PLAYer
*/
	VOID
ap_tplay(pbuff, length, scale)
	REG LONG	pbuff;
	WORD		length;
	WORD		scale;
{
	REG WORD	i;
	FPD		f;
	LONG		ad_f;

	ad_f = ADDR(&f);

	gl_play = TRUE;
	for(i=0; i<length; i++)
	{
						/* get an event to play	*/
	  LBCOPY(ad_f, pbuff, sizeof(FPD));
	  pbuff += sizeof(FPD);
						/* convert it to machine*/
						/*   specific form	*/
	  switch( ((WORD)(f.f_code)) )
	  {
	    case TCHNG:
		ev_timer( (f.f_data*100L) / scale );
		f.f_code = 0;
		break;
	    case MCHNG:
		f.f_code = &mchange;
		break;
	    case BCHNG:
		f.f_code = &bchange;
		break;
	    case KCHNG:
		f.f_code = &kchange;
		break;
	  }
						/* play it		*/
	  if (f.f_code)
	    forkq(f.f_code, f.f_data);
						/* let someone else	*/
						/*   hear it and respond*/
	  dsptch();
	}
	gl_play = FALSE;
} /* ap_tplay */

/*
*	APplication Tape RECorDer
*/
	WORD
ap_trecd(pbuff, length)
	REG LONG	pbuff;
	REG WORD	length;
{
	REG WORD	i;
	REG WORD	code;
	BYTE		*proutine;

						/* start recording in	*/
						/*   forker()		*/
	cli();
	gl_recd = TRUE;
	gl_rlen = length;
	gl_rbuf = pbuff;
	sti();
	  					/* 1/10 of a second	*/
						/*   sample rate	*/
	while( gl_recd )
	  ev_timer( 100L );
						/* done recording so	*/
						/*   figure out length	*/
	cli();
	gl_recd = FALSE;
	gl_rlen = 0;
	length = ((WORD)(gl_rbuf - pbuff)) / sizeof(FPD);
	gl_rbuf = 0x0L;
	sti();
						/* convert to machine	*/
						/*   independent 	*/
						/*   recording		*/
	for(i=0; i<length; i++)
	{
#if MC68K
	  proutine = (BYTE *)LLGET(pbuff);
#endif
#if I8086
	  proutine = (BYTE *)LWGET(pbuff);
#endif
	  if(proutine == &tchange)
	  {
	    code = TCHNG;
	    LLSET(pbuff+sizeof(WORD *), LLGET(pbuff+sizeof(WORD *)) * 
			gl_ticktime);
	  }
	  if(proutine == &mchange)
	    code = MCHNG;
	  if(proutine == &kchange)
	    code = KCHNG;
	  if(proutine == &bchange)
	    code = BCHNG;
	  LWSET(pbuff, code);
	  pbuff += sizeof(FPD);
	}
	return(length);
} /* ap_trecd */

	VOID
ap_exit(isgem)
	WORD		isgem;
{
	wm_update(TRUE);
#if SINGLAPP
	mn_clsda();
#endif
	if (rlr->p_qindex)
	  ap_rdwr(MU_MESAG, rlr, rlr->p_qindex, ad_valstr);
	gsx_mfset(ad_armice);
	wm_update(FALSE);
	all_run();
#if MULTIAPP
	if (isgem)
	  dos_term();
#endif
}

#if MULTIAPP
/*
*	special abort for accessories
*/
	VOID
ap_accexit()
{
	ap_exit(TRUE);
}
#endif
