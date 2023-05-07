/*	DESKSUPP.C	05/04/84 - 06/20/85	Lee Lorenzen		*/
/*	for 3.0		3/12/86	 - 10/3/86	MDF			*/

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
#include <dos.h>
#include <desktop.h>
#include <infodef.h>
#include <gembind.h>
#include <deskapp.h>
#include <deskfpd.h>
#include <deskwin.h>
#include <deskbind.h>
						/* in DESKFPD.C		*/
EXTERN PNODE	*pn_open();
EXTERN WORD	pn_close();
EXTERN WORD	pn_active();
						/* in DESKWIN.C		*/
EXTERN WNODE	*win_alloc();
EXTERN WNODE	*win_find();
EXTERN WNODE	*win_ith();
EXTERN WORD	win_free();
EXTERN VOID	win_bldview();
						/* in DESKACT.C		*/
EXTERN WORD	act_chg();
EXTERN WORD	act_allchg();
						/* in DESKINF.C		*/
EXTERN WORD	inf_file();
						/* in DESKTOP.C		*/
EXTERN ANODE	*i_find();

EXTERN BYTE	*ini_str();

						/* External Routines	*/
EXTERN WORD	wind_get();
EXTERN WORD	graf_mouse();
EXTERN WORD	rc_intersect();
EXTERN WORD	objc_draw();
EXTERN WORD	max();
EXTERN WORD	wind_open();
EXTERN WORD	wind_close();
EXTERN WORD	rc_equal();
EXTERN WORD	wind_set();
EXTERN VOID	win_sname();
EXTERN VOID	win_top();
EXTERN VOID	fun_msg();
EXTERN VOID	pro_chdir();
EXTERN WORD	opn_appl();
EXTERN WORD	fun_alert();
EXTERN WORD	wildcmp();
EXTERN WORD	pro_cmd();
EXTERN WORD	pro_run();
EXTERN WORD	rsrc_gaddr();
EXTERN WORD	form_alert();
EXTERN VOID	fpd_parse();
EXTERN VOID	fun_mkdir();
EXTERN VOID	fun_rebld();
EXTERN VOID	inf_show();
EXTERN VOID	inf_folder();
EXTERN VOID	inf_disk();
EXTERN WORD	shel_find();
EXTERN VOID	takedos();
EXTERN VOID	takekey();
EXTERN VOID	takevid();
EXTERN VOID	givevid();
EXTERN VOID	givekey();
EXTERN VOID	givedos();
EXTERN WORD	strlen();

EXTERN WORD	DOS_ERR;
EXTERN WORD	DOS_AX;
EXTERN WORD	gl_hbox;
EXTERN WORD	gl_whsiztop;
EXTERN GRECT	gl_rfull;
EXTERN GRECT	gl_normwin;

#if MULTIAPP
EXTERN WORD	pr_kbytes;
EXTERN LONG	pr_ssize;
EXTERN LONG	pr_topdsk;
EXTERN LONG	pr_topmem;
EXTERN WORD	gl_keepac;

#else
EXTERN LONG	dos_alloc();
EXTERN LONG	dos_avail();
EXTERN WORD	dos_free();
#endif

EXTERN GLOBES	G;

/*
*	Clear out the selections for this particular window
*/
	VOID
desk_clear(wh)
	WORD		wh;
{
	WNODE		*pw;
	GRECT		c;

						/* get current size	*/
	wind_get(wh, WF_WXYWH, &c.g_x, &c.g_y, &c.g_w, &c.g_h);
						/* find its tree of 	*/
						/*   items		*/
	pw = win_find(wh);
	if (pw)
	{
						/* clear all selections	*/
	  act_allchg(wh, G.a_screen, pw->w_root, 0, &gl_rfull, &c,
		 SELECTED, FALSE, TRUE);
	}
}

/*
*	Verify window display by building a new view.
*/
	VOID
