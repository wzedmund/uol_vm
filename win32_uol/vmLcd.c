#include "vm.h"
#include "font.h"
#include <windows.h>

extern HWND Global_hwnd;

#define RGB565_MASK_RED        0xF800  
#define RGB565_MASK_GREEN      0x07E0  
#define RGB565_MASK_BLUE       0x001F 

#define LCD_START_X	0
#define LCD_START_Y	0

DWORD lcd_buf[240*320];
int TouchX;
int TouchY;
char isTouch;


void rgb565Torgb24(BYTE *rgb24, DWORD rgb565)  
{   
 //extract RGB  
 rgb24[2] = (rgb565 & RGB565_MASK_RED) >> 11 <<3;     
 rgb24[1] = (rgb565 & RGB565_MASK_GREEN) >> 5 << 2;  
 rgb24[0] = (rgb565 & RGB565_MASK_BLUE) << 3;  
}   

//void onOff(bool b)
CELL graphic_Graphic_onOff(MicroVM* vm, CELL* params)
{
  	int onOff = params[0].ival;
	CELL res;	
  	return res;
}

//void clear(int color)
CELL graphic_Graphic_clear(MicroVM* vm, CELL* params)
{
  	int color = params[0].ival;
	int i;
	CELL res;	
	HDC hdc;
	RECT rect;
	HBRUSH hBrush;
	BYTE rgbBuf[3];
	DWORD color24;
	rgb565Torgb24(rgbBuf,color);
	color24 = RGB (rgbBuf[2],rgbBuf[1],rgbBuf[0]);
	hdc = GetDC (Global_hwnd) ;
	SetRect (&rect,0,0,239, 219) ;
    hBrush = CreateSolidBrush (color24) ;
	FillRect (hdc, &rect, hBrush) ;
	for(i=0;i<240*320;i++)
	{
		lcd_buf[i] = color24;
	}
  	return res;
}

//int getWidth()
CELL graphic_Graphic_getWidth(MicroVM* vm, CELL* params)
{
	CELL res;	
	res.ival = 240;
  	return res;
}

//int getHeight()
CELL graphic_Graphic_getHeight(MicroVM* vm, CELL* params)
{
	CELL res;	
	res.ival = 320;
  	return res;
}

//int getPoint(int x, int y)
CELL graphic_Graphic_getPoint(MicroVM* vm, CELL* params)
{
  	int x = params[0].ival;
	int y = params[1].ival;
	CELL res;	
	HDC hdc;
	hdc = GetDC (Global_hwnd) ;
	res.ival = GetPixel (hdc, x, y) ;
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

//void setPoint(int x, int y, int color)
CELL graphic_Graphic_setPoint(MicroVM* vm, CELL* params)
{
  	int x = params[0].ival;
	int y = params[1].ival;
	int c = params[2].ival;
	CELL res;
	HDC hdc;
	BYTE rgbBuf[3];
	DWORD color24;
	rgb565Torgb24(rgbBuf,c);
	color24 = RGB (rgbBuf[2],rgbBuf[1],rgbBuf[0]);
	hdc = GetDC (Global_hwnd) ;
	lcd_buf[x+y*240] = color24;
	SetPixel (hdc, x, y, color24) ;
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

//void drawChar(int x, int y, int c, int font, int color)
CELL graphic_Graphic_drawChar(MicroVM* vm, CELL* params)
{
  	int x = params[0].ival;
	int y = params[1].ival;
	unsigned char c = (unsigned char)params[2].ival;
	int f = params[3].ival;
	int color = params[4].ival;
	int pos,size = f;
	char temp,t;
	CELL res;
	HDC hdc;
	BYTE rgbBuf[3];
	DWORD color24;
	rgb565Torgb24(rgbBuf,color);
	color24 = RGB (rgbBuf[2],rgbBuf[1],rgbBuf[0]);

	hdc = GetDC (Global_hwnd) ;	
	c=c-' ';
	for(pos=0;pos<size;pos++)
	{
		if(size==12)temp=asc2_1206[c][pos];//调用1206字体
		else temp=asc2_1608[c][pos];		 //调用1608字体
		for(t=0;t<size/2;t++)
	    {                 
	       if(temp&0x01) 
		   {
				lcd_buf[x+t+(y+pos)*240] = color24;
				SetPixel (hdc, x+t, y+pos, color24) ;
		   }
	       temp>>=1; 
	    }
	}
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

//void fill(int x1, int y1, int x2, int y2, int color)
CELL graphic_Graphic_fill(MicroVM* vm, CELL* params)
{
	int x1 = params[0].ival;
	int y1 = params[1].ival;
	int x2 = params[2].ival;
	int y2 = params[3].ival;
	int c = params[4].ival;
	CELL res;	
	HDC hdc;
	RECT rect;
	HBRUSH hBrush;
	BYTE rgbBuf[3];
	int countX,countY;
	DWORD color24;
	rgb565Torgb24(rgbBuf,c);
	color24 = RGB (rgbBuf[2],rgbBuf[1],rgbBuf[0]);
	hdc = GetDC (Global_hwnd) ;	
	
	for(countX = x1;countX<x2;countX++)
	{
		for(countY = y1;countY<y2;countY++)
		{
			SetPixel (hdc, countX, countY, color24) ;
			lcd_buf[countX+countY*240] = color24;
		}
	}
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

//int isTouch()
CELL graphic_TouchDispatcher_isTouch(MicroVM* vm, CELL* params)
{
  	CELL res;
	if(isTouch == 0)
		res.ival = 0;
	else
		res.ival = 1;
	return res;
}

//int touchX()
CELL graphic_TouchDispatcher_touchX(MicroVM* vm, CELL* params)
{
  	CELL res;
	res.ival = TouchX;
	return res;
}

//int touchY()
CELL graphic_TouchDispatcher_touchY(MicroVM* vm, CELL* params)
{
  	CELL res;
	res.ival = TouchY;
	return res;
}
