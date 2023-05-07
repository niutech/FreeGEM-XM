/*	GEMSHLIB.C	4/18/84 - 09/13/85	Lee Lorenzen		*/
/*			6/26/86 - 9/8/86	MDF			*/

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
#include <dos.h>
#include <gem.h>
#include <gemlib.h>
#include <gemerr.h>

#if MULTIAPP

#define ALPHA   0
#define GRAPHIC 1

EXTERN PD	*fpdnm();
EXTERN WORD	mn_ppdtoid();
EXTERN WORD	dos_open();
EXTERN WORD	dos_close();
EXTERN PD	*desk_ppd[];
EXTERN BYTE 	*desk_str[];
EXTERN WORD	gl_dacnt;
EXTERN WORD	gl_mnpds[];
EXTERN PD	*gl_mnppd;
EXTERN LONG	menu_tree[];
EXTERN WORD	gl_pids;		/* bit vector of pids used	*/
EXTERN VOID	acancel();
EXTERN VOID	apret();
EXTERN PD	*gl_lastnpd;
EXTERN WORD	gl_numaccs;
EXTERN ACCNODE	gl_caccs[];
EXTERN WORD	proc_msg[8];
EXTERN UWORD	hdr_buff[HDR_LENGTH/2];
EXTERN PD	*w_topapp();
#endif

EXTERN BYTE	*op_gname();
EXTERN VOID	cpmcod();
EXTERN WORD	strlen();
EXTERN WORD	fm_error();
EXTERN WORD	terminate();
EXTERN VOID	dos_exec();
EXTERN WORD	dos_wait();

EXTERN WORD	DOS_AX;
EXTERN WORD	DOS_ERR;
EXTERN WORD	cli();
EXTERN WORD	sti();
EXTERN WORD	takeerr();
EXTERN WORD	giveerr();
EXTERN WORD	retake();
EXTERN WORD	gsx_init();
EXTERN WORD	gsx_graphic();
EXTERN WORD	gsx_wsclose();
EXTERN WORD	gsx_malloc();
EXTERN WORD	gsx_mfree();
EXTERN BYTE	*rs_str();
						/* in GSXIF.C		*/
EXTERN WORD	gl_handle;
EXTERN WORD	gl_width;
EXTERN WORD	gl_height;
EXTERN WORD	gl_wchar;
EXTERN WORD	gl_hchar;
EXTERN WORD	gl_wbox;
EXTERN WORD	gl_hbox;
EXTERN GRECT	gl_rscreen;
EXTERN GRECT	gl_rfull;
EXTERN GRECT	gl_rmenu;

EXTERN LONG	tikaddr;
EXTERN LONG	tiksav;
EXTERN WORD	gl_ticktime;

EXTERN LONG	ad_sysglo;
EXTERN LONG	ad_envrn;
EXTERN LONG	ad_stdesk;
EXTERN LONG	ad_armice;
EXTERN LONG	ad_hgmice;

EXTERN PD	*ctl_pd;

EXTERN LONG	desk_tree[];

EXTERN THEGLO	D;

#if MULTIAPP
GLOBAL WORD	nulp_msg[8];
GLOBAL LONG	gl_efnorm, gl_efsave;
GLOBAL LONG	gl_strtaes;
GLOBAL LONG	gl_endaes;
GLOBAL WORD	gl_ldpid = -1;
GLOBAL WORD	gl_scrnmode = ALPHA;
GLOBAL BYTE	gl_shcmd[128], gl_shtail[128];
#endif

GLOBAL SHELL	sh[NUM_PDS];

#if GEMDOS
GLOBAL BYTE	sh_apdir[70];			/* holds directory of	*/
						/*   applications to be	*/
						/*   run from desktop.	*/
						/*   GEMDOS resets dir	*/
						/*   to parent's on ret	*/
						/*   from exec.		*/
#endif
GLOBAL LONG	ad_scmd;
GLOBAL LONG	ad_stail;
#if GEMDOS
#else
GLOBAL LONG	ad_s1fcb;
GLOBAL LONG	ad_s2fcb;
#endif
GLOBAL LONG	ad_ssave;
GLOBAL LONG	ad_dta;
GLOBAL LONG	ad_path;

GLOBAL LONG	ad_pfile;

GLOBAL WORD	gl_shgem;
GLOBAL BYTE	gl_rsname[16];

	WORD