desk_verify(wh, changed)
	WORD		wh;
	WORD		changed;
{
	WNODE		*pw;
	WORD		xc, yc, wc, hc;

						/* get current size	*/
	pw = win_find(wh);
	if (pw)
	{
	  if (changed)
	  {
	    wind_get(wh, WF_WXYWH, &xc, &yc, &wc, &hc);
	    win_bldview(pw, xc, yc, wc, hc);
	  }
	  G.g_croot = pw->w_root;
	}

	G.g_cwin = wh;
	G.g_wlastsel = wh;
}


	WORD
do_wredraw(w_handle, xc, yc, wc, hc)
	WORD		w_handle;
	WORD		xc, yc, wc, hc;
{
	GRECT		clip_r, t;
	WNODE		*pw;
	LONG		tree;
	WORD		root;

	clip_r.g_x = xc;
	clip_r.g_y = yc;
	clip_r.g_w = wc;
	clip_r.g_h = hc;
	if (w_handle != 0)
	{
	  pw = win_find(w_handle);
	  tree = G.a_screen;
	  root = pw->w_root;
	}
	else
	  return( TRUE );

	graf_mouse(M_OFF, 0x0L);

	wind_get(w_handle, WF_FIRSTXYWH, &t.g_x, &t.g_y, &t.g_w, &t.g_h);
	while ( t.g_w && t.g_h )
	{
	  if ( rc_intersect(&clip_r, &t) )
	    objc_draw(tree, root, MAX_DEPTH, t.g_x, t.g_y, t.g_w, t.g_h);
	  wind_get(w_handle, WF_NEXTXYWH, &t.g_x, &t.g_y, &t.g_w, &t.g_h);
	}
	graf_mouse(M_ON, 0x0L);
}


/*
*	Picks ob_x, ob_y, ob_width, ob_height fields out of object list.
*/

	VOID
get_xywh(olist, obj, px, py, pw, ph)
	OBJECT		olist[];
	WORD		obj;
	WORD		*px, *py, *pw, *ph;
{
	*px = olist[obj].ob_x;
	*py = olist[obj].ob_y;
	*pw = olist[obj].ob_width;
	*ph = olist[obj].ob_height;
}

/*
*	Picks ob_spec field out of object list.
*/

	LONG
get_spec(olist, obj)
	OBJECT		olist[];
	WORD		obj;
{
	return( olist[obj].ob_spec );
}

	VOID
do_xyfix(px, py)
	WORD		*px, *py;
{
	WORD		tx, ty, tw, th;

	wind_get(0, WF_WXYWH, &tx, &ty, &tw, &th);
	tx = *px;
	*px = (tx & 0x000f);
	if ( *px < 8 )
	  *px = tx & 0xfff0;
	else
	  *px = (tx & 0xfff0) + 16;
	*py = max(*py, ty);
}

	VOID
do_wopen(new_win, wh, curr, x, y, w, h)
	WORD		new_win;
	WORD		wh;
	WORD		curr;
	WORD		x, y, w, h;
{
	GRECT		c;

	do_xyfix(&x, &y);
	get_xywh(G.g_screen, G.g_croot, &c.g_x, &c.g_y, &c.g_w, &c.g_h);
	act_chg(G.g_cwin, G.a_screen, G.g_croot, curr, &c, SELECTED, 
			FALSE, TRUE, TRUE);
	if (new_win)
	  wind_open(wh, x, y, w, h);

	G.g_wlastsel = wh;
}

	VOID
do_wclose(old_win, wh)
	WORD		old_win;
	WORD		wh;
{
	  
	if (old_win)
	  wind_close(wh);
}

	VOID
