/*	DESKIACC.C	3/24/86	- 9/10/86	MDF		*/

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*	-------------------------------------------------------------
*	GEM Desktop					  Version 3.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1986			Digital Research Inc.
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



EXTERN WORD	appl_find();
EXTERN VOID	dos_chdir();
EXTERN VOID	dos_sdrv();
EXTERN WORD	dos_sdta();
EXTERN WORD	dos_sfirst();
EXTERN WORD	dos_snext();
EXTERN WORD	form_alert();
EXTERN WORD	form_center();
EXTERN WORD	form_dial();
EXTERN WORD	form_do();
EXTERN WORD	graf_mkstate();
EXTERN WORD	graf_slidebox();
EXTERN UWORD	inside();
EXTERN WORD	max();
EXTERN WORD	min();
EXTERN WORD	mul_div();
EXTERN VOID 	ob_actxywh();
EXTERN VOID	ob_relxywh();
EXTERN WORD	objc_change();
EXTERN WORD	objc_draw();
EXTERN WORD	objc_offset();
EXTERN WORD	proc_create();
EXTERN WORD	proc_delete();
EXTERN WORD	proc_info();
EXTERN WORD	proc_run();
EXTERN WORD	rsrc_gaddr();
EXTERN VOID	show_hide();
EXTERN BYTE	*scasb();
EXTERN WORD	sound();
EXTERN BYTE	*strcpy();
EXTERN WORD	strchk();
EXTERN WORD	wind_update();


EXTERN GLOBES	G;

EXTERN ACCNODE gl_caccs[];

MLOCAL WORD	iac_chkd;

EXTERN LONG	pr_begacc;
EXTERN LONG	pr_begdsk;
EXTERN WORD	gl_hbox;
EXTERN BYTE	gl_bootdr;

#define NUM_AFILES 20
#define NUM_FSNAME  8
#define LEN_FSNAME 16

#define BEG_UPDATE 1
#define END_UPDATE 0

GLOBAL LONG	ad_tmp1;
GLOBAL BYTE	gl_tmp1[LEN_FSNAME];
GLOBAL LONG	ad_tmp2;
GLOBAL BYTE	gl_tmp2[LEN_FSNAME];
GLOBAL BYTE	*g_fslist[NUM_AFILES];
GLOBAL BYTE	g_fsnames[LEN_FSNAME * NUM_AFILES];
GLOBAL WORD	gl_fmemflg;
GLOBAL WORD	gl_keepac;


#if DEBUG
	VOID
printstr(lst)
	LONG		lst;
{
	BYTE		ch;
	WORD		i;
	
	i = 0;
	while ((ch = LBGET(lst + i++)) != 0)
	  dbg("%c", ch);
}
#endif

	WORD
iac_isnam(lst)
	LONG		lst;
{
	BYTE		ch;
	
	ch = LBGET(lst);
	return((ch>='A') && (ch<='Z'));
}

	VOID
iac_init()
{
	WORD		i, j;
	BYTE		*npt;
	BYTE		accstr[9];

	gl_keepac = FALSE;
	for (i=0; i<3; i++)
	{
	  npt = &gl_caccs[i].acc_name[0];
	  if (iac_isnam(ADDR(npt)))
	  {
	    strcpy("        ", &accstr[0]);	/* eight blanks		*/
	    for (j=0; (j<8) && npt[j] && (npt[j] != '.'); j++)
	      accstr[j] = npt[j];
	    if (appl_find(ADDR(&accstr[0])) < 0)
	      npt[0] = '\0';
	    else				/* check for no full step */
	      gl_keepac |= (gl_caccs[i].acc_swap == 'N');
	  }
	}
}


	VOID
iac_strcop(tree, obj, src)
	LONG		tree;
	WORD		obj;
	LONG		src;

{
	LONG		dst;

	switch (LWGET(OB_TYPE(obj)))
	{
	  case G_BUTTON:
	  case G_STRING:
	    dst = LLGET(OB_SPEC(obj));
	    break;
	  default:
	    dst = LLGET(LLGET(OB_SPEC(obj)));
	    break;
	}
	LSTCPY(dst, src);
}



	VOID
iac_schar(tree, obj, ch)
	LONG		tree;
	WORD		obj;
	BYTE		ch;
{
	LONG		longch;
	LONG		spec;

	spec = LLGET(OB_SPEC(obj));
 	longch = ch;
	longch = longch << 24;
	spec = (spec & 0xFFFFFFl) | longch;
	LLSET(OB_SPEC(obj), spec);
}
		    

	VOID
