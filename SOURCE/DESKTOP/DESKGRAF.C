/*	DESKGRAF.C	04/11/84 - 03/17/85	Lee Lorenzen		*/

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 2.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1985			Digital Research Inc.
*	-------------------------------------------------------------
*/

#include <portab.h>
#include <machine.h>
#include <obdefs.h>
#include <gsxdefs.h>
#include <bind.h>
#include <funcdef.h>

#define ORGADDR 0x0L
						/* in GSXBIND.C		*/
#define vsf_interior( x )	gsx_1code(S_FILL_STYLE, x)
#define vsl_type( x )		gsx_1code(S_LINE_TYPE, x)
#define vsf_style( x )		gsx_1code(S_FILL_INDEX, x)
#define vsf_color( x )		gsx_1code(S_FILL_COLOR, x)
#define vsl_udsty( x )		gsx_1code(ST_UD_LINE_STYLE, x)
						/* External Routines	*/
EXTERN VOID	r_get();
EXTERN WORD	vst_clip();
EXTERN VOID	r_set();
EXTERN VOID	gsx2();
EXTERN VOID	gsx_moff();
EXTERN WORD	vro_cpyfm();
EXTERN WORD	vrt_cpyfm();
EXTERN VOID	gsx_mon();
EXTERN WORD	vrn_trnfm();
EXTERN VOID	gsx_1code();

GLOBAL WORD	gl_width;
GLOBAL WORD	gl_height;

GLOBAL WORD	gl_nrows;
GLOBAL WORD	gl_ncols;

GLOBAL WORD	gl_wchar;
GLOBAL WORD	gl_hchar;

GLOBAL WORD	gl_wschar;
GLOBAL WORD	gl_hschar;

GLOBAL WORD	gl_wptschar;
GLOBAL WORD	gl_hptschar;

GLOBAL WORD	gl_wbox;
GLOBAL WORD	gl_hbox;

GLOBAL WORD	gl_xclip;
GLOBAL WORD	gl_yclip;
GLOBAL WORD	gl_wclip;
GLOBAL WORD	gl_hclip;

GLOBAL WORD 	gl_nplanes;
GLOBAL WORD 	gl_handle;

GLOBAL FDB	gl_src;
GLOBAL FDB	gl_dst;

GLOBAL WS	gl_ws;
GLOBAL WORD	contrl[12];
GLOBAL WORD	intin[128];
GLOBAL WORD	ptsin[20];
GLOBAL WORD	intout[10];
GLOBAL WORD	ptsout[10];

GLOBAL LONG	ad_intin;

GLOBAL WORD	gl_mode;
GLOBAL WORD	gl_mask;
GLOBAL WORD	gl_tcolor;
GLOBAL WORD	gl_lcolor;
GLOBAL WORD	gl_fis;
GLOBAL WORD	gl_patt;
GLOBAL WORD	gl_font;

GLOBAL GRECT	gl_rscreen;
GLOBAL GRECT	gl_rfull;
GLOBAL GRECT	gl_rzero;
GLOBAL GRECT	gl_rcenter;
GLOBAL GRECT	gl_rmenu;

/*
*	Routine to set the clip rectangle.  If the w,h of the clip
*	is 0, then no clip should be set.  Ohterwise, set the 
*	appropriate clip.
*/
	WORD
gsx_sclip(pt)
	GRECT		*pt;
{
	r_get(pt, &gl_xclip, &gl_yclip, &gl_wclip, &gl_hclip);

	if ( gl_wclip && gl_hclip )
	{
	  ptsin[0] = gl_xclip;
	  ptsin[1] = gl_yclip;
	  ptsin[2] = gl_xclip + gl_wclip - 1;
	  ptsin[3] = gl_yclip + gl_hclip - 1;
	  vst_clip( TRUE, &ptsin[0]);
	}
	else
	  vst_clip( FALSE, &ptsin[0]);
	return( TRUE );
}

/*
*	Routine to get the current clip setting
*/
	VOID
gsx_gclip(pt)
	GRECT		*pt;
{
	r_set(pt, gl_xclip, gl_yclip, gl_wclip, gl_hclip);
}

	VOID