do_wfull(wh)
	WORD		wh;
{
	WORD		tmp_wh, y;
	GRECT		curr, prev, full, temp;

	gl_whsiztop = NIL;
	wind_get(wh, WF_CXYWH, &curr.g_x, &curr.g_y, &curr.g_w, &curr.g_h);
	wind_get(wh, WF_PXYWH, &prev.g_x, &prev.g_y, &prev.g_w, &prev.g_h);
	wind_get(wh, WF_FXYWH, &full.g_x, &full.g_y, &full.g_w, &full.g_h);
			/* have to check for shrinking a window that	*/
			/* was full when Desktop was first started.	*/
	if ( (rc_equal(&curr, &prev)) && (curr.g_h > gl_normwin.g_h) )
	{				/* shrink full window		*/
		/* find the other window (assuming only 2 windows)	*/
	  if ( G.g_wlist[0].w_id == wh)
	    tmp_wh = G.g_wlist[1].w_id;
	  else
	    tmp_wh = G.g_wlist[0].w_id;
	    			/* decide which window we're shrinking	*/
	  wind_get(tmp_wh, WF_CXYWH, &temp.g_x, &temp.g_y,
	  	   &temp.g_w, &temp.g_h);
	  if (temp.g_y > gl_normwin.g_y)
	    y = gl_normwin.g_y;		/* shrinking upper window	*/
	  else				/* shrinking lower window	*/
	    y = gl_normwin.g_y + gl_normwin.g_h + (gl_hbox / 2);
 	  wind_set(wh, WF_CXYWH, gl_normwin.g_x, y,
	  	   gl_normwin.g_w, gl_normwin.g_h);
	} /* if */
					/* already full, so change	*/
					/* back to previous		*/
	else if ( rc_equal(&curr, &full) )
	  wind_set(wh, WF_CXYWH, prev.g_x, prev.g_y, prev.g_w, prev.g_h);
	  				/* make it full			*/
	else
	{
	  gl_whsiztop = wh;
	  wind_set(wh, WF_SIZTOP, full.g_x, full.g_y, full.g_w, full.g_h);
	}
} /* do_wfull */

/*
*	Open a directory, it may be the root or a subdirectory.
*/

	WORD
do_diropen(pw, new_win, curr_icon, drv, ppath, pname, pext, pt, redraw)
	WNODE		*pw;
	WORD		new_win;
	WORD		curr_icon;
	WORD		drv;
	BYTE		*ppath, *pname, *pext;
	GRECT		*pt;
	WORD		redraw;
{
	WORD		ret;
						/* convert to hourglass	*/
	graf_mouse(HGLASS, 0x0L);
						/* open a path node	*/
	pw->w_path = pn_open(drv, ppath, pname, pext, F_SUBDIR);
						/* activate path by 	*/
						/*   search and sort	*/
						/*   of directory	*/
	ret = pn_active(pw->w_path);
	if ( ret != E_NOFILES )
	{
						/* some error condition	*/
	}
						/* set new name and info*/
						/*   lines for window	*/
	win_sname(pw);
	wind_set(pw->w_id, WF_NAME, ADDR(&pw->w_name[0]), 0, 0);
/* BugFix	*/
/*	win_sinfo(pw);
	wind_set(pw->w_id, WF_INFO, ADDR(&pw->w_info[0]), 0, 0);
*/						/* do actual wind_open	*/
/* */
	do_wopen(new_win, pw->w_id, curr_icon, 
				pt->g_x, pt->g_y, pt->g_w, pt->g_h);
	if (new_win)
	  win_top(pw);
						/* verify contents of	*/
						/*   windows object list*/
						/*   by building view	*/
						/*   and make it curr.	*/
	desk_verify(pw->w_id, TRUE);
						/* make it redraw	*/
	if (redraw && ( !new_win ))
	  fun_msg(WM_REDRAW, pw->w_id, pt->g_x, pt->g_y, pt->g_w, pt->g_h);

	graf_mouse(ARROW, 0x0L);
	return(TRUE);
} /* do_diropen */

/*
*	Open an application
*/

	WORD
