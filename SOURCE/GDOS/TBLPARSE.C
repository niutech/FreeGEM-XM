/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*/
/************************************************************************/
/*	File:	tblparse.c						*/
/************************************************************************/
/*									*/
/*		     GGGGG        EEEEEEEE     MM      MM		*/
/*		   GG             EE           MMMM  MMMM		*/
/*		   GG   GGG       EEEEE        MM  MM  MM		*/
/*		   GG   GG        EE           MM      MM		*/
/*		     GGGGG        EEEEEEEE     MM      MM		*/
/*									*/
/************************************************************************/
/*									*/
/*			  +--------------------------+			*/
/*			  | Digital Research, Inc.   |			*/
/*			  | 60 Garden Court	     |			*/
/*			  | Monterey, CA.     93940  |			*/
/*			  +--------------------------+			*/
/*									*/
/************************************************************************/


#include "portab.h"				/* portable coding conv	*/
#include "machine.h"				/* machine depndnt conv	*/


EXTERN	WORD	strlen();


	WORD
strlcmp(str,adr)
	BYTE	*str;
	LONG	adr;
{
	WORD	cmp;

	while (*str)
	{
	  if (cmp = (*str++ - LBGET(adr++)))
	    return(cmp);
	}
	return(0);
}

	LONG
match(str, buff, bufflen)
	BYTE	*str;
	LONG	buff;
	WORD	bufflen;
{
	WORD	slen;
	LONG	obuff;

	slen = strlen(str);
	obuff = buff;
	while (buff+slen < obuff+bufflen)
	{
	  if (strlcmp(str,buff) == 0)
	    return(buff+slen);
	  buff++;
	}
	return(0l);
}

	WORD
parse_tbl(fbuff,flen,keys,adrs,nkwrds)
	LONG	fbuff;				/* long ptr to file buff*/
	WORD	flen;				/* num bytes in file	*/
	BYTE	*keys[];			/* keywords		*/
	LONG	adrs[];				/* place to stuff addrs	*/
	WORD	nkwrds;				/* num keywords/pointers*/
{
	WORD	i, ret;

	ret = 0;
	for (i=0; i<nkwrds; i++)
	{
	  adrs[i] = match(keys[i], fbuff, flen);
	  if (adrs[i])
	    ret++;				/* found keyword	*/
	}
	for (i=0; i<flen; i++)
	  if (LBGET(fbuff+i) < 27)		/* CTRL character	*/
	    LBSET(fbuff+i, '\000');		/*  NULL 'em out	*/	
	return(ret);
}


