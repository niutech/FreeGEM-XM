/*	GEMNULP.C	5/14/84 - 07/11/85	Lee Jay Lorenzen	*/

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
#include <taddr.h>
#include <struct.h>
#include <basepage.h>
#include <gemlib.h>

EXTERN VOID	sh_ldapp();
GLOBAL WORD	recv_msg[8];

hnul_keybd()
{
}


hnul_mesag()
{
	switch(recv_msg[0])
	{
	  case NP_RUNIT:
		sh_ldapp();
		break;
	  default:
	  	break;
	}
}


/*
*	Internal process context used to control the screen for use by
*	the menu manager, and the window manager.
*	This process never terminates and forms an integral part of
*	the system.
*/
	VOID
nulmgr()
{
	REG WORD	ev_which;
	WORD		rets[6];
	MOBLK		mb;

	while(TRUE)
	{
	  ev_which = ev_multi(MU_KEYBD | MU_MESAG, 
			&mb, &mb, 
			0x0L, 0x0001ff01L, ADDR(&recv_msg[0]), &rets[0]);

	  if (ev_which & MU_KEYBD)		/* bit bucket keyboard	*/
	    hnul_keybd();
						/* must be a load	*/
	  if (ev_which & MU_MESAG)
	    hnul_mesag();
	}
}