do_aopen(pa, isapp, curr, drv, ppath, pname)
	ANODE		*pa;
	WORD		isapp;
	WORD		curr;
	WORD		drv;
	BYTE		*ppath;
	BYTE		*pname;
{
	WORD		ret, done;
	WORD		isgraf, isover, isparm, uninstalled;
/*	WORD		xc, yc, wc, hc;
	WORD		ix, iy, iw, ih;
*/
	BYTE		*ptmp, *pcmd, *ptail;
	BYTE		name[13];

/*	get_xywh(G.g_screen, curr, &ix, &iy, &iw, &ih);
	get_xywh(G.g_screen, G.g_croot, &xc, &yc, &wc, &hc);
	ix += xc;
	iy += yc;
*/
	done = FALSE;
						/* set flags		*/
	isgraf = pa->a_flags & AF_ISGRAF;
#if MULTIAPP
	pr_kbytes = pa->a_memreq;		/* K bytes needed for app */
	isover = (pa->a_flags & AF_ISFMEM) ? 2 : -1;
	if ((isover == 2) && gl_keepac)		/* full-step ok?	*/
	{
	  rsrc_gaddr(R_STRING, STNOFSTP, &G.a_alert);
	  form_alert(1, G.a_alert);
	  return(FALSE);
	}
#else
	isover = (pa->a_flags & AF_ISFMEM) ? 2 : 1;
#endif
	isparm = pa->a_flags & AF_ISPARM;
	uninstalled = ( (*pa->a_pappl == '*') || 
			(*pa->a_pappl == '?') ||
			(*pa->a_pappl == NULL) );
						/* change current dir.	*/
						/*   to selected icon's	*/
	pro_chdir(drv, ppath);
						/* see if application	*/
						/*   was selected 	*/
						/*   directly or a 	*/
						/*   data file with an	*/
						/*   associated primary	*/
						/*   application	*/
	pcmd = ptail = NULLPTR;
	G.g_cmd[0] = G.g_tail[1] = NULL;
	ret = TRUE;

	if ( (!uninstalled) && (!isapp) )
	{
						/* an installed	docum.	*/
	  pcmd = pa->a_pappl;
	  ptail = pname;
	}
	else
	{
	  if ( isapp )
	  {
						/* DOS-based app. has	*/
						/*   been selected	*/
	    if (isparm)
	    {
	      pcmd = &G.g_cmd[0];
	      ptail = &G.g_tail[1];
	      ret = opn_appl(pname, "\0", pcmd, ptail);
	    }
	    else
	      pcmd = pname;
	  } /* if isapp */
	  else
	  {
						/* DOS-based document 	*/
						/*   has been selected	*/
	    fun_alert(1, STNOAPPL, NULLPTR);
	    ret = FALSE;
	  } /* else */
	} /* else */
						/* see if it is a 	*/
						/*   batch file		*/
	if ( wildcmp( ini_str(STGEMBAT), pcmd) )
	{
						/* if is app. then copy	*/
						/*   typed in parameter	*/
						/*   tail, else it was	*/
						/*   a document installed*/
						/*   to run a batch file*/
	  strcpy( (isapp) ? &G.g_tail[1] : ptail, &G.g_1text[0]);
	  ptmp = &name[0];
	  pname = pcmd;
	  while ( *pname != '.' )
	    *ptmp++ = *pname++;
	  *ptmp = NULL;
	  ret = pro_cmd(&name[0], &G.g_1text[0], TRUE);
	  pcmd = &G.g_cmd[0];
	  ptail = &G.g_tail[1];
	} /* if */
						/* user wants to run	*/
						/*   another application*/
	if (ret)
	{
	  if ( (pcmd != &G.g_cmd[0]) &&
	       (pcmd != NULLPTR) )
	    strcpy(pcmd, &G.g_cmd[0]);
	  if ( (ptail != &G.g_tail[1])  &&
	       (ptail != NULLPTR) )
	    strcpy(ptail, &G.g_tail[1]);
	  done = pro_run(isgraf, isover, G.g_cwin, curr);
	} /* if ret */
	return(done);
} /* do_aopen */


