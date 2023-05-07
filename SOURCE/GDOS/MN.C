/*	MN.C	7/22/85 - 8/15/85	Lowell Webster		*/

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

#define BEG_COL 56

EXTERN LONG	LLGET();

EXTERN WORD	gl_pid;		/* cur pid				 */
EXTERN LONG	gl_addr[];	/* array of channel addresses		 */
EXTERN LONG	ad_pname;	/* address of name long pointer list     */
EXTERN LONG	ad_mnpid;	/* address of pids associated with names */
EXTERN LONG	ad_scrn;	/* address of screen buffer		 */

	WORD
mn_dispmn(pstrings, dfault, extra) /* displays menu on character screen  */  
				   /* returns user user selection	 */
	LONG	pstrings;	   /* pointer to selection text		 */  
				   /* pointer to null terminated array	 */
				   /* of pointers			 */ 
	WORD	dfault;		   /* index of default item to highlight */ 
	WORD	extra;		   /* If TRUE then copy, paste and cancel*/  
				   /* items to selection		 */

{
	WORD	i, crow, brow, ch, done, numapps, numlines;
	LONG	scrnbuf, pstr, ca;

	++dfault;		   	/* allow for border		*/
	ca = gl_addr[gl_pid]; 
	scrnbuf = ca + LLGET(ca + OF_S_OF);
	numapps = 0;
	while(((numapps) < NUM_DESKACC) && LLGET(pstrings+((numapps)*4)))
	  numapps++;			/* count number of strings	*/
	numlines = 2+extra+numapps;
	mn_chscrn(scrnbuf, TRUE, BEG_COL, 0, 24, numlines);
	mn_border(TRUE, 0);
	if (extra)			/* do extra lines for copy	*/ 
	{				/* paste and cancel		*/
	  mn_line(1, ADDR("  COPY"));	
	  mn_line(2, ADDR("  PASTE"));	
	  mn_line(3, ADDR("  CANCEL"));
	  mn_line(4, ADDR("----------------------"));
	}
	for(i=extra; pstr=LLGET(pstrings); i++)
	{
	  mn_line(i + 1, pstr);		/* do caller's lines		*/
	  pstrings += 4;
	}
	brow = numapps+extra;
	mn_border(FALSE, brow+1);
	crow = dfault;
	mn_hilite(TRUE, crow);
	done = FALSE;
	while(!done)
	{
	  ch = bio_kb(0);	/* get a char from keyboard	*/ 
	  if(DBG)dbg("char=%x\r\n",ch);	
	  switch(ch)
	  {
		case UP:
		case LEFT:
		case BACKTAB:
		case BACKSPACE:
		  if (crow == 1)
		    break;
		  mn_hilite(FALSE, crow);
		  crow--;
		  if (crow == extra)
		    crow--;			/* skip dashed line in menu */
		  mn_hilite(TRUE, crow);	/* rev vid selection	*/
		  break;
		case SPACE:
		case DOWN:
		case RIGHT:
		case TAB:
		  if (crow == brow)
		    break;
		  mn_hilite(FALSE, crow);
	 	  crow++;
		  if (crow == extra)
		    crow++;			/* skip dashed line in menu */
		  mn_hilite(TRUE, crow);	/* rev vid selection	*/
		  break;
		case RETURN:
		case HOME:
		  mn_hilite(FALSE, crow);
		  done = TRUE;
		  break;
		case ESCAPE:
		  done = TRUE;
		  crow = dfault;
		  break;
	  }
	}
	if(DBG)dbg("string index=%x\r\n",crow-1);
	mn_chscrn(scrnbuf, FALSE, BEG_COL, 0, 24, numlines);
	return(crow-1);
}
	VOID
mn_hilite(isrev, row)		/* highlight line by setting attribute	*/ 
	WORD	isrev, row;	/* attribute = isrev ? 0x70 : 0x07	*/

{
	WORD	col, i, attr;

	col = ((row * 80) + BEG_COL) * 2;
	col++;
	attr = isrev ? 0x70 : 0x07;
	for(i=0; i<22; i++)
	{
	  col +=2;
	  LBSET(ad_scrn+col, attr);
	}
}
	VOID
mn_line(row, pstr)		/* draw line on screen, at given row	*/
	WORD		row;	/* and text given by pstr		*/
	LONG		pstr;
{
	WORD		col, i, ch;

	col = ((row * 80) + BEG_COL) * 2;
	LWSET(ad_scrn+col, 0x0FBA);		/* left border		*/
	for(i=0; ch = LBGET(pstr++); i++)
	{
	  col +=2;
	  LWSET(ad_scrn+col, ch | 0x0700);
	};
	while (i++ < 22)
	{
	  col +=2;
	  LWSET(ad_scrn+col, 0x0720);
	};
	col +=2;
	LWSET(ad_scrn+col, 0x0FBA);		/* rt border		*/
}
	VOID
mn_border(istop, row)			/* draw top or bottom border */
	WORD		istop, row;
{
	WORD		col, border, i;

	col = ((row * 80) + BEG_COL) * 2;
	border = istop ? 0x0FC9 : 0x0FC8;	/* left corner	*/
	LWSET(ad_scrn+col, border);
	for(i=0; i<22; i++)
	{
	  col +=2;
	  LWSET(ad_scrn+col, 0x0FCD);
	}
	col +=2;
	border = istop ? 0x0FBB : 0x0FBC;	/* rt corner*/
	LWSET(ad_scrn+col, border);
}
	VOID
mn_chscrn(pbuf, issave, x, y, w, h)	/* saves and restores screen 	*/ 
					/* screen under menu		*/
	LONG		pbuf;		/* pointer to save buffer	*/
	WORD		issave;
	WORD		x, y, w, h;
{
	WORD		i, offset;

	for(i=0; i<h; i++)
	{
	  offset = (( (i+y) * 80) + x) * 2;
	  if (issave)
	    LWCOPY(pbuf, ad_scrn+offset, w);
	  else
	    LWCOPY(ad_scrn+offset, pbuf, w);
	  pbuf += (w*2);			/* byte pointer	*/
	}
}
