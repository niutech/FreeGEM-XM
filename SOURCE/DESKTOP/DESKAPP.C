/*	 DESKAPP.C	06/11/84 - 07/11/85		Lee Lorenzen	*/
/*	for 3.0		3/6/86   - 5/6/86		MDF		*/

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
#include <dos.h>
#include <deskapp.h>
#include <deskfpd.h>
#include <deskwin.h>
#include <desktop.h>
#include <infodef.h>
#include <gembind.h>
#include <deskbind.h>

#define MIN_WINT 4
#define MIN_HINT 2
						/* External Routines	*/
EXTERN LONG	dos_alloc();
EXTERN BYTE	*ini_str();
EXTERN WORD	wildcmp();
EXTERN WORD	rsrc_gaddr();
EXTERN VOID	gsx_trans();
EXTERN WORD	dos_gdrv();
EXTERN WORD	dos_sdrv();
EXTERN WORD	shel_find();
EXTERN WORD	dos_open();
EXTERN WORD	dos_create();
EXTERN WORD	dos_read();
EXTERN VOID	movs();
EXTERN WORD	max();
EXTERN WORD	dos_close();
EXTERN WORD	shel_get();
EXTERN WORD	sound();
EXTERN VOID	bfill();
EXTERN VOID	shel_put();
EXTERN WORD	fun_alert();
EXTERN WORD	dos_write();
EXTERN VOID	obj_wfree();
EXTERN WORD	obj_ialloc();
EXTERN WORD	appl_bvset();

EXTERN WORD	gl_wchar;
EXTERN WORD	gl_hchar;
EXTERN WORD	gl_wschar;
EXTERN WORD	gl_hschar;
EXTERN WORD	gl_width;
EXTERN WORD	gl_height;
EXTERN WORD	gl_hbox;
EXTERN WORD	DOS_ERR;
EXTERN UWORD	global[];

EXTERN GRECT	gl_savewin[NUM_WNODES];
/* BugFix	*/
EXTERN ICONBLK	gl_icons[];
/* */

EXTERN GLOBES	G;

/* forward declaration	*/
BYTE		get_defdrv();

#if ALCYON
GLOBAL BYTE	*gl_pstart;
#endif

#if I8086
GLOBAL WORD	gl_pstart;
#endif

GLOBAL WORD	gl_poffset;
GLOBAL WORD	gl_numics;

GLOBAL WORD	gl_stdrv;

GLOBAL BYTE	gl_afile[SIZE_AFILE];
GLOBAL BYTE	gl_buffer[SIZE_BUFF];

#if MULTIAPP
GLOBAL ACCNODE	gl_caccs[3];
#endif

/*
*	Allocate an application object.
*/
	ANODE
*app_alloc(tohead)
	WORD		tohead;
{
	ANODE		*pa, *ptmpa;

	pa = G.g_aavail;
	if (pa)
	{
	  G.g_aavail = pa->a_next;
	  if ( (tohead) ||
	       (!G.g_ahead) )
	  {
	    pa->a_next = G.g_ahead;
	    G.g_ahead = pa;
	  }
	  else
	  {
	    ptmpa = G.g_ahead;
	    while( ptmpa->a_next )
	      ptmpa = ptmpa->a_next;
	    ptmpa->a_next = pa;
	    pa->a_next = (ANODE *) NULL;
	  }
	}
	return(pa);
}


/*
*	Free an application object.
*/

	VOID
app_free(pa)
	ANODE		*pa;
{
	ANODE		*ptmpa;

	if (G.g_ahead == pa)
	  G.g_ahead = pa->a_next;
	else
	{
	  ptmpa = G.g_ahead;
	  while ( (ptmpa) &&
		  (ptmpa->a_next != pa) )
	    ptmpa = ptmpa->a_next;
	  if (ptmpa)
	    ptmpa->a_next = pa->a_next;
	}
	pa->a_next = G.g_aavail;
	G.g_aavail = pa;
}


/*
*	Convert a single hex ASCII digit to a number
*/

	WORD
hex_dig(achar)
	BYTE		achar;
{
	if ( (achar >= '0') &&
	     (achar <= '9') )
	  return(achar - '0');	
	if ( (achar >= 'A') &&
	     (achar <= 'F') )
	  return(achar - 'A' + 10);
	return(0);
}
	

/*
*	Reverse of hex_dig().
*/

	BYTE
uhex_dig(wd)
	WORD		wd;
{
	if ( (wd >= 0) &&
	     (wd <= 9) )
	  return(wd + '0');	
	if ( (wd >= 0x0a) &&
	     (wd <= 0x0f) )
	  return(wd + 'A' - 0x0a);
	return(' ');
}
	

