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
#include <ch.h>
#include <gemerr.h>

EXTERN WORD	gl_dbgprt;

EXTERN LONG     sw_segoff();
EXTERN LONG     sw_long();
EXTERN LONG     sw_io();
EXTERN LONG     sw_sysize();
EXTERN LONG     sw_shrink();
EXTERN BYTE*	sw_fname();

GLOBAL LONG	ad_names;	/* address of menu name list       */
GLOBAL BYTE	gl_names[NUM_DESKACC*SIZEDESKSTR];
GLOBAL LONG	gl_pnames[NUM_DESKACC+1];  /* 0L terminated array  */
GLOBAL LONG	ad_dacnt;	/* address of desk accessory count */
GLOBAL WORD	gl_dacnt;

GLOBAL WORD	gl_gemss;
GLOBAL WORD	gl_gemsp;
GLOBAL WORD	gl_vdmode;
GLOBAL LONG	ad_scrn; 
GLOBAL LONG	ad_comint;	/* communication int before AES    */
GLOBAL LONG	ad_timerint;	/* timer         int before AES    */
GLOBAL WORD	gl_scrnsize = 4000;
GLOBAL WORD	gl_init = 0;
GLOBAL LONG	ad_rootarena;
GLOBAL LONG	ad_memstrt;
GLOBAL LONG	ad_endofmem;
GLOBAL BYTE	gl_aesint[INTSIZE];
GLOBAL BYTE	gl_gdint[INTSIZE];
GLOBAL WORD	gl_sysid;
GLOBAL LONG	gl_sysdta;
GLOBAL WORD     gl_syspsp;
GLOBAL WORD	gl_nmdisk;
GLOBAL WORD	gl_syscurdsk;
GLOBAL WORD	gl_pid;
GLOBAL LONG	gl_addr[MAXCHNLS];
GLOBAL LONG	gl_size[MAXCHNLS]; 
GLOBAL WORD	gl_fhndl[MAXCHNLS];
GLOBAL WORD	gl_isgem[MAXCHNLS];
GLOBAL WORD	gl_chksum[MAXCHNLS];
GLOBAL LONG	gl_mark;
GLOBAL WORD	gl_curss, gl_cursp;

GLOBAL BYTE 	gl_zyxg[4] = { 'z', 'y', 'x', 'g' };	 
GLOBAL BYTE	gl_vdrivechar = 1; /* primary drive selection mode:	 */
				   /* 0 = no primary drive		 */
				   /* 1 = use highest existing drive	 */
				   /* 'a'-'z' = specific drive		 */ 
GLOBAL BYTE	gl_esckey = 0x4e;  /* escape key scan code, def = 4eh,'+'*/
GLOBAL BYTE	gl_fcflg = TRUE;   /* if FALSE then swap files for the   */
				   /* AES&acc's are made on the hard disk*/
				   /* to conserve ramdisk space		 */
GLOBAL LONG	gl_deskstrt = 0L;  /* starting address of desktop, used	 */
				   /* in conjunction with gl_fcflg       */
				   /* used to see if chnl below desktop  */
GLOBAL BYTE	gl_fcb[40];
GLOBAL WORD	gl_intson = FALSE;
GLOBAL BYTE	gl_snapchar = '0';
GLOBAL BYTE	gl_fnm1[] = "v:\\gem0.swp"; /* ramdisk swapfile template */
GLOBAL BYTE	gl_fnm2[] = "c:\\gemsys\\gem0.swp"; /* hard disk template*/
GLOBAL LONG	ad_softint = 0L;	/* user software interrupt to be */
					/* called upon the load of any   */
					/* channel			 */
	VOID
ch_savescreen(ca)		/* routine called by switcher to save	*/
	LONG	ca;		/* screen into channel system area	*/ 
{
		LBCOPY(ca + LLGET(ca + OF_S_OF), ad_scrn, gl_scrnsize);
					/* save screen characters/attr	*/
		LWSET(ca + OF_CPOS, vd_gcpos(0));
					/* save cursor position		*/
}
	VOID