sh_read(pcmd, ptail)
	LONG		pcmd, ptail;
{
	LBCOPY(pcmd, ad_scmd, 128);
	LBCOPY(ptail, ad_stail, 128);
}


/*
*	Routine to set the next application to run
*
*		isgem = 0   then run in character mode
*		isgem = 1   them run in graphic mode
*
*		isover = 0  then run above DESKTOP
*		isover = 1  then run over DESKTOP
*		isover = 2  then run over AES and DESKTOP
*/
	WORD
sh_write(doex, isgem, isover, pcmd, ptail)
	WORD		doex, isgem, isover;
	LONG		pcmd, ptail;
{
	SHELL		*psh;
#if GEMDOS
	WORD		drive;
#endif

	LBCOPY(ad_scmd, pcmd, 128);
	LBCOPY(ad_stail, ptail, 128);
	if (isover > 0)
	{
						/* stepaside to run	*/
	  psh = &sh[rlr->p_pid];
	  psh->sh_isgem = (isgem != FALSE);
	  psh->sh_doexec = doex;
	  psh->sh_dodef = FALSE;
	  psh->sh_fullstep = isover - 1;
#if GEMDOS					
	  sh_curdir(ADDR(&sh_apdir[0]));	/* save apps. current 	*/
	  					/* directory		*/
#endif
	}
	else
	{
	  sh_fixtail(FALSE);
						/* run it above us	*/
	  if ( sh_find(ad_scmd, NULLPTR) )
	  {
#if GEMDOS
	    dos_exec(ad_scmd, ad_envrn, ad_stail);
#else
	    dos_exec(ad_scmd, LHIWD(ad_envrn),
	    	     ad_stail, ad_s1fcb, ad_s2fcb);
#endif
	  }
	  else
	    return(FALSE);
	}
	return(TRUE);				/* for the future	*/
}


/*
*	Used by the DESKTOP to recall 1024 bytes worth of previously
*	'put' desktop-context information.
*/
	WORD
sh_get(pbuffer, len)
	LONG		pbuffer;
	WORD		len;
{
	LBCOPY(pbuffer, ad_ssave, len);
}


/*
*	Used by the DESKTOP to save away 1024 bytes worth of desktop-
*	context information.
*/
	WORD
sh_put(pdata, len)
	LONG		pdata;
	WORD		len;
{
	LBCOPY(ad_ssave, pdata, len);
}


/*
*	Convert the screen to graphics-mode in preparation for the 
*	running of a GEM-based graphic application.
*/
	WORD
sh_tographic()
{
#if MULTIAPP
	if (gl_scrnmode == ALPHA)
	  gl_scrnmode = GRAPHIC;
	else
	  return;
#endif
						/* retake ints that may	*/
						/*   have been stepped	*/
						/*   on by char. appl.	*/
						/*   including err. 	*/
						/*   handler and gem.int*/
	cli();
	retake();
	sti();
						/* convert to graphic	*/
	gsx_graphic(TRUE);
						/* set initial clip rect*/
	gsx_sclip(&gl_rscreen);
#if SINGLAPP
						/* allocate screen space*/
	gsx_malloc();
#endif
						/* start up the mouse	*/
	ratinit();
						/* put mouse to hourglass*/
	gsx_mfset(ad_hgmice);
}


/*
*	Convert the screen and system back to alpha-mode in preparation for
*	the running of a DOS-based character application.
*/
	WORD
sh_toalpha()
{
#if MULTIAPP
	if (gl_scrnmode == GRAPHIC)
	  gl_scrnmode = ALPHA;
	else
	  return;
#endif
						/* put mouse to arrow	*/
	gsx_mfset(ad_armice);
						/* give back the error	*/
						/*   handler since ours	*/
						/*   is graphic		*/
	cli();
	giveerr();
	sti();
						/* turn off the mouse	*/
	ratexit();
#if SINGLAPP
						/* return screen space	*/
	gsx_mfree();
#endif
						/* close workstation	*/
	gsx_graphic(FALSE);
}



/*
*	Routine called everytime dos_find has another path to search
*/
	VOID
sh_draw(lcmd, start, depth)
	LONG		lcmd;
	WORD		start;
	WORD		depth;
{
	LONG		tree;
	SHELL		*psh;

	psh = &sh[rlr->p_pid];

	if (gl_shgem)
	{
	  tree = ad_stdesk;
	  gsx_sclip(&gl_rscreen);
	  LLSET(ad_pfile, lcmd);
	  ob_draw(tree, start, depth);
	}
}


