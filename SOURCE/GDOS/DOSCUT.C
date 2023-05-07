/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*/


#include <portab.h>
#include <machine.h>
#include <ch.h>

EXTERN WORD	bio_kb();			/* keyboard character	*/
EXTERN WORD	ros_gcur();			/* get cursor position	*/
EXTERN		ros_scur();			/* set cursor		*/
EXTERN BYTE	ros_gchar();			/* get screen character */
EXTERN		ros_revvid();			/* reverse-video a line */
EXTERN WORD	ros_gmod();			/* get cursor mode	*/
EXTERN		ros_smod();			/* set cursor mode	*/
EXTERN WORD	dos_create();
EXTERN		dos_close();
EXTERN WORD	mn_dispmn();

BYTE	*menu1[] = {"SELECT AREA", "CANCEL"};
BYTE	*menu2[] = {"SELECT NEW AREA", "COPY MARKED AREA", "CANCEL"};

LONG	menu1lng[3];
LONG	menu2lng[4];

LONG	menu1ptr;
LONG	menu2ptr;


MLOCAL	WORD	dc_xbase;
MLOCAL	WORD	dc_ybase;
MLOCAL	WORD	dc_oldw;
MLOCAL	WORD	dc_oldh;

MLOCAL	WORD	drag;


WORD		dc_xcurs;
WORD		dc_ycurs;

	WORD
min(i,j)
	REG WORD	i;
	REG WORD	j;
{
	return(i<j?i:j);
}
	WORD
max(i,j)
	REG WORD	i;
	REG WORD	j;
{
	return(i>j?i:j);
}

	WORD
abs(i)
	REG WORD	i;
{
	return(i<0?-i:i);
}

/************************************************************************/
/*									*/
/* dc_revvid()								*/
/*									*/
/*	flip reverse-video attributes in box				*/
/*									*/
/************************************************************************/


	VOID
dc_revvid(x,y,w,h)
	WORD		x;		/* starting column -- zero-based*/
	WORD		y;		/* starting row -- zero-based	*/
	WORD		w;		/* width in chars		*/
	REG WORD	h;		/* height in chars		*/
{
	REG WORD	offset;

	if (w<1)			/* no funny boxes		*/
	  return;
	offset = y*160 + x*2;		/* offset into screen buffer	*/
	for (;h > 0; h--)		/* bad height handled here	*/
	{
	  ros_revvid(offset,w);
	  offset += 160;		/* char/attribute = 2 bytes	*/
	}
}


/************************************************************************/
/*									*/
/* dc_wrbox()								*/
/*									*/
/*	writes lines in selected rectangle to an ASCII file.  Each	*/
/*	line is terminated with a CR/LF pair.				*/
/*									*/
/************************************************************************/
	VOID
dc_wrbox(outfile)
	WORD	outfile;		/* file handle for ASCII file	*/
{
	REG WORD	x;		/* starting column -- zero-based*/
	REG WORD	y;		/* starting row -- zero-based	*/
	WORD		w;		/* width in chars		*/
	WORD		h;		/* height in chars		*/
	WORD		i, j;
	BYTE		buff[82];	/* maxline + CR,LF		*/

	x = dc_xbase;
	y = dc_ybase;
	w = dc_oldw;
	h = dc_oldh;

	for (j=0; j < h; j++)
	{
	  for (i=0; i < w; i++)
	    buff[i] = ros_gchar((x+i)*2 + (y+j)*160);
	  buff[w] = '\015';			/* carriage return	*/
	  buff[w+1] = '\012';			/* line feed		*/
	  dos_write(outfile, w+2, ADDR(&buff[0]));
	}
	dc_revvid(x,y,w,h);			/* unmark box		*/
}

/************************************************************************/
/*									*/
/* dc_dragpos()								*/
/*									*/
/*	handles position of mouse during drag.  fixes reverse-video	*/
/*	box to correspond to current position of mouse.			*/
/*									*/
/*									*/
/************************************************************************/
	VOID
dc_dragpos(x,y)
	WORD	x;
	WORD	y;
{
	REG WORD	neww;
	REG WORD	newh;
	WORD		x1,y1,w1,h1;
	WORD		x2,y2,w2,h2;

	if ((x < dc_xbase) || (y < dc_ybase))
	{
	  x = dc_xbase;				/* outside of box --	*/
	  y = dc_ybase;				/* reduce to one cell	*/
	}

	neww = (x - dc_xbase) + 1;
	newh = (y - dc_ybase) + 1;

	x1 = dc_xbase;
	y1 = min(y+1,dc_ybase+dc_oldh);
	w1 = min(dc_oldw,neww);
	h1 = abs(newh-dc_oldh); 

	x2 = min(x+1,dc_xbase+dc_oldw);
	y2 = dc_ybase;
	w2 = abs(neww-dc_oldw);
	if ( ((dc_xbase+dc_oldw-1-x) > 0) !=
	     ((dc_ybase+dc_oldh-1-y) > 0))  /* lower left to upper right */
	  h2 = min(newh, dc_oldh);
	else
	  h2 = max(newh,dc_oldh);

	dc_revvid(x1,y1,w1,h1);
	dc_revvid(x2,y2,w2,h2);

	dc_oldw = neww;			/* new one is now old...	*/
	dc_oldh = newh;
}

