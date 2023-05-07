/*	FILTAB.H	1/30/86					MDF	*/

/* Definitions for filter table structure				*/


/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                                                         
*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 3.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1986			Digital Research Inc.
*	-------------------------------------------------------------
*/

/* Filter table strings are WORDs instead of BYTEs because the stored	*/
/* characters have the ASCII code (lo byte) and the extended code --	*/
/* usually the scan code (hi byte).					*/


#define FILTABLE	struct filtable
#define FLTSTRSZ	4

FILTABLE
{
	WORD	flt_strt[FLTSTRSZ];		/* paste begin		*/
	WORD	flt_skip[FLTSTRSZ];		/* characters to skip	*/
	WORD	flt_eol[FLTSTRSZ];		/* end of line		*/
	WORD	flt_etxt[FLTSTRSZ];		/* paste end		*/
	WORD	flt_flch;			/* left margin pad char	*/
	BYTE	flt_curs;			/* uses cursor keys 	*/
	BYTE	flt_marg;			/* left margin		*/
};
