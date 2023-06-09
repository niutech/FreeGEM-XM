/*	DESKBIND.H		12/13/84-06/09/85	Lee Lorenzen	*/
/*	for 3.0			4/9/86	- 5/5/86	MDF		*/


				/* leftovers from old deskdefn.h	*/


/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                    
*/

#if MULTIAPP
#define NUM_ADTREES 16
#endif

#define	THESCREEN 0
#define THEBAR 1
#define THEACTIVE 2


#define THEMENUS 7
							/* Desk Menu	*/
#define L1ITEM 10           
#define DSK1ITEM 11
#define DSK2ITEM 12          
#define DSK3ITEM 13
#define DSK4ITEM 14      
#define DSK5ITEM 15     
#define DSK6ITEM 16   
#define FROOT 0
#define FSTITLE 1
#define FDIRECTORY 2
#define FSELECTION 3
#define FCLSBOX 4
#define FTITLE 5
#define FILEBOX 6
#define F1NAME 7
#define F2NAME 8
#define F3NAME 9
#define F4NAME 10
#define F5NAME 11
#define F6NAME 12
#define F7NAME 13
#define F8NAME 14
#define F9NAME 15
#define SCRLBAR 16
#define FUPAROW 17
#define FDNAROW 18
#define FSVSLID 19
#define FSVELEV 20
#define OK 21
#define CANCEL 22

#ifndef ARROW
#define ARROW 0x0
#endif
#define HRGLASS 0x2

typedef struct moblk
{
	WORD		m_out;
	WORD		m_x;
	WORD		m_y;
	WORD		m_w;
	WORD		m_h;
} MOBLK ;

#if MULTIAPP
#define DESKPID 0
#define GEMPID 1
#endif


#define MAX_OBS 60
#define MAX_LEVEL 8

