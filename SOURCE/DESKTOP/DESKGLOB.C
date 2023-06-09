/*	DESKGLOB.C	06/11/84 - 06/09/85		Lee Lorenzen	*/

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
#include <taddr.h>
#include <deskapp.h>
#include <deskfpd.h>
#include <deskwin.h>
#include <dos.h>
#include <infodef.h>
#include <desktop.h>
#include <gembind.h>
#include <deskbind.h>

/* BugFix	*/
/* this has been pulled out of GLOBES & moved here from DESKBIND.H	*/
GLOBAL ICONBLK		gl_icons[NUM_WOBS];
/* */

#if MC68K
GLOBAL GLOBES		G;  
#endif

GLOBAL	WORD		G;