iac_redrw(tree, obj, state, depth)
	LONG		tree;
	WORD		obj, state;
	WORD		depth;
{
	WORD		x, y, w, h;

	objc_offset(tree, obj, &x, &y);
	w = LWGET(OB_WIDTH(obj));
	h = LWGET(OB_HEIGHT(obj));
	LWSET(OB_STATE(obj), state);
	objc_draw(tree, obj, depth, x, y, w, h);
}

	VOID
iac_elev(tree, currtop, count)
	LONG		tree;
	WORD		currtop;
	WORD		count;
{
	WORD		h,y,th;

	y = 0;
	th = h = LWGET(OB_HEIGHT(ACFSVSLI));  
	if ( count > NUM_FSNAME)
	{
	  h = mul_div(NUM_FSNAME, h, count);
	  h = max(gl_hbox/2, h);		/* min size elevator	*/
	  y = mul_div(currtop, th-h, count-NUM_FSNAME);
	}
	LWSET(OB_Y(ACFSELEV), y);
	LWSET(OB_HEIGHT(ACFSELEV), h);
}


	WORD
iac_comp()
{
	WORD		chk;

	if ( (gl_tmp1[0] == ' ') &&
	     (gl_tmp2[0] == ' ') )
	{
	  chk = strchk( scasb(&gl_tmp1[0], '.'), 
			scasb(&gl_tmp2[0], '.') );
	  if ( chk )
	    return( chk );
	}
	return ( strchk(&gl_tmp1[0], &gl_tmp2[0]) );
}

	VOID
iac_mvnames(tree, start, num)
	LONG		tree;
	WORD		start;
	WORD		num;
{
	WORD		i, j, k;
	WORD		len;

	for (i=0; i<num; i++)
	{
	  LSTCPY(ad_tmp1, ADDR(g_fslist[i+start]));
	  len = 0;
	  while (gl_tmp1[len] != '.')
	    len++;
	  if (len < 8)				/* blank pad in middle	*/
	  {
	    for (j=11, k=len+3; j > 7; j--, k--)
	      gl_tmp1[j] = gl_tmp1[k];
	    for (j=len; j < 8; j++)
	      gl_tmp1[j] = ' ';
	  }
	  iac_strcop(tree, ACA1NAME+i, ad_tmp1);
	}
}

	WORD
iac_names(tree)
	LONG		tree;
{
	WORD		ret;
	WORD		len;
	WORD		i, j, gap;
	WORD		thefile;
	BYTE		*ptr, *temp;

					/* find all .acc files in \GEMBOOT */
					/* stuff first 8 in object	*/
					/* adjust elevator size to number */
	thefile = 0;
	ptr = &g_fsnames[0];
	ad_tmp1 = ADDR(&gl_tmp1[0]);
	ad_tmp2 = ADDR(&gl_tmp2[0]);
	dos_sdta(G.a_wdta);
	strcpy(":\\GEMBOOT\\*.ACC", &G.g_cmd[1]);
	G.g_cmd[0] = gl_bootdr;
	ret = dos_sfirst(G.a_cmd, 0x16);
	while ( ret )
	{
	  len = LSTCPY(ADDR(g_fslist[thefile] = ptr), G.a_wdta+30);
	  ptr += len+1;

	  ret = dos_snext();

	  if (thefile++ >= NUM_AFILES)
	  {
	    ret = FALSE;
	    sound(TRUE, 660, 4);
	  }
	}

	for(gap = thefile/2; gap > 0; gap /= 2)
	{
	  for(i = gap; i < thefile; i++)
	  {
	    for (j = i-gap; j >= 0; j -= gap)
	    {
	      LSTCPY(ad_tmp1, ADDR(g_fslist[j]));
	      LSTCPY(ad_tmp2, ADDR(g_fslist[j+gap]));
	      if ( iac_comp() <= 0 )
		break;
	      temp = g_fslist[j];
	      g_fslist[j] = g_fslist[j+gap];
	      g_fslist[j+gap] = temp;
	    }
	  }
	}
	iac_mvnames(tree, 0, min(thefile, NUM_FSNAME));
	return(thefile);
}


#define ACCMIN	0x2000


	VOID
