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
EXTERN WORD	gl_gemss;
EXTERN WORD	gl_gemsp;
EXTERN WORD	gl_curss;
EXTERN WORD	gl_cursp;
EXTERN WORD	gl_vdmode;
EXTERN WORD	gl_init;
EXTERN LONG	ad_rootarena;
EXTERN LONG	ad_memstrt;
EXTERN LONG	ad_endofmem;
EXTERN WORD	gl_scrnsize;
EXTERN WORD	gl_sysid;
EXTERN LONG	gl_sysdta;
EXTERN WORD     gl_syspsp;
EXTERN WORD	gl_nmdisk;
EXTERN WORD	gl_syscurdsk;
EXTERN WORD	gl_pid;
EXTERN LONG	gl_addr[MAXCHNLS];
EXTERN LONG	gl_size[MAXCHNLS]; 
EXTERN WORD	gl_fhndl[MAXCHNLS];
EXTERN WORD	gl_isgem[MAXCHNLS];
EXTERN WORD	gl_chksum[MAXCHNLS];
EXTERN LONG	gl_mark;
EXTERN WORD	gl_curss, gl_cursp;	 
EXTERN BYTE	gl_vdrivechar;	
EXTERN BYTE	gl_fnm1[] = "v:\\gem0.swp";	
EXTERN BYTE	gl_fnm2[] = "c:\\gemsys\\gem0.swp";	

/* unsigned long inequality routines */

ULLESS(al,ah,bl,bh)
	UWORD	al,ah,bl,bh;
{
	if (ah == bh)
    return(al < bl);
	else
    return(ah < bh);
}

ULGRTR(al,ah,bl,bh)
	UWORD	al,ah,bl,bh;
{
	if (ah == bh)
    return(al > bl);
	else
    return(ah > bh);
}

ULLSEQ(al,ah,bl,bh)
	UWORD	al,ah,bl,bh;
{
	if (ah == bh)
    return(al <= bl);
	else
    return(ah <= bh);
}

ULGREQ(al,ah,bl,bh)
	UWORD	al,ah,bl,bh;
{
	if (ah == bh)
    return(al >= bl);
	else
    return(ah >= bh);
}

#if I8086
	LONG
sw_segoff(l)		     /* converts long integer to segment-offset */
	LONG	l;
{
	LONG 	offset;
	LONG	seg;
	
	offset =   l & 0x0000000FL;
	seg    = ( l & 0x000FFFF0L ) << 12;	
	return(seg + offset);	
}  
	LONG
sw_long(l)		   /* converts segment-offset to long integer	*/
	LONG	l;
{
	LONG 	offset;
	LONG	seg;
	
	offset =   l & 0x0000FFFFL;
	seg    = ( l >> 12) & 0x000FFFF0L;	
	return(seg + offset);	
}
#endif

	LONG
sw_io(mode, handle, cnt, pbuffer)    /* long read and write to a file	*/
	WORD		mode, handle; 
	LONG		cnt;
	LONG		pbuffer;
{
	LONG	bytesleft;
	UWORD	reqstlen;
	LONG	retlen;

	if(DBG)dbg("sw_io, mode=%x, handle=%x, cnt=%lx, pbuffer=%lx\r\n",
		mode, handle, cnt, pbuffer);

	bytesleft = cnt;
	do
	{
	    if (bytesleft > 0xFE00L) 
		reqstlen = 0xFE00;
	    else
		reqstlen = bytesleft;

	    if (mode)
		retlen = dos_write(handle, reqstlen, pbuffer);
	    else	
		retlen = dos_read(handle, reqstlen, pbuffer);
	    if (DOS_ERR)
	    {
		if(DBG)dbg("DOS ERROR\r\n");
    return(retlen);
	    }
	    pbuffer += sw_segoff(retlen);		
	    bytesleft -= retlen;
	}
	while (bytesleft && (retlen == reqstlen));
    return(cnt - bytesleft);	
}
	WORD