ch_restorescreen(ca)		/* routine called by switcher to restore*/
	LONG	ca;		/* screen from channel system area	*/
{
		LBCOPY(ad_scrn, ca + LLGET(ca + OF_S_OF), gl_scrnsize);
					/* restore screen character/attr*/
		vd_scpos(0, LWGET(ca + OF_CPOS));
					/* restore cursor position	*/
}	
	VOID			/* saves a channel's interrupts into	*/ 
ch_getints(pid)			/* it's system area			*/
	WORD	pid;
{
	LONG	curca;

	gd_cli();
	curca = gl_addr[gl_pid]; /* save current channel's interrupts	*/
	LWCOPY(curca + LLGET(curca + OF_I_OF), 0L, INTSIZE/2);
}
	VOID
ch_setints(pid)			/* sets a channel's interrupts from	*/ 
	WORD	pid;		/* it's system area			*/
{	
	LONG	curca;

	gd_cli();
	curca = gl_addr[gl_pid]; /* save current channel's interrupts	*/
	LWCOPY(0L, curca + LLGET(curca + OF_I_OF), INTSIZE/2);
}
	VOID
ch_getgdints()			/* gets default interrupts for dos apps	*/
{
	gd_cli();
	LWCOPY(ADDR(gl_gdint), 0L, INTSIZE/2);
}
	VOID
ch_setgdints()			/* sets default interrupts for dos apps */ 
				/* upon startup				*/
{
	gd_cli();
	LWCOPY(0L, ADDR(gl_gdint), INTSIZE/2);
/*
dbg("ch_setgdints, addr of ints = %lx\r\n", ADDR(gl_aesint));
dbg("THE  ints, 10h=%lx, 16h=%lx, 21h=%lx\r\n", 
	LLGET(4L*0x10L),
	LLGET(4L*0x16L),
	LLGET(4L*0x21L));

dbg("gl_aesint, 10h=%lx, 16h=%lx, 21h=%lx\r\n", 
	LLGET(ADDR(gl_gdint) + 4L*0x10L),
	LLGET(ADDR(gl_gdint) + 4L*0x16L),
	LLGET(ADDR(gl_gdint) + 4L*0x21L));
*/
}
	VOID
ch_getaesints()			/* gets default interrupts for GEM apps */
{
	gd_cli();
	LWCOPY(ADDR(gl_aesint), 0L, INTSIZE/2);
}
	VOID
ch_setaesints()			/* sets default interrupts for GEM apps */
				/* upon startup				*/
{
	gd_cli();
	LWCOPY(0L, ADDR(gl_aesint), INTSIZE/2);
}
	VOID
