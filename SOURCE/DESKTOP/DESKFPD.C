/*	DESKFPD.C	06/11/84 - 03/29/85		Lee Lorenzen	*/

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
#include <deskapp.h>
#include <deskfpd.h>
#include <deskwin.h>
#include <dos.h>
#include <infodef.h>
#include <desktop.h>
#include <gembind.h>
#include <deskbind.h>

WORD pncount = 0;

EXTERN BYTE	*scasb();
EXTERN WORD	dos_gdrv();
EXTERN WORD	strchk();
EXTERN WORD	dos_sdta();
EXTERN VOID	movs();
EXTERN WORD	dos_sfirst();
EXTERN WORD	dos_snext();

EXTERN WORD	DOS_AX;
EXTERN GLOBES	G;

GLOBAL FNODE	*ml_pfndx[NUM_FNODES];

/*
*	Initialize the list of fnodes
*/
	VOID
fn_init()
{
	WORD		i;

	for(i=NUM_FNODES - 2; i >= 0; i--)
	  G.g_flist[i].f_next = &G.g_flist[i + 1];
	G.g_favail = &G.g_flist[0];
	G.g_flist[NUM_FNODES - 1].f_next = (FNODE *) NULL;
}


/*
*	Initialize the list of pnodes
*/
	VOID
pn_init()
{
	WORD		i;

	for(i=NUM_PNODES - 2; i >= 0; i--)
	  G.g_plist[i].p_next = &G.g_plist[i + 1];
	G.g_pavail = &G.g_plist[0];
	G.g_phead = (PNODE *) NULL;
	G.g_plist[NUM_PNODES - 1].p_next = (PNODE *) NULL;
}

/*
*	Start up by initializing global variables
*/
	VOID
fpd_start()
{
	G.a_wdta = ADDR(&G.g_wdta[0]);
	G.a_wspec = ADDR(&G.g_wspec[0]);
	fn_init();
	pn_init();
}

/*
*	Build a filespec out of drive letter, a pointer to a path, a pointer
*	to a filename, and a pointer to an extension.
*/
	VOID
fpd_bldspec(drive, ppath, pname, pext, pspec)
	WORD		drive;
	BYTE		*ppath, *pname, *pext;
	BYTE		*pspec;
{
	*pspec++ = drive;
	*pspec++ = ':';
	*pspec++ = '\\';
	if (*ppath)
	{
	  while (*ppath)
	    *pspec++ = *ppath++;
	  if (*pname)
	    *pspec++ = '\\';
	}

	if (*pname)
	{
	  while (*pname)
	    *pspec++ = *pname++;
	  if (*pext)
	  {
	    *pspec++ = '.';
	    while (*pext)
	      *pspec++ = *pext++;
	  }
	}
	*pspec++ = NULL;
}

/*
*	Parse a filespec into its drive, path, name, and extension
*	parts.
*/
	VOID
fpd_parse(pspec, pdrv, ppath, pname, pext)
	BYTE		*pspec;
	WORD		*pdrv;
	BYTE		*ppath, *pname, *pext;
{
	BYTE		*pstart, *p1st, *plast, *pperiod;

	pstart = pspec;
						/* get the drive */
	while ( (*pspec) &&
		(*pspec != ':') )
	  pspec++;
	if (*pspec == ':')
	{
	  pspec--;
	  *pdrv = (WORD) *pspec;
	  pspec++;
	  pspec++;
	  if ( *pspec == '\\')
	    pspec++;
	}
	else
	{
	  *pdrv = (WORD) (dos_gdrv() + 'A');
	  pspec = pstart;
	}
						/* scan for key bytes	*/
	p1st = pspec;
	plast = pspec;
	pperiod = NULLPTR;
	while( *pspec )
	{
	  if (*pspec == '\\')
	    plast = pspec;
	  if (*pspec == '.')
	    pperiod = pspec;
	  pspec++;
	}
	if (pperiod == NULLPTR)
	  pperiod = pspec;
						/* get the path	*/
	while (p1st != plast)
	  *ppath++ = *p1st++;
	*ppath = NULL;
	if (*plast == '\\')
	  plast++;
						/* get the name	*/
	while (plast != pperiod)
	  *pname++ = *plast++;
	*pname = NULL;
						/* get the ext	*/
	if ( *pperiod )
	{
	  pperiod++;
	  while (pperiod != pspec)
	    *pext++ = *pperiod++; 
	}
	*pext = NULL;
}



