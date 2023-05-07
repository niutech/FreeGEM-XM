/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                                                       
*                  Historical Copyright  
*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 3.0
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1986			Digital Research Inc.
*	-------------------------------------------------------------
*/


/* GEMDBG.C	from greg rossi	*/

#include <portab.h>
#include <machine.h>

#define  MAXSTR    140

EXTERN WORD	bio_kb();
EXTERN WORD	dos_write();


#if HIGH_C
VOID	dbg_putc();
VOID	dbg();
VOID	_dprintf();
VOID	_prt10();
VOID	_prtl10();
VOID	_prt16();
VOID	_prtl16();
#endif


GLOBAL	WORD	gl_int = 0;


GLOBAL  WORD	gl_dbgprt = 0;


EXTERN	WORD	DOS_AX;
EXTERN	WORD	DOS_BX;
EXTERN	WORD	DOS_CX;



	WORD
dos_write(handle, cnt, pbuffer)
	WORD		handle;
	WORD		cnt;
	LONG		pbuffer;
{
	DOS_CX = cnt;
	DOS_BX = handle;
	dos_func(0x4000, pbuffer);
	return(DOS_AX);
}





#if 1
dbg_setport()
{
	switch(bio_kb(0) & 0x00ff)
	{
	    case '0': gl_dbgprt = -1; break;
	    case '1': gl_dbgprt = 0; break;
	    case '2': gl_dbgprt = 1; break;
	    default : gl_dbgprt = 2;			
	} 
}
#endif

dbg_wait()
{
  dbg("PRESS ANY KEY TO CONTINUE....\r\n");
  bio_kb(0);
}



#if 0
	WORD
dbg_chkint()
{
	WORD	old;

	dbg_putc(0);	
	old = gl_int;
	gl_int = 0;
	return(old);
}
#endif

#if 1
	BYTE
dbg_chpoll()
{
	BYTE  temp;

	temp = 0;
	
	bio_com(gl_dbgprt, 0x0100);

	if (bio_com(gl_dbgprt, 0x0300) & 0x0100)
	    temp = bio_com(gl_dbgprt, 0x0200);
	return(temp);
}
#endif


	VOID
dbg_putc(ch) BYTE ch;		/* output character to the commo port */
{

	BYTE temp;

	if (gl_dbgprt == -1)		/* output to stdout		*/
	{
	  dos_write(1,1,ADDR(&ch));
	  return;
	}

	temp = 0;

	temp = dbg_chpoll();
	if (temp == 's')			   /* if the char = 's'	   */
	    while (temp != 'q')			   /* look for 'q'	   */
	    {
		temp = dbg_chpoll();
		if (temp == 'i')
		    gl_int = 1;		/* interrupt key pressed */
	    }
	if (temp == 'i')
	    gl_int = 1;			/* interrupt key pressed */

	bio_com(gl_dbgprt,0x0100 + ch);		   /* send the input char  */
}

static	BYTE	outbuf[ MAXSTR ];
static  WORD	idx;	

/****************************************************************/
	VOID
dbg( fmt, args)
BYTE   *fmt;
WORD   args;
{
   static BYTE *p;
   int i;

    if (gl_dbgprt==2)
return;

   for(idx=0;idx<MAXSTR;idx++) outbuf[idx] = '\0';
   _dprintf( outbuf, fmt, &args);

   for (p=outbuf; *p!='\0'; p++)
   {
     for (i=1;i<250;i++);	/* time delay for AT */
     dbg_putc(*p);
   }
/*
   dos_write(      1, dbglen(outbuf), ADDR(outbuf));	
   dos_write(dbghndl, dbglen(outbuf), ADDR(outbuf));	
*/
}



	BYTE *
dbgcpy( to, from )
	BYTE  *to,*from;
{
   while ((*to++ = *from++) != 0);
   return(from);	
}

/****************************************************************/
	VOID
sprintf( buf, fmt, args)
BYTE   *buf;
BYTE   *fmt;
WORD   args;
{
   for(idx=0;idx<MAXSTR;idx++) outbuf[idx] = '\0';
   _dprintf(outbuf, fmt, &args);
   dbgcpy( buf, outbuf );
}