/*
*	Scan off and convert the next two hex digits and return with
*	pcurr pointing one space past the end of the four hex digits
*/

	BYTE
*scan_2(pcurr, pwd)
	BYTE		*pcurr;
	UWORD		*pwd;
{
	UWORD		temp;
	
	temp = 0x0;
	temp |= hex_dig(*pcurr++) << 4;
	temp |= hex_dig(*pcurr++);
	if (temp == 0x00ff)
	  temp = NIL;
	*pwd = temp;
	return(	pcurr );
}

/*
*	Reverse of scan_2().
*/

	BYTE
*save_2(pcurr, wd)
	BYTE		*pcurr;
	UWORD		wd;
{
	*pcurr++ = uhex_dig((wd >> 4) & 0x000f);
	*pcurr++ = uhex_dig(wd & 0x000f);
	return(	pcurr );
}

#if MULTIAPP


/*
*	Scan off and convert the next four hex digits and return with
*	pcurr pointing one space past the end of the four hex digits.
*	Start of field is marked with an 'R'.  If no field, set it to
*	default memory size -- DEFMEMREQ.
*/

	BYTE
*scan_memsz(pcurr, pwd)
	BYTE		*pcurr;
	UWORD		*pwd;
{
	UWORD		temp1, temp2;
	
	temp1 = 0x0;
	while (*pcurr == ' ')
	  pcurr++;
	if (*pcurr == 'R')
	{
	  pcurr++;				
	  pcurr = scan_2(pcurr, &temp1);		/* hi byte	*/
	  pcurr = scan_2(pcurr, &temp2);		/* lo byte	*/
	  temp1 = ((temp1 << 8) & 0xff00) | temp2;
	}
	if (temp1 == 0)
	  temp1 = DEFMEMREQ;
	*pwd = temp1;
	return(	pcurr );
}

/*
*	Reverse of scan_memsz().
*/

	BYTE
*save_memsz(pcurr, wd)
	BYTE		*pcurr;
	UWORD		wd;
{
	*pcurr++ = 'R';
	pcurr = save_2(pcurr, LHIBT(wd));
	pcurr = save_2(pcurr, LLOBT(wd));
	return(	pcurr );
}

#endif

/*
*	Scan off spaces until a string is encountered.  An @ denotes
*	a null string.  Copy the string into a string buffer until
*	a @ is encountered.  This denotes the end of the string.  Advance
*	pcurr past the last byte of the string.
*/

	BYTE
*scan_str(pcurr, ppstr)
	BYTE		*pcurr;
	BYTE		**ppstr;
{
	while(*pcurr == ' ')
	  pcurr++;
	*ppstr = G.g_pbuff;
	while(*pcurr != '@')
	  *G.g_pbuff++ = *pcurr++;
	*G.g_pbuff++ = NULL;
	pcurr++;
	return(pcurr);
}


/*
*	Reverse of scan_str.
*/

	BYTE
*save_str(pcurr, pstr)
	BYTE		*pcurr;
	BYTE		*pstr;
{
	while(*pstr)
	  *pcurr++ = *pstr++;
	*pcurr++ = '@';
	*pcurr++ = ' ';
	return(pcurr);
}


/*
*	Parse a single line from the DESKTOP.INF file.
*/

	BYTE
*app_parse(pcurr, pa)
	BYTE		*pcurr;
	ANODE		*pa;
{
	switch(*pcurr)
	{
	  case 'M':				/* Storage Media	*/
		pa->a_type = AT_ISDISK;
		pa->a_flags = AF_ISCRYS | AF_ISGRAF | AF_ISDESK;
		break;
	  case 'G':				/* GEM App File		*/
		pa->a_type = AT_ISFILE;
		pa->a_flags = AF_ISCRYS | AF_ISGRAF;
		break;
	  case 'F':				/* DOS File no parms	*/
	  case 'f':				/*   needs full memory	*/
		pa->a_type = AT_ISFILE;
		pa->a_flags = (*pcurr == 'F') ? NONE : AF_ISFMEM;
		break;
	  case 'P':				/* DOS App needs parms	*/
	  case 'p':				/*   needs full memory	*/
		pa->a_type = AT_ISFILE;
		pa->a_flags = (*pcurr == 'P') ? 
				AF_ISPARM : AF_ISPARM | AF_ISFMEM;
		break;
	  case 'D':				/* Directory (Folder)	*/
		pa->a_type = AT_ISFOLD;
		break;
	}
	pcurr++;
	if (pa->a_flags & AF_ISDESK)
	{
	  pcurr = scan_2(pcurr, &pa->a_xspot);
	  pcurr = scan_2(pcurr, &pa->a_yspot);
	}
	pcurr = scan_2(pcurr, &pa->a_aicon);
	pcurr = scan_2(pcurr, &pa->a_dicon);
	pcurr++;
	if (pa->a_flags & AF_ISDESK)
	{
	  pa->a_letter = (*pcurr == ' ') ? NULL : *pcurr;
	  pcurr += 2;
	}
	pcurr = scan_str(pcurr, &pa->a_pappl);
	pcurr = scan_str(pcurr, &pa->a_pdata);
#if MULTIAPP
	if (!(pa->a_flags & AF_ISDESK))			/* only for apps */
	  pcurr = scan_memsz(pcurr, &pa->a_memreq);
#endif
	return(pcurr);
}

	VOID