/*
*	Open a disk
*/

	WORD
do_dopen(curr)
	WORD		curr;
{
	WORD		drv;
	WNODE		*pw;
	ICONBLK		*pib;

	pib = (ICONBLK *) get_spec(G.g_screen, curr);
	pw = win_alloc();
	if (pw)
	{
	  drv = (0x00ff & pib->ib_char);
	  pro_chdir(drv, "");
	  if (!DOS_ERR)
 	    do_diropen(pw, TRUE, curr, drv, "", "*", "*", 
			&G.g_screen[pw->w_root].ob_x, TRUE);
	  else
	    win_free(pw);
	}
	else
	{
	  rsrc_gaddr(R_STRING, STNOWIND, &G.a_alert);
	  form_alert(1, G.a_alert);
	}
	return( FALSE );
}


/*
*	Open a folder
*/
	VOID
do_fopen(pw, curr, drv, ppath, pname, pext, chkall, redraw)
	WNODE		*pw;
	WORD		curr;
	WORD		drv;
	BYTE		*ppath, *pname, *pext;
	WORD		chkall;	/* TRUE if coming from do_chkall	*/
	WORD		redraw;
{
	GRECT		t;
	WORD		ok;
	BYTE		*pp, *pnew;

	
	ok = TRUE;
	pnew = ppath;
	wind_get(pw->w_id, WF_WXYWH, &t.g_x, &t.g_y, &t.g_w, &t.g_h);
	pro_chdir(drv, "");
	if (DOS_ERR)
	{
	  if ( DOS_AX == E_PATHNOTFND )
	  {
	    if (!chkall)
	      fun_alert(1, STDEEPPA, NULLPTR);
	    else
	    {
	      pro_chdir(drv, "");
	      pnew = "";
	    }
	  } /* if */
	  else
	    return;			/* error opening disk drive	*/
	} /* if DOS_ERR */
	else
	{
	  pro_chdir(drv, ppath);
	  if (DOS_ERR)
	  {
	    if ( DOS_AX == E_PATHNOTFND )
	    {				/* DOS path is too long?	*/
	      if (chkall)
	      {
	        pro_chdir(drv, "");
		pnew = "";
	      }
	      else
	      {
	        fun_alert(1, STDEEPPA, NULLPTR);
	    					/* back up one level	*/
		pp = ppath;
		while (*pp)
		  pp++;
		while(*pp != '\\')
		  pp--;
		*pp = NULL;
		pname = "*";
		pext  = "*";
		return;
	      } /* else */
	    } /* if DOS_AX */
	    else
	      return;			/* error opening disk drive	*/
	  } /* if DOS_ERR */
	} /* else */
	pn_close(pw->w_path);
	if (ok)
	  do_diropen(pw, FALSE, curr, drv, pnew, pname, pext, &t, redraw);
} /* do_fopen */


/*
*	Open an icon
*/

	WORD
do_open(curr)
	WORD		curr;
{
	WORD		done;
	ANODE		*pa;
	WNODE		*pw;
	FNODE		*pf;
	WORD		drv, isapp;
	BYTE		path[65], name[9], ext[4];

	done = FALSE;

	pa = i_find(G.g_cwin, curr, &pf, &isapp);
	pw = win_find(G.g_cwin);
	if ( pf )
	  fpd_parse(&pw->w_path->p_spec[0],&drv,&path[0],&name[0],&ext[0]);

	if ( pa )
	{	
	  switch( pa->a_type )
	  {
	    case AT_ISFILE:
#if MULTIAPP
					/* don't want to exit		*/
		do_aopen(pa, isapp, curr, drv, &path[0], &pf->f_name[0]);
#else
		done = do_aopen(pa, isapp, curr, drv, &path[0],
					&pf->f_name[0]);
#endif
		break;
	    case AT_ISFOLD:
		if ( (pf->f_attr & F_FAKE) && pw )
		  fun_mkdir(pw);
		else
		{
		  if (path[0] != NULL)
		    strcat("\\", &path[0]);
		  strcat(&pf->f_name[0], &path[0]);
		  do_fopen(pw, curr, drv, &path[0], &name[0], &ext[0],
					 FALSE, TRUE);
		}
		break;
	    case AT_ISDISK:
		drv = (0x00ff & pa->a_letter);
		path[0] = NULL;
		name[0] = ext[0] = '*';
		name[1] = ext[1] = NULL;
		do_fopen(pw, curr, drv, &path[0], &name[0], &ext[0],
					 FALSE, TRUE);
		break;
	  }
	}

	return(done);
}