/*
*	Routine called everytime dos_find has another path to search
*/

sh_show(lcmd)
	LONG		lcmd;
{
	WORD		i;

	for(i=1; i<3; i++)
	  sh_draw(lcmd, i, 0);
}


/*
*	Routine to take a full path, and scan back from the end to 
*	find the starting byte of the particular filename
*/
	BYTE
*sh_name(ppath)
	REG BYTE	*ppath;
{
	REG BYTE	*pname;

	pname = &ppath[strlen(ppath)];
	while ( (pname >= ppath) &&
		(*pname != '\\') &&
		(*pname != ':') )
	  pname--;
	pname++;
	return(pname);
}


/*
*	Search for a particular string in the DOS environment and return
*	a long pointer to the character after the string if it is found. 
*	Otherwise, return a NULLPTR
*/
	VOID
sh_envrn(ppath, psrch)
	LONG		ppath;
	REG LONG	psrch;
{
	LONG		lp, ad_loc1;
	WORD		len, findend;
	BYTE		tmp, loc1[10], loc2[10];

	len = LSTCPY(ADDR(&loc2[0]), psrch);
	len--;
	ad_loc1 = ADDR(&loc1[0]);
	loc1[len] = NULL;

	lp = ad_envrn;
	findend = FALSE;
	do
	{
	  tmp = LBGET(lp);
	  lp++;
	  if ( (findend) &&
	       (tmp == NULL) )
	  {
	    findend = FALSE;
	    tmp = 0xFF;
	  }
	  else
	  {
	    if (tmp == loc2[0])
	    {
	      LBCOPY(ADDR(&loc1[0]), lp, len);
	      if ( strcmp(&loc1[0], &loc2[1]) )
	      {
	        lp += len;
	        break;
	      }
	    }
	    else
	      findend = TRUE;
	  }
	} while( tmp );
	if (!tmp)
	  lp = 0x0L;
	LLSET(ppath, lp);
}


/*
*	Search first, search next style routine to pick up each path
*	in the PATH= portion of the DOS environment.  It returns the
*	next higher number to look for until there are no more
*	paths to find.
*/

	WORD
sh_path(whichone, dp, pname)
	WORD		whichone;
	LONG		dp;
	REG BYTE	*pname;
{
	REG BYTE	tmp, last;
	LONG		lp;
	REG WORD	i;
						/* find PATH= in the	*/
						/*   command tail which	*/
						/*   is a double null-	*/
						/*   terminated string	*/
	sh_envrn(ADDR(&lp), ADDR(rs_str(STPATH)));
	if (!lp)
	  return(0);
						/* if found count in to	*/
						/*   appropriate path	*/
	i = whichone;
	tmp = ';';
	while (i)
	{
	  while ( tmp = LBGET(lp) )
	  {
	    lp++;
	    if (tmp == ';')
	      break;
	  }
	  i--;
	}
	if (!tmp)
	  return(0);
						/* copy over path	*/
	while ( tmp = LBGET(lp) )
	{
	  if ( tmp != ';' )
	  {
	    LBSET(dp++, tmp);
	    last = tmp;
	    lp++;
	  }
	  else
	    break;
	}
						/* see if extra slash	*/
						/*   is needed		*/
	if ( (last != '\\') &&
	     (last != ':') )
	  LBSET(dp++, '\\');
						/* append file name	*/
	LSTCPY(dp, ADDR(pname));
						/* make whichone refer	*/
						/*   to next path	*/
	return(whichone+1);
}
	VOID
sh_curdir(ppath)
	LONG	ppath;
{
	WORD	drive;
						/* remember current	*/
						/*  directory		*/
	drive = dos_gdrv();
	LBSET(ppath++, (drive + 'A') );
	LBSET(ppath++, ':');
	LBSET(ppath++, '\\');
	dos_gdir( drive+1, ppath );
}


/*
*	Routine to verify that a file is present.  It first looks in the
*	current directory and then looks down the search path.  Before
*	it looks at each point it firsts call the passed-in routine with
*	the filespec that is looking for.
*/
	WORD
