/* DOSPASTE.C		1/28/86	- 2/18/86			MDF	*/

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*/


#include <portab.h>
#include <machine.h>
#include <filtab.h>

#define	CR	'\015'
#define LF	'\012'

#define	RDBUFFSZ	512
#define PBUFFSZ		2000

EXTERN	WORD	DOS_ERR;



EXTERN	WORD	dos_open();
EXTERN	WORD	dos_read();
EXTERN	VOID	dos_close();
EXTERN	LONG	dos_alloc();
EXTERN	VOID	dos_free();

EXTERN	VOID	take_kbd();			/* in stealkbd.a86	*/
EXTERN	VOID	rest_kbd();

EXTERN	WORD	parse_tbl();			/* in tblparse.c	*/


MLOCAL	WORD	next_key;
MLOCAL	WORD	dp_savch;		/* first char after margin	*/
MLOCAL	WORD	dp_infile;
MLOCAL	WORD	dp_marg;
MLOCAL	WORD*	dp_chpt;
MLOCAL	WORD	dp_chnum;
MLOCAL	WORD	dp_atend;
MLOCAL  LONG	dp_rdbuf;
MLOCAL	WORD	dp_rdlen;


/* Table of scan codes ordered by ASCII value of characters.	*/
/* Values come from diagram on page 1-76 of PC-DOS 2.02 Tech.	*/
/* Reference.							*/

MLOCAL UBYTE	scancodes[] = {
	/* 0-9	*/ 0, 30, 48, 46, 32, 18, 8, 34, 14, 15, 
	/* 10	*/ 36, 37, 38, 28, 49, 24, 25, 16, 19, 31,
	/* 20	*/ 20, 22, 47, 17, 45, 21, 44, 1, 43, 27,
	/* 30	*/ 0, 0, 57, 2, 40, 4, 5, 6, 8, 40,
	/* 40	*/ 10, 11, 55, 13, 51, 12, 52, 53, 11, 2,
	/* 50	*/ 3, 4, 5, 6, 7, 8, 9, 10, 39, 39,
	/* 60	*/ 51, 13, 52, 53, 3, 30, 48, 46, 32, 18,
	/* 70	*/ 33, 34, 35, 23, 36, 37, 38, 50, 49, 24,
	/* 80	*/ 25, 16, 19, 31, 20, 22, 47, 17, 45, 21,
	/* 90	*/ 44, 26, 43, 27, 7, 12, 41, 30, 48, 46,
	/* 100	*/ 32, 18, 8, 34, 35, 23, 36, 37, 38, 50,
	/* 110	*/ 49, 24, 25, 16, 19, 31, 20, 22, 47, 17,
	/* 120	*/ 45, 21, 44, 26, 43, 27, 41, 14 };

MLOCAL	FILTABLE	dp_filtbl;
MLOCAL	FILTABLE	dp_deftbl =
	{
		{0},
		{0},
		{0x1C0D},				/* RETURN key	*/
		{0},
		0x3920,					/* space bar	*/
		0,
		0
	};

/*	Filter Table Keywords, etc. 					*/

#define NKWRDS	7

#define PBKEY	0
#define	PSKEY	1
#define PLKEY	2
#define	PEKEY	3
#define	PFKEY	4
#define	CMKEY	5
#define	PMKEY	6

BYTE	*keywords[NKWRDS] = {	"PB=",
				"PS=",
				"PL=",
				"PE=",
				"PF=",
				"CM=",
				"PM="	};




/************************************************************************/
/*									*/
/* dp_schar() -- returns scan code/ASCII character pair in word		*/
/*									*/
/*	Looks up scan code based on ASCII value and puts scan code	*/
/*	into high byte, character into low byte.			*/
/*									*/
/************************************************************************/

	WORD
dp_schar(ch)
	BYTE	ch;

{
	WORD	code;

	code = scancodes[ch & 0x7F] << 8;	/* mask out high bit	*/
	return(code + ch);
}



/************************************************************************/
/*									*/
/* dp_isskip() -- returns true if character should be ignored		*/
/*									*/
/*									*/
/************************************************************************/

	WORD
dp_isskip(wch)
	WORD	wch;
{
	WORD	i;

	for (i=0; i<FLTSTRSZ; i++)
	  if (wch == dp_filtbl.flt_skip[i])
	    return(1);
	return(0);
}


/************************************************************************/
/*									*/
/* dp_bufftst() -- check filter buffer and init if needed		*/
/*									*/
/*									*/
/************************************************************************/

	WORD
dp_bufftst(buff)
	WORD*	buff;
{
	dp_chpt = (WORD*)0;
	dp_chnum = 0;
	if (buff[0])				/* anything there?	*/
	{
	  dp_chpt = buff;
	  dp_chnum = 1;				/* count characters	*/
	  while ((dp_chnum < FLTSTRSZ) && dp_chpt[dp_chnum])
	    dp_chnum++;
	  return(dp_chnum);
	}
}