/****************************************************************/
static	VOID
_dprintf( outbuf, fmt, args )
BYTE   outbuf[];
BYTE   *fmt;
WORD   *args;
{
static	WORD   c;
static	WORD   i;
static	WORD   f;
static	BYTE   *str;
static	BYTE   string[20];
static	WORD   length;
static	BYTE   fill;
static	WORD   leftjust;
static	WORD   longflag;
static	WORD   fmax,fmin;
static	WORD   leading;
static	BYTE   sign;
static	BYTE   digit1;

idx = 0;
for(;;) {
   while( (c = *fmt++) != '%') {   /* Echo chars till % or end */
      if (c == '\0') return;
      outbuf[idx++] = c;
   }
   if (*fmt == '%' ) {             /* '%%' */
      outbuf[idx++] = *fmt++;
      continue;
   }
   if (leftjust = ((*fmt=='-') ? 1 : 0 )  ) fmt++;
   fill = (*fmt=='0') ? *fmt++ : ' ';
   fmin = 0;
   if (*fmt == '*') {
      fmin = *args++;
      ++fmt;
   } else
      while ('0' <= *fmt && *fmt <= '9') {
         fmin = fmin * 10 + *fmt++ - '0';
      }
   fmax = 0;
   if (*fmt == '.') {
      if ( *(++fmt) == '*') {
         fmax = *args++;
         ++fmt;
      } else
         while ('0' <= *fmt && *fmt <= '9') {
            fmax = fmax * 10 + *fmt++ - '0';
         }
      }

   if (longflag = (( *fmt == 'l') ? 1 : 0) ) fmt++;
   str = string;
   if ( (f = *fmt++) == '\0') {
      outbuf[idx++] = '%';
      return;
   }
   sign = '\0';

   switch ( f ) {
      case 'c' :
                string[0] = (BYTE)*args;
                string[1] = '\0';
                fmax      = 0;
                fill      = ' ';
                break;
      case 's':
                str  = (BYTE *)*args;
                fill = ' ';
                break;
      case 'D':
                longflag = 1;
      case 'd':
                if (longflag) {
                   if (*(long *)args < 0) {
                      sign = '-';
                      *(long *)args = -*(long *)args;
                   }
                } else {
                   if ( *args < 0) {
                      sign = '-';
                      *args = -*args;
                   }
                }
                longflag--;

      case 'U':
                longflag++;
      case 'u':
                if ( longflag ) {
                    digit1 = '\0';
                    while (*(long *)args < 0) {
                       *(long *)args -= 1000000000L;
                       ++digit1;
                    }
                    _prtl10(*(long *)args, str);
                    str[0] += digit1;
                    ++args;
                 } else
                    _prt10( *args, str);
                 fmax = 0;
                 break;
       case 'X':
                 longflag++;
       case 'x':
                 if (longflag) {
                     _prtl16(*(long *)args, str);
                     ++args;
                 } else
                     _prt16(*args, str);
                 fmax = 0;
                 break;
        default:
                  outbuf[idx++] = f;
                  break;
     }
     args++;
     for (length=0; str[length] != '\0'; length++) ;
     if (fmin >MAXSTR || fmin < 0) fmin = 0;
     if (fmax >MAXSTR || fmax < 0) fmax = 0;
     leading = 0;
     if (fmax != 0 || fmin != 0) {
        if (fmax != 0)
            if (length > fmax) length = fmax;
        if (fmin != 0) leading = fmin - length;
        if (sign == '-')  --leading;
     }
     if (sign == '-'  &&  fill == '0' )   outbuf[idx++] = sign;
     if (leftjust == 0)
        for (i=0;i<leading;i++) outbuf[idx++] = fill;
     if (sign == '-'  &&  fill == ' ' )  outbuf[idx++] = sign;
     for (i=0; i<length;i++)  outbuf[idx++] =  str[i];
     if (leftjust != 0)
        for(i=0;i<leading;i++)  outbuf[idx++] = fill;
   }
}

VOID
_prt10(num,str)
UWORD  num;
BYTE   *str;
{
WORD  i;
BYTE  c,temp[8];

   temp[0]  = '\0';
   for(i=1;i<=5;i++) {
      temp[i] = num % 10 + '0';
      num = num / 10;
   }
   for(i=5;temp[i] == '0'; i--);
   if (i == 0) i++;
   while (i >= 0) *str++ = temp[i--];
}


#if HIGH_C

	WORD
_dwreml(num, div)
	LONG	num;
	WORD	div;
{
	return(num - (div * (num / div)));

}

#endif	



VOID
_prtl10(num,str)
LONG   num;
BYTE   *str;
{
WORD   i;
BYTE   c,temp[11];

   temp[0] = '\0';
   for(i=1;i<=10;i++) {
#if HIGH_C
      temp[i] = _dwreml(num,10) + '0';
#else
      temp[i]  = num % 10 + '0';
#endif
      num = num / 10;
   }
   for(i=10;temp[i] == '0'; i--);
   if (i == 0) i++;
   while (i >= 0) *str++ = temp[i--];
}

VOID
_prt16(num,str)
UWORD   num;
BYTE    *str;
{
WORD  i;
BYTE  c,temp[5];

   temp[0]  = '\0';
   for(i=1;i<=4;i++) {
      temp[i] = "0123456789abcdef"[num & 0x0f];
      num = num >> 4;
   }
   for(i=4;temp[i] == '0'; i--);
   if (i == 0) i++;
   while ( i >= 0 ) *str++ = temp[i--];
}

VOID
_prtl16(num,str)
LONG  num;
BYTE  *str;
{
WORD  i;
BYTE  c,temp[9];

   temp[0] = '\0';
   for(i=1;i<=8;i++) {
      temp[i] = "0123456789abcdef"[num & 0x0f];
      num = num >> 4;
   }
   for(i=8;temp[i] == '0';i--);
   if (i == 0) i++;
   while(i >= 0) *str++ = temp[i--];
}

#if 0
dump(addr,len)
	LONG	addr;
	WORD	len;
{
	LONG	p;
	WORD	i, j;
	BYTE	c;
	BYTE	st[17];

	for (p=addr; p<addr+len; p+=2) 
	{
	    if (((p-addr)&0xF)==0x0)
	    {
		dbg("\r\n%lx - ", p);
		i = 0;
	    }
	    dbg("%04x ", LWGET(p));
	    for (j=0; j<=1; j++)
	    {
	      c = LBGET(p + j);
	      if ((c < 32) || (c > 126))
		c = '.';
	      st[i+j] = c;
	    }
	    i += 2;
	    if (i > 15)
	    {
	      i = 0;
	      st[16] = 0;
	      dbg("      %s", &st[0]);
	    }
	}
	dbg("\r\n");
}
#endif


#if 0
/*	WORD
dbglen( p )
	BYTE  *p;
{
	WORD  ilen;
  ilen = 0;
  while(*p++) ilen++;
  return( ilen );
}
*/
#endif