app_tran(bi_num)
	WORD		bi_num;
{
	LONG		lpbi;
	BITBLK		lb;

	rsrc_gaddr(R_BITBLK, bi_num, &lpbi);

	LBCOPY(ADDR(&lb), lpbi, sizeof(BITBLK));
	gsx_trans(lb.bi_pdata, lb.bi_wb, lb.bi_pdata, lb.bi_wb, lb.bi_hl);
}


	WORD
app_getfh(openit, pname, attr)
	WORD		openit;
	BYTE		*pname;
	WORD		attr;
{
	WORD		handle, tmpdrv;
	LONG		lp;

	handle = 0;
	strcpy(pname, &G.g_srcpth[0]);
	lp = ADDR(&G.g_srcpth[0]);
	tmpdrv = dos_gdrv();
	if (tmpdrv != gl_stdrv)
	  dos_sdrv(gl_stdrv);
	if ( shel_find(lp) )
	{
	  if (openit)
	    handle = dos_open(lp, attr);
	  else
	    handle = dos_create(lp, attr);
	  if ( DOS_ERR )
	  {
	    handle = 0;
	  }
	}
	if (tmpdrv != gl_stdrv)
	  dos_sdrv(tmpdrv);
	return(handle);
}

/************************************************************************/
/* a p p _ r d i c o n							*/
/************************************************************************/
	WORD
