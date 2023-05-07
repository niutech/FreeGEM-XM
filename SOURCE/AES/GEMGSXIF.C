/*	GEMGSXIF.C	05/06/84 - 06/13/85	Lee Lorenzen		*/

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
#include <obdefs.h>
#include <gsxdefs.h>
#include <bind.h>
#include <funcdef.h>
#include <gem.h>

/*
*	Calls used in Crystal:
*
*	vsf_interior();
*	vr_recfl();
*	vst_height(); 
*	vsl_type();
*	vsl_udsty();
*	vsl_width();
*	v_pline();
*	vst_clip();
*	vex_butv();
*	vex_motv();
*	vex_curv();
*	vex_timv();
*	vr_cpyfm();
*	v_opnwk();
*	v_clswk();
*	vq_extnd();
*	v_clsvwk( handle )
*	v_opnvwk( pwork_in, phandle, pwork_out )
*/

						/* in GEMDOS.C		*/
EXTERN LONG	dos_alloc();
EXTERN WORD	dos_free();
						/* in DOSIF.A86		*/
EXTERN WORD	justretf();
						/* in GSX2.A86		*/
EXTERN		gsx2();
EXTERN		i_ptsin();
EXTERN		i_intin();
EXTERN		i_ptsout();
EXTERN		i_intout();
EXTERN		i_ptr();
EXTERN		i_ptr2();
EXTERN		i_lptr1();	
EXTERN		m_lptr2();
						/* in OPTIMIZE.C	*/
EXTERN WORD	max();
EXTERN WORD	min();
						/* in RSLIB.C		*/
EXTERN BYTE	*rs_str();
						/* in DOSIF.A86		*/
EXTERN WORD	far_bcha();
EXTERN WORD	far_mcha();
EXTERN VOID	drawrat();
EXTERN LONG	drwaddr;
						/* in APGSXIF.C		*/
EXTERN WORD	xrat, yrat, button;

EXTERN WORD	gl_width;
EXTERN WORD	gl_height;

EXTERN WORD	gl_nrows;
EXTERN WORD	gl_ncols;

EXTERN WORD	gl_wchar;
EXTERN WORD	gl_hchar;

EXTERN WORD	gl_wschar;
EXTERN WORD	gl_hschar;

EXTERN WORD	gl_wptschar;
EXTERN WORD	gl_hptschar;

EXTERN WORD	gl_wsptschar;
EXTERN WORD	gl_hsptschar;

EXTERN WORD	gl_wbox;
EXTERN WORD	gl_hbox;

EXTERN WORD	gl_xclip;
EXTERN WORD	gl_yclip;
EXTERN WORD	gl_wclip;
EXTERN WORD	gl_hclip;

EXTERN WORD	gl_nplanes;
EXTERN	WORD 	gl_handle;

EXTERN FDB		gl_src;
EXTERN FDB		gl_dst;

EXTERN WS		gl_ws;
EXTERN WORD		contrl[];
EXTERN WORD		intin[];
EXTERN WORD		ptsin[];
EXTERN WORD		intout[];
EXTERN WORD		ptsout[];
EXTERN LONG		ad_intin;

EXTERN WORD		gl_ctmown;
EXTERN LONG		ad_mouse;

GLOBAL FDB		gl_tmp;

GLOBAL LONG		old_mcode;
GLOBAL LONG		old_bcode;
GLOBAL WORD		gl_moff;		/* counting semaphore	*/
						/*  == 0 implies ON	*/
						/*  >  0 implies OFF	*/
GLOBAL LONG		gl_mlen;
GLOBAL WORD		gl_graphic;

	ULONG
gsx_mcalc()
{
	ULONG		paras;

	gsx_fix(&gl_tmp, 0x0L, 0, 0);
						/* save 1/4 of a screen	*/
	paras = (ULONG)((gl_tmp.fd_wdwidth+7) / 8)
		* (ULONG)((gl_tmp.fd_h * gl_nplanes) / 4); 
	gl_mlen = paras * 16;

	return(gl_mlen);
}

	WORD
gsx_malloc()
{
	gl_tmp.fd_addr = dos_alloc( gsx_mcalc() );
}


	WORD
