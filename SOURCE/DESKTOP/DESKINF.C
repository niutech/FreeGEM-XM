/*	DESKINF.C	09/03/84 - 05/29/85	Gregg Morris		*/
/*	for 3.0	& 2.1	5/5/86			MDF			*/

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
#include <desktop.h>
#include <dos.h>
#include <deskapp.h>
#include <deskfpd.h>
#include <deskwin.h>
#include <infodef.h>
#include <gembind.h>
#include <deskbind.h>

EXTERN WORD	dos_rename();
EXTERN WORD	dos_chmod();
EXTERN VOID	merge_str();
EXTERN WORD	strlen();
EXTERN VOID	bb_fill();
EXTERN VOID	gsx_attr();
EXTERN VOID	gsx_tblt();
EXTERN VOID	gsx_gclip();
EXTERN VOID	gsx_sclip();
EXTERN WORD	form_center();
EXTERN WORD	form_dial();
EXTERN WORD	objc_draw();
EXTERN WORD	form_do();
EXTERN WORD	d_doop();
EXTERN VOID	inf_sset();
EXTERN VOID	fmt_str();
EXTERN VOID	inf_fldset();
EXTERN WORD	inf_what();
EXTERN WORD	graf_mouse();
EXTERN VOID	inf_sget();
EXTERN VOID	unfmt_str();
EXTERN WORD	strcmp();
EXTERN WORD	d_errmsg();
EXTERN WORD	dos_space();
EXTERN WORD	dos_label();
EXTERN WORD	evnt_dclick();
EXTERN WORD	sound();
EXTERN WORD	inf_gindex();

EXTERN LONG	ad_intin;

EXTERN GLOBES	G;

#define SFCB struct sfcb
SFCB
{
	BYTE		sfcb_junk;
	BYTE		sfcb_attr;
	WORD		sfcb_time;
	WORD		sfcb_date;
	LONG		sfcb_size;
	BYTE		sfcb_name[13];
};

/************************************************************************/
/* m y _ i t o a							*/
/************************************************************************/
	VOID
my_itoa(number, pnumstr)
	UWORD		number;
	BYTE		*pnumstr;
{
	WORD		ii;

	for (ii = 0; ii < 2; pnumstr[ii++] = '0');
	pnumstr[2] = NULL;
	if (number > 9)
	  merge_str(pnumstr, "%W", &number);
	else
	  merge_str(pnumstr+1, "%W", &number);
} /* my_itoa */

/*
*	Routine to format DOS style time.
*
*	15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
*	<     hh     > <    mm    > <   xx  >
*	hh = binary 0-23
*	mm = binary 0-59
*	xx = binary seconds \ 2 
*
*	put into this form 12:45 pm
*/
	VOID
fmt_time(time, ptime)
	UWORD		time;
	BYTE		*ptime;
{
	WORD		pm, val;

	val = ((time & 0xf800) >> 11) & 0x001f;
	if (G.g_ctimeform)
	{
	  if (val >= 12)
	  {
	    if (val > 12)
	      val -= 12;
	    pm = TRUE;
	  }
	  else
	  {
	    if (val == 0)
	      val = 12;
	    pm = FALSE;
	  }
	}
	my_itoa( val, &ptime[0]);
	my_itoa( ((time & 0x07e0) >> 5) & 0x003f, &ptime[2]);
	if (G.g_ctimeform)
	  strcpy((pm?"pm":"am"), &ptime[4]);
	else
	  strcpy("  ", &ptime[4]);
} /* fmt_time */

/*
*	Routine to format DOS style date.
*	
*	15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
*	<     yy          > < mm  > <  dd   >
*	yy = 0 - 119 (1980 - 2099)
*	mm = 1 - 12
*	dd = 1 - 31
*/
	VOID