/*
*	Find the file node that matches a particular object id.
*/

	FNODE
*fpd_ofind(pf, obj)
	FNODE		*pf;
	WORD		obj;
{
	while(pf)
	{
	  if (pf->f_obid == obj)
	    return(pf);
	  pf = pf->f_next;
	}
	return(0);
}


/*
*	Find the list item that is after start and points to stop item.
*/

	BYTE
*fpd_elist(pfpd, pstop)
	FNODE		*pfpd;
	FNODE		*pstop;
{
	while( pfpd->f_next != pstop )
	  pfpd = pfpd->f_next;
	return( (BYTE *) pfpd);
}


/*
*	Free a single file node
*/
	VOID
fn_free(thefile)
	FNODE		*thefile;
{
	thefile->f_next = G.g_favail;
	G.g_favail = thefile;
}


/*
*	Free a list of file nodes.
*/
	VOID
fl_free(pflist)
	FNODE		*pflist;
{
	FNODE		*thelast;

	if (pflist)
	{
	  thelast = (FNODE *) fpd_elist(pflist, NULLPTR);
	  thelast->f_next = G.g_favail;
	  G.g_favail = pflist;
	}
}


/*
*	Allocate a file node.
*/

	FNODE
*fn_alloc()
{
	FNODE		*thefile;

	if ( G.g_favail )
	{
	  thefile = G.g_favail;
	  G.g_favail = G.g_favail->f_next;
	  return(thefile);
	}
	return(0);
}


/*
*	Allocate a path node.
*/

	PNODE
*pn_alloc()
{
	PNODE		*thepath;
	
pncount++;
	
	if ( G.g_pavail )
	{
						/* get up off the avail	*/
						/*   list		*/
	  thepath = G.g_pavail;
	  G.g_pavail = G.g_pavail->p_next;
						/* put us on the active	*/
						/*   list		*/
	  thepath->p_next = G.g_phead;
	  G.g_phead = thepath;
						/* init. and return	*/
	  thepath->p_flags = 0x0;
	  thepath->p_flist = (FNODE *) NULL;
	  return(thepath);
	}
	return(0);
}

/*
*	Free a path node.
*/
	VOID
pn_free(thepath)
	PNODE		*thepath;
{
	PNODE		*pp;
pncount--;

						/* free our file list	*/
	fl_free(thepath->p_flist);
						/* if first in list	*/
						/*   unlink by changing	*/
						/*   phead else by	*/
						/*   finding and chang-	*/
						/*   our previouse guy	*/
	pp = (PNODE *) &G.g_phead;
	while (pp->p_next != thepath)
	  pp = pp->p_next;
	pp->p_next = thepath->p_next;
						/* put us on the avail	*/
						/*   list		*/
	thepath->p_next = G.g_pavail;
	G.g_pavail = thepath;
}

/*
*	Open a particular path.  
*/
	PNODE
*pn_open(drive, path, name, ext, attr)
	WORD		drive;
	BYTE		*path, *name, *ext;
	WORD		attr;
{
	PNODE		*thepath;

	thepath = pn_alloc();
	if (thepath)
	{
	  fpd_bldspec(drive, path, name, ext, &thepath->p_spec[0]);
	  thepath->p_attr = attr;
	  return(thepath);
	}
	else
	  return(0);
}

/*
*	Close a particular path.
*/
	VOID
pn_close(thepath)
	PNODE		*thepath;
{
	pn_free(thepath);
}

/*
*	Compare file nodes pf1 & pf2, using a field
*	determined by which
*/
	WORD