sh_find(pspec, routine)
	LONG		pspec;
	WORD		(*routine)();
{
	WORD		len, path, drive;
	BYTE		*pname, tmpname[14];

	dos_sdta(ad_dta);

	len = LSTCPY(ad_path, pspec);		/* copy to local buffer	*/
	pname = sh_name(&D.g_dir[0]);		/* get ptr to name	*/
	strcpy(pname, &tmpname[0]);		/* save name		*/
	sh_curdir(ad_path);			/* get current drive/dir*/
	if (D.g_dir[3] != NULL)			/* if not at root	*/
	  strcat("\\", &D.g_dir[0]);		/*  add foreslash	*/
	strcat(&tmpname[0], &D.g_dir[0]);	/* append name to drive	*/
						/* and directory.	*/
	path = 0;
	do
	{

	  dos_sfirst(ad_path, F_RDONLY | F_SYSTEM);

	  if ( (DOS_AX == E_PATHNOTFND) ||
	        ((DOS_ERR) && 
	         ((DOS_AX == E_NOFILES) ||
	          (DOS_AX == E_PATHNOTFND) ||
		  (DOS_AX == E_FILENOTFND))) )
	  {
	    path = sh_path(path, ad_path, &tmpname[0]);
	    DOS_ERR = TRUE;
	  }
	  else
	    path = 0;
	} while ( DOS_ERR && path );

	if (!DOS_ERR)
	{
	  LSTCPY(pspec, ad_path);
	  if (routine)
	    (*routine)(ad_path);
	}
	return(!DOS_ERR);
}


 	BYTE
*sh_parse(psrc, pfcb)
	REG BYTE	*psrc;
	REG BYTE	*pfcb;
{
	REG BYTE	*ptmp;
	BYTE		*sfcb;
	BYTE		drv;

	sfcb = pfcb;
						/* scan off white space	*/
	while ( (*psrc) &&
	        (*psrc == ' ') )
	  *psrc++;
	if (*psrc == NULL)
	  return(psrc);
						/* remember the start	*/
	ptmp = psrc;
						/* look for a colon	*/
	while ( (*psrc) &&
		(*psrc != ' ') &&
		(*psrc != ':') )
	  *psrc++;
						/* pick off drive letter*/
	drv = 0;
	if ( *psrc == ':' )
	{
	  drv = toupper(*(psrc - 1)) - 'A' + 1;
	  psrc++;
	}
	else
	  psrc = ptmp;
	*pfcb++ = drv;
	if (*psrc == NULL)
	  return(psrc);
						/* scan off filename	*/
	while ( (*psrc) &&
		(*psrc != ' ') &&
		(*psrc != '*') &&
		(*psrc != '.') &&
		(pfcb <= &sfcb[8]) )
	  *pfcb++ = toupper(*psrc++);
						/* pad out with blanks	*/
	while ( pfcb <= &sfcb[8] )
	  *pfcb++ = (*psrc == '*') ? ('?') : (' ');
	if (*psrc == '*')
	  psrc++;
						/* scan off file ext.	*/
	if ( *psrc == '.')
	{
	  psrc++;
	  while ( (*psrc) &&
		  (*psrc != ' ') &&
		  (*psrc != '*') &&
		  (pfcb <= &sfcb[11]) )
	    *pfcb++ = toupper(*psrc++);
	}
	while ( pfcb <= &sfcb[11] )
	  *pfcb++ = (*psrc == '*') ? ('?') : (' ');
	if (*psrc == '*')
	  psrc++;
						/* return pointer to	*/
						/*   remainder of line	*/
	return(psrc);
}


/*
*	Routine to fix up the command tail and parse FCBs for a coming
*	exec.
*/
	VOID
sh_fixtail(iscpm)
	WORD		iscpm;
{
	REG WORD	i;
	WORD		len;
	BYTE		*s_tail;
	BYTE		*ptmp;
	BYTE		s_fcbs[32];
						/* reuse part of globals*/
	s_tail = &D.g_dir[0];
	i = 0;

	if (iscpm)
	{
	  s_tail[i++] = NULL;
	  ptmp = &D.s_cmd[0];
	  while ( (*ptmp) &&
		  (*ptmp != '.') )
	    s_tail[i++] = *ptmp++;
	}

	LBCOPY(ADDR(&s_tail[i]), ad_stail, 128 - i);

	if (iscpm)
	{
						/* pick up the length	*/
	  len = s_tail[i];
						/* null over carriage ret*/
	  s_tail[i + len + 1] = NULL;
						/* copy down space,tail	*/
	  strcpy(&s_tail[i+1], &s_tail[i]);
	}
	else
	{
						/* zero the fcbs	*/
	  bfill(32, 0, &s_fcbs[0]);
	  bfill(11, ' ',  &s_fcbs[1]);
	  bfill(11, ' ',  &s_fcbs[17]);
						/* parse the fcbs	*/
	  if ( s_tail[0] )
	  {
	    s_tail[ 1 + s_tail[0] ] = NULL;
	    ptmp = sh_parse(&s_tail[1], &s_fcbs[0]);
	    if (*ptmp != NULL)
	      sh_parse(ptmp, &s_fcbs[16]);
	    s_tail[ 1 + s_tail[0] ] = 0x0d;
	  }
#if GEMDOS
#else
						/* copy into true fcbs	*/
	  LBCOPY(ad_s1fcb, ADDR(&s_fcbs[0]), 16); 
	  LBCOPY(ad_s2fcb, ADDR(&s_fcbs[16]), 16); 
#endif
	}
						/* copy into true tail	*/
	LBCOPY(ad_stail, ADDR(s_tail), 128);
}