/************************************************************************/
/*									*/
/* dc_setbase()								*/
/*									*/
/*	set upper left corner of box					*/
/*									*/
/************************************************************************/
	VOID
dc_setbase(x,y)
	WORD	x;
	WORD	y;
{
	if ((x!=dc_xbase) || (y!=dc_ybase))
					/* get rid of old box		*/
	  dc_revvid(dc_xbase, dc_ybase, dc_oldw, dc_oldh);	
	dc_xbase = x;
	dc_ybase = y;
	dc_oldw = 1;
	dc_oldh = 1;
	dc_revvid(x,y,1,1);
}

/************************************************************************/
/*									*/
/* dc_hdlch()								*/
/*									*/
/*	handle keyboard character, specifically cursor keys		*/
/*									*/
/*	returns:	-1	keep going				*/
/*			0	start over				*/
/*			1	write box				*/
/*			2	quit					*/
/*									*/
/************************************************************************/

	WORD
dc_hdlch(ch)
	WORD	ch;
{
	switch (ch & 0x7fff)
	{
	  case LEFT:
	    dc_xcurs = max(0,dc_xcurs-1);
	    break;
	  case RIGHT:
	    dc_xcurs = min(79,dc_xcurs+1);
	    break;
	  case UP:
	    dc_ycurs = max(0,dc_ycurs-1);
	    break;
	  case DOWN:
	    dc_ycurs = min(24,dc_ycurs+1);
	    break;
	  case ENDKEY:
	    drag = TRUE;
	    dc_setbase(dc_xcurs,dc_ycurs);
	    return(-1);
	  case HOME:
	    if (!drag)				/* click? (vs. button up) */
	      return(0);
	    drag = FALSE;
	    return(mn_dispmn(menu2ptr,1,0));	/*write/quit/try again	*/
	  default:
	    return(-1);
	}
	ros_scur(dc_xcurs,dc_ycurs);
	if (drag)
	  dc_dragpos(dc_xcurs,dc_ycurs);
	return(-1);
}



/************************************************************************/
/*									*/
/* dc_init()								*/
/*									*/
/*	initialize LONG pointers used in DOS copy			*/
/*									*/
/************************************************************************/

	VOID
dc_init()
{
	WORD	i;

	for (i=0; i<2; i++)
	  menu1lng[i] = ADDR(menu1[i]);
	menu1lng[2] = 0l;
	for (i=0; i<3; i++)
	  menu2lng[i] = ADDR(menu2[i]);
	menu2lng[3] = 0l;

	menu1ptr = ADDR(&menu1lng[0]);
	menu2ptr = ADDR(&menu2lng[0]);
}

/************************************************************************/
/*									*/
/* dc_cut()								*/
/*									*/
/*	main routine for copying COPY from a DOS app			*/
/*									*/
/************************************************************************/

	VOID
dc_cut()
{
	WORD	file;
	WORD	savex,savey;
	WORD	savecurs;
	WORD	done;

	switch(mn_dispmn(menu1ptr,0,0))		/* keep going?		*/
	{
	  case 0:
	    break;
	  default:
	    return;
	}

	dc_xcurs = 0;
	dc_ycurs = 0;
	dc_xbase = 0;
	dc_ybase = 0;
	dc_oldw = 0;
	dc_oldh = 0;
	drag = FALSE;

#if 0
	savecurs = ros_gmod();			/* save cursor mode	*/
#endif
	savex = ros_gcur();			/* save app's cursor	*/
	savey = (savex & 0xff00) >> 8;		/* ah/al = row/column	*/
	savex &= 0xff;
	ros_scur(0,0);				/* we start upper left	*/
#if 0
	if (savecurs == 0x0607)			/* change cursor look	*/
	  ros_smod(0x0007);			/*  block cursor	*/
	else
	  ros_smod(0x0607);			/*  underscore cursor	*/
#endif
	done = FALSE;
	while (!done)				/* do interactive cut	*/
	{
	  switch (dc_hdlch(bio_kb(0)))
	  {
	    case -1:				/* still working	*/
	      break;
	    case 2:				/* time to quit...	*/
	      done = TRUE;			/* fall through to next	*/
	    case 0:				/* select new area	*/
	      if (dc_oldw)			/* box on screen?	*/
	      {
		dc_revvid(dc_xbase,dc_ybase,dc_oldw,dc_oldh);
		dc_oldw = dc_oldh = 0;		/* junk old box		*/
	      }
	      break;
	    case 1:				/* output it		*/
	      file = dos_create(ADDR("c:\\gemscrap\\scrap.txt"), 0);
	      dc_wrbox(file);
	      dos_close(file);
	      done = TRUE;
	      break;
	    default:				/* forget it		*/
	      done = TRUE;
	      break;
	  }
	}
	ros_scur(savex,savey);			/* restore cursor	*/
#if 0
	ros_smod(savecurs);
#endif
}


/************************************************************************/
/*									*/
/*									*/
/*									*/
/************************************************************************/
