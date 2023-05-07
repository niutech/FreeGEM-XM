/*	DESKGSX.C	06/01/84 - 02/02/85	Lee Lorenzen	*/

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

/*
*	Calls used in DESKTOP:
*
*	vsf_interior();
*	vr_recfl();
*	vst_height(); 
*	vsl_type();
*	vsl_udsty();
*	vsl_width();
*	v_pline();
*	vst_clip();
*	vr_cpyfm();
*	vq_extnd();
*	v_clsvwk( handle )
*	v_opnvwk( pwork_in, phandle, pwork_out )
*/


#include	<portab.h>
#include 	<gsxdefs.h>
#include 	<bind.h>
#include	<funcdef.h>

EXTERN WORD	gl_handle;
EXTERN WS	gl_ws;

EXTERN WORD	contrl[];
EXTERN WORD	intin[];
EXTERN WORD	ptsin[];
EXTERN WORD	intout[];
EXTERN WORD	ptsout[];

EXTERN VOID	gsx2();
EXTERN VOID	i_ptsin();
EXTERN VOID	i_intin();
EXTERN VOID	i_ptsout();
EXTERN VOID	i_intout();
EXTERN VOID	i_ptr();
EXTERN VOID	i_ptr2();

	VOID
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

	VOID
gsx_vclose()
{
	gsx_ncode(CLOSE_VWORKSTATION, 0, 0);
}

	VOID
v_pline( count, pxyarray )
	WORD	count;
	WORD	*pxyarray;
{
	i_ptsin( pxyarray );
	gsx_ncode(POLYLINE, count, 0);
	i_ptsin( ptsin );
}

	VOID
gsx_1code(code, value)
	WORD		code;
	WORD		value;
{
	intin[0] = value;
	gsx_ncode(code, 0, 1);
}

	VOID
v_opnvwk( pwork_in, phandle, pwork_out )
	WORD		*pwork_in;
	WORD		*phandle;
	WORD		*pwork_out;
{
	WORD		*ptsptr;

	i_intin( pwork_in );	/* set intin to point to callers data  */
	i_intout( pwork_out );	/* set intout to point to callers data */
	ptsptr = pwork_out + 45;
	i_ptsout( ptsptr );	/* set ptsout to work_out array */
	contrl[0] = 100;
	contrl[1] = 0;		  /* no points to xform */
	contrl[3] = 11;		  /* pass down xform mode also */
	contrl[6] = *phandle;
	gsx2();
	*phandle = contrl[6];	 
				 /* reset pointers for next call to binding */
	i_intin( intin );
	i_intout( intout );
	i_ptsin( ptsin );
	i_ptsout( ptsout );
}

	VOID
gsx_vopen()
{
	WORD		i;

	for(i=0; i<10; i++)
	  intin[i] = 1;
	intin[10] = 2;			/* device coordinate space */

	v_opnvwk( intin, &gl_handle, &gl_ws );
}

	WORD
vst_clip( clip_flag, pxyarray )
	WORD	clip_flag;
	WORD	*pxyarray;
{
	WORD		value;

	value = ( clip_flag != 0 ) ? 2 : 0;
	i_ptsin( pxyarray );
	intin[0] = clip_flag;
	gsx_ncode(TEXT_CLIP, value, 1);
	i_ptsin(ptsin);
	return (0);
}

	VOID
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
vsl_width( width )
	WORD	width;
{
	ptsin[0] = width;
	ptsin[1] = 0;
	gsx_ncode(S_LINE_WIDTH, 1, 0);
}

	VOID
vro_cpyfm( wr_mode, pxyarray, psrcMFDB, pdesMFDB, )
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
	i_ptsin( ptsin );
}

	VOID
vrt_cpyfm( wr_mode, pxyarray, psrcMFDB, pdesMFDB, fgcolor, bgcolor )
	WORD	 wr_mode;
	WORD	 *pxyarray;
	WORD	 *psrcMFDB;
	WORD	 *pdesMFDB;
	WORD	fgcolor, bgcolor;
{
	intin[0] = wr_mode;
	intin[1] = fgcolor;
	intin[2] = bgcolor;
	i_ptr( psrcMFDB );
	i_ptr2( pdesMFDB );
	i_ptsin( pxyarray );
	gsx_ncode(121, 4, 3);
	i_ptsin( ptsin );
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
vr_recfl( pxyarray, pdesMFDB )
	WORD	*pxyarray;
	WORD	*pdesMFDB;
{
	i_ptr( pdesMFDB );
	i_ptsin( pxyarray );
	gsx_ncode(FILL_RECTANGLE, 2, 1);
	i_ptsin( ptsin );
}