/*
*	Read the default application to invoke.
*/

sh_rdef(lpcmd, lpdir)
	LONG		lpcmd;
	LONG		lpdir;
{
	SHELL		*psh;

	psh = &sh[rlr->p_pid];

	LSTCPY(lpcmd, ADDR(&psh->sh_desk[0]));
	LSTCPY(lpdir, ADDR(&psh->sh_cdir[0]));
}


/*
*	Write the default application to invoke
*/

sh_wdef(lpcmd, lpdir)
	LONG		lpcmd;
	LONG		lpdir;
{
	SHELL		*psh;

	psh = &sh[rlr->p_pid];

	LSTCPY(ADDR(&psh->sh_desk[0]), lpcmd);
	LSTCPY(ADDR(&psh->sh_cdir[0]), lpdir);
}


sh_chgrf(psh)
	SHELL		*psh;
{
	if ( psh->sh_isgem != gl_shgem )
	{
	  gl_shgem = psh->sh_isgem;
	  if ( gl_shgem )
	    sh_tographic();
	  else
	    sh_toalpha();
	}
}

/*
*
*/
	VOID
sh_chdef(psh)
	SHELL		*psh;
{
						/* if we should exec	*/
						/*   the default command*/
						/*   then let it be	*/
						/*   known that it is	*/
						/*   a gem appl.	*/
	psh->sh_isdef = FALSE;
	if ( psh->sh_dodef )
	{
	  psh->sh_isdef = psh->sh_isgem = TRUE;
	  psh->sh_fullstep = 0;
	  dos_sdrv(psh->sh_cdir[0] - 'A');
	  dos_chdir(ADDR(&psh->sh_cdir[0]));
	  strcpy(&psh->sh_desk[0], &D.s_cmd[0]);
	}
#if GEMDOS
	else
	{			
	  dos_sdrv(sh_apdir[0] - 'A');		/* desktop's def. dir	*/
	  dos_chdir(sh_apdir);
	}
#endif
}

#if MULTIAPP

	PD*
sh_chmsg(owner)
	PD	*owner;
{
	WORD	mesag;


	mesag = 0x0;

	if (! sh[owner->p_pid].sh_isgem)
	{    /* DOS app */ 
	  w_windfix(owner);
	  mn_bar(0L, FALSE, 0);
	  mesag = AC_OPEN;
	}    /* accy	*/
	else if (sh[owner->p_pid].sh_isacc)
	{
	  mesag = AC_OPEN;
	}
	else /* GEM app */
	{
	  if (owner != w_topapp())
	  {
	    w_windfix(owner);
	    mn_bar(0L, FALSE, 0);
	    w_buildall();	
	  }
	  w_menu_desk(owner->p_pid, TRUE);
	}
	return(mesag);
}

/* 
*	Special case DOS load
*	Used from pr_exec when loading a dos app
*/
	VOID