app_rdicon()
{
	LONG		temp, stmp, dtmp;
	WORD		handle, length, i, iwb, ih;
	WORD		num_icons, num_masks, last_icon, num_wds, 
			num_bytes, msk_bytes, tmp;
#if ALCYON
	WORD		stlength, ret;
	BYTE		*fixup, *poffset, **namelist;
#endif
#if I8086
	WORD		fixup, poffset;
#endif
						/* open the file	*/
	handle = app_getfh(TRUE, 
		ini_str( (gl_height <= 300) ? STGEMLOI : STGEMHIC), 2);
	if (!handle)
	  return(FALSE);
						/* how much to read?	*/
	length = NUM_IBLKS * sizeof(ICONBLK);
#if ALCYON
	ret = dos_read(handle, 2, ADDR(&stlength));
#endif
	dos_read(handle, sizeof(BYTE *), ADDR(&gl_pstart));
	dos_read(handle, sizeof(BYTE *), ADDR(&poffset));
	poffset += (2 * sizeof(BYTE *)) + length;
	gl_pstart -= poffset;
						/* read it		*/
	dos_read(handle, length, ADDR(&G.g_idlist[0]) );
	movs(length, &G.g_idlist[0], &G.g_iblist[0]);
						/* find no. of icons	*/
						/*   actually used	*/
	num_icons = last_icon = 0;
	while ( (last_icon < NUM_IBLKS) &&
		(G.g_idlist[last_icon].ib_pmask != -1L) )
	{
	  tmp = max( LLOWD(G.g_idlist[last_icon].ib_pmask),
		     LLOWD(G.g_idlist[last_icon].ib_pdata) );
	  num_icons = max(num_icons, tmp);
	  last_icon++;
	}
	num_icons++;
						/* how many words of 	*/
						/*   data to read?	*/
						/* assume all icons are	*/
						/*   same w,h as first	*/
	num_wds = (G.g_idlist[0].ib_wicon * G.g_idlist[0].ib_hicon) / 16;
	num_bytes = num_wds * 2;

						/* allocate some memory	*/
						/*   in bytes     	*/
				/* gl_pstart = size of icon bit blocks	*/
				/*   and strings on Lattice C		*/
				/* NUM_NAMICS is for string pointers	*/
				/* stlength is strings on ALCYON C	*/
#if ALCYON
	length = gl_pstart + ((NUM_NAMICS + 1) * 4) + stlength;
#else
	length = gl_pstart + (NUM_NAMICS * 2);
#endif
	G.a_datastart = dos_alloc( LW(length) );
						/* read it		*/
	dos_read(handle, length, G.a_datastart);
	dos_close(handle);
						/* fix up str ptrs	*/
	gl_numics = 0;
#if ALCYON
	namelist = (BYTE *) G.a_datastart + gl_pstart;
	for (i = 0; i < NUM_NAMICS; i++)
	{
	  fixup = *namelist - poffset;
	  *namelist++ = fixup;
#endif
#if I8086
	for (i = 0; i < NUM_NAMICS; i++)
	{
	  fixup = LWGET(G.a_datastart + gl_pstart + (i*2)) - poffset;
	  LWSET((G.a_datastart + gl_pstart + (i*2)), fixup);
#endif
	  if ( LBGET(G.a_datastart + fixup) )
	    gl_numics++;
	}
						/* figure out which are	*/
						/*   mask & which data	*/
	for (i=0; i<last_icon; i++)
	{
	  G.g_ismask[ (WORD) G.g_idlist[i].ib_pmask] = TRUE;
	  G.g_ismask[ (WORD) G.g_idlist[i].ib_pdata] = FALSE;
	}
						/* fix up mask ptrs	*/
	num_masks = 0;
	for (i=0; i<num_icons; i++)
	{
	  if (G.g_ismask[i])
	  {
	    G.g_ismask[i] = num_masks;
	    num_masks++;
	  }
	  else
	    G.g_ismask[i] = -1;
	}
						/* allocate memory for	*/
						/*   transformed mask	*/
						/*   forms		*/
	msk_bytes = num_masks * num_bytes;
	G.a_buffstart = dos_alloc( LW(msk_bytes) );
						/* fix up icon pointers	*/
	for (i=0; i<last_icon; i++)
	{
						/* first the mask	*/
	  temp = ( G.g_ismask[ G.g_idlist[i].ib_pmask ] * ((LONG) num_bytes));
	  G.g_iblist[i].ib_pmask = G.a_buffstart + LW(temp);
	  temp = ( G.g_idlist[i].ib_pmask * ((LONG) num_bytes));
	  G.g_idlist[i].ib_pmask = G.a_datastart + LW(temp);
						/* now the data		*/
	  temp = ( G.g_idlist[i].ib_pdata * ((LONG) num_bytes));
	  G.g_iblist[i].ib_pdata = G.g_idlist[i].ib_pdata = 
		G.a_datastart + LW(temp);
						/* now the text ptrs	*/
	  G.g_idlist[i].ib_ytext = G.g_iblist[i].ib_ytext = 
			G.g_idlist[0].ib_hicon;
	  G.g_idlist[i].ib_wtext = G.g_iblist[i].ib_wtext = 12 * gl_wschar;
	  G.g_idlist[i].ib_htext = G.g_iblist[i].ib_htext = gl_hschar + 2;
	}
						/* transform forms	*/
	iwb = G.g_idlist[0].ib_wicon / 8;
	ih = G.g_idlist[0].ib_hicon;

	for (i=0; i<num_icons; i++)
	{
	  if (G.g_ismask[i] != -1)
	  {
						/* preserve standard	*/
						/*   form of masks	*/
	    stmp = G.a_datastart + (i * num_bytes);
	    dtmp = G.a_buffstart + (G.g_ismask[i] * num_bytes);
	    LWCOPY(dtmp, stmp, num_wds);
	  }
	  else
	  {
						/* transform over std.	*/
						/*   form of datas	*/
	    dtmp = G.a_datastart + (i * num_bytes);
	  }
	  gsx_trans(dtmp, iwb, dtmp, iwb, ih);
	}
	for (i=0; i<last_icon; i++)
	{
	  if ( i == IG_FOLDER )
	    G.g_iblist[i].ib_pmask = G.g_iblist[IG_TRASH].ib_pmask;
	  if ( ( i == IG_FLOPPY ) ||
	       ( i == IG_HARD ) )
	    G.g_iblist[i].ib_pmask = G.g_iblist[IG_TRASH].ib_pdata;
	  if ( (i >= IA_GENERIC) &&
	       (i < ID_GENERIC) )
	    G.g_iblist[i].ib_pmask = G.g_iblist[IA_GENERIC].ib_pdata;
	  if ( (i >= ID_GENERIC) &&
	       (i < (NUM_ANODES - 1)) )
	    G.g_iblist[i].ib_pmask = G.g_iblist[ID_GENERIC].ib_pdata;
	}
	app_tran(0);
	return(TRUE);
} /* app_rdicon */


/*
*	Initialize the application list by reading in the DESKTOP.INF
*	file, either from memory or from the disk if the shel_get
*	indicates no message is there.
*/
	WORD
app_start()
{
	WORD		i, x, y, w, h;
	ANODE		*pa;
	WSAVE		*pws;
	BYTE		*pcurr, *ptmp, prevdisk;
        WORD		fh, envr, xcnt, ycnt, xcent, wincnt;
#if MULTIAPP
	WORD		numaccs;
	BYTE		*savbuff;
	
	numaccs = 0;
#endif		
						/* remember start drive	*/
	gl_stdrv = dos_gdrv();

	G.g_pbuff = &gl_buffer[0];
	
	for(i=NUM_ANODES - 2; i >= 0; i--)
	  G.g_alist[i].a_next = &G.g_alist[i + 1];
	G.g_ahead = (ANODE *) NULL;
	G.g_aavail = &G.g_alist[0];
	G.g_alist[NUM_ANODES - 1].a_next = (ANODE *) NULL;

	shel_get(ADDR(&gl_afile[0]), SIZE_AFILE);
	if (gl_afile[0] != '#')
	{
						/* invalid signature	*/
						/*   so read from disk	*/
	  fh = app_getfh(TRUE, ini_str(STGEMAPP), 0x0);
	  if (!fh)
	    return(FALSE);
	  G.g_afsize = dos_read(fh, SIZE_AFILE, ADDR(&gl_afile[0]));
	  dos_close(fh);
	  gl_afile[G.g_afsize] = NULL;
	}
	

	wincnt = 0;
	pcurr = &gl_afile[0];
	prevdisk = ' ';
	while (*pcurr)
	{
	  if (*pcurr != '#')
	    pcurr++;
	  else
	  {
	    pcurr++;
	    switch(*pcurr)
	    {
	      case 'M':				/* Media (Hard/Floppy)	*/
	      case 'G':				/* GEM Application	*/
	      case 'F':				/* File	(DOS w/o parms)	*/
	      case 'f':				/*   use full memory	*/
	      case 'P':				/* Parm	(DOS w/ parms)	*/
	      case 'p':				/*   use full memory	*/
	      case 'D':				/* Directory		*/
			if ( *pcurr == 'M' )
			  prevdisk = 'M';
			else
			{
						/* rest of standards	*/
						/*   after last disk	*/
			  if (prevdisk == 'M') 
			  {
			    for (i = 0; i < 6; i++)
		            {
			      pa = app_alloc(TRUE);
	      		      app_parse(ini_str(ST1STD+i)+1, pa);
			    } /* for */
			  } /* if */
			  prevdisk = ' ';
			}
			pa = app_alloc(TRUE);
	      		pcurr = app_parse(pcurr, pa);
			break;
#if MULTIAPP			
	      case 'A':				/* Desk Accessory	*/
		        pcurr++;
			pcurr = scan_2(pcurr, &(gl_caccs[numaccs].acc_swap));
			savbuff = G.g_pbuff;
			G.g_pbuff = &(gl_caccs[numaccs].acc_name[0]);
			pcurr = scan_str(pcurr, &ptmp);
			G.g_pbuff = savbuff;
			numaccs++;
			break;
#endif
	      case 'W':				/* Window		*/
			pcurr++;
			if ( wincnt < NUM_WNODES )
			{
			  pws = &G.g_cnxsave.win_save[wincnt];
			  pcurr = scan_2(pcurr, &pws->hsl_save);
			  pcurr = scan_2(pcurr, &pws->vsl_save);
/* BugFix	*/
			  pcurr = scan_2(pcurr, &x);
			  pcurr = scan_2(pcurr, &y);
			  pcurr = scan_2(pcurr, &w);
			  pcurr = scan_2(pcurr, &h);
/* */
			  pcurr = scan_2(pcurr, &pws->obid_save);
			  ptmp = &pws->pth_save[0];
			  pcurr++;
			  while ( *pcurr != '@' )
			    *ptmp++ = *pcurr++;
			  *ptmp = NULL;
			  gl_savewin[wincnt].g_x = x * gl_wchar;
			  gl_savewin[wincnt].g_y = y * gl_hchar;
			  gl_savewin[wincnt].g_w = w * gl_wchar;
			  gl_savewin[wincnt++].g_h = h * gl_hchar;
			}
			break;
	      case 'E':
			pcurr++;
			pcurr = scan_2(pcurr, &envr);
			G.g_cnxsave.vitem_save = ( (envr & 0x80) != 0);
			G.g_cnxsave.sitem_save = ( (envr & 0x60) >> 5);
			G.g_cnxsave.cdele_save = ( (envr & 0x10) != 0);
			G.g_cnxsave.ccopy_save = ( (envr & 0x08) != 0);
			G.g_cnxsave.cdclk_save = envr & 0x07;
			pcurr = scan_2(pcurr, &envr);
			G.g_cnxsave.cdtfm_save = ( (envr & 0x04) == 0);
			G.g_cnxsave.ctmfm_save = ( (envr & 0x02) == 0);
			sound(FALSE, !(envr & 0x01), 0);
			break;
	    }
	  }
	}
	if (!app_rdicon())
	  return(FALSE);	
	G.g_wicon = (12 * gl_wschar) + (2 * G.g_idlist[0].ib_xtext);
	G.g_hicon = G.g_idlist[0].ib_hicon + gl_hschar + 2;

	G.g_icw = (gl_height <= 300) ? 0 : 8;
	G.g_icw += G.g_wicon;
	xcnt = (gl_width/G.g_icw);
	G.g_icw += (gl_width % G.g_icw) / xcnt;
	G.g_ich = G.g_hicon + MIN_HINT;
	ycnt = ((gl_height-gl_hbox) / G.g_ich);
	G.g_ich += ((gl_height-gl_hbox) % G.g_ich) / ycnt;

	xcent = (G.g_wicon - G.g_idlist[0].ib_wicon) / 2;
	G.g_nmicon = 9;
	G.g_xyicon[0] = xcent;  G.g_xyicon[1] = 0;
	G.g_xyicon[2]=xcent; G.g_xyicon[3]=G.g_hicon-gl_hschar-2;
	G.g_xyicon[4] = 0;  G.g_xyicon[5] = G.g_hicon-gl_hschar-2;
	G.g_xyicon[6] = 0;  G.g_xyicon[7] = G.g_hicon;
	G.g_xyicon[8] = G.g_wicon;  G.g_xyicon[9] = G.g_hicon;
	G.g_xyicon[10]=G.g_wicon; G.g_xyicon[11] = G.g_hicon-gl_hschar-2;
	G.g_xyicon[12]=G.g_wicon - xcent; G.g_xyicon[13]=G.g_hicon-gl_hschar-2;
	G.g_xyicon[14] = G.g_wicon - xcent;  G.g_xyicon[15] = 0;
	G.g_xyicon[16] = xcent;  G.g_xyicon[17] = 0;
	G.g_nmtext = 5;
	G.g_xytext[0] = 0;  		G.g_xytext[1] = 0;
	G.g_xytext[2] = gl_wchar * 12; 	G.g_xytext[3] = 0;
	G.g_xytext[4] = gl_wchar * 12;   G.g_xytext[5] = gl_hchar;
	G.g_xytext[6] = 0;  		G.g_xytext[7] = gl_hchar;
	G.g_xytext[8] = 0; 		G.g_xytext[9] = 0;
	return(TRUE);
}


/*
*	Reverse list when we write so that we can read it in naturally
*/
	VOID
app_revit()
{
	ANODE		*pa;
	ANODE		*pnxtpa;
						/* reverse list		*/
	pa = G.g_ahead;
	G.g_ahead = (ANODE *) NULL;
	while(pa)
	{
	  pnxtpa = pa->a_next;
	  pa->a_next = G.g_ahead;
	  G.g_ahead = pa;
	  pa = pnxtpa;
	}
}


/*
*	Save the current state of all the icons to a file called 
*	DESKTOP.INF
*/
	WORD
app_save(todisk)
	WORD		todisk;
{
	WORD		i, fh, ret, envr;
	BYTE		*pcurr, *ptmp;
	ANODE		*pa;
	WSAVE		*pws;

	bfill(SIZE_AFILE, 0, &gl_afile[0]);
	pcurr = &gl_afile[0];
						/* save evironment	*/
	*pcurr++ = '#';
	*pcurr++ = 'E';
	envr = 0x0;
	envr |= (G.g_cnxsave.vitem_save) ? 0x80 : 0x00;
	envr |= ((G.g_cnxsave.sitem_save) << 5) & 0x60;
	envr |= (G.g_cnxsave.cdele_save) ? 0x10 : 0x00;
	envr |= (G.g_cnxsave.ccopy_save) ? 0x08 : 0x00;
	envr |= G.g_cnxsave.cdclk_save;
	pcurr = save_2(pcurr, envr);

	envr  = (G.g_cnxsave.cdtfm_save) ? 0x00 : 0x04;
	envr |= (G.g_cnxsave.ctmfm_save) ? 0x00 : 0x02;
	envr |= sound(FALSE, 0xFFFF, 0)  ? 0x00 : 0x01;
	pcurr = save_2(pcurr, envr );

	*pcurr++ = 0x0d;
	*pcurr++ = 0x0a;
						/* save windows		*/
	for(i=0; i<NUM_WNODES; i++)
	{
	  *pcurr++ = '#';
	  *pcurr++ = 'W';
	  pws = &G.g_cnxsave.win_save[i];
	  pcurr = save_2(pcurr, pws->hsl_save);
	  pcurr = save_2(pcurr, pws->vsl_save);
	  pcurr = save_2(pcurr, pws->x_save / gl_wchar);
	  pcurr = save_2(pcurr, pws->y_save / gl_hchar);
	  pcurr = save_2(pcurr, pws->w_save / gl_wchar);
	  pcurr = save_2(pcurr, pws->h_save / gl_hchar);
	  pcurr = save_2(pcurr, pws->obid_save);
	  ptmp = &pws->pth_save[0];
	  *pcurr++ = ' ';
	  if (*ptmp != '@')
	  {
	    while (*ptmp)
	      *pcurr++ = *ptmp++;
	  }
	  *pcurr++ = '@';
	  *pcurr++ = 0x0d;
	  *pcurr++ = 0x0a;
	}		
#if MULTIAPP
	for (i=0; i<3; i++)
	  if (gl_caccs[i].acc_name[0])
	  {
	    *pcurr++ = '#';
	    *pcurr++ = 'A';
	    pcurr = save_2(pcurr, gl_caccs[i].acc_swap);
	    *pcurr++ = ' ';
	    pcurr = save_str(pcurr, &(gl_caccs[i].acc_name[0]));
	    pcurr--;
	    *pcurr++ = 0x0d;
	    *pcurr++ = 0x0a;
	  }
#endif	
						/* reverse ANODE list	*/
	app_revit();
						/* save ANODE list	*/
	for(pa=G.g_ahead; pa; pa=pa->a_next)
	{
	  *pcurr++ = '#';
	  switch(pa->a_type)
	  {
	    case AT_ISDISK:
		*pcurr++ = 'M';
		break;
	    case AT_ISFILE:
		if ( (pa->a_flags & AF_ISCRYS) &&
		     (pa->a_flags & AF_ISGRAF) )
		  *pcurr++ = 'G';
		else
		{  
		  *pcurr = (pa->a_flags & AF_ISPARM) ? 'P' : 'F';
		  if (pa->a_flags & AF_ISFMEM)
		    *pcurr += 'a' - 'A';
		  pcurr++;
		}
		break;
	    case AT_ISFOLD:
		*pcurr++ = 'D';
		break;
	  }
	  if (pa->a_flags & AF_ISDESK)
	  {
	    pcurr = save_2(pcurr, pa->a_xspot / G.g_icw);
	    pcurr = save_2(pcurr, (pa->a_yspot - G.g_ydesk) / G.g_ich);
	  }
	  pcurr = save_2(pcurr, pa->a_aicon);
	  pcurr = save_2(pcurr, pa->a_dicon);
	  *pcurr++ = ' ';
	  if (pa->a_flags & AF_ISDESK)
	  {
	    *pcurr++ = (pa->a_letter == NULL) ? ' ' : pa->a_letter;
	    *pcurr++ = ' ';
	  }
	  pcurr = save_str(pcurr, pa->a_pappl);
	  pcurr = save_str(pcurr, pa->a_pdata);
	  pcurr--;
#if MULTIAPP
	  if (!(pa->a_flags & AF_ISDESK))	/* only for apps	*/
	  {
	    pcurr++;				/* leave blank		*/
	    pcurr = save_memsz(pcurr, pa->a_memreq);
	  }
#endif
	  *pcurr++ = 0x0d;
	  *pcurr++ = 0x0a;
						/* skip standards	*/
	  if ( (pa->a_type == AT_ISDISK) && 
	       (pa->a_next->a_type != AT_ISDISK) )
	  {
	    for(i=0; i<6; i++)
	      pa = pa->a_next;
	  }
	}
	*pcurr++ = 0x1a;
	*pcurr++ = 0x0;
						/* reverse list back	*/
	app_revit();
						/* calculate size 	*/
	G.g_afsize = pcurr - &gl_afile[0];
						/* save in memory	*/
	shel_put(ADDR(&gl_afile[0]), G.g_afsize);
						/* save to disk		*/
	if (todisk)
	{
	  G.g_afsize--;
	  fh = 0;
	  while (!fh)
	  {
	    fh = app_getfh(FALSE, ini_str(STGEMAPP), 0x0);
	    if (!fh)
	    {
	      ret = fun_alert(1, STNOINF, NULLPTR);
	      if (ret == 2)
	        return(FALSE);
	    }
	  }
	  G.g_afsize = dos_write(fh, G.g_afsize, ADDR(&gl_afile[0]));
	  dos_close(fh);
	}
}


/*
*	Build the desktop list of objects based on this current 
*	application list.
*/

	BYTE
app_blddesk()
{
	WORD		obid;
	UWORD		bvdisk, bvhard, bvect;
	ANODE		*pa;
	OBJECT		*pob;
	ICONBLK		*pic;
#if ALCYON
	LONG		*ptr;
#endif						/* free all this windows*/
						/*   kids and set size	*/
	obj_wfree(DROOT, 0, 0, gl_width, gl_height);
#if ALCYON
	ptr = &global[3];
	G.g_screen[DROOT].ob_spec = LLGET(ptr);
#else
	G.g_screen[DROOT].ob_spec = LW(global[3]) + HW(global[4]);
#endif
	bvdisk = bvhard = 0x0;

	for(pa = G.g_ahead; pa; pa = pa->a_next)
	{
	  if (pa->a_flags & AF_ISDESK)
	  {
	    obid = obj_ialloc(DROOT, pa->a_xspot, pa->a_yspot,
					G.g_wicon, G.g_hicon);
	    if (!obid)
	    {
	    /* error case, no more obs */
	    }
						/* set up disk vector	*/
	    if (pa->a_type == AT_ISDISK)
	    {
	      bvect = ((UWORD) 0x8000) >> ((UWORD) (pa->a_letter - 'A'));
	      bvdisk |= bvect;
	      if (pa->a_aicon == IG_HARD)
		bvhard |= bvect;
	    }
						/* remember it		*/
	    pa->a_obid = obid;
						/* build object		*/
	    pob = &G.g_screen[obid];
	    pob->ob_state = NORMAL;
	    pob->ob_flags = NONE;
	    pob->ob_type = G_ICON;
	    G.g_index[obid] = pa->a_aicon;
	    pob->ob_spec = ADDR( pic = &gl_icons[obid] );
	    movs(sizeof(ICONBLK), &G.g_iblist[pa->a_aicon], pic);
	    pic->ib_xicon = ((G.g_wicon - pic->ib_wicon) / 2);
	    pic->ib_ptext = ADDR(pa->a_pappl);
	    pic->ib_char |= (0x00ff & pa->a_letter);
	  } /* if */
	} /* for */
	appl_bvset(bvdisk, bvhard);
	return( get_defdrv(bvdisk, bvhard) );
} /* app_blddesk */


/*
*	Find the ANODE that is appropriate for this object.
*/
	ANODE
*app_afind(isdesk, atype, obid, pname, pisapp)
	WORD		isdesk;
	WORD		atype;
	WORD		obid;
	BYTE		*pname;
	WORD		*pisapp;
{
	ANODE		*pa;

	for(pa = G.g_ahead; pa; pa = pa->a_next)
	{
	  if (isdesk)
	  {
	    if (pa->a_obid == obid)
	      return(pa);
	  }
	  else
	  {
	    if ( (pa->a_type == atype) && !(pa->a_flags & AF_ISDESK) )
	    {
	      if ( wildcmp(pa->a_pdata, pname) )
	      {
		*pisapp = FALSE;
		return(pa);
	      }
	      if ( wildcmp(pa->a_pappl, pname) )
	      {
		*pisapp = TRUE;
		return(pa);
	      } /* if */
	    } /* if */
	  } /* else */
	} /* for */
	return(0);
} /* app_afind */

/************************************************************************/
/* g e t _ d e f d r v							*/
/************************************************************************/
	BYTE
get_defdrv(dr_exist, dr_hard)
	UWORD		dr_exist, dr_hard;
{
/* this routine returns the drive letter of the lowest drive: lowest	*/
/* lettered hard disk if possible, otherwise lowest lettered floppy	*/
/* (which is usually A)							*/
/* in dr_exist, MSbit = A						*/
	UWORD		mask, hd_disk;
	WORD		ii;
	BYTE		drvletr;

	mask = 0x8000;
	hd_disk = dr_exist & dr_hard;
	if (hd_disk)
	{		/* there's a hard disk out there somewhere	*/
	  for (ii = 0; ii <= 15; ii++)
	  {
	    if (mask & hd_disk)
	    {
	      drvletr = ii + 'A';
	      break;
	    } /* if */
	    mask >>= 1;
	  } /* for */
	} /* if hd_disk */
	else
	  drvletr = 'A';			/* assume A is always	*/
	  					/* lowest floppy	*/
	return(drvletr);
} /* get_defdrv */