gsx_xline( ptscount, ppoints )
	WORD		ptscount, *ppoints;
{
	static	WORD	hztltbl[2] = { 0x5555, 0xaaaa };
	static  WORD	verttbl[4] = { 0x5555, 0xaaaa, 0xaaaa, 0x5555 };
	WORD		*linexy,i;
	WORD		st;

	for ( i = 1; i < ptscount; i++ )
	{
	  if ( *ppoints == *(ppoints + 2) )
	  {
	    st = verttbl[( (( *ppoints) & 1) | ((*(ppoints + 1) & 1 ) << 1))];
	  }	
	  else
	  {
	    linexy = ( *ppoints < *( ppoints + 2 )) ? ppoints : ppoints + 2;
	    st = hztltbl[( *(linexy + 1) & 1)];
	  }
	  vsl_udsty( st );
	  v_pline( 2, ppoints );
	  ppoints += 2;
	}
	vsl_udsty( 0xffff );
}	

/*
*	Routine to draw a certain number of points in a polyline
*	relative to a given x,y offset.
*/
	VOID
gsx_pline(offx, offy, cnt, pts)
	WORD		offx, offy;
	WORD		cnt;
	WORD		*pts;
{
	WORD		i, j;

	for (i=0; i<cnt; i++)
	{
	  j = i * 2;
	  ptsin[j] = offx + pts[j];
	  ptsin[j+1] = offy + pts[j+1];
	}

	gsx_xline( cnt, &ptsin[0]);
}

/*
*	Routine to set the text, writing mode, and color attributes.
*/
	VOID
gsx_attr(text, mode, color)
	UWORD		text, mode, color;
{
	WORD		tmp;

	tmp = intin[0];
	contrl[1] = 0;
	contrl[3] = 1;
	contrl[6] = gl_handle;
	if (mode != gl_mode)
	{
	  contrl[0] = SET_WRITING_MODE;
	  intin[0] = gl_mode = mode;
	  gsx2();
	}
	contrl[0] = FALSE;
	if (text)
	{
	  if (color != gl_tcolor)
	  {
	    contrl[0] = S_TEXT_COLOR;
	    gl_tcolor = color;
	  }
	}	
	else
	{
	  if (color != gl_lcolor)
	  {
	    contrl[0] = S_LINE_COLOR;
	    gl_lcolor = color;
	  }
	}
	if (contrl[0])
	{
	  intin[0] = color;
	  gsx2();
	}
	intin[0] = tmp;
}

/*
*	Routine to fix up the MFDB of a particular raster form
*/
	VOID
gsx_fix(pfd, theaddr, wb, h)
	FDB		*pfd;
	LONG		theaddr;
	WORD		wb, h;
{
	if (theaddr == ORGADDR)
	{
	  pfd->fd_w = gl_ws.ws_xres + 1;
	  pfd->fd_wdwidth = pfd->fd_w / 16;
	  pfd->fd_h = gl_ws.ws_yres + 1;
	  pfd->fd_nplanes = gl_nplanes;
	}
	else
	{
	  pfd->fd_wdwidth = wb / 2;
	  pfd->fd_w = wb * 8;
	  pfd->fd_h = h;
	  pfd->fd_nplanes = 1;
	}
	pfd->fd_stand = FALSE;
	pfd->fd_addr = theaddr;
}

/*
*	Routine to blit, to and from a specific area
*/
	VOID
gsx_blt(saddr, sx, sy, swb, daddr, dx, dy, dwb, w, h, rule, fgcolor, bgcolor)
	LONG		saddr;
	UWORD		sx, sy, swb;
	LONG		daddr;
	UWORD		dx, dy, dwb;
	UWORD		w, h, rule;
	WORD		fgcolor, bgcolor;
{
	gsx_fix(&gl_src, saddr, swb, h);
	gsx_fix(&gl_dst, daddr, dwb, h);

	gsx_moff();
	ptsin[0] = sx;
	ptsin[1] = sy;
	ptsin[2] = sx + w - 1;
	ptsin[3] = sy + h - 1;
	ptsin[4] = dx;
	ptsin[5] = dy;
	ptsin[6] = dx + w - 1;
	ptsin[7] = dy + h - 1 ;
	if (fgcolor == -1)
	  vro_cpyfm( rule, &ptsin[0], &gl_src, &gl_dst);
	else
	  vrt_cpyfm( rule, &ptsin[0], &gl_src, &gl_dst, fgcolor, bgcolor);
	gsx_mon();
}

