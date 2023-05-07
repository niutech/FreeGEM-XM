/*	INFODEF.H	09/26/84 - 09/26/84		Gregg Morris	*/


/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*/


#define FCB struct fcb

FCB
{
	BYTE		fcb_reserved[21];
	BYTE		fcb_attr;
	WORD		fcb_time;
	WORD		fcb_date;
	LONG		fcb_size;
	BYTE		fcb_name[13];
};

#define ARROW 0
#define HGLASS 2

#define FLOPPY 0
#define HARD 1