/************************************************************************/
/*									*/
/* dp_buffch() -- get next char from read buffer			*/
/*									*/
/************************************************************************/

	WORD
dp_buffch(ptr)
	BYTE	*ptr;
{
	if (dp_rdlen <= 0)
	  return(0);
	*ptr = LBGET(dp_rdbuf);
	dp_rdbuf += 1;
	dp_rdlen -= 1;
	return(1);
}

/************************************************************************/
/*									*/
/* dp_getch() -- get next character for paste operation			*/
/*									*/
/*	Interprets filter table to set up next paste character.		*/
/*	The character is placed in next_key.				*/
/*	Returns 1 if a character read, 0 if no more.			*/
/*									*/
/************************************************************************/

	WORD
dp_getch()
{
	BYTE	ch;
	WORD	len;

	if (dp_chnum)				/* buffered characters?	*/
	{
	  next_key = *dp_chpt;
	  dp_chpt--;
	  dp_chnum--;
	  return(1);
	}

	if (dp_marg && !dp_atend)		/* need to pad margin?	*/
	{
	  if (dp_marg-- == 1)			/* first real character	*/
	  {
	    if (dp_savch && dp_isskip(dp_savch))
	      return(dp_getch());		/* discard character	*/
	    return(((next_key = dp_savch) == 0)?0:1);
	  }
	  next_key = dp_filtbl.flt_flch;
	  return(1);
	}

	len = dp_buffch(&ch);
	switch(ch)
	{
	  case CR:
	    len = dp_buffch(&ch); /* discard following LF*/
	    if (ch == LF)
	      len = dp_buffch(&ch);
	    dp_savch = len?ch:0;		/* anything left?	*/
	    dp_marg = dp_filtbl.flt_marg + 1;	/* may need margin	*/
	    if (dp_bufftst(&dp_filtbl.flt_eol[0]))
	    {
	      next_key = *dp_chpt;
	      dp_chpt--;
	      dp_chnum--;
	      if (len == 0)
		return(dp_chnum);
	    }
	    else
	      next_key = dp_schar(CR);			/* default EOL	*/
	    break;

	  default:
	    next_key = dp_schar(ch);
	    if (dp_isskip(next_key))
	      return(dp_getch());		/* discard character	*/
	    break;
	}
	return(len);

}

/************************************************************************/
/*									*/
/* dp_kbint() -- routine to handle keyboard interrupt			*/
/*									*/ 
/*	Returns next control or data character until scrap file is 	*/
/*	empty, then puts orignal BIOS handler back in.			*/
/*									*/
/************************************************************************/

	WORD
dp_kbint(caseval)
	WORD	caseval;
{
	WORD	retval;

	retval = next_key;			/* get before read	*/
	switch (caseval)
	{
	  case 0:				/* regular read		*/
	    if (dp_getch() == 0)		/* all done?		*/
	    {
	      if (!dp_atend && dp_bufftst(&dp_filtbl.flt_etxt[0]))
	      {
		dp_atend = 1;			/* don't read again	*/
	        next_key = *dp_chpt;
	        dp_chpt--;
	        if (--dp_chnum == 0)
	        {
		  rest_kbd();
	        }
	      }
	      else
	      {
		rest_kbd();			/* regular inter. back	*/
	      }
	    }
						/* fall through		*/
	  case 1:				/* status only		*/
	    return(retval);

	  default:				/* shift flags		*/ 
	    return(0);				/* TEMP?, may need flags*/
	}
}


/************************************************************************/
/*									*/
/* dp_paste() -- paste initialization routine				*/
/*									*/
/*	1) Opens and reads text scrap file				*/
/*	2) Copies filter table to local structure			*/
/*	3) Inits global pate variables					*/
/*	4) Replaces standard keyboard interrupt routine with dp_kbint	*/
/*									*/
/*	Returns first character in file, or 0 if no file		*/
/*									*/
/************************************************************************/


	WORD