#define GLOBES struct glnode
GLOBES
{
/*GLOBAL*/ FNODE	g_flist[NUM_FNODES];
/*GLOBAL*/ FNODE	*g_favail;

/*GLOBAL*/ PNODE	g_plist[NUM_PNODES];
/*GLOBAL*/ PNODE	*g_pavail;
/*GLOBAL*/ PNODE	*g_phead;

/*GLOBAL*/ BYTE		g_wspec[67];
/*GLOBAL*/ LONG		a_wspec;
/*GLOBAL*/ BYTE		g_wdta[128];
/*GLOBAL*/ LONG		a_wdta;

/*GLOBAL*/ WNODE	g_wlist[NUM_WNODES];
/*GLOBAL*/ WORD		g_wcnt;

/* BugFix	*/
/* this has been moved into gl_icons[NUM_WOBS] & declared in DESKGLOB.C	*/
/*GLOBAL*/ /* ICONBLK	g_icons[NUM_WOBS];*/
/* */

/*GLOBAL*/ WORD		g_index[NUM_WOBS];

/*GLOBAL*/ USERBLK	g_udefs[NUM_WOBS];

						/* view related parms	*/
/*GLOBAL*/ WORD		g_num;			/* number of points 	*/
/*GLOBAL*/ WORD		*g_pxy;		/* outline pts to drag	*/
/*GLOBAL*/ WORD		g_iview;		/* current view type	*/
/*GLOBAL*/ WORD		g_iwext;		/* w,h of extent of a	*/
/*GLOBAL*/ WORD		g_ihext;		/*   single iten	*/
/*GLOBAL*/ WORD		g_iwint;		/* w,h of interval	*/
/*GLOBAL*/ WORD		g_ihint;		/*   between item	*/
/*GLOBAL*/ WORD		g_iwspc;		/* w,h of extent of a	*/
/*GLOBAL*/ WORD		g_ihspc;		/*   single iten	*/
/*GLOBAL*/ WORD		g_incol;		/* # of cols in full	*/
						/*   window		*/
/*GLOBAL*/ WORD		g_isort;		/* current sort type	*/

/*GLOBAL*/ BYTE		g_srcpth[82];
/*GLOBAL*/ BYTE		g_dstpth[82];
				 		/* data xfer buffer and	*/
						/*   length for copying	*/
/*GLOBAL*/ LONG		g_xbuf;
/*GLOBAL*/ UWORD	g_xlen;
						/* stack of fcb's to use*/
						/*   for non-recursive	*/
						/*   directory tree	*/
						/*   traversal		*/
/*GLOBAL*/ FCB		g_fcbstk[MAX_LEVEL];
/*GLOBAL*/ LONG		g_nfiles;
/*GLOBAL*/ LONG		g_ndirs;
/*GLOBAL*/ LONG		g_size;

/*GLOBAL*/ BYTE		g_tmppth[82];

/*GLOBAL*/ WORD		g_xyobpts[MAX_OBS * 2];


/*GLOBAL*/ WORD		g_rmsg[8];
/*GLOBAL*/ LONG		a_rmsg;


/*GLOBAL*/ WORD		g_xdesk;
/*GLOBAL*/ WORD		g_ydesk;
/*GLOBAL*/ WORD		g_wdesk;
/*GLOBAL*/ WORD		g_hdesk;

/*GLOBAL*/ WORD		g_xfull;
/*GLOBAL*/ WORD		g_yfull;
/*GLOBAL*/ WORD		g_wfull;
/*GLOBAL*/ WORD		g_hfull;

/*GLOBAL*/ BYTE		g_cmd[128];
/*GLOBAL*/ LONG		a_cmd;

/*GLOBAL*/ BYTE		g_tail[128];
/*GLOBAL*/ LONG		a_tail;

/*GLOBAL*/ BYTE		g_fcb1[36];
/*GLOBAL*/ LONG		a_fcb1;

/*GLOBAL*/ BYTE		g_fcb2[36];
/*GLOBAL*/ LONG		a_fcb2;
	
/*GLOBAL*/ LONG		a_alert;

/*GLOBAL*/ LONG		a_trees[NUM_ADTREES];

/*GLOBAL*/ WORD		g_croot;		/* current pseudo root	*/

/*GLOBAL*/ WORD		g_cwin;			/* current window #	*/
/*GLOBAL*/ WORD		g_wlastsel;		/* window holding last	*/
						/*   selection		*/
/*GLOBAL*/ WORD		g_csortitem;		/* curr. sort item chked*/
/*GLOBAL*/ WORD		g_ccopypref;		/* curr. copy pref.	*/
/*GLOBAL*/ WORD		g_cdelepref;		/* curr. delete pref.	*/
/*GLOBAL*/ WORD		g_cdclkpref;		/* curr. double click	*/
/*GLOBAL*/ WORD		g_ctimeform;		/* curr. time format	*/
/*GLOBAL*/ WORD		g_cdateform;		/* curr. date format	*/

/*GLOBAL*/ BYTE		g_1text[256];
/*GLOBAL*/ BYTE		g_2text[256];

/*GLOBAL*/ WORD		g_icw;
/*GLOBAL*/ WORD		g_ich;
/*GLOBAL*/ WORD		g_nmicon;
/*GLOBAL*/ WORD		g_nmtext;
/*GLOBAL*/ WORD		g_xyicon[18];
/*GLOBAL*/ WORD		g_xytext[18];

/*GLOBAL*/ WORD		g_wicon;
/*GLOBAL*/ WORD		g_hicon;

/*GLOBAL*/ WORD		g_afsize;
/*GLOBAL*/ BYTE		*g_pbuff;
/*GLOBAL*/ ANODE	g_alist[NUM_ANODES];

/*GLOBAL*/ ANODE	*g_aavail;
/*GLOBAL*/ ANODE	*g_ahead;
/*GLOBAL*/ ICONBLK	g_idlist[NUM_IBLKS];
/*GLOBAL*/ ICONBLK	g_iblist[NUM_IBLKS];
/*GLOBAL*/ WORD		g_ismask[NUM_IBLKS*2];


/*GLOBAL*/ CSAVE	g_cnxsave;
/*GLOBAL*/ LONG		a_datastart;
/*GLOBAL*/ LONG		a_buffstart;

/*GLOBAL*/ LONG		a_screen;
/*GLOBAL*/ OBJECT	g_screen[NUM_SOBS];		/* NUM_SOBS	*/
};