sh_dosexec()
{
	WORD		done;
	PD		*owner;
	WORD		mbuff[8];
	WORD		pid;
	WORD		ret;

	pid = rlr->p_pid;

	pr_exec(pid, ad_scmd, ad_stail, ad_envrn, ad_s1fcb, ad_s2fcb);

	done = FALSE;
	while (!done)
	{
/**/	  sh_toalpha();

	  sh[pid].sh_loadable = TRUE;

	  ret = pr_load(pid);

	  sh[pid].sh_loadable = FALSE;
	  gl_ldpid = 1;/* screen manager is automatically load after dos */
	  if (ret == OKGEM)
	  {
	      owner = desk_ppd[pr_retid()];

/**/	      if (sh[owner->p_pid].sh_isgem)
	      {	
		sh_tographic();	
		w_windfix(owner);
	        w_buildall();
	        w_menu_desk(owner->p_pid, TRUE);	
	      }								

	      if (sh[owner->p_pid].sh_isacc || !sh[owner->p_pid].sh_isgem)
		ap_sendmsg(proc_msg, AC_OPEN,
				 owner, 0, mn_indextoid(pr_retid()), 0, 0, 0);
	  } 
	  else
	  {
	    sh_tographic();
	    done = TRUE;
	  }

	  if (!done)
	  {
	    mbuff[0] = 0;
	    while (mbuff[0] != AC_OPEN)
	      ap_rdwr(MU_MESAG, rlr, 16, ADDR(&mbuff[0]) );
	  }
	} 
	return(ret);
}

BYTE cmd[128], tail[128];
	VOID
sh_ldapp()
{
	WORD		ret, i;
	SHELL		*psh;
	PD		*ppd;
	BYTE		name[11], *str;
	WORD		isacc, isdosacc, isgem;
	WORD		da_id;
	WORD		pid;
	WORD		fd;


	pid = rlr->p_pid;

	psh = &sh[pid];
	psh->sh_doexec = FALSE;
	psh->sh_state |= RUNNING;
	isacc = psh->sh_isacc;
	isgem = psh->sh_isgem; 
	isdosacc = isacc && !isgem;

dbg("ldapp, isgem=%x, isacc=%x, pid=%x\r\n", isgem, isacc, rlr->p_pid);

	if (!isacc)
	{
	  for (i=0; i<10; i++)
	    dsptch();
					/* take the old guy's windows	*/
					/*  out of the W_TREE		*/
	  w_windfix(rlr);
	  if (isgem && !(psh->sh_state & CONTINUOUS))
	  {
	    w_buildall();

	    for (i=0; i<10; i++)
	      dsptch();
	  }
	}

	pr_load(0);
ldagain:

sh_read(ADDR(cmd), ADDR(tail));
dbg("command=%s,tail=%s\r\n",cmd,tail);

	if ( !sh_find(ad_scmd, NULLPTR/*(isacc ? NULLPTR : &sh_show)*/) )
	{
					/* error, cant find app or acc	*/
	  goto ldend;
	}
	sh_fixtail(FALSE);

	desk_tree[pid] = 0x0L;
	p_nameit(rlr, sh_name(&D.s_cmd[0]));

					/* clear his desk field	*/
	if ( !isacc)
	{
	    name[0] = name[1] = ' ';
	    movs(8, &rlr->p_name[0], &name[2]);
	    name[10] = ' ';
	    name[11] = NULL;
	    gl_mnpds[pid] = mn_register(pid, ADDR(&name[0]));
	}
					/* normal dos exec if gem app	*/
					/*  or if dos app that is an acc*/
	if (isgem)
	{	
	  psh->sh_loadable = TRUE;
					/* make sure channel is clean	*/
	  ret = pr_load(pid);
	  if (ret != 0)
	  {
	    if (ret == 4)
	      ret = NOHANDLES;	
	  }
	  else
	  {
	    if (psh->sh_state & SHRINK)	/* set up for setblock	*/
	    {
						/*   read the header	*/
	      fd = dos_open( ad_scmd, RMODE_RD);
	      if ( !DOS_ERR )
	      {
	        dos_read(fd, 28, ADDR(&hdr_buff[0]));
		dos_close(fd);
	      }
	      if ( !DOS_ERR )
	        psh->sh_shrsz = ((hdr_buff[1] + 15) >> 4) + (hdr_buff[2] << 5);
	      else
		psh->sh_state &= ~SHRINK;	/* don't try shrink	*/
	    }
	    LLSET(0xefL * 4, LLCS() | LW(&cpmcod));
	    				/* exec app into channel	*/
	    ret = dos_exec(ad_scmd, LHIWD(ad_envrn), ad_stail, 
		   ad_s1fcb, ad_s2fcb);
	    if (! DOS_ERR)
	      ret = 0; 	
	    if (wind_spb.sy_owner == rlr)	/* if he still owns screen*/
	      unsync(&wind_spb);		/*   then take him off.	*/
	  }

	}
	else if (isacc)			/* dos app that is an acc	*/
	{				/*  this must come back here	*/
					/*  without any switch key( + )	*/
					/*  having been depressed.	*/
	  pr_exec(pid, ad_scmd, ad_stail,
	  	  ad_envrn, ad_s1fcb, ad_s2fcb);

	  psh->sh_loadable = TRUE;

	  ret = pr_load(pid);
	}
	else				/* normal dos app		*/
	  ret = sh_dosexec();

dbg("exec return, pid=%x, ret=%x\r\n", pid, ret);

	if (ret)
	  fm_error(ret);

	if (psh->sh_isacc)
	{
	  acancel(rlr->p_evbits);	/* cancel his event's		*/
	  if (rlr->p_evflg)
	    apret(rlr->p_evflg);
	}

					/* take all the items out of	*/
					/*  the DESKTOP menu.		*/
	while ( (da_id = mn_ppdtoid(rlr)) != NIL )
	  mn_unregister( da_id );

	gl_mnpds[pid] = 0x0;
	menu_tree[pid] = 0L;
					/* set name to availnul		*/
	p_nameit(rlr, op_gname(STAVAIL) );
	desk_tree[pid] = 0x0L;
					/* if the current process has	*/
					/*  done a sh_write to load a	*/
					/*  process in place of itself,	*/
					/*  then we must clear out his	*/
					/*  old windows and load in the	*/
					/*  new app.			*/

	if (isgem)
	  w_clswin();			/* close rlr's windows 		*/

	if (psh->sh_doexec)
	{
	  psh->sh_doexec = FALSE;

	  w_windfix(rlr);
	  w_buildall();

	  for (i=0; i<10; i++)
	    dsptch();

	  goto ldagain;
	}

ldend:
	psh->sh_loadable = FALSE;

	pr_delete(pid);			/* close the channel		*/

	ap_sendmsg(proc_msg, PR_FINISH, fpdnm(NULLPTR, 0), pid, 0, 0, 0, 0);

	if (!isdosacc)
	{
	  if ((pid != 0) && !isacc)
	    if (psh->sh_state & CONTINUOUS)
	      w_menu_desk(1, TRUE);	
	    else
	    {
					/*dbg("switch to desk\r\n");*/
	      ppd = fpdnm(NULLPTR, 0);	/* get pd *			*/
	      w_windfix(ppd);		/* make desktop's windows active*/
	      w_buildall();
	      w_menu_desk(0, TRUE);
	    }
	}

	psh->sh_isacc = FALSE;
	psh->sh_state = NULL;
	psh->sh_isgem = TRUE;
}
	VOID
