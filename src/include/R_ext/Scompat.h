/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1999  The R Development Core Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* is this a good idea? - not the same as in S.h in S-PLUS */
void	call_R(char*, long, void**, char**, long*, char**, long, char**);
#define call_S call_R

typedef struct {
	double re;
	double im;
} S_complex;

#define complex S_complex
