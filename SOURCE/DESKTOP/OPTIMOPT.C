/*	OPTIMOPT.C	1/25/84 - 03/20/85	Lee Jay Lorenzen	*/

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
#include <taddr.h>
#include <obdefs.h>

	VOID
r_get(pxywh, px, py, pw, ph)
	WORD		*pxywh;
	WORD		*px, *py, *pw, *ph;
{
	*px = pxywh[0];
	*py = pxywh[1];
	*pw = pxywh[2];
	*ph = pxywh[3];
}


	VOID
r_set(pxywh, x, y, w, h)
	WORD		*pxywh;
	WORD		x, y, w, h;
{
	pxywh[0] = x;
	pxywh[1] = y;
	pxywh[2] = w;
	pxywh[3] = h;
}


	WORD
rc_equal(p1, p2)
	WORD		*p1, *p2;
{
	WORD		i;

	for(i=0; i<4; i++)
	{
	  if (*p1++ != *p2++)
	    return(FALSE);
	}
	return(TRUE);
}


	VOID
rc_copy(psxywh, pdxywh)
	WORD		*psxywh;
	WORD		*pdxywh;
{
	*pdxywh++ = *psxywh++;
	*pdxywh++ = *psxywh++;
	*pdxywh++ = *psxywh++;
	*pdxywh++ = *psxywh++;
}


	UWORD
inside(x, y, pt)
	WORD		x, y;
	GRECT		*pt;
{
	if ( (x >= pt->g_x) && (y >= pt->g_y) &&
	     (x < pt->g_x + pt->g_w) && (y < pt->g_y + pt->g_h) )
	  return(TRUE);
	else
	  return(FALSE);
}

	WORD
min(a, b)
	WORD		a, b;
{
	return( (a < b) ? a : b );
}


	WORD
max(a, b)
	WORD		a, b;
{
	return( (a > b) ? a : b );
}



	BYTE
toupper(ch)
	BYTE		ch;
{
	if ( (ch >= 'a') &&
	     (ch <= 'z') )
	  return(ch - 32);
	else
	  return(ch);
}


	BYTE
*scasb(p, b)
	BYTE		*p;
	BYTE		b;
{
	for(; *p && *p != b; p++);
	return (p);
}


	VOID
movs(num, ps, pd)
	WORD		num;
	BYTE		*ps, *pd;
{
	do
	  *pd++ = *ps++;
	while (--num);
}


	VOID
bfill(num, bval, addr)
	REG WORD	num;
	BYTE		bval;
	REG BYTE	*addr;
{
	do
	  *addr++ = bval;
	while (--num);
}

	WORD
strlen(p1)
	BYTE		*p1;
{
	WORD		len;

	len = 0;
	while( *p1++ )
	  len++;

	return(len);
}


	WORD
strcmp(p1, p2)
	BYTE		*p1, *p2;
{
	while(*p1)
	{
	  if (*p1++ != *p2++)
	    return(FALSE);
	}
	if (*p2)
	  return(FALSE);
	return(TRUE);
}


/*
*	Return <0 if s<t, 0 if s==t, >0 if s>t
*/

	WORD
strchk(s, t)
	BYTE		s[], t[];
{
	WORD		i;

	i = 0;
	while( s[i] == t[i] )
	  if (s[i++] == NULL)
	    return(0);
	return(s[i] - t[i]);
}


	BYTE
*strcpy(ps, pd)
	BYTE		*ps, *pd;
{
	while( (*pd++ = *ps++) != 0 )
	  ;
	return(pd);
}
