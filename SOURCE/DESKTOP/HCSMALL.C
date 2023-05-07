/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                          
*/


/* routine to fool High-C's need for SMALL library	10/24/85	*/
	void
small()
{
}
pragma alias(small, "SMALL?");
