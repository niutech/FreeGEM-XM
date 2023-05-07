/*	GEMSCLIB.C	07/10/84 - 02/02/85	Lee Lorenzen		*/
/*	for 2.0		10/8/85	 - 10/15/85	MDF			*/

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
#include <basepage.h>
#include <obdefs.h>
#include <gemlib.h>
#include <crysbind.h>
#include <dos.h>
#include <gem.h>

#define NUM_SCRAPS	6

EXTERN LONG		ad_dta;
EXTERN LONG		ad_sysglo;
EXTERN THEGLO		D;
GLOBAL LONG		ad_scrap;
GLOBAL BYTE	*sc_types[NUM_SCRAPS] =
			 {"CSV", "TXT", "GEM",
			  "IMG", "DCA", "USR"};
GLOBAL WORD	sc_bits[NUM_SCRAPS] =
			 {SC_FTCSV,SC_FTTXT,SC_FTGEM,
			  SC_FTIMG,SC_FTDCA,SC_FTUSR};

	WORD
sc_clrd(isread)
	WORD		isread;
{
	LONG		ptmp, ptype;
	WORD		bitvect, ii;

	ptmp = ad_scrap;
	while(LBGET(ptmp))			/* find null		*/
	  ptmp++;
	rs_gaddr(ad_sysglo, R_STRING, STSCRAP, &ptype);
	LSTCPY(ptmp, ptype); 
	ptype = ptmp+7;				/* point just past '.'	*/
	LBSET(ptmp, 0);				/* keep just path name	*/
	bitvect = 0;
	dos_sdta(ad_dta);			/* make sure dta ok	*/
	for (ii = 0; ii < NUM_SCRAPS; ii++)
	{
	  LSTCPY(ptype, ADDR(sc_types[ii]));	/* cat on file type	*/
	  if (dos_sfirst(ad_scrap, F_SUBDIR))
	  {
	    if (isread)
	      bitvect |= sc_bits[ii];		/* set corresponding bit */
	    else
	      dos_delete(ad_scrap);		/* delete scrap.*	*/
	  }
	}
	if ( !isread)
	  bitvect = TRUE;
	return(bitvect);
}

/************************************************************************/
/*									*/
/* sc_read() -- get info about current scrap directory			*/
/*									*/
/*	copies the current scrap directory path to the passed-in 	*/
/*	address and returns a bit vector with bits set for specific	*/
/*	file types present in the directory.  Looks for scrap.* files.	*/
/*									*/
/************************************************************************/

	WORD
sc_read(pscrap)
	LONG		pscrap;
{
	LSTCPY(pscrap, ad_scrap);		/* current scrap directory */
	return( sc_clrd(TRUE) );
}

/************************************************************************/
/*									*/
/* sc_write() -- sets the current scrap directory			*/
/*									*/
/*	pscrap must be the long address of a valid path.  Returns	*/
/*	TRUE if no error occurs in validating the path name.		*/
/*									*/
/************************************************************************/

	WORD
sc_write(pscrap)
	LONG		pscrap;
{
	LSTCPY(ad_scrap, pscrap);		/* new scrap directory	*/
	dos_sdta(ad_dta);			/* use our dta 		*/
	return(dos_sfirst(ad_scrap, F_SUBDIR));	/* make sure path ok	*/
}

/************************************************************************/
/*									*/
/* sc_clear() -- delete scrap files from current scrap directory	*/
/*									*/
/*	Assumes *ad_scrap holds a valid directory path.  Returns TRUE	*/
/*									*/
/************************************************************************/

	WORD
sc_clear()
{
	return( sc_clrd(FALSE) );
}