dp_paste(ad_ftbl, ad_rdbuf, needchar)
	LONG	ad_ftbl;		/* address of filter table	*/
	LONG	ad_rdbuf;		/* address of paste read buffer	*/
	WORD	needchar;		/* TRUE if character needed	*/
{
	WORD	retval;

	dp_rdbuf = ad_rdbuf;
	dp_infile = dos_open(ADDR("c:\\gemscrap\\scrap.txt"), 0);
	if (DOS_ERR)
	  return(0);			/* dp_infile = error code	*/

					/* copy to local structure	*/
	LBCOPY(ADDR(&dp_filtbl), ad_ftbl, sizeof(FILTABLE));
	dp_marg = dp_filtbl.flt_marg + 1;
	dp_atend = 0;

	dp_bufftst(&dp_filtbl.flt_strt[0]); /* paste start characters?	*/


	dp_rdlen = dos_read(dp_infile, PBUFFSZ, ad_rdbuff);
	if (DOS_ERR)
	  dp_rdlen = 0;
	dos_close(dp_infile);
					/* read 1st usable char in file	*/

	while (dp_buffch(&dp_savch) &&
		(dp_isskip(dp_savch = dp_schar(dp_savch))) );
	
	if (dp_rdlen)			/* was there a usable char?	*/
	{
	  dp_getch();			/* could be margin padding	*/
	  retval = next_key;		/* first char to paste		*/
	}
	else
	  retval = 0;			/* nothing in file		*/

	if (retval)
	{
	  if (needchar)
	    dp_getch();			/* 2nd char, 1st through inter.	*/
	  take_kbd();			/* 	grab keyboard inter.	*/
	}
	return(retval);
}

/******* ROUTINES FOR PARSING AND INTERPRETING FILTER TABLE FILE ********/

	WORD
strlen(p1)
	BYTE		*p1;
{
	WORD		len;

	len = 0;
	while( *p1++ )
	  len++;

	return(len);
}


	BYTE
toupper(ch)
	BYTE	ch;
{
	if ( (ch >= 'a') && (ch <= 'z') )
	  return(ch - 32);
	else
	  return(ch);
}

	WORD
is_digit(ch)
	BYTE	ch;
{
	return((ch>='0') && (ch<='9'));
}

	WORD
is_xdigit(ch)
	BYTE	ch;
{
	ch = toupper(ch);
	return(is_digit(ch) || ((ch>='A') && (ch<='F')));
}

	WORD
dp_dconv(ch)
	BYTE	ch;
{
	return(is_digit(ch)?(ch - '0'):(toupper(ch)-'A'+10));
}

/************************************************************************/
/*									*/
/* dp_atoi() -- convert ASCII to integer				*/
/*									*/
/*									*/
/************************************************************************/

	WORD
dp_atoi(st_ad, len, base)
	LONG	st_ad;
	WORD	len;
{
	WORD	i;
	WORD	total;

	total = 0;
	for (i=0; i<len; i++)
	  total = total*base + dp_dconv(LBGET(st_ad+i));
	return(total);
}

/************************************************************************/
/*									*/
/* dp_numget() -- convert decimal or hex number from input		*/
/*									*/
/*	Figures out how many digits and the base, then calls the	*/
/*	conversion routine with the base and len.			*/
/*	Returns number of bytes read.					*/
/*									*/
/************************************************************************/

	WORD
dp_numget(num_pt, st_ad)
	WORD*	num_pt;
	LONG	st_ad;
{
	WORD	len;
	BYTE	ch;

	len = 0;
	while (is_digit(ch = LBGET(st_ad + len)) || is_xdigit(ch))
	  len++;
	if (toupper(ch)=='H')
	{	
	  *num_pt = dp_atoi(st_ad, len, 16);
	  len++;				/* skip over 'H'	*/
	}
	else
	  *num_pt = dp_atoi(st_ad, len, 10);
	return(len);
}

/************************************************************************/
/*									*/
/* dp_wchget() -- convert scan code/ ASCII value char from input	*/
/*									*/
/*	When the current input is a quoted string, the first character	*/
/*	in the string is converted, then replaced with the quote to	*/
/*	mark the string for the next time through.			*/
/*	Returns number of bytes read.					*/
/*									*/
/************************************************************************/

	WORD
dp_wchget(wd_pt, st_ad)
	WORD*	wd_pt;
	LONG	st_ad;
{
	BYTE	ch;
	WORD	len;
	WORD	scancode;

	len = 0;
	ch = LBGET(st_ad);
	switch(ch)
	{
	  case 'C':
	    len = 2;
	    *wd_pt = dp_schar(CR);
	    break;

	  case 'L':
	    len = 2;
	    *wd_pt = dp_schar(LF);
	    break;

	  case '\'':
	  case '"':
	    *wd_pt = dp_schar(LBGET(st_ad+1));
	    if (LBGET(st_ad+2) == ch)		/* end of string?	*/
	      len = 3;
	    else
	    {
	      LBSET(st_ad+1,ch);		/* mark for next time	*/
	      len = 1;
	    }
	    break;

	  default:				/* should be digit	*/
	    if (is_digit(ch))
	    {
	      len = dp_numget(wd_pt, st_ad);
	      if (LBGET(st_ad+len) == ',')
	      {
		scancode = 0;
		ch = LBGET(st_ad+len+1);
		if (ch == '[')
		  len += (3 + dp_numget(&scancode, st_ad+len+2));
		else if (is_digit(ch) && (*wd_pt == 0))
		  len += (1 + dp_numget(&scancode, st_ad+len+1));
		*wd_pt += scancode << 8;
	      }
	    }
	    break;
	}
	if (LBGET(st_ad+len) == ',')
	  len++;				/* skip past comma	*/
	return(len);
}