sw_fc(pfhndl, filespec, size)		/* creates a swap file for a channel	*/
	WORD	*pfhndl;
	BYTE	*filespec;
	LONG	size;
{
	UWORD	ret;
	WORD	fhndl;

	if(DBG)dbg("fc, filespec=%s, size=%lx\r\n", filespec, size);

	*pfhndl = -1;

	fhndl = dos_create(ADDR(filespec), 2);
	if (DOS_ERR)
	{
	    if(DBG)dbg("cannot create file, rc=%x\r\n", fhndl);
    return(fhndl);	
	}

	dos_lseek(fhndl, 0, size - 1);

	ret = dos_write(fhndl, 1, 0L);	
	if ((ret != 1) || DOS_ERR)
	{
	    if(DBG)dbg("not enough space, rc=%x\r\n", ret);
	    dos_close(fhndl);
	    dos_delete(ADDR(filespec));
    return(NOSPACE);
	}

	*pfhndl = fhndl;
    return(0);	
}
	BYTE *
sw_fname(pid, number)
	BYTE	pid;
	WORD	number;
{ 

	switch(number)
	{
	    case 1:	
		gl_fnm1[0] = gl_vdrivechar;
		gl_fnm1[6] = "0123456789abcdef"[pid];
		return(gl_fnm1);
	    case 2:
		gl_fnm2[13] = "0123456789abcdef"[pid];
		return(gl_fnm2);
	}
}
	WORD
sw_fcreate(pid, tryvdisk)	/* creates a swap file for a channel	*/
	WORD	pid;
	WORD	tryvdisk;
{
	WORD	fhndl;
	LONG	size;
	WORD	ret;

/* fix 6-19-86 */
	fhndl = -1;
/*  */
	size = sw_long(gl_size[pid]);

	if(DBG)dbg("fcreate, pid=%x, size=%lx\r\n", pid, size);

	if (gl_vdrivechar && tryvdisk)	/* try creating the file on the vdisk*/
	    ret = sw_fc(&fhndl, sw_fname(pid, 1), size);

	if (fhndl == -1)		/* create the file on the hard disk */
	{
	    ret = sw_fc(&fhndl, sw_fname(pid, 2), size);
	    if (fhndl == -1)
    return(ret);	
	}

	gl_fhndl[pid] = fhndl;

	if(DBG)dbg("fhndl=%x\r\n", fhndl);
    return(0);	
}
	WORD		/* writes an area of memory to a given file	*/
sw_snap(s, addr, len)
	BYTE	*s;
	LONG	addr, len;	
{
	WORD	fhndl;
	
	if(DBG)dbg("%s, addr= %lx, len= %lx\r\n", s, addr, len);
	fhndl =	dos_create(ADDR(s), 0);
	if (DOS_ERR) 
	{
	    if(DBG)dbg("creation error\r\n");  
    return;
	}	
	dos_lseek(fhndl, 0, 0L);
	sw_io(WRITE, fhndl, sw_long(len), addr);
	dos_close(fhndl);
}
	VOID
sw_delete(pid)				/* delete channel from memory	*/ 
	WORD pid;			/* and disk for reuse		*/
{
	LONG	ca;

	if(DBG)dbg("delete, pid=%x\r\n", pid);
	if (pid == -1) 
    return; /* this should never execute */				
 
	ca = ad_memstrt;		/* set all fragments belonging	*/
	do				/* to pid to -1			*/
	{	
	    if ( pid == LWGET(ca + OF_CHNPID) ) 
		LWSET(ca + OF_CHNPID, -1);
	    ca += LLGET(ca + OF_FRAGSIZE);
	}
	while (ULLESS(ca, ad_endofmem));	

	if (gl_fhndl[pid] != -1) 
	{
	    if(DBG)dbg("closing fhndl #%x\r\n", gl_fhndl[pid]);	
	    dos_close(gl_fhndl[pid]);	/* close and delete swap file	*/
	}
	dos_delete(ADDR(sw_fname(pid,1)));
	dos_delete(ADDR(sw_fname(pid,2)));
	gl_fhndl[pid] = -1;
}
	WORD