iac_save(tree)
	LONG		tree;
{
	WORD		i;
	WORD		chnum;
	WORD		isswap;
	LONG		lst;
	LONG		begaddr;
	LONG		csize;
	LONG		junk;
	BYTE		didalert;
	WORD 		didrun;

	wind_update(END_UPDATE);
	strcpy(":\\GEMBOOT", &G.g_cmd[1]);
	G.g_cmd[0] = gl_bootdr;			/* get correct drive	*/
	dos_sdrv(gl_bootdr - 'A');		/* set drive & directory */
	dos_chdir(G.a_cmd);

	begaddr = pr_begacc;
	didalert = FALSE;
	gl_keepac = FALSE;
	proc_delete(-1);		/* delete all accessories	*/
	for (i=0; i<3; i++)
	{
	  lst = LLGET(LLGET(OB_SPEC(ACC1NAME+i)));
	  if (iac_isnam(lst))
	  {
            LSTCPY(ADDR(&(gl_caccs[i].acc_name[0])), lst);

	    junk = LLGET(OB_SPEC(ACC1FMEM+i));
	    gl_caccs[i].acc_swap = LHIBT(LHIWD(junk));
	    isswap = (gl_caccs[i].acc_swap == 'N');
	    gl_keepac |= isswap;	/* if TRUE, no full step	*/
	    if (gl_keepac && gl_fmemflg)
	    {
	      if (!didalert)
	      {
		rsrc_gaddr(R_STRING, STACFMEM, &G.a_alert);
		form_alert(1, G.a_alert);
		didalert = TRUE;
	      }
	      gl_keepac = FALSE;	/* this guy won't run		*/
	      begaddr = pr_begdsk;	/* quit trying, but get names	*/
	    }
	    if  ((begaddr + ACCMIN <= pr_begdsk) &&
		(proc_create(LSEGOFF(begaddr), pr_begdsk-begaddr,
/*				 isswap, 1, &chnum)))			*/
/* ISSWAP = 0 FOR GDOS */	 0     , 1, &chnum)))
	    {
	      didrun = proc_run(chnum, 1, 3, lst, ADDR("\0"));
	      if (didrun)
	      { 
	        proc_info(chnum,&junk,&junk,&junk,&csize,&junk,&junk,&junk);
	        begaddr += csize;
	      }
	    }
	    else
	      didrun = FALSE;
	    if ((!didalert) && (!didrun))
	    {
	      rsrc_gaddr(R_STRING, STACCMEM, &G.a_alert);
	      form_alert(1, G.a_alert);
	      didalert = TRUE;
	    }
	  }
	  else
	    gl_caccs[i].acc_name[0] = '\0';
        }
	wind_update(BEG_UPDATE);
}


	WORD
iac_scroll(tree, currtop, count, move)
	LONG		tree;
	WORD		currtop;
	WORD		count;
	WORD		move;
{
	WORD		newtop;

	if (count <= NUM_FSNAME)
	  return(0);
	newtop = currtop + move;
	newtop = max(newtop, 0);
	newtop = min(newtop, count - NUM_FSNAME);
	if (newtop == currtop)
	  return(currtop);
	iac_elev(tree, newtop, count);
	iac_mvnames(tree, newtop, NUM_FSNAME);
	iac_redrw(tree, ACNAMBOX, NORMAL, 1);
	iac_redrw(tree, ACFSVSLI, NORMAL, 1);
	return(newtop);
}


	WORD