/*
*	Routine to blit around something on the screen
*/
	VOID
bb_screen(scrule, scsx, scsy, scdx, scdy, scw, sch)
	WORD		scrule, scsx, scsy, scdx, scdy, scw, sch;
{
	gsx_blt(0x0L, scsx, scsy, 0, 
		0x0L, scdx, scdy, 0,
		scw, sch, scrule, -1, -1);
}

/*
*	Routine to transform a standard form to device specific
*	form.
*/
	VOID
gsx_trans(saddr, swb, daddr, dwb, h)
	LONG		saddr;
	UWORD		swb;
	LONG		daddr;
	UWORD		dwb;
	UWORD		h;
{
	gsx_fix(&gl_src, saddr, swb, h);
	gl_src.fd_stand = TRUE;
	gl_src.fd_nplanes = 1;

	gsx_fix(&gl_dst, daddr, dwb, h);
	vrn_trnfm( &gl_src, &gl_dst );
}

/*
*	Routine to initialize all the global variables dealing
*	with a particular workstation open
*/
	VOID
gsx_start()
{
	WORD		char_height, nc;

	gl_xclip = 0;
	gl_yclip = 0;
	gl_width = gl_wclip = gl_ws.ws_xres + 1;
	gl_height = gl_hclip = gl_ws.ws_yres + 1;

	nc = gl_ws.ws_ncolors;
	gl_nplanes = 0;
	while (nc != 1)
	{
	  nc >>= 1;
	  gl_nplanes++;
	}
	char_height = gl_ws.ws_chminh;
	vst_height( char_height, &gl_wptschar, &gl_hptschar, 
				&gl_wschar, &gl_hschar );
	char_height = gl_ws.ws_chmaxh;
	vst_height( char_height, &gl_wptschar, &gl_hptschar, 
				&gl_wchar, &gl_hchar );
	gl_ncols = gl_width / gl_wchar;
	gl_nrows = gl_height / gl_hchar;
	gl_hbox = gl_hchar + 3;
	gl_wbox = (gl_hbox * gl_ws.ws_hpixel) / gl_ws.ws_wpixel;
	vsl_type( 7 );
	vsl_width( 1 );
	vsl_udsty( 0xffff );
	r_set(&gl_rscreen, 0, 0, gl_width, gl_height);
	r_set(&gl_rfull, 0, gl_hbox, gl_width, (gl_height - gl_hbox));
	r_set(&gl_rzero, 0, 0, 0, 0);
	r_set(&gl_rcenter, (gl_width-gl_wbox)/2, (gl_height-(2*gl_hbox))/2, 
			gl_wbox, gl_hbox);
	r_set(&gl_rmenu, 0, 0, gl_width, gl_hbox);
	ad_intin = ADDR(&intin[0]);
}

	VOID
gsx_tblt(tb_f, x, y, tb_nc)
	WORD		tb_f;
	WORD		x, y;
	WORD		tb_nc;
{
	WORD		pts_height;

	if (tb_f == IBM)
	{
	  if (tb_f != gl_font)
	  {
	    pts_height = gl_ws.ws_chmaxh;
	    vst_height( pts_height, &gl_wptschar, &gl_hptschar, 
				&gl_wchar, &gl_hchar );
	    gl_font = tb_f;
	  }
	  y += gl_hptschar;
	}

	contrl[0] = 8;		/* TEXT */
	contrl[1] = 1;
	contrl[6] = gl_handle;
	ptsin[0] = x;
	ptsin[1] = y;
	contrl[3] = tb_nc;
	gsx2();
}

/*
*	Routine to do a filled bit blit, (a rectangle).
*/
	VOID
bb_fill(mode, fis, patt, hx, hy, hw, hh)
	WORD		mode, fis, patt, hx, hy, hw, hh;
{

	gsx_fix(&gl_dst, 0x0L, 0, 0);
	ptsin[0] = hx;
	ptsin[1] = hy;
	ptsin[2] = hx + hw - 1;
	ptsin[3] = hy + hh - 1;

	gsx_attr(TRUE, mode, gl_tcolor);
	if (fis != gl_fis)
	{
	  vsf_interior( fis);
	  gl_fis = fis;
	}
	if (patt != gl_patt)
	{
	  vsf_style( patt );
	  gl_patt = patt;
	}
	vr_recfl( &ptsin[0], &gl_dst );
}