/************************************************************************/
/*									*/
/* dp_lstget() -- convert to string of WORD characters from ASCII.	*/
/*									*/
/*									*/
/************************************************************************/

	VOID
dp_lstget(dst_pt, src_ad)
	WORD*	dst_pt;
	LONG	src_ad;
{
	WORD	i;

	for (i=0; i < FLTSTRSZ; i++)
	{
	  if (LBGET(src_ad) == 0)		/* end of string	*/
	    break;
	  src_ad += dp_wchget(&dst_pt[i], src_ad); 
	}
}

/************************************************************************/
/*									*/
/* db_fltget()	-- convert file data to internal FILTABLE structure	*/
/*									*/
/*									*/
/************************************************************************/

	VOID
db_fltget(addrs)
	LONG	addrs[];
{
	BYTE	ch;
	WORD	i;

	LBCOPY(ADDR(&dp_filtbl), ADDR(&dp_deftbl), sizeof(FILTABLE));

	for (i=0; i<NKWRDS; i++)
	  if (addrs[i])
	    switch (i)
	    {
	      case PBKEY:
	      case PSKEY:
	      case PLKEY:
	      case PEKEY:
		dp_lstget(&dp_filtbl.flt_strt[i*FLTSTRSZ], addrs[i]);
		break;

	      case PFKEY:
		dp_wchget(&dp_filtbl.flt_flch, addrs[PFKEY]);
		break;

	      case CMKEY:
		ch = LBGET(addrs[CMKEY]);
		dp_filtbl.flt_curs = (toupper(ch) == 'Y');
		break;

	      case PMKEY:
		dp_numget(&dp_filtbl.flt_marg, addrs[PMKEY]);
		break;
	    }
}


/************************************************************************/
/*									*/
/* dp_rdtbl() -- read and internalize filter table			*/
/*									*/
/*	Copy the internalized table into the specified buffer.  If 	*/
/*	the filter file does not make sense, use default table.		*/
/*									*/
/************************************************************************/

	VOID
dp_rdtbl(ftbl_addr, nm_addr)
	LONG	ftbl_addr;		/* place to put filter table	*/
	LONG	nm_addr;		/* path/name of application	*/

{
	WORD	file_handle;
	LONG	buff_addr;
	LONG	temp_addr;
	WORD	bytes_read;
	WORD	len;
	WORD	rd_err;
	LONG	fltadrs[NKWRDS];


	rd_err = 0;
	bytes_read = 0;
	buff_addr = dos_alloc((LONG) RDBUFFSZ);
	if (buff_addr == 0)
	  rd_err++;				/* no room to read file	*/

	if (!rd_err)
	{
	  len = LSTCPY(buff_addr, nm_addr);	/* get full file spec	*/
	  temp_addr = buff_addr + len;
	  while (LBGET(temp_addr) != '.')	/* change ext. to .TBL */
	  {
	    temp_addr--;
	    if (temp_addr < buff_addr)		/* no '.'		*/
	    {
	      temp_addr = buff_addr + len;
	      break;
	    }
	  }
	  LSTCPY(temp_addr, ADDR(".TBL"));
	  file_handle = dos_open(buff_addr, 0);
	  if (DOS_ERR)
	    rd_err++;
	}
	
	if (!rd_err)
	{
	  bytes_read = dos_read(file_handle, RDBUFFSZ, buff_addr);

	  dos_close(file_handle);
	  len = parse_tbl(buff_addr,bytes_read,keywords,fltadrs,NKWRDS);
	  if (len)
	  {
	    db_fltget(fltadrs);
	    LBCOPY(ftbl_addr, ADDR(&dp_filtbl), sizeof(FILTABLE));
	  }
	  else	
	    rd_err++;
	}

	if (rd_err)			/* use default filter table	*/
	  LBCOPY(ftbl_addr, ADDR(&dp_deftbl), sizeof(FILTABLE));

	if (buff_addr)
	  dos_free(buff_addr);
#if 0
	dbg("ftbl_addr\r\n");
	dump(ftbl_addr, sizeof(FILTABLE)); 
#endif
}


/************************************************************************/
/*									*/
/*									*/
/*									*/
/*************************************************************************/
#if 0
dump(addr,len)
	LONG	addr;
	WORD	len;
{
	LONG	p;

	for (p=addr; p<addr+len; p+=2) 
	{
	    if (((p-addr)&0x7)==0x0)
		dbg("\r\n%lx - ", p);
	    dbg("%04x ", LWGET(p));
	}
	dbg("\r\n");
}
#endif
