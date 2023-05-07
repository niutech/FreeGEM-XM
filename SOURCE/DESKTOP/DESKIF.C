/*	DESKIF.C	12/03/84 - 02/09/85	Lee Lorenzen		*/

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
#include <gembind.h>

EXTERN WORD	graf_mouse();

	VOID
gsx_moff()
{
	graf_mouse(M_OFF, 0x0L);
}

	VOID
gsx_mon()
{
	graf_mouse(M_ON, 0x0L);
}

	LONG
obaddr(tree, obj, fld_off)
	LONG		tree;
	WORD		obj;
	WORD		fld_off;
{
	return( (tree + ((obj) * sizeof(OBJECT) + fld_off)) );
}
