/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                     
*/
EXTERN WORD	v_opnwk();
EXTERN WORD	v_clswk();
EXTERN WORD	v_clrwk();
EXTERN WORD	v_clswk();
EXTERN WORD	vq_chcells();
EXTERN WORD	v_exit_cur();
EXTERN WORD	v_enter_cur();
EXTERN WORD	v_curup();
EXTERN WORD	v_curdown();
EXTERN WORD	v_curright();
EXTERN WORD	v_curleft();
EXTERN WORD	v_curhome();
EXTERN WORD	v_eeos();
EXTERN WORD	v_eeol();
EXTERN WORD	vs_curaddress();
EXTERN WORD	v_curtext();
EXTERN WORD	v_rvon();
EXTERN WORD	v_rvoff();
EXTERN WORD	vq_curaddress();
EXTERN WORD	vq_tabstatus();
EXTERN WORD	v_hardcopy ();
EXTERN WORD	v_dspcur();
EXTERN WORD	v_rmcur ();
EXTERN VOID	v_pline();
EXTERN WORD	v_pmarker();
EXTERN WORD	v_gtext();
EXTERN WORD	v_fillarea();

EXTERN WORD	v_bar();
EXTERN WORD	v_circle();
EXTERN WORD	v_arc();
EXTERN WORD	v_pieslice();
EXTERN WORD	v_ellipse();
EXTERN WORD	v_ellarc();
EXTERN WORD	v_ellpie();

EXTERN VOID	vst_height();
EXTERN WORD	vst_rotation();
EXTERN WORD	vs_color();
EXTERN WORD	vsl_type();
EXTERN VOID	vsl_width();
EXTERN WORD	vsl_color();
EXTERN WORD	vsm_type();
EXTERN WORD	vsm_height();
EXTERN WORD	vsm_color();
EXTERN WORD	vst_font();
EXTERN WORD	vst_color();
EXTERN WORD	vsf_interior();
EXTERN WORD	vsf_style();
EXTERN WORD	vsf_color();
EXTERN WORD	vq_color();

EXTERN WORD	vrq_locator();
EXTERN WORD	vsm_locator();
EXTERN WORD	vrq_valuator();
EXTERN WORD	vsm_valuator();
EXTERN WORD	vrq_choice();
EXTERN WORD	vsm_choice();
EXTERN WORD	vrq_string();
EXTERN WORD	vsm_string();
EXTERN WORD	vswr_mode();
EXTERN WORD	vsin_mode();

EXTERN WORD	vsf_perimeter();

EXTERN WORD	vr_cpyfm();
EXTERN WORD	vr_trnfm();
EXTERN WORD	vsc_form();
EXTERN WORD	vsf_udpat();
EXTERN WORD	vsl_udsty();
EXTERN VOID	vr_recfl();
EXTERN WORD	v_show_c();
EXTERN WORD	v_hide_c();
EXTERN WORD	vq_mouse();
EXTERN WORD	vex_butv();
EXTERN WORD	vex_motv();
EXTERN WORD	vex_curv();
EXTERN WORD	vq_key_s();			