sh_delaccs()
{
	WORD pid;

				/*dbg("enter delaccs\r\n");*/

	for(pid=0; pid<NUM_PDS; pid++)
	  if (sh[pid].sh_isacc && sh[pid].sh_loadable)
	  {
				/*dbg("deleting pid =%x\r\n", pid);*/
	    pr_abort(pid);
				/*dbg("done deleting pid =%x\r\n", pid);*/
	  }	
				/*dbg("fixing windows\r\n");*/
	w_windfix(fpdnm(NULLPTR, 0));
				/*dbg("fixing menus\r\n");*/
	w_buildall();
	w_menu_desk(0, TRUE);
				/*dbg("done deleting accs\r\n");*/
}

#endif

#if SINGLAPP
	VOID
sh_ldapp()
{
	WORD		ret, badtry, retry;
	SHELL		*psh;

	psh = &sh[rlr->p_pid];
#if GEMDOS
	strcpy(ad_scdir,sh_apdir);		/* initialize sh_apdir	*/
#endif
	badtry = 0;	

	strcpy(rs_str(STDESKTP), &psh->sh_desk[0]);
	strcpy(&D.s_cdir[0], &psh->sh_cdir[0]);

	do
	{
	  sh_chdef(psh);
						/* set up so that we	*/
						/*   will exec the 	*/
						/*   default next time	*/
						/*   unless the		*/
						/*   application does	*/
						/*   a set command	*/
	  psh->sh_dodef = TRUE;
						/* init graph/char mode	*/
	  sh_chgrf(psh);
	  if (gl_shgem)
	  {
	    wm_start();
	    ratinit();
	  }
						/* fix up/parse cmd tail*/ 
	  sh_fixtail(psh->sh_fullstep == 2);
	  sh_draw(ad_scmd, 0, 0);		/* redraw the desktop	*/

						/* clear his desk field	*/
	  desk_tree[rlr->p_pid] = 0x0L;
	  	  	  	  	  /* exec it	  	  */
					  /* handle bad try msg	*/
	  if (badtry)
	  {
	    ret = fm_show(badtry, NULLPTR, 1);
	    if (badtry == ALNOFIT)
	      break;
	    badtry = 0;
	  }
	  do
	  {
	    retry = FALSE;
	    if ( sh_find(ad_scmd, sh_show) )
	    {
	      p_nameit(rlr, sh_name(&D.s_cmd[0]));
	      if (psh->sh_fullstep == 0)
	      {
#if GEMDOS
	        dos_exec(ad_scmd, ad_envrn, ad_stail);
#else
	        dos_exec(ad_scmd, LHIWD(ad_envrn), ad_stail, 
			 ad_s1fcb, ad_s2fcb);
#endif
	      }
	      else if (psh->sh_fullstep == 1)
	      {
#if GEMDOS
	        dos_exec(ad_scmd, ad_envrn, ad_stail);
#else
	        gsx_exec(ad_scmd, LHIWD(ad_envrn), ad_stail, 
			 ad_s1fcb, ad_s2fcb);
#endif
	        DOS_ERR = psh->sh_doexec = FALSE;
	      }
	      if (DOS_ERR)
		badtry = (psh->sh_isdef) ? ALNOFIT : AL08ERR;
/*  02/11/86 LKW begin	*/
		if (wind_spb.sy_owner == rlr)	/* if he still owns screen*/
		  unsync(&wind_spb);		/*   then take him off.	*/
/*  02/11/86 LKW end	*/
	    }
	    else
	    {
	      if ( (gl_shgem) &&
		   (psh->sh_isdef) )
	      {
		ret = fm_show(ALOKDESK, NULLPTR, 1);
		if (ret == 1)
		  retry = TRUE;
		else
		  retry = psh->sh_doexec = FALSE;
	      }
	      else
		badtry = AL18ERR;
	    }
	  } while (retry && !badtry);
						/* clear his desk field	*/
	  desk_tree[rlr->p_pid] = 0x0L;

	} while(psh->sh_doexec);
}
#endif

	VOID
