// Copyright (C) 2007 Andrey Gruber (aka lamer)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <windows.h>
#include <shlwapi.h>

static int calpha(int arg);
static int cblue(int arg);
static int cgreen(int arg);
static int cred(int arg);
static int ccolorhb(int arg);

/*-@@+@@------------------------------------------------------------------
 Procedure: Fill2ColorsRectangle
 Created  : Sun Sep  9 13:31:07 2007
 Modified : Sun Sep  9 13:31:07 2007

 Synopsys : Fills specified rectangle with 2-color gradient
 Input    : hdc - device context
            lprc - rectangle for filling
            clr1 - the first color
            clr2 - the second color
            mode - gradient mode
 Output   : Nothing
 Errors   : 
 ------------------------------------------------------------------@@-@@-*/
void Fill2ColorsRectangle(HDC hdc, LPRECT lprc, COLORREF clr1, COLORREF clr2, ULONG mode){
	GRADIENT_RECT		grc;
	TRIVERTEX			tvx[2];

	//set first trivertex
	tvx[0].x = lprc->left;
	tvx[0].y = lprc->top;
	//set first trivertex colors
	tvx[0].Red = cred(clr1);
	tvx[0].Green = cgreen(clr1);
	tvx[0].Blue = cblue(clr1);
	tvx[0].Alpha = calpha(clr1);
	//set second trivertex
	tvx[1].x = lprc->right;
	tvx[1].y = lprc->bottom;
	//set second trivertex colors
	tvx[1].Red = cred(clr2);
	tvx[1].Green = cgreen(clr2);
	tvx[1].Blue = cblue(clr2);
	tvx[1].Alpha = calpha(clr2);
	//set graphic rectangle
	grc.UpperLeft = 0;
	grc.LowerRight = 1;
	//fill with gradient
	GradientFill(hdc, tvx, 2, &grc, 1, mode);
}

static int calpha(int arg){
	__asm{
		xor edx,edx
		mov ecx,1000000h
		mov eax,arg
		div ecx
		push eax
		call ccolorhb
	}
}

static int cblue(int arg){
	__asm{
		mov eax,arg
		shr eax,16
		push eax
		call ccolorhb
	}
}

static int cgreen(int arg){
	__asm{
		mov eax,arg
		shr eax,8
		push eax
		call ccolorhb
	}
}

static int cred(int arg){
	__asm{
		push arg
		call ccolorhb
	}
}

static int ccolorhb(int arg){
	__asm{
		mov eax,arg
		and eax,0xff
		mov ecx,eax
		and eax,7Fh
		shl eax,8
		and ecx,80h
		jz _exit
		xor eax,0x7fff
		not eax
		_exit:
	}
}