iac_dial(tree)
	LONG		tree;
{
	WORD		touchob;
	WORD		cont;
	WORD		xd, yd, wd, hd;
	LONG		spec;
	WORD		newstate;
	WORD		i;
	WORD		fcurrtop, fcount;
	WORD		move;
	GRECT		pt;
	WORD		mx, my, kret, bret;
	
	iac_chkd = ACC1NAME;
	fcount = iac_names(tree);
	fcurrtop = 0;
	iac_elev(tree, fcurrtop, fcount);
	for (i=0; i<3; i++)
	{
	  LWSET(OB_STATE(ACC1NAME+i), NORMAL);
	  spec = ADDR(&(gl_caccs[i].acc_name[0]));
	  if (iac_isnam(spec))
	  {
	    iac_strcop(tree, ACC1NAME+i, spec);
	    iac_schar(tree, ACC1FMEM+i, gl_caccs[i].acc_swap);
	  }
	  else
	  {
	    iac_strcop(tree, ACC1NAME+i, ADDR("unused"));
	    iac_schar(tree, ACC1FMEM+i, 'Y');
	  }
	}
	LWSET(OB_STATE(iac_chkd), CHECKED);

	form_center(tree, &xd, &yd, &wd, &hd);
	form_dial(FMD_START, 0, 0, 0, 0, xd, yd, wd, hd);
	objc_draw(tree, ROOT, MAX_DEPTH, xd, yd, wd, hd);
	
	cont = TRUE;
	while (cont)
	{
	  touchob = form_do(tree, 0);
	  touchob &= 0x7fff;
	  newstate = NORMAL;
	  move = 0;
	  graf_mkstate(&mx, &my, &kret, &bret);
	  switch (touchob)
	  {
	    case ACC1NAME:
	    case ACC2NAME:
	    case ACC3NAME:
	      objc_change(tree, iac_chkd, 0, xd, yd, wd, hd, NORMAL, TRUE);
	      iac_chkd = touchob;
	      newstate = CHECKED;
	      break;

	    case ACC1FMEM:
	    case ACC2FMEM:
	    case ACC3FMEM:
	      spec = LLGET(OB_SPEC(touchob));
	      iac_schar(tree, touchob, LHIBT(LHIWD(spec))=='Y'?'N':'Y');
	      iac_redrw(tree, touchob, NORMAL, 0);
	      break;

	    case ACREMV:
	      iac_strcop(tree, iac_chkd, ADDR("unused"));
	      iac_redrw(tree, iac_chkd, CHECKED, 0);
	      break;

	    case ACA1NAME:
	    case ACA2NAME:
	    case ACA3NAME:
	    case ACA4NAME:
	    case ACA5NAME:
	    case ACA6NAME:
	    case ACA7NAME:
	    case ACA8NAME:
	      spec = LLGET(OB_SPEC(touchob));
	      if (iac_isnam(spec))
	      {
	        iac_strcop(tree, iac_chkd, spec);
	        iac_redrw(tree, iac_chkd, CHECKED, 0);
	      }
	      break;

	    case ACFUPARO:
	      move = -1;
	      break;

	    case ACFDNARO:
	      move = 1;
	      break;
	    
	    case ACFSVSLI:
		ob_actxywh(tree, ACFSELEV, &pt);
		pt.g_x -= 3;
		pt.g_w += 6;
		if ( inside(mx, my, &pt) )
		  goto dofelev;
		move = (my <= pt.g_y) ? -1 : 1;
		break;
	    case ACFSELEV:
dofelev:	wind_update(3);
		ob_relxywh(tree, ACFSVSLI, &pt);
		pt.g_x += 3;
		pt.g_w -= 6;
		LWSET(OB_X(ACFSVSLI), pt.g_x);
		LWSET(OB_WIDTH(ACFSVSLI), pt.g_w);
		move = graf_slidebox(tree, ACFSVSLI, ACFSELEV, TRUE);
		pt.g_x -= 3;
		pt.g_w += 6;
		LWSET(OB_X(ACFSVSLI), pt.g_x);
		LWSET(OB_WIDTH(ACFSVSLI), pt.g_w);
		wind_update(2);
		move = mul_div(move, fcount-1, 1000) - fcurrtop;
		break;


	    case ACINST:
	    case ACCNCL:
	      cont = FALSE;
	      break;

	    default:
	      break;
	  }
	  objc_change(tree, touchob, 0, xd, yd, wd, hd, newstate, TRUE);
	  if (move)
	  {
	    fcurrtop = iac_scroll(tree, fcurrtop, fcount, move);
	  }
    
        }					/* undraw the form	*/
	show_hide(FMD_FINISH, tree);
	LWSET(OB_STATE(iac_chkd), NORMAL);
	return(touchob);
}

/************************************************************************/
/* i n s _ a c c	  						*/
/************************************************************************/

	VOID
ins_acc()
{			       
	LONG		tree;

	tree = G.a_trees[ADINSACC];

/* get current accessory names */
/* stuff them in slots in dialog */

	if (iac_dial(tree) == ACINST)
	{
	  iac_save(tree);
/* copy names from tree to current acc list */
/* delete some/all current accs and free channels */
/* run the new accessories */

	}
} /* ins_acc */


