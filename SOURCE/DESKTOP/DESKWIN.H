/*	DESKWIN.H	06/11/84 - 01/04/85		Lee Lorenzen	*/

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                          
*/

#define DROOT 1
#define NUM_WNODES 2
#define NUM_WOBS 128
#define NUM_SOBS (NUM_WOBS + NUM_WNODES + 1)

#define WNODE struct windnode

WNODE
{
	WNODE		*w_next;
	WORD		w_flags;
	WORD		w_id;			/* window handle id #	*/
	WORD		w_obid;			/* desktop object id	*/
	WORD		w_root;			/* pseudo root ob. in	*/
						/*   gl_screen for this	*/
						/*   windows objects	*/
	WORD		w_cvcol;		/* current virt. col	*/
	WORD		w_cvrow;		/* current virt. row	*/
	WORD		w_pncol;		/* physical # of cols	*/
	WORD		w_pnrow;		/* physical # of rows	*/
	WORD		w_vncol;		/* virtual # of cols 	*/
	WORD		w_vnrow;		/* virtual # of rows	*/
	PNODE		*w_path;
	BYTE		w_name[67];
/*	BYTE		w_info[81];		NOT USED v2.0		*/
};