fmt_date(date, pdate)
	UWORD		date;
	BYTE		*pdate;
{
	if (G.g_cdateform)
	{
	  my_itoa( (date & 0x01e0) >> 5, &pdate[0]);
	  my_itoa(date & 0x001f, &pdate[2]);
	}
	else
	{
	  my_itoa(date & 0x001f, &pdate[0]);
	  my_itoa( (date & 0x01e0) >> 5, &pdate[2]);
	}
	my_itoa(80 + (((date & 0xfe00) >> 9) & 0x007f), &pdate[4]);
} /* fmt_date */

	WORD
ob_sfcb(psfcb, pfmt)
	LONG		psfcb;
	BYTE		*pfmt;
{
	SFCB		sf;
	BYTE		*pdst, *psrc;
	BYTE		pdate_str[7], ptime_str[7], psize_str[9];
	WORD		cnt;

	LBCOPY(ADDR(&sf.sfcb_junk), psfcb, sizeof(SFCB));
	pdst = pfmt;
	psrc = &sf.sfcb_name[0];
	*pdst++ = ' ';
	*pdst++ = (sf.sfcb_attr & F_SUBDIR) ? 0x07 : ' ';
	*pdst++ = ' ';
	if (sf.sfcb_attr & F_DESKTOP)
	{
	  *pdst++ = sf.sfcb_junk;
	  *pdst++ = ':';
	  *pdst++ = ' ';
	}
	else
	{
	  while( (*psrc) &&
	         (*psrc != '.') )
	    *pdst++ = *psrc++;
	  while( (pdst - pfmt) < 12 )
	    *pdst++ = ' ';
	  if (*psrc)
	    psrc++;
	  while (*psrc)
	    *pdst++ = *psrc++;
	}
	while( (pdst - pfmt) < 16 )
	  *pdst++ = ' ';
	psrc = &psize_str[0];
	if (sf.sfcb_attr & F_SUBDIR)
	  *psrc = NULL;
	else
	  merge_str(&psize_str[0], "%L", &sf.sfcb_size);
	for(cnt = 8 - strlen(psrc); cnt--; *pdst++ = ' ');
	while (*psrc)
	  *pdst++ = *psrc++;
	*pdst++ = ' ';
	*pdst++ = ' ';
	fmt_date(sf.sfcb_date, &pdate_str[0]);
	psrc = &pdate_str[0];
	for(cnt = 3; cnt--; )
	{
	  *pdst++ = *psrc++;
	  *pdst++ = *psrc++;
	  if (cnt)
	    *pdst++ = '-';
	}
	*pdst++ = ' ';
	*pdst++ = ' ';
	fmt_time(sf.sfcb_time, &ptime_str[0]);
	psrc = &ptime_str[0];
	for(cnt = 2; cnt--; )
	{
	  *pdst++ = *psrc++;
	  *pdst++ = *psrc++;
	  if (cnt)
	    *pdst++ = ':';
	}
	*pdst++ = ' ';
	strcpy(&ptime_str[4], pdst);
	pdst += 3;
	return(pdst - pfmt);
}	

	WORD
dr_fnode(last_state, curr_state, x, y, w, h, psfcb)
	UWORD		last_state, curr_state;
	WORD		x, y, w, h;
	LONG		psfcb;
{
	WORD		len;

	if ((last_state ^ curr_state) & SELECTED)
	  bb_fill(MD_XOR, FIS_SOLID, IP_SOLID, x, y, w, h);
	else
	{
 	  len = ob_sfcb(psfcb, &G.g_tmppth[0]);
	  gsx_attr(TRUE, MD_REPLACE, BLACK);
	  LBWMOV(ad_intin, ADDR(&G.g_tmppth[0]));
	  gsx_tblt(IBM, x, y, len);
	  gsx_attr(FALSE, MD_XOR, BLACK);
	}
	return(curr_state);
}

	WORD
dr_code(pparms)
	LONG		pparms;
{
	PARMBLK		pb;
	GRECT		oc;
	WORD		state;

	LBCOPY(ADDR(&pb), pparms, sizeof(PARMBLK));
	gsx_gclip(&oc);
	gsx_sclip(&pb.pb_xc);
	state = dr_fnode(pb.pb_prevstate, pb.pb_currstate,
			 pb.pb_x, pb.pb_y, pb.pb_w, pb.pb_h, pb.pb_parm);
	gsx_sclip(&oc);
	return(state);
}