sw_iofrag(mode, pid, start, end)/* read or write fragment from/to disk	*/
	WORD	mode;		/* from/to memory			*/
	WORD	pid;
	LONG	start, end;
{
	WORD	fhndl;
	LONG	position, reqstlen, retlen;

	if(DBG)dbg("sw_iofrag, mode=%x, pid=%x, start=%lx, end=%lx\r\n",
		mode, pid, start, end);

	if (pid == -1)
    return(TRUE);

	fhndl = gl_fhndl[pid];
	if (fhndl == -1) 
    return(FALSE);

	if (mode == WRITE)	/* in write mode, don't write fcb's to	*/
	    start += SIZEARENA;	/* file					*/

	position = sw_long(start - gl_addr[pid]);
	if(DBG)dbg("dos_lseek, fhndl=%x, position=%lx\r\n", fhndl, position);
	dos_lseek(fhndl, 0, position );			
	if (DOS_ERR) 
    return(FALSE);	

	reqstlen = sw_long(end - start);

	retlen = sw_io(mode, fhndl, reqstlen, start);
	if ((retlen != reqstlen) || DOS_ERR)
    return(FALSE);

    return(TRUE);
}
	VOID
sw_setfcb(pid, start, end) 		/* sets fragment control block	*/
	WORD	pid;
	LONG	start, end; 
{
	LONG	i;

	for (i=0; i <= 12; i+=4)
	    LLSET(start + i, 0L); 
	LLSET(start + OF_FRAGSIZE, end - start);
	LWSET(start + OF_CHNPID,   pid);	
}
	WORD
sw_swap(pid, ca, size, mode)	/* swaps out fragments and in the	*/
	WORD	pid;		/* new channel given by pid		*/
	LONG	ca, size;
	WORD	mode;
{
	LONG	endofch, curca, nextca, startread, endread, startwrite;
	WORD	curpid, nextpid, ok, ret;

	if(DBG)dbg("sw_swap, pid=%x, ca=%lx, size=%lx, mode=%x\r\n",
			 pid, ca, size, mode);

	endofch = ca + size;
	nextca  = ad_memstrt;
	nextpid = LWGET(nextca + OF_CHNPID);
	startread = 0;	

	do
	{
 	    do			/* get info on current and next fragment */
	    {
		if (ULGREQ(nextca, endofch))	
		{
/* new 8-26-86 
		  if (ULGRTR(nextca, endofch))
		  {
		    LLSET(endofch + OF_FRAGSIZE, nextca - endofch);
		    LWSET(endofch + OF_CHNPID,  nextpid);	
		  }	
*/
    goto swapend;
		}
		curca   = nextca;
		curpid  = nextpid;
		nextca += LLGET(curca + OF_FRAGSIZE);
		if (ULLESS(nextca, ad_endofmem))
		    nextpid = LWGET(nextca + OF_CHNPID);
		else
		    nextpid = -1;
	
		if(DBG)dbg("curpid =%x,curca =%lx\r\n",curpid, curca);
		if(DBG)dbg("nextpid=%x,nextca=%lx\r\n",nextpid,nextca);
	    }
	    while ( ULLSEQ(nextca, ca) || (pid == curpid) );
				/* skip over fragments before channel	*/
				/* fragments or belonging to channel	*/
	    if (startread == 0)
		startread = curca;
		
	    if (ULLESS(curca, ca))
	    {	
		     		/* if current fragment starts before    */
		        	/* channel then split fragment          */
		startwrite = ca - SIZEARENA;
		startread  = ca;
		LLSET(curca + OF_FRAGSIZE, ca - curca);
	    }
	    else
		startwrite = curca;

	    if (ULLSEQ(nextca, endofch))
		ok = sw_iofrag(WRITE, curpid, startwrite, nextca);
	    else
	    {   		/* if current fragment goes beyond end  */
		        	/* of channel then split fragment       */
		ok = sw_iofrag(WRITE, curpid, startwrite, endofch + SIZEARENA);

		LLSET(endofch + OF_FRAGSIZE, nextca - endofch);
		LWSET(endofch + OF_CHNPID,   curpid);	
		nextca = endofch; 	
	    }

	    if (! ok)		/* if write-error, patch new channel	*/ 
	    {			/* and delete channel that caused error	*/
		sw_setfcb(pid, ca, nextca);
		sw_delete(curpid);	
		if(DBG)dbg("SWAP OUT ERROR\r\n");
		if(DBG)dbg_frags(ad_memstrt, ad_endofmem);
	    }	 

	    if (mode == OUTANDIN) /* read in new channel		*/
	    if ( (nextpid == pid) || ULGREQ(nextca, endofch) )
	    {
/* new 3-20-86 */
		if (nextpid == pid)
		{			/* if next frag ours read data  */
					/* over its fcb			*/
		  endread = nextca + SIZEARENA;

		  nextca += LLGET(nextca + OF_FRAGSIZE);
		  if (ULLESS(nextca, ad_endofmem))
		      nextpid = LWGET(nextca + OF_CHNPID);
		  else
		      nextpid = -1;
		}
		else
		  endread = nextca;   
/* new */
	        if (sw_iofrag(READ, pid, startread, endread))
		    startread = 0;
		else
		{		/* if read-error, patch new channel and	*/
				/* delete new channel			*/ 
		    sw_setfcb(pid, ca, nextca);
		    sw_delete(pid);	
		    if(DBG)dbg("SWAP IN ERROR\r\n");
		    if(DBG)dbg_frags(ad_memstrt, ad_endofmem);
    return(SWAPINERR);
		}	
	    }
	}
	while (ULLESS(nextca, endofch));
	
swapend:
	sw_setfcb(pid, ca, endofch); /* set new channel to full size	*/

	if(DBG)dbg("no error\r\n");
	if(DBG)dbg_frags(ad_memstrt, ad_endofmem);

    return(0);	
}
	VOID
