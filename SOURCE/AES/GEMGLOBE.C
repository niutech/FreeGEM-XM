/*	GEMGLOBE.C 	4/23/84 - 06/23/85	Lee Lorenzen		*/

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
#include <obdefs.h>
#include <gemlib.h>

#if I8086
GLOBAL WORD		D;
#endif

#if MC68K

GLOBAL THEGLO		D;


	VOID
far_call(fcode, fdata)
	WORD		(*fcode)();
	LONG		fdata;
{
	(*fcode)(fdata);
}
#endif