pn_fcomp(pf1, pf2, which)
	FNODE		*pf1, *pf2;
	WORD		which;
{
	WORD		chk;
	BYTE		*ps1, *ps2;

	ps1 = &pf1->f_name[0];
	ps2 = &pf2->f_name[0];

	switch (which)
	{
	  case S_SIZE:
		if (pf2->f_size > pf1->f_size)
		  return(1);
		if (pf2->f_size < pf1->f_size)
		  return(-1);
		return( strchk(ps1, ps2) );
          case S_TYPE:
	  	chk = strchk(scasb(ps1, '.'), scasb(ps2, '.'));
		if (chk)
		  return(chk);
							/* == falls thru*/
	  case S_NAME:
		return( strchk(ps1, ps2) );
	  case S_DATE:
		chk = pf2->f_date - pf1->f_date;
		if (chk)
		  return(chk);
		else
		{
/* BugFix	*/
/*		  return((pf2->f_time >> 5) - (pf1->f_time >> 5));*/
		  chk = (pf2->f_time >> 11) - (pf1->f_time >> 11);
		  if (chk)
		    return(chk);
		  chk = ((pf2->f_time >> 5) & 0x003F) -
		  	((pf1->f_time >> 5) & 0x003F);
		  if (chk)
		    return(chk);
		  return ( (pf2->f_time & 0x001F) - (pf1->f_time & 0x001F) );
		} /* else */
/* */
	} /* of switch */
}

/*
*	Routine to compare two fnodes to see which one is greater.
*	Folders always sort out first, and then it is based on
*	the G.g_isort parameter.  Return (-1) if pf1 < pf2, (0) if
*	pf1 == pf2, and (1) if pf1 > pf2.
*/
	WORD
pn_comp(pf1, pf2)
	FNODE		*pf1, *pf2;
{
	if (pf1->f_attr & F_FAKE)
	  return( -1 );

	if (pf2->f_attr & F_FAKE)
	  return( 1 );

	if ( (pf1->f_attr ^ pf2->f_attr) & F_SUBDIR)
	  return ((pf1->f_attr & F_SUBDIR)? -1: 1);
	else
	  return (pn_fcomp(pf1,pf2,G.g_isort)); 
}

/*
*
*
*/
	FNODE
*pn_sort(lstcnt, pflist)
	WORD		lstcnt;
	FNODE		*pflist;
{
	FNODE		*pf, *pftemp;
	FNODE		*newlist;
	WORD		gap, i, j;
						/* build index array	*/
						/*   if necessary	*/
	if (lstcnt == -1)
	{
	  lstcnt = 0;
	  for(pf=pflist; pf; pf=pf->f_next)
	    ml_pfndx[lstcnt++] = pf;
	}
						/* sort files using shell*/
						/*   sort on page 108 of */
						/*   K&R C Prog. Lang.	*/
	for(gap = lstcnt/2; gap > 0; gap /= 2)
	{
	  for(i = gap; i < lstcnt; i++)
	  {
	    for (j = i-gap; j >= 0; j -= gap)
	    {
	      if ( pn_comp(ml_pfndx[j], ml_pfndx[j+gap]) <= 0 )
		break;
	      pftemp = ml_pfndx[j];
	      ml_pfndx[j] = ml_pfndx[j+gap];
	      ml_pfndx[j+gap] = pftemp;
	    }
	  }
	}
						/* link up the list in	*/
						/*   order		*/
	newlist = (FNODE *) NULL;
	pf = (FNODE *) &newlist;
	for(i=0; i<lstcnt; i++)
	{
	  pf->f_next = ml_pfndx[i];
	  pf = ml_pfndx[i];
	}
	pf->f_next = (FNODE *) NULL;
	return(newlist);
}



/*
*	Make a particular path the active path.  This involves
*	reading its directory, initializing a file list, and filling
*	out the information in the path node.
*/
	WORD