ch_entry(kbescflg)		/* called by gdos interrupt escape */ 
	WORD	kbescflg;
{
	BYTE	selchar;
	WORD	function, pid, sel, off, ret, do_paste, blanks;
	WORD	ch, firstpchar;
	WORD	i, j, row, col, start, cnt;
	LONG	tmpaddress, tmpsize, curca;

	static LONG	ca;		/* for create info	*/ 
	static LONG	size;	
	static WORD	swap;
	static WORD	isgem;
	static WORD	created = -1;  

	static LONG	cmd;		/* for exec info	*/
	static WORD	environ;
	static LONG	tail;
	static LONG	fcb1;
	static LONG	fcb2;

	if (gl_pid != 1)	   /* if current channel not the AES	*/
	{			   
	  ch_getints(gl_pid);      /* save channel's interrupts		*/

	  if (gl_intson)
	  if (kbescflg)
	    ch_setgdints();	   /* if in DOS set dos default interrupts */ 
	  else
	    ch_setaesints();	   /* if gem app set gem default interrupts*/
	}

	if(DBG)
	  if (gl_pid != 1)	   
	  {
	    dbg("saving interrupts on pid = %x\r\n", gl_pid);
	    if (gl_intson)
	      dbg("switching to system interrupts\r\n");
	  }

	do_paste = FALSE;

	if (kbescflg)			/* true if user hit escape key	*/
	{				/* user is assumed to be in dos */
		if(DBG)			/* app in char mode		*/
		{
			dbg("keyboard escape, kbescflg=%x\r\n", kbescflg);
			dbg("vd_gmode=%x\r\n", vd_gmode());
		}

		blanks = cnt_blks(ad_scrn, gl_scrnsize);
		if(DBG)dbg("cnt_blks, cnt=%x, blanks=%x\r\n",
							 gl_scrnsize, blanks);
		if (gl_isgem[gl_pid])
	goto ch_entret;			/* don't do keyboard escape	*/

		if (blanks < 10)	/* must be in graphics so don't */
	goto ch_entret;			/* do keyboard escape		*/
	
		sel = mn_dispmn(ADDR(gl_pnames), 0, 4); /* display menu */
		switch( sel )		/* respond to user selection	*/
		{
		    case 0: /*copy*/
				    dc_cut(); 
						break;
		    case 1: /*paste*/ 	
				    do_paste = TRUE;
						break; 
		    case 2: /*nop*/   		break;	
		    case 3: /*nop*/   		break;	
		    default:
			sel -= 4;	/* if > 4 then application selected */
			if(DBG)dbg("gdos acc sel=%x\r\n", sel);
			sw_save();	/* switch in AES 		    */
			sw_load(1);

			ch_setaesints();

			LWSET(ad_intout+0L, OKGEM); 
			LWSET(ad_intout+2L, sel);   /* return user selection*/
			LWSET(ad_intout+4L, 1);
		}
    goto ch_entret;	
	}

/* otherwise entry must be to do a switcher-swapper function */ 

	function = LWGET(ad_contr + 10);
	if(DBG)dbg("%d\r\n",function);

	switch(function)
	{
		case 10:  /* create */ /* set create variables */
			pid   = LWGET(ad_intin+0L);
			isgem = LWGET(ad_intin+12L);
			ca    = LLGET(ad_intin+2L);
			size  = sw_segoff(LLGET(ad_intin+6L));
			swap  = LWGET(ad_intin+10L);	
			created = pid;
			break;
		case 11:  /* shrink */ 
			if (! gl_intson)	/* get system interrupts */
			{			/* this should occur once*/
						/* during shrink of AES  */
			  ch_getaesints();
			  gl_intson = TRUE;
			  if(DBG)dbg("getting system interrupts\r\n");
			}

			pid = LWGET(ad_intin+0L);	     	
			LLSET(ad_intout+0L,   gl_addr[pid]); /*return ch addr*/
			LLSET(ad_intout+4L, sw_shrink(pid)); /*shrink channel*/
			ret = 0;
			if (pid == 0)
			  gl_deskstrt = gl_addr[0];
			if (LWGET(ad_intin+2L)) /*if caller wants file   */
			{
			  sw_save();      /* switch to system context */ 
			  ret = sw_fcreate(pid, /* create swap file   */
			  (gl_fcflg ||    /* decide to try ram disk   */
			   ( 
			    gl_deskstrt &&
			    ULGREQ(gl_addr[pid], gl_deskstrt)
			   ) 
			  )
					  );
			  sw_load(gl_pid);
			}
			if (DBG)dbg("shrink ret = %x", ret);
			LWSET(ad_intout+8L, ret);
			break;
		case 12:  /* load */ 
			sw_save();
			pid = LWGET(ad_intin+0L);
			if (ad_softint)
			  callfar(ad_softint, pid); /* call user soft-	*/
					/* ware interrupt, special for 	*/
					/* customer: NWIS		*/ 

					/* copy character menu data	*/
			gl_dacnt = LWGET(ad_dacnt);

			LBCOPY(ADDR(gl_names), ad_names,
					 NUM_DESKACC*SIZEDESKSTR);
					/* set up menu pointer array	*/
			j = 0;
			for (i=0; i<gl_dacnt; i++)
			{
			  gl_pnames[i] = ADDR(gl_names + j);
			  j += SIZEDESKSTR;
			}  
			gl_pnames[gl_dacnt] = 0L;/* null terminate 	*/
						/* pointer array	*/
			if (pid == created)	/* firstload */
			{	
			    created = -1;
			    ret = sw_firstload(pid, ca, size, swap, isgem);
			    if (ret)
				sw_load(1); 	/* if error load AES 	*/
			    else /* load OK */
			    if (gl_isgem[pid])
			    {
				ret = 0;
				LWSET(ad_intout+0L,ret);
				LWSET(ad_intout+4L, gl_pid);
/*** on firstload skip loading interrupts */ 
return;
			    }
			    else /* dos app */
			    {	 	 /* first load of DOS app execs it */
				if(DBG)dbg("exec dos app\r\n");
				
				dp_rdtbl(ca + OF_FT, cmd); /* read and parse*/
					/* filter file into filter table    */
					/* which is in channel system area  */
				ch_setgdints();
					/* set default interrupts	    */
					/* exec program			    */
				ret = gd_exec(cmd, ADDR(&environ));
				        /*restore system ints after dos exit*/
				do_paste = FALSE; /* reset paste flag */ 
				if (gl_intson)
				{
				  ch_setgdints();	
					/* switch to system interrupts	    */ 
				  if(DBG)
				    dbg("switching to system interrupts\r\n");
				}

			        if(DBG)dbg("exec ret =%x",ret);
				sw_save();	/* swap in AES		    */
				sw_load(1);
					/* switch to AES ints after loaded  */
				ch_setaesints();
				if(DBG)
				  dbg("switching to system interrupts\r\n");
			    }
			}
			else  /* not first load of channel */
			{
			  if (! gl_isgem[pid])
			    ch_setgdint();	/* if DOS app set gdos ints */ 
			  ret = sw_load(pid);
			  if (ret)		/* if error, load AES and   */
			  {			/* its interrupts	    */
			    sw_load(1);
			    ch_setaesints();
			  }
			}
			if(DBG)dbg("load, ret=%x\r\n", ret);	
			LWSET(ad_intout+0L,ret);
			LWSET(ad_intout+4L, gl_pid);
			break;
		case 13:  /* exec dos */  	/* set DOS exec variables  */
			pid      = LWGET(ad_intin+0L); 
			cmd      = LLGET(ad_intin+2L);
			environ	 = LWGET(ad_intin+8L);
			tail     = LLGET(ad_intin+10L);
			fcb1     = LLGET(ad_intin+14L);
			fcb2     = LLGET(ad_intin+18L);
		        break;		
		case 14:  /* exit */		/* exit a channel	  */
			pid = LWGET(ad_intin+0L);
			sw_save();
			sw_delete(pid);		/* do cleanup		  */
			sw_load(1);		/* load AES 		  */
			ch_setaesints();	/* set AES interrupts	  */
			break; 
		case 15:  /* get menu address and item count */
			if(DBG)dbg("menu address\r\n");
			ad_names  = LLGET(ad_intin+0L);
			ad_dacnt  = LLGET(ad_intin+4L);
			break;	
		case 16: /* get channel info */
			pid = LWGET(ad_intin+0L);
			LLSET(ad_intout+0L,  gl_addr[pid]);
			LLSET(ad_intout+4L,  sw_long(gl_size[pid]));
			LWSET(ad_intout+8L,  (gl_fhndl[pid] != -1));
			LWSET(ad_intout+10L, gl_isgem[pid]);
			LLSET(ad_intout+12L, ad_endofmem);
			LLSET(ad_intout+16L, 0x2000L);
			LLSET(ad_intout+20L, ADDR(gl_gdint));
			break;
		case 17: /* memory alloc */
			tmpsize = sw_segoff(LLGET(ad_intin+0L));
			sw_save(); 
			sw_malloc(&tmpaddress, &tmpsize);
			sw_load(gl_pid);
			LLSET(ad_intout+0L, tmpaddress);
			LLSET(ad_intout+4L, sw_long(tmpsize));
			break;
		case 18: /* set switch software interrupt */
			ad_softint = LLGET(ad_intin+0L);
			break;
		case 30: /* make a snapshot file of a channel */
			pid = LWGET(ad_intin+0L);
			ch_snap(pid);
			break;
	}

    ch_entret:   
	if (gl_pid != 1)
	{
	    if(DBG)dbg("loading interrupts on pid = %x\r\n", gl_pid);
	    ch_setints(gl_pid);  	/* restore channel's interrupts */
	}

if(DBG)
  if (kbescflg == 2)
    dbg("INSIDE DOS (for paste) \r\n");		

	if (do_paste)
	{				/* paste is done with app's 	*/
					/* interrupts loaded 		*/
	  curca = gl_addr[gl_pid];
	  firstpchar = (kbescflg == 1); /* if not in DOS get first paste*/
					/* char				*/
	  ch = dp_paste(curca + OF_FT, 
			curca + LLGET(curca + OF_S_OF),
			firstpchar);

	  if (firstpchar)		/* set character to send to 	*/ 
	    setpchar(ch);		/* on return			*/
	}
}
	BYTE