/*
*	Get information on an icon.
*/

	WORD
do_info(curr)
	WORD		curr;
{
	WORD		ret, junk;
	ANODE		*pa;
	WNODE		*pw;
	FNODE		*pf;
	LONG		tree;

	pa = i_find(G.g_cwin, curr, &pf, &junk);
	pw = win_find(G.g_cwin);

	if ( pa )
	{	
	  switch( pa->a_type )
	  {
	    case AT_ISFILE:
		ret = inf_file(&pw->w_path->p_spec[0], pf);
		if (ret)
		  fun_rebld(pw);
		break;
	    case AT_ISFOLD:
		if (pf->f_attr & F_FAKE)
		{
		  tree = G.a_trees[ADNFINFO];
		  inf_show(tree, 0);
		  LWSET(OB_STATE(NFINOK), NORMAL);
		}
		else
		  inf_folder(&pw->w_path->p_spec[0], pf);
		break;
	    case AT_ISDISK:
		inf_disk( pf->f_junk );
		break;
	  }
	}
	return( FALSE );
}

#if MC68K

/* don't need this routine */

#else

/*
*	This routines purpose is to format a disk by execing a
*	FORMAT.COM above us in memory.  Unfortunately, the ROM BIOS
*	has a bug of using the contents of FORMAT's PSP while doing
*	a Disk Verify function using INT 13h.  This forces us to 
*	place the FORMAT we exec into a safe location in memory.
*	The safe location is an address with segment values between
*	x00x and xEDx. We fudge this on both side by 400 paragraphs.
*	Thanks alot, Bill and Phil.
*/
/*	The MULTIAPP version of this routine is closely tied to the	*/
/*	routine pro_chcalc() in DESKPRO.C.  The high and low memory	*/
/*	boundaries have to be jimmied to force the channel allocator	*/
/*	to put FORMAT in the right place.				*/

	VOID
romerr(curr)
	WORD		curr;
{

#if MULTIAPP

	LONG		savelo, savehi;


	savelo = pr_topdsk;
	savehi = pr_topmem;

	if ( (pr_topdsk & 0xff00) >= 0xe900)
	  pr_topdsk += 0x00001b00L;

	pr_kbytes = 32;
	pr_topmem = pr_topdsk + 0x8000l + pr_ssize;

	if (pr_topmem <= savehi)
	{
	  wind_set(curr, WF_TATTRB, WA_SUBWIN | WA_KEEPWIN, 0, 0, 0, 0);
	  pro_run(FALSE, 3, -1, curr);
	  wind_set(curr, WF_TATTRB, WA_SUBWIN, 0, 0, 0, 0);
	}
	pr_topdsk = savelo;
	pr_topmem = savehi;

#else
	UWORD		seg;
	LONG		testform, lavail;

	lavail = dos_avail();
	testform = dos_alloc( lavail );
	seg = testform >> 16;
	dos_free(testform);
	testform = 0x0L;
	if ( ((seg << 4) & 0xff00) >= 0xe900)
	  testform = dos_alloc( 0x00001b00L );

	pro_run(FALSE, 0, -1, curr);

	if (testform)
	  dos_free(testform);

#endif
} /* romerr */