pn_folder(thepath)
	PNODE		*thepath;
{
	WORD		ret, firstime;
	FNODE		*thefile, *prevfile;

	thepath->p_count = 0;
	thepath->p_size = 0x0L;
	fl_free(thepath->p_flist);
	thefile = (FNODE *) NULLPTR;
	prevfile = (FNODE *) &thepath->p_flist;
						/* set of fake new	*/
						/*   folder entry	*/
	thefile = fn_alloc();
	thefile->f_junk = 0x0;
	thefile->f_attr = F_FAKE | F_SUBDIR;
	thefile->f_time = 0x0;
	thefile->f_date = 0x0;
	thefile->f_size = 0x0L;
	strcpy("New Folder", &thefile->f_name[0]);
						/* init for while loop	*/
	G.g_wdta[30] = NULL;
	dos_sdta(G.a_wdta);
	ret = firstime = TRUE;
	while ( ret )
	{
	  if ( !thefile )
	  {
	    thefile = fn_alloc();
	    if ( !thefile )
	    {	
	      ret = FALSE;
	      DOS_AX = E_NOFNODES;
	    }
	  }
	  else
	  {
						/* make so each dir.	*/
						/*   has a available new*/
						/*   folder		*/
	    if ( G.g_wdta[30] != '.' )
	    {
						/* if it is a real file	*/
						/*   or directory then	*/
						/*   save it		*/
	      if (!firstime)
	      {
	        movs(23, &G.g_wdta[20], &thefile->f_junk);
	        thefile->f_attr &= ~(F_DESKTOP | F_FAKE);
	      }
	      thepath->p_size += thefile->f_size;
	      prevfile->f_next = ml_pfndx[thepath->p_count++] = thefile;
	      prevfile = thefile;
	      thefile = (FNODE *) NULL;
	    }
	    if (firstime)
	    {
	      ret = dos_sfirst(ADDR(&thepath->p_spec[0]), thepath->p_attr);
	      firstime = FALSE;
	    }
	    else
	      ret = dos_snext();
	  }
	}
	prevfile->f_next = (FNODE *) NULLPTR;
	if ( thefile )
	  fn_free(thefile);
	thepath->p_flist = pn_sort(thepath->p_count, thepath->p_flist);
	return(DOS_AX);
}

/*
*	Make a particular path the active path.  This involves
*	reading its directory, initializing a file list, and filling
*	out the information in the path node.
*/
	WORD
pn_desktop(thepath)
	PNODE		*thepath;
{
	FNODE		*thefile, *prevfile;
	ANODE		*pa;

	thepath->p_count = 0;
	thepath->p_size = 0x0L;
	fl_free(thepath->p_flist);
	thefile = (FNODE *) NULLPTR;
	prevfile = (FNODE *) &thepath->p_flist;
	pa = G.g_ahead;
	while ( pa )
	{
	  if ( !thefile )
	  {
	    thefile = fn_alloc();
	    if ( !thefile )
	    {	
	      pa = 0;
	      DOS_AX = E_NOFNODES;
	    }
	  }
	  else
	  {
	    if (pa->a_flags & AF_ISDESK)
	    {
	      thefile->f_junk = (0x00ff & pa->a_letter);
	      thefile->f_attr = F_DESKTOP | F_SUBDIR;
	      thefile->f_time = 0x0;
	      thefile->f_date = 0x0;
	      thefile->f_size = 0x0L;
	      thefile->f_isap = TRUE;
	      strcpy(pa->a_pappl, &thefile->f_name[0]);
	      thefile->f_obid = pa->a_obid;
	      thefile->f_pa = pa;
	      thepath->p_size += thefile->f_size;
	      prevfile->f_next = ml_pfndx[thepath->p_count++] = thefile;
	      prevfile = thefile;
	      thefile = (FNODE *) NULL;
	    }
	    pa = pa->a_next;
	  }
	}
	prevfile->f_next = (FNODE *) NULLPTR;
	if ( thefile )
	  fn_free(thefile);
	thepath->p_flist = pn_sort(thepath->p_count, thepath->p_flist);
	return(0);
}


	WORD
pn_active(thepath)
	PNODE		*thepath;
{
	if (thepath->p_spec[0] == '@')
	  return( pn_desktop(thepath) );
	else
	  return( pn_folder(thepath) );
}