/*
*	Put up dialog box & call form_do.
*/
	WORD
inf_show(tree, start)
	LONG		tree;
	WORD		start;
{
	WORD		xd, yd, wd, hd;

	form_center(tree, &xd, &yd, &wd, &hd);
	form_dial(FMD_START, 0, 0, 0, 0, xd, yd, wd, hd);
	objc_draw(tree, ROOT, MAX_DEPTH, xd, yd, wd, hd);
	form_do(tree, start);
	form_dial(FMD_FINISH, 0, 0, 0, 0, xd, yd, wd, hd);
	return(TRUE);
}

/*
*	Routine for finishing off a simple ok-only dialog box
*/
	VOID
inf_finish(tree, dl_ok)
	LONG		tree;
	WORD		dl_ok;
{
	inf_show(tree, 0);
	LWSET(OB_STATE(dl_ok), NORMAL);
}

/*
*	Routine to get number of files and folders and stuff them in
*	a dialog box.
*/
	WORD
inf_fifo(tree, dl_fi, dl_fo, ppath)
	LONG		tree;
	WORD		dl_fi, dl_fo;
	BYTE		*ppath;
{
	WORD		junk, more;
	BYTE		nf_str[6], nd_str[6];

	G.g_nfiles = 0x0L;
	G.g_ndirs = 0x0L;
	G.g_size = 0x0L;
	more = d_doop(OP_COUNT, 0x0L, ppath, ppath, &junk, &junk);
	if (!more)
	  return(FALSE);
	G.g_ndirs--;

	merge_str(&nf_str[0], "%L", &G.g_nfiles);
	inf_sset(tree, dl_fi, &nf_str[0]);

	merge_str(&nd_str[0], "%L", &G.g_ndirs);
	inf_sset(tree, dl_fo, &nd_str[0]);
	return(TRUE);
}

	VOID
inf_dttmsz(tree, pf, dl_dt, dl_tm, dl_sz, psize)
	LONG		tree;
	FNODE		*pf;
	WORD		dl_dt, dl_tm, dl_sz;
	LONG		*psize;
{
	BYTE		psize_str[9], ptime_str[7], pdate_str[7];

	fmt_date(pf->f_date, &pdate_str[0]);
	inf_sset(tree, dl_dt, &pdate_str[0]);

	fmt_time(pf->f_time, &ptime_str[0]);
	inf_sset(tree, dl_tm, &ptime_str[0]);

	merge_str(&psize_str[0], "%L", psize);
	inf_sset(tree, dl_sz, &psize_str[0]);
}

/************************************************************************/
/* i n f _ f i l e							*/
/************************************************************************/
	WORD