#endif

/*
*	Format the currently selected disk.
*/
	VOID
do_format(curr)
	WORD		curr;
{
	WORD		junk, ret;
	WORD		*wtemp;
	BYTE		msg[6];
	ANODE		*pa;
	FNODE		*pf;

	pa = i_find(G.g_cwin, curr, &pf, &junk);

	if ( (pa) && (pa->a_type == AT_ISDISK) )
	{
	  msg[0] = pf->f_junk ;
	  msg[1] = NULL;
	  wtemp = &msg[0];
	  ret = fun_alert(2, STFORMAT, &wtemp);
	  strcpy(":", &msg[1]);
	  if (ret == 1)
	  {
#if MC68K
	    ret = pro_cmd( ini_str(STDKFORM), &msg[0], TRUE);
	    if (ret)
	      done = pro_run(FALSE, FALSE, G.g_cwin, curr);
#else
	    strcpy( ini_str(STDKFORM), &G.g_cmd[0]);
	    if ( shel_find(G.a_cmd) )
	    {
	      strcpy(&msg[0], &G.g_tail[1]);

	      takedos();
	      takekey();
	      takevid();

	      romerr(curr);
	      givevid();
	      givekey();
	      givedos();
	    } /* if */
#endif
	    graf_mouse(ARROW, 0x0L);	
	  } /* if ret */
	} /* if */
} /* do_format */

/*
*	Routine to check the all windows directory by doing a change
*	disk/directory to it and redrawing the window;
*/
	VOID
do_chkall(redraw)
	WORD		redraw;
{
	WORD		ii;
	WORD		drv;
	BYTE		path[65], name[9], ext[4];
	WNODE		*pw;
	for(ii = 0; ii < NUM_WNODES; ii++)
	{
	  pw = &G.g_wlist[ii];
	  if (pw->w_id)
	  {
	    fpd_parse(&pw->w_path->p_spec[0], &drv, &path[0],
	    	      &name[0], &ext[0]);
	    do_fopen(pw, 0, drv, &path[0], &name[0], &ext[0], TRUE, redraw);
	  }
	  else
{
	    desk_verify(0, TRUE);

}
	}
} /* do_chkall */


/*
*	Close the top folder window back into itself
*/
	VOID
do_fclose(pw, drv, ppath, pname, pext)
	WNODE		*pw;
	WORD		drv;
	BYTE		*ppath, *pname, *pext;
{
	do_wclose(FALSE, pw->w_id);
	do_fopen(pw, 0, drv, ppath, pname, pext, FALSE, TRUE);
}


/*
*	Close a window back into a folder or a disk.
*/

	VOID
do_close(pw, do_all)
	WNODE		*pw;
	WORD		do_all;
{
	WORD		drv;
	BYTE		path[65], name[9], ext[4];
	BYTE		*pbeg, *pend;

	graf_mouse(HGLASS, 0x0L);	

	fpd_parse(&pw->w_path->p_spec[0], &drv, &path[0], &name[0], &ext[0]);

	if (do_all)
	  path[0] = NULL;

	if (path[0] == NULL)
	{
						/* No more path so 	*/
						/*   close to show disk	*/
						/*   icons		*/
	  if (drv != '@')
	    do_fclose(pw, '@', &path[0], &name[0], &ext[0]);
	}
	else
	{
						/* Some path left so	*/
						/*   scan off last	*/
						/*   directory and open	*/
						/*   the parent		*/
	  pbeg = pend = &path[0];
	  pend += (strlen(&path[0]) - 1);
						/* Remove last name	*/
	  while ( (pend != pbeg) &&
		  (*pend != '\\') )
	    pend--;
						/* Scan off trailing	*/
						/*   slash		*/
	  *pend = NULL;
	  do_fclose(pw, drv, &path[0], &name[0], &ext[0]);
	}

	graf_mouse(ARROW, 0x0L);	
}


