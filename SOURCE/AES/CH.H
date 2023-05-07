/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                                                       
*                  Historical Copyright  

*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 2.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1986			Digital Research Inc.
*	-------------------------------------------------------------
*/

/* DOS memory allocation control block offsets ( ARENA ) */

#define FREE		0
#define	ALLOCATED	1
#define MAXCHNLS	12
#define READ		0
#define WRITE		1
#define OUTANDIN	0
#define OUTONLY		1
#define FAILURE		-1
#define OKGEM		0
#define TERMINATED	1
#define STOPPED		2

#if I8086			/* all must be increments of paragraphs */
#define SIZEARENA	0x00010000L 	/* 16 bytes - the size of an arena */
#define OF_FREE		0x00040000L	/* 64 - free space beyond data */
#endif

#if MC68K0
#define SIZEARENA	16
#endif

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