ch_getvdrive()			/* returns highest letter active drive  */
				/* if secondary disk return 0		*/
{					/* since "path" is modifed this	   */
	static BYTE path[] = "z:\\";	/* routine may not be called twice */

	while (*path > 'a' + gl_syscurdsk) 
	{
	    if (dos_parsefilename(ADDR(path), ADDR(&gl_fcb), 0) == 0xff)
	    	--*path;		/* if drive does not exist */
	    else
		return(*path);
	}
	return(0);
}
	VOID
ch_start()			/* called before the exec of the AES	*/
{
	WORD	i, fhndl, video_mode;
	LONG	memsize;

	if(DBG)			/* set debug port			*/
	  switch(bio_kb(0)&0x00ff)
	  {
	    case '1': gl_dbgprt = 0; break;
	    case '2': gl_dbgprt = 1; break;
	    default : gl_dbgprt = 2;			
	  } 
	if(DBG)dbg("----------------------------------------------\r\n");		

	for(i=0; i<MAXCHNLS; i++) 
		gl_fhndl[i] = -1;
				/* initialize cut and paste module	*/
	dc_init();	

	gl_syspsp    = dos_gpsp();
	gl_sysdta    = dos_gdta();
	gl_syscurdsk = dos_gdrv();

	gl_fnm2[0] = 'a' + gl_syscurdsk;	/* secondary swap drive	  */
						/* set to gdos startup drv*/ 
						
	video_mode = vd_gmode() & 0x00ff;	/* deduce screen buffer addr */
	switch(video_mode)
	{
	    case 1:
	    case 3:	ad_scrn = 0xb8000000; break;
	    case 2:
	    case 7:	ad_scrn = 0xb0000000; break;
	}

	if (gl_vdrivechar == 1)			
	  gl_vdrivechar = ch_getvdrive();	/* search for vdrive	  */

	if (gl_esckey)				/* set DOS app escape key */
	  setescky(gl_esckey);

	if(DBG)
	{
	  dbg("video mode =%x,ad_scrn=%lx\r\n",video_mode,ad_scrn);
	  dbg("vdrivechar = %c\r\n", gl_vdrivechar);
	}


	gl_nmdisk    = dos_sdrv(gl_syscurdsk);	/* return number of disks */

	memsize      = dos_avail();
	ad_memstrt   = dos_alloc(memsize);

	ad_endofmem  = ad_memstrt + sw_segoff(memsize);
	ad_rootarena = ad_memstrt - SIZEARENA; 

	gl_sysid = LWGET(ad_rootarena + OF_OWNER); 

	sw_setfcb(-1, ad_memstrt, ad_endofmem); /* init all mem to null ch */

	ch_getgdints();

    return(sw_firstload(1, ad_memstrt, 0L, FALSE, TRUE));
					/* create channel for the AES	*/
					/* give the AES all of mem and 	*/
					/* it will shrink itself	*/
}

ch_end()			/* called after the termination of AES  */ 
{
	WORD	pid;

	if(DBG)dbg("after AES\r\n");
	for (pid=0; pid<MAXCHNLS; pid++)
	  sw_delete(pid);		/* remaove all swap files	*/
	sw_setend(ad_rootarena, FREE);	/* free all memory		*/
}

ch_snap(pid)			/* make snapshot file of channel	*/
	WORD	pid;
{
	BYTE	*s;

	if (gl_snapchar > '9')		/* set file name 		*/
    return;
	s = "\\snap0.snp";
	s[5] = gl_snapchar++;		/* increment file name digit	*/
					/* for next call		*/
	sw_snap(s, gl_addr[pid], gl_size[pid]); 
}