gsx_mfree()
{
	dos_free(gl_tmp.fd_addr);
}


	VOID
gsx_mret(pmaddr, pmlen)
	LONG		*pmaddr;
	LONG		*pmlen;
{
	*pmaddr = gl_tmp.fd_addr;
	*pmlen = gl_mlen;
}


gsx_ncode(code, n, m)
	WORD		code;
	WORD		n, m;
{
	contrl[0] = code;
	contrl[1] = n;
	contrl[3] = m;
	contrl[6] = gl_handle;
	gsx2();
}


gsx_1code(code, value)
	WORD		code;
	WORD		value;
{
	intin[0] = value;
	gsx_ncode(code, 0, 1);
}


	WORD
gsx_init()
{
	gsx_wsopen();
	gsx_start();
	gsx_setmb(&far_bcha, &far_mcha, &drwaddr);
	gsx_ncode(MOUSE_ST, 0, 0); 
	xrat = ptsout[0];
	yrat = ptsout[1];
}

	VOID
gsx_exec(pcspec, segenv, pcmdln, pfcb1, pfcb2)
	LONG		pcspec;
	WORD		segenv;
	LONG		pcmdln;
	LONG		pfcb1;
	LONG		pfcb2;
{
	EXEC_BLK	exec;
	LONG		lpstr;

	exec.eb_segenv = segenv;
	exec.eb_pcmdln = pcmdln;
	exec.eb_pfcb1 = pfcb1;
	exec.eb_pfcb2 = pfcb2;

	intin[0] = LLOWD(pcspec);
	intin[1] = LHIWD(pcspec);
	intin[2] = LLOWD(ADDR(&exec));
	intin[3] = LHIWD(ADDR(&exec));
	lpstr = ADDR(rs_str(STGDOS));
	intin[4] = LLOWD( lpstr );
	intin[5] = LHIWD( lpstr );
	contrl[5] = 1;
	gsx_ncode(-1, 0, 6);
}


	WORD
gsx_graphic(tographic)
	WORD		tographic;
{
	if (gl_graphic != tographic)
	{
	  gl_graphic = tographic;
	  if (gl_graphic)
	  {
	    contrl[5] = 2;
	    gsx_ncode(5, 0, 0);
	    gsx_setmb(&far_bcha, &far_mcha, &drwaddr);
	  }
	  else
	  {
	    contrl[5] = 3;
	    gsx_ncode(5, 0, 0);
	    gsx_resetmb();
	  }
	}
}


	WORD
gsx_wsopen()
{
	WORD		i;

	for(i=0; i<10; i++)
	  intin[i] = 1;
	intin[10] = 2;			/* device coordinate space */
	v_opnwk( &intin[0], &gl_handle, &gl_ws );
	gl_graphic = TRUE;
}


	VOID
gsx_wsclose()
{
	gsx_ncode(CLOSE_WORKSTATION, 0, 0);
}

	WORD
ratinit()
{
	gsx_1code(SHOW_CUR, 0);
	gl_moff = 0;
}


	WORD
ratexit()
{
	gsx_moff();
}


bb_set(sx, sy, sw, sh, pts1, pts2, pfd, psrc, pdst)
	REG WORD	sx, sy, sw, sh;
	REG WORD	*pts1, *pts2;
	FDB		*pfd;
	FDB		*psrc, *pdst;
{
	WORD		oldsx;

						/* get on word boundary	*/
	oldsx = sx;
	sx = (sx / 16) * 16;
	sw = ( ((oldsx - sx) + (sw + 15)) / 16 ) * 16;

	gl_tmp.fd_stand = TRUE;
	gl_tmp.fd_wdwidth = sw / 16;
	gl_tmp.fd_w = sw;
	gl_tmp.fd_h = sh;

	gsx_moff();
	pts1[0] = sx;
	pts1[1] = sy;
	pts1[2] = sx + sw - 1;
	pts1[3] = sy + sh - 1;
	pts2[0] = 0;
	pts2[1] = 0;
	pts2[2] = sw - 1;
	pts2[3] = sh - 1 ;

	gsx_fix(pfd, 0, 0, 0, 0);
	vro_cpyfm( S_ONLY, &ptsin[0], psrc, pdst );
	gsx_mon();
}


	VOID