sh_main()
{
#if MULTIAPP
	WORD		ii, ret;

	for (ii = 0; ii < 2; ii++)		/* init pid 0 and 1	*/
	{
	  sh[ii].sh_isgem = TRUE;
	  sh[ii].sh_isacc = FALSE;
	  sh[ii].sh_state = 0x0;
	  pr_scpid(ii, FALSE);
	}
	sh[1].sh_loadable = TRUE;
	pr_menu(ADDR(desk_str[0]), ADDR(&gl_dacnt));
						/* allocate screen space*/
	gsx_malloc();
	ratinit();
						/* fix up aes channel	*/
	ret = pr_shrink(SCR_MGR, TRUE, &gl_strtaes, &gl_endaes);
	if (ret == 0) /* no error */
	{
	  sh_rdinf();
	  sh_ldacc();		
					     /* create the desktop chnl */
	  pr_create(rlr->p_pid, gl_endaes, 0L, FALSE, TRUE);
						/* do the exec		*/
	  LBCOPY( ad_scmd, ADDR( gl_shcmd), 128);
	  LBCOPY(ad_stail, ADDR(gl_shtail), 128);

	  sh_ldapp();
	}
	else
	  fm_error(ret);
						/* return screen space	*/
	gsx_mfree();
#endif

#if SINGLAPP
	sh_ldapp();
#endif
						/* get back to alpha	*/
						/*   mode if necessary	*/
	if (gl_shgem)
	  sh_toalpha();

}

#if MULTIAPP
	VOID
sh_ldacc()
{
	WORD		id, ii, ret;
	WORD		junk;
	ULONG		caddr, csize, endmem, cssize, intaddr;

	for(ii = 0; ii < gl_numaccs; ii++)
	{
	  prc_create(gl_endaes, 0x00010000L, FALSE, TRUE, &id);
	  ret = pr_run(id, TRUE, SH_ACC, ADDR(&gl_caccs[ii].acc_name[0]),
	  	       ad_stail);
	  if (ret)
	  {
	    pr_info(id, &junk, &junk, &caddr, &csize, &endmem, 
			&cssize, &intaddr);
	    gl_endaes = caddr + (csize << 12);	/* + (cssize << 12);*/
	  }
	}
}
#endif