sw_setarena(lp, size, flag)	/* sets values in an arena	*/
	LONG	lp;
	LONG	size;
	WORD 	flag;
{
	BYTE	pic_char;
	WORD	owner;
	LONG	nxt_arena, diff;

	nxt_arena = lp + SIZEARENA + size;
	diff = ad_endofmem - nxt_arena;
	if (ULGRTR(diff, 16L))
		pic_char = 'M';
	else
		pic_char = 'Z';	
	LBSET(lp + OF_PIC, pic_char);

	if (flag == ALLOCATED)
		owner = gl_sysid;
	else
		owner = 0x0;
	LWSET(lp + OF_OWNER, owner);
	
	LWSET(lp + OF_ALOCSIZE, (UWORD)(size >> 16));
}
	VOID
sw_setend(lp,flag)			/* sets arena to allocate the rest of 	*/ 
	LONG	lp;		/* memory				*/
	WORD	flag;
{
	sw_setarena(lp, ad_endofmem - lp - SIZEARENA, flag);
}
	WORD
sw_firstload(pid, ca, chnsize, swapable, isgem)/* create new channel	*/
	WORD	pid;
	LONG	ca, chnsize;
	WORD	swapable;
	WORD	isgem;
{
	LONG	off, size_free, ret;
	WORD	fhndl;	

	if (chnsize == 0L)		/* maximum size */
	    chnsize = ad_endofmem - ca;

	if(DBG)dbg("firstload, pid=%x, addr=%lx, size=%lx\r\n",pid,ca,chnsize);

	if (ULGRTR(ca + chnsize, ad_endofmem))
		chnsize = ad_endofmem - ca;

	gl_addr [pid] = ca;
	gl_size [pid] = chnsize;
	gl_isgem[pid] = isgem;
					/* allocate channel swap file	*/ 
	if (swapable)
	{
	    ret = sw_fcreate(pid, TRUE);
	    if (ret)
    return(ret);
	}
					/* clear area for channel	*/
	sw_swap(pid, ca, chnsize, OUTONLY);
/*
	if (! isgem)		/* if dos app				*/
	    vd_scpos(0,0);	/* set cursor in top left corner	*/
					/* sets channel system info	*/
*/
	LLSET(ca + OF_FRAGSIZE, chnsize);
	LWSET(ca + OF_CHNPID,   pid);

	off = OF_FREE;
	if (pid != 1)
	{
	  LLSET(ca + OF_I_OF,	off);			  /* interrupts */
	  off += sw_segoff(INTSIZE, 0);
	}

	if (! isgem)					  /* if dos app */
	{
	    LLSET(ca + OF_S_OF,	off);
	    off += sw_segoff(gl_scrnsize,0);
	}

	LLSET(ca + OF_A_OF,     off);
					/* set up arenas		*/
	size_free = chnsize - off - (2*SIZEARENA);
					/* set channel work area	*/
	sw_setarena(ca + off, size_free, FREE);
					/* allocate memory after channel*/
	sw_setend(ca + chnsize - SIZEARENA, ALLOCATED);	

	gl_pid = pid;			/* change context to new channel*/ 
	sw_setarena(ad_rootarena, ca + off - ad_memstrt, ALLOCATED);

	if(DBG)dbg("no error\r\n");

return(0);
}
	LONG		
