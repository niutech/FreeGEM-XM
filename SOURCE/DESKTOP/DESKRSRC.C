/*	DESKRSRC.C	05/04/84 - 06/10/85	Lee Lorenzen		*/
/*	for 3.0		4/25/86			MDF			*/

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

#include "portab.h"
#include "machine.h"
#include "obdefs.h"

#define R_STRING 5

EXTERN WORD	rsrc_gaddr();

GLOBAL	BYTE	gl_lngstr[256];

/*
ini_str(ST_RSRCFILE);
ini_str(ST_TYPE);		;
ini_str(ST_PRINT);		;
ini_str(STFORMAT);		;
ini_str(ST_DISKCOPY);		;
ini_str(STGEMHIC);		;
ini_str(STGEMAPP);		;
ini_str(STGEMLOI);		;
ini_str(STGEMOUT);		;
ini_str(STGEMBAT);		;
*/
	BYTE
*ini_str(stnum)
	WORD		stnum;
{
	LONG		lstr;

	rsrc_gaddr(R_STRING, stnum, &lstr);
	LSTCPY(ADDR(&gl_lngstr[0]), lstr);
	return(&gl_lngstr[0]);
}
