/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*              Historical Copyright                             
*/

/*	DESKINFO.H	07/18/84 - 09/12/84	Lee Lorenzen		*/
/*
012345678901234567890123456789012345678901234567
+----------------------------------------------+
|                DESKTOP 1.0                   |
|       			               |
|                  AUTHORS	               |
|  XXXX        ----------------      [  OK  ]  |
|  XXXX        Lee Jay Lorenzen	               |
|  XXXX          Gregg Morris                  |
|  XXXX         Lowell Webster                 |
|					       |
|  Copyright (c) 1984, Digital Research, Inc.  |
+----------------------------------------------+
*/

#define NUM_DES 10
#define NUM_DETEDS 8
#define NUM_DESTRS 8

OBJECT		de_objs[NUM_DES] = 
{
	-1, 1, 9, G_BOX, NONE, OUTLINED, 0x00020007L, 0, 0, 48, 11,
	2, -1, -1, G_IMAGE, NONE, NORMAL, 0x0L, 3, 4, 4, 4,
	3, -1, -1, G_TEXT, NONE, NORMAL, 0x0L, 17, 1, 11, 1,
	4, -1, -1, G_TEXT, NONE, NORMAL, 0x1L, 19, 3, 7, 1,
	5, -1, -1, G_TEXT, NONE, NORMAL, 0x2L, 15, 4, 16, 1,
	6, -1, -1, G_TEXT, NONE, NORMAL, 0x3L, 15, 5, 16, 1,
	7, -1, -1, G_TEXT, NONE, NORMAL, 0x4L, 17, 6, 12, 1,
	8, -1, -1, G_TEXT, NONE, NORMAL, 0x5L, 16, 7, 14, 1,
	9, -1, -1, G_TEXT, NONE, NORMAL, 0x6L, 3, 9, 42, 1,
	0, -1, -1, G_BOXTEXT, 0x27, NORMAL, 0x7L, 37, 4, 8, 1
};


TEDINFO		de_teds[NUM_DETEDS] =
{
	0x0L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x1L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x2L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x3L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x4L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x5L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x6L, -1L, -1L, IBM, S_XOR_D, TE_LEFT, BLACK, WHITE, 0, 0, 0,
	0x7L, -1L, -1L, IBM, S_XOR_D, TE_CNTR, BLACK, WHITE, -3, 0, 0
};


BYTE		*de_strs[NUM_DESTR] =
{
	"DESKTOP 1.0",
	"AUTHORS",
	"----------------",	
	"Lee Jay Lorenzen",
	"Gregg Morris",
	"Lowell Webster",
	"Copyright (c) 1984, Digital Research, Inc.",
	"OK"
};


BITBLK		de_image[0] =
{
	0x0L, 4, 32, 0, 0, S_XOR_D
};


UWORD logoicon[64] =
{ 0x0000, 0x0000, 0xFF3F, 0xF8FF, 
  0xFF7F, 0xFCFF, 0x00E0, 0x0E00, 
  0x00C0, 0x0600, 0x00C0, 0x0600, 
  0xF8C3, 0x863F, 0xFCC7, 0xC67F, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC660, 0xFCC7, 0xC660, 
  0xFCC7, 0xC67F, 0xF8C3, 0x863F, 
  0x00C0, 0x0600, 0x00C0, 0x0600, 
  0x00E0, 0x0E00, 0xFF7F, 0xFCFF, 
  0xFF3F, 0xF8FF, 0x0000, 0x0000
};