sw_shrink(pid)			/* shrink channel by eliminating it's	*/ 
	WORD	pid;		/* free memory				*/
{
	LONG	newendofch, ca, oldsize;

	if(DBG)dbg("shrink pid= %x\r\n", pid);

	ca = gl_addr[pid];
	oldsize = gl_size[pid];
	newendofch = dos_alloc(0x100L);

	if (DOS_ERR || ULLESS(newendofch, ad_rootarena))
	  newendofch = ca + oldsize;
	else
	{	
	  sw_setfcb(pid,         ca,   newendofch);
	  sw_setfcb( -1, newendofch, ca + oldsize);

  	  gl_size[pid] = newendofch - ca;
	  sw_setend(newendofch - SIZEARENA, ALLOCATED);
	}
	return(newendofch);
}	
	WORD			/* swap in channel and switch to it	*/
sw_load(pid)
	WORD	pid;
{
	LONG	ca, arena_of, ptr;
	WORD    cdsk, pdsk, sum, vdmode, ret;
	BYTE	ch, *p, path[70], different; 

	if(DBG)dbg("LOAD %x\r\n",pid);
	gl_pid = pid;
	ca = gl_addr[pid];

	ret = sw_swap(pid, ca, gl_size[gl_pid], OUTANDIN);
	if (ret)
    return(ret);	

	if (DBG)			/* for debugging purposes	*/
	{
	  if (gl_dbgprt != 2)
	  {
	    sum = chk_sum(ca, ca + gl_size[pid]);
	    dbg("chksum %x: prev= %x, cur= %x\r\n",pid,gl_chksum[pid],sum);
	  }
	}
	if (! gl_isgem[pid])		/* if dos app restore screen	*/
	{
		ch_restorescreen(ca);
					/* context switch stacks	*/
		gl_curss = LWGET(ca + OF_SS);
		gl_cursp = LWGET(ca + OF_SP);
	}
	else
	{
		gl_curss = gl_gemss;
		gl_cursp = gl_gemsp;
	}
					/* set new dos context		*/
	dos_spsp( LWGET(ca + OF_PSP) );		/* set process psp	*/

	dos_sdta( LLGET(ca + OF_DTA) );		/* set process dta	*/

	cdsk = dos_gdrv();			/* get current disk	*/
	pdsk = LWGET(ca + OF_CURDSK);		/* get process disk	*/
	if ( cdsk != pdsk )
	  dos_sdrv(pdsk);

	dos_gcdir(cdsk+1, ADDR(path));

	p = path;
	ptr = ca + OF_CDIR;
	different = FALSE;

	do
	{
	  ch = *p++;
	  if (ch != LBGET(ptr++))
	  {
	    different = TRUE;
	    break; 	
	  }
	}
	while (ch);
	
	if (different)
	{
	  if(DBG)dbg("different directories\r\n");	

	  p = path;
	  *p++ = 'A'+cdsk;
	  *p++ = ':';
	  *p++ = '\\';
	
	  LBCOPY(ADDR(p), ca + OF_CDIR, 67);

	  dos_scdir(ADDR(path));	
						/* set current directory*/
	}
	else
	  if(DBG)dbg("same directories\r\n");	

	arena_of = LLGET(ca + OF_A_OF); 	/* get offset of arena	*/
						/* from channel header	*/ 
						/* switch-in channel	*/
	sw_setarena(ad_rootarena, ca + arena_of - ad_memstrt, ALLOCATED);

    return(0);	
}

	VOID		