bb_save(ps)
	GRECT		*ps;
{	
	bb_set(ps->g_x, ps->g_y, ps->g_w, ps->g_h, &ptsin[0], &ptsin[4], 
		&gl_src, &gl_src, &gl_tmp);
}


	VOID
bb_restore(pr)
	GRECT		*pr;
{
	bb_set(pr->g_x, pr->g_y, pr->g_w, pr->g_h, &ptsin[4], &ptsin[0], 
		&gl_dst, &gl_tmp, &gl_dst);
}


	WORD
gsx_setmb(boff, moff, pdrwaddr)
	UWORD		*boff, *moff;
	LONG		*pdrwaddr;
{
	i_lptr1( boff, 0x0 );	
	gsx_ncode(BUT_VECX, 0, 0);
	m_lptr2( &old_bcode );

	i_lptr1( moff, 0x0 );	
	gsx_ncode(MOT_VECX, 0, 0);
	m_lptr2( &old_mcode );

	i_lptr1( justretf, 0x0 );	
	gsx_ncode(CUR_VECX, 0, 0);
	m_lptr2( pdrwaddr );
}


	WORD
gsx_resetmb()
{
	i_lptr1( old_bcode );	
	gsx_ncode(BUT_VECX, 0, 0);

	i_lptr1( old_mcode );
	gsx_ncode(MOT_VECX, 0, 0);

	i_lptr1( drwaddr );	
	gsx_ncode(CUR_VECX, 0, 0);
}

	WORD
gsx_tick(tcode, ptsave)
	LONG		tcode;
	LONG		*ptsave;
{
	i_lptr1( tcode );	
	gsx_ncode(TIM_VECX, 0, 0);
	m_lptr2( ptsave );
	return(intout[0]);
}


	WORD
gsx_mfset(pmfnew)
	LONG		pmfnew;
{
	gsx_moff();
	if (!gl_ctmown)
	  LWCOPY(ad_mouse, pmfnew, 37);
	LWCOPY(ad_intin, pmfnew, 37);
 	gsx_ncode(ST_CUR_FORM, 0, 37);
	gsx_mon();
}


	VOID
gsx_mxmy(pmx, pmy)
	WORD		*pmx, *pmy;
{
	*pmx = xrat;
	*pmy = yrat;
}


	WORD
gsx_button()
{
	return( button );
}


	WORD
gsx_kstate()
{
	gsx_ncode(KEY_SHST, 0, 0);
	return(intout[0]);
}


	VOID
gsx_moff()
{
	if (!gl_moff)
	  gsx_ncode(HIDE_CUR, 0, 0);

	gl_moff++;
}


	VOID
gsx_mon()
{
	gl_moff--;
	if (!gl_moff)
	  gsx_1code(SHOW_CUR, 1);
}



	WORD
gsx_char()
{
	intin[0] = 4;
	intin[1] = 2;
	gsx_ncode(33, 0, 2);
#if MC68K
/* CHANGED BACK TO OLD STYLE GSX CALL */
	intin[0] = 1;
	intin[1] = -1;
	intin[2] = FALSE;        /* no echo */
	gsx_ncode(31, FALSE, 3);
	if (contrl[4])
	{
	  switch(intout[0])
	  {
	    case 0x0008:  intout[0] = 0x0e08;
		break;
	    case 0x0020:  intout[0] = 0x3920;
		break;
	    case 0x0005:  intout[0] = 0x4800;
		break;
	    case 0x0018:  intout[0] = 0x5000;
		break;
	    case 0x0013:  intout[0] = 0x4b00;
		break;
	    case 0x0004:  intout[0] = 0x4d00;
		break;
	    case 0x007f:  intout[0] = 0x5300;
		break;
	    case 0x0009:  intout[0] = 0x0f09;
		break;
	    case 0x0001:  intout[0] = 0x0f00;
		break;
	    case 0x000d:  intout[0] = 0x1c0d;
		break;
	    case 0x001b:  intout[0] = 0x011b;
		break;
	
	  }
	  return(intout[0]);
	}
	else
	  return(0);
#endif
#if I8086
	intin[0] = -1;
	intin[1] = FALSE;        /* no echo */
	gsx_ncode(31, FALSE, 2);
	if (contrl[4])
	  return(intout[0]);
	else
	  return(0);
#endif
}

	WORD
