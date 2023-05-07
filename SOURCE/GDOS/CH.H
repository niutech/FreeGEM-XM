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

#define DBG		0
#define FREE		0
#define	ALLOCATED	1
#define MAXCHNLS	12
#define READ		0
#define WRITE		1
#define OUTANDIN	0
#define OUTONLY		1

#define NUM_DESKACC	17
#define SIZEDESKSTR	22


#if I8086			/* all must be increments of paragraphs */
#define SIZEARENA	0x00010000L 	/* 16 bytes - the size of an arena */
#define OF_FREE		0x00100000L	/* 256 - free space beyond data */
#endif

#if MC68K0
#define SIZEARENA	16
#endif

#define INTSIZE		1024

/*
	ARENA
	0 byte	status	'M'	beginning or middle in chain
			'Z'	last one in chain
	1 word	segment of owner memory block
		  if 0 then this block is free
	3 word	# of paragraphs
*/

/* dos arena offsets */

#define OF_PIC		0	/* BYTE */
#define OF_OWNER	1	/* WORD */
#define OF_ALOCSIZE	3	/* WORD */

/* fragment control block offsets */		 
						
#define	OF_FRAGSIZE	0 	/* LONG */	
#define	OF_CHNPID	4	/* WORD */	
			
/* system information offsets */
							
#define	OF_A_OF		16	/* LONG */
#define OF_PSP		20	/* WORD */ 
#define OF_DTA		24	/* LONG */
#define	OF_CURDSK	28	/* WORD */
#define OF_CPOS		32	/* WORD */
#define OF_SCRNSIZE	36	/* WORD */
#define OF_S_OF		40	/* LONG */
#define OF_INTSIZE	44	/* WORD */
#define OF_I_OF		48	/* LONG */	
#define OF_SS		52	/* WORD */	
#define OF_SP		56	/* WORD */	
#define OF_FT		58	/* 40 bytes - check sizeof(FILTABLE) */    
#define OF_CDIR		98	/* 67 byte string */   	

#if MC68K0
#define sw_segoff(l)	l
#define sw_long(l)	l
#endif

/* intin array parameter offsets */	

EXTERN LONG	dos_alloc();
EXTERN LONG	dos_avail();
EXTERN LONG	dos_gdta();
EXTERN UWORD	dos_read();
EXTERN UWORD	dos_write();
EXTERN UWORD	DOS_ERR;
EXTERN LONG	ad_contr;
EXTERN LONG	ad_intin;
EXTERN LONG	ad_ptsin;
EXTERN LONG	ad_intou;
EXTERN LONG	ad_ptsou;

/* keyboard scan codes */

#define ENDKEY 0x4F00				/* end key		*/
#define BACKSPACE 0x0E08			/* backspace		*/
#define SPACE 0x3920				/* ASCII <space>	*/
#define ESCAPE 0x011B				/* up arrow		*/
#define HOME 0x4700				/* up arrow		*/
#define UP 0x4800				/* up arrow		*/
#define DOWN 0x5000				/* down arrow		*/
#define LEFT 0x4B00				/* left arrow		*/
#define RIGHT 0x4D00				/* right arrow		*/
#define DELETE 0x5300				/* keypad delete	*/
#define TAB 0x0F09				/* tab			*/
#define BACKTAB 0x0F00				/* backtab		*/
#define RETURN 0x1C0D				/* carriage return	*/