sw_save()			/* save current channel's context	*/
{
 	LONG	ca;
	WORD	pid, sum; 
	WORD	cdsk, pdsk;
	LONG	temp1,temp2, temp3;

	pid = gl_pid;
	ca  = gl_addr[pid];
				/* save current process context		*/
	LWSET(ca + OF_PSP,    dos_gpsp());

	cdsk = dos_gdrv();
	LWSET(ca + OF_CURDSK, cdsk);

	dos_gcdir(cdsk+1, ca + OF_CDIR);

	LLSET(ca + OF_DTA,    dos_gdta());

	if(DBG)dbg("save %x\r\n", pid);

	if (! gl_isgem[pid])			/* if dos app	*/
	{					/* save screen	*/
		LWSET(ca + OF_SS, gl_curss);
		LWSET(ca + OF_SP, gl_cursp);

		ch_savescreen(ca);
	}
	else
	{				/* if gem app	*/
		gl_gemss = gl_curss;	/* save gem stack	*/
		gl_gemsp = gl_cursp;	
	}
				/* switch to gdos process context	*/
	dos_spsp( gl_syspsp );
	dos_sdta( gl_sysdta );			/* set process dta	*/
		

	
if (DBG)
    {
	if (gl_dbgprt != 2)
	{
	  sum = chk_sum(ca, ca + gl_size[pid]);
	  gl_chksum[pid] = sum;
	}
    }
}
	WORD
sw_malloc(paddress, psize)
	LONG	*paddress, *psize;
{
	LONG	tmpaddr1, tmpsize1, tmpaddr2, tmpsize2;
	WORD	pid;

	if (DBG) dbg("sw_malloc, size = %lx\r\n", *psize);
	
	*psize += (2 * SIZEARENA);

	tmpaddr1 = gl_addr[0];			/* size below channel     */ 
	tmpsize1 = gl_addr[gl_pid] - tmpaddr1;

	tmpaddr2 = gl_addr[gl_pid] + gl_size[gl_pid]; /* size above channel */
	tmpsize2 = ad_endofmem - tmpaddr2;

	if (ULGREQ(tmpsize2, *psize)) 		/* favor using area above */
	  *paddress = tmpaddr2;
	else if (ULGREQ(tmpsize1, *psize))	/* else try area below	  */
	  *paddress = tmpaddr1;
	else 					/* not enough space find  */
	{					/* return max avail	  */
	  *paddress = 0L; 
	  if (ULGRTR(tmpsize1, tmpsize2))
	    *psize = tmpsize1;	
	  else
	    *psize = tmpsize2;

	  return(FALSE); 
	}  
						/* clear allocated memory */ 
	sw_swap(256, *paddress, *psize, OUTONLY);
	LWSET(*paddress + OF_CHNPID, -1);	/* delete channel	  */
	*paddress += SIZEARENA;			/* allow for fcb(fragment */
	*psize    -= (2 * SIZEARENA);		/* control block)	  */
	return(TRUE);
}

UWORD	uw(x)
	WORD	x;
{
	return(x);
}

dbg_links(off, cur)
	UWORD	off, cur;
{
	BYTE	ch;
	UWORD	owner, pghs;

	if (DBG)
	{
	  dbg("dbg_links\r\n");

	  while (ULLESS(0, cur, ad_endofmem)) 
	  {
	    ch	  = LBGET(0, cur);
	    owner = LWGET(1, cur);
	    pghs  = uw(LWGET(3, cur));
	  
	    dbg("%x, %c, %x, %x\r\n", cur, ch, pghs, owner);	  

	    if (ch == 'Z')
	      break;

	    cur = cur + pghs + 1;  	  	   
 	  }	

	  dbg("last cur = %x\r\n", cur);
	}
}