v_opnwk( pwork_in, phandle, pwork_out )
	WORD		*pwork_in;
	WORD		*phandle;
	REG WORD	*pwork_out;
{
	WORD		*ptsptr;

	ptsptr = pwork_out + 45;
	i_ptsout( ptsptr );	/* set ptsout to work_out array */
	i_intin( pwork_in );	/* set intin to point to callers data  */
	i_intout( pwork_out );	/* set intout to point to callers data */
	gsx_ncode(OPEN_WORKSTATION, 0, 11);

	*phandle = contrl[6];	
	i_intin( &intin );
	i_intout( &intout );
 	i_ptsin( &ptsin );
	i_ptsout( &ptsout );
}

	WORD
v_pline( count, pxyarray )
	WORD		count;
	WORD		*pxyarray;
{
	i_ptsin( pxyarray );
	gsx_ncode(POLYLINE, count, 0);
	i_ptsin( &ptsin );
}


	WORD
vst_clip( clip_flag, pxyarray )
	REG WORD	clip_flag;
	WORD		*pxyarray;
{
	WORD		tmp, value;

	value = ( clip_flag != 0 ) ? 2 : 0;
	i_ptsin( pxyarray );
	intin[0] = clip_flag;
	gsx_ncode(TEXT_CLIP, value, 1);
	i_ptsin(&ptsin);
}


vst_height( height, pchr_width, pchr_height, pcell_width, pcell_height )
	WORD	height;
	WORD	*pchr_width;
	WORD	*pchr_height;
	WORD	*pcell_width;
	WORD	*pcell_height;
{
	ptsin[0] = 0;
	ptsin[1] = height;
	gsx_ncode(CHAR_HEIGHT, 1, 0);
	*pchr_width = ptsout[0];
	*pchr_height = ptsout[1];
	*pcell_width = ptsout[2];
	*pcell_height = ptsout[3];
}


	VOID
vr_recfl( pxyarray, pdesMFDB )
	WORD	*pxyarray;
	WORD	*pdesMFDB;
{
	i_ptr( pdesMFDB );
	i_ptsin( pxyarray );
	gsx_ncode(FILL_RECTANGLE, 2, 1);
	i_ptsin( &ptsin );
}


	VOID
vro_cpyfm( wr_mode, pxyarray, psrcMFDB, pdesMFDB )
	WORD	wr_mode;
	WORD	*pxyarray;
	WORD	*psrcMFDB;
	WORD	*pdesMFDB;
{
	intin[0] = wr_mode;
	i_ptr( psrcMFDB );
	i_ptr2( pdesMFDB );
	i_ptsin( pxyarray );
	gsx_ncode(COPY_RASTER_FORM, 4, 1);
	i_ptsin( &ptsin );
}


    WORD
vrt_cpyfm( wr_mode, pxyarray, psrcMFDB, pdesMFDB, fgcolor, bgcolor )
	WORD    wr_mode;
	WORD    *pxyarray;
	WORD    *psrcMFDB;
	WORD    *pdesMFDB;
	WORD	fgcolor, bgcolor;
{
	intin[0] = wr_mode;
	intin[1] = fgcolor;
	intin[2] = bgcolor;
	i_ptr( psrcMFDB );
	i_ptr2( pdesMFDB );
	i_ptsin( pxyarray );
	gsx_ncode(121, 4, 3);
	i_ptsin( &ptsin );
}


	VOID
vrn_trnfm( psrcMFDB, pdesMFDB )
	WORD	*psrcMFDB;
	WORD	*pdesMFDB;
{
	i_ptr( psrcMFDB );
	i_ptr2( pdesMFDB );
	gsx_ncode(TRANSFORM_FORM, 0, 0);
}


	VOID
vsl_width( width )
	WORD	width;
{
	ptsin[0] = width;
	ptsin[1] = 0;
	gsx_ncode(S_LINE_WIDTH, 1, 0);
}