inf_file(ppath, pfnode)
	BYTE		*ppath;
	FNODE		*pfnode;
{
	LONG		tree;
	WORD		attr, more, nmidx;
	BYTE		poname[13], pnname[13];

	tree = G.a_trees[ADFILEIN];

	strcpy(ppath, &G.g_srcpth[0]);
	strcpy(ppath, &G.g_dstpth[0]);
	nmidx = 0;
	while (G.g_srcpth[nmidx] != '*')
	  nmidx++;

	fmt_str(&pfnode->f_name[0], &poname[0]);

	inf_sset(tree, FINAME, &poname[0]);

	inf_dttmsz(tree, pfnode, FIDATE, FITIME, FISIZE, &pfnode->f_size);

	inf_fldset(tree, FIRONLY, pfnode->f_attr, F_RDONLY,SELECTED, NORMAL);
	inf_fldset(tree, FIRWRITE, pfnode->f_attr, F_RDONLY,NORMAL,SELECTED);

	inf_show(tree, 0);
					/* now find out what happened	*/
						/* was it OK or CANCEL?	*/
	if ( inf_what(tree, FIOK, FICNCL) )
	{
	  graf_mouse(HGLASS, 0x0L);

	  more = TRUE;
	  inf_sget(tree, FINAME, &pnname[0]);
	  				/* unformat the strings		*/
	  unfmt_str(&poname[0], &G.g_srcpth[nmidx]);
	  unfmt_str(&pnname[0], &G.g_dstpth[nmidx]);
						/* do the DOS rename	*/
	  if ( !strcmp(&G.g_srcpth[nmidx], &G.g_dstpth[nmidx]) )
	  {
	    dos_rename(ADDR(&G.g_srcpth[0]), ADDR(&G.g_dstpth[0]));
	    if ( (more = d_errmsg()) != 0 )
	      strcpy(&G.g_dstpth[nmidx], &pfnode->f_name[0]);
	  } /* if */
					/* update the attributes	*/
	  attr = pfnode->f_attr;
	  if (LWGET(OB_STATE(FIRONLY)) & SELECTED)
	    attr |= F_RDONLY;
	  else
	    attr &= ~F_RDONLY;
	  if ( (BYTE) attr != pfnode->f_attr )
	  {
	    dos_chmod(ADDR(&G.g_dstpth[0]), F_SETMOD, attr);
	    if ( (more = d_errmsg()) != 0 )
	      pfnode->f_attr = attr;
	  }
	  graf_mouse(ARROW, 0x0L);
	  return(more);
	}
	else
	  return(FALSE);
} /* inf_file */

/************************************************************************/
/* i n f _ f o l d							*/
/************************************************************************/
	WORD
inf_fold(ppath, pf)
	BYTE		*ppath;
	FNODE		*pf;
{
	LONG		tree;
	WORD		more;
	BYTE		*pname, fname[13];

	graf_mouse(HGLASS, 0x0L);	

	tree = G.a_trees[ADFOLDIN];

	strcpy(ppath, &G.g_srcpth[0]);
	pname = &G.g_srcpth[0];
	while (*pname != '*')
	  pname++;
	pname = strcpy(&pf->f_name[0], pname);
	strcpy("\\*.*", pname-1);

	more = inf_fifo(tree, FOLNFILE, FOLNFOLD, &G.g_srcpth[0]);

	graf_mouse(ARROW, 0x0L);
	if (more)
	{
	  fmt_str(&pf->f_name[0], &fname[0]);
	  inf_sset(tree, FOLNAME, &fname[0]);

	  inf_dttmsz(tree, pf, FOLDATE, FOLTIME, FOLSIZE, &G.g_size);
	  inf_finish(tree, FOLOK);
	}
	return(TRUE);
} /* inf_fold */

/************************************************************************/
/* i n f _ d i s k							*/
/************************************************************************/
	WORD
inf_disk(dr_id)
	BYTE		dr_id;
{
	LONG		tree;
	LONG		total, avail;
	WORD		more;
	BYTE		puse_str[9], pav_str[9], plab_str[12];
	BYTE		drive[2];
	
	graf_mouse(HGLASS, 0x0L);
	tree = G.a_trees[ADDISKIN];

	drive[0] = dr_id;
	drive[1] = NULL;
	G.g_srcpth[0] = drive[0];
	strcpy(":\\*.*", &G.g_srcpth[1]);

	more = inf_fifo(tree, DINFILES, DINFOLDS, &G.g_srcpth[0]);

	graf_mouse(ARROW, 0x0L);
	if (more)
	{
	  dos_space(dr_id - 'A' + 1, &total, &avail);
	  dos_label(dr_id - 'A' + 1, &plab_str[0]);

	  inf_sset(tree, DIDRIVE, &drive[0]);
	  inf_sset(tree, DIVOLUME, &plab_str[0]);

	  merge_str(&puse_str[0], "%L", &G.g_size);
	  inf_sset(tree, DIUSED, &puse_str[0]);
	  
	  merge_str(&pav_str[0], "%L", &avail);
	  inf_sset(tree, DIAVAIL, &pav_str[0]);

	  inf_finish(tree, DIOK);
	}
	return(TRUE);
} /* inf_disk */

/*
*	Set preferences dialog.
*/
	WORD
inf_pref()
{
	LONG		tree;
	WORD		cyes, cno, i;
	WORD		sndefpref;
	WORD		rbld;
	
	tree = G.a_trees[ADSETPRE];
	rbld = FALSE;

	cyes = (G.g_cdelepref) ? SELECTED : NORMAL;
	cno = (G.g_cdelepref) ? NORMAL : SELECTED;
	LWSET(OB_STATE(SPCDYES), cyes);
	LWSET(OB_STATE(SPCDNO), cno);

	cyes = (G.g_ccopypref) ? SELECTED : NORMAL;
	cno = (G.g_ccopypref) ? NORMAL : SELECTED;
	LWSET(OB_STATE(SPCCYES), cyes);
	LWSET(OB_STATE(SPCCNO), cno);

	cyes = (G.g_ctimeform) ? SELECTED : NORMAL;
	cno = (G.g_ctimeform) ? NORMAL : SELECTED;
	LWSET(OB_STATE(SPTF12HR), cyes);
	LWSET(OB_STATE(SPTF24HR), cno);

	cyes = (G.g_cdateform) ? SELECTED : NORMAL;
	cno = (G.g_cdateform) ? NORMAL : SELECTED;
	LWSET(OB_STATE(SPDFMMDD), cyes);
	LWSET(OB_STATE(SPDFDDMM), cno);

	for(i=0; i<5; i++)
	  LWSET(OB_STATE(SPDC1+i), NORMAL);

	G.g_cdclkpref = evnt_dclick(0, FALSE);
	LWSET(OB_STATE(SPDC1+G.g_cdclkpref), SELECTED);

	sndefpref = !sound(FALSE, 0xFFFF, 0);

	cyes = (sndefpref) ? SELECTED : NORMAL;
	cno = (sndefpref) ? NORMAL : SELECTED;
	LWSET(OB_STATE(SPSEYES), cyes);
	LWSET(OB_STATE(SPSENO), cno);

	inf_show(tree, 0);

	if ( inf_what(tree, SPOK, SPCNCL) )
	{
	  G.g_cdelepref = inf_what(tree, SPCDYES, SPCDNO);
	  G.g_ccopypref = inf_what(tree, SPCCYES, SPCCNO);
	  G.g_cdclkpref = inf_gindex(tree, SPDC1, 5);
	  G.g_cdclkpref = evnt_dclick(G.g_cdclkpref, TRUE);
	  sndefpref = inf_what(tree, SPSEYES, SPSENO);
					/* changes if file display? */
	  cyes = inf_what(tree, SPTF12HR, SPTF24HR);
	  if (G.g_ctimeform != cyes)
	  {
	    rbld = (G.g_iview == V_TEXT);
	    G.g_ctimeform = cyes;
	  }
	  cyes = inf_what(tree, SPDFMMDD, SPDFDDMM);
	  if (G.g_cdateform != cyes)
	  {
	    rbld |= (G.g_iview == V_TEXT);
	    G.g_cdateform = cyes;
	  }
	  sound(FALSE, !sndefpref, 0);
	}
	return(rbld);
} /* inf_pref */

/*
*	Open application icon
*/
	WORD
opn_appl(papname, papparms, pcmd, ptail)
	BYTE		*papname, *papparms;
	BYTE		*pcmd, *ptail;
{
	LONG		tree;
	BYTE		poname[13];

	tree = G.a_trees[ADOPENAP];

	fmt_str(papname, &poname[0]);

	inf_sset(tree, APPLNAME, &poname[0]);

	inf_sset(tree, APPLPARM, papparms);

	inf_show(tree, APPLPARM);
					/* now find out what happened	*/
	if ( inf_what(tree, APPLOK, APPLCNCL) )
	{
	  inf_sget(tree, APPLNAME, &poname[0]);
	  unfmt_str(&poname[0], pcmd);
	  inf_sget(tree, APPLPARM, ptail);
	  return(TRUE);
	}
	else
	  return(FALSE);
}
