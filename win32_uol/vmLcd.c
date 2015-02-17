#include "vm.h"
#include "font.h"
#include <windows.h>
int lcd_x1,lcd_y1,lcd_x2,lcd_y2;
extern HWND Global_hwnd;

#define RGB565_MASK_RED        				0xF800  
#define RGB565_MASK_GREEN                          0x07E0  
#define RGB565_MASK_BLUE                         	0x001F 

#define LCD_START_X	0
#define LCD_START_Y	0

DWORD lcd_buf[241*321];
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

CELL doLcdInit(MicroVM* vm, CELL* params)
{
  	CELL res;	
	int i;
	for(i=0;i<240*320;i++)
	{
		lcd_buf[i] = 0xffffff;
	}
	TouchY = 0;
	TouchX = 0;
	isTouch = 0;
  	return res;
}

CELL doLcdOnOff(MicroVM* vm, CELL* params)
{
  	int onOff = params[0].ival;
	CELL res;	
  	return res;
}

CELL doLcdClear(MicroVM* vm, CELL* params)
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
	SetRect (&rect, 0, 0,241, 321) ;
       hBrush = CreateSolidBrush (color24) ;
	FillRect (hdc, &rect, hBrush) ;
	for(i=0;i<240*320;i++)
	{
		lcd_buf[i] = color24;
	}
  	return res;
}

CELL doLcdGetWidth(MicroVM* vm, CELL* params)
{
	CELL res;	
	res.ival = 240;
  	return res;
}

CELL doLcdGetHeight(MicroVM* vm, CELL* params)
{
	CELL res;	
	res.ival = 320;
  	return res;
}

CELL doLcdGetPoint(MicroVM* vm, CELL* params)
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

CELL doLcdSetPoint(MicroVM* vm, CELL* params)
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
	
	if((x>=lcd_x1&&x<lcd_x2)&&(y>=lcd_y1&&y<lcd_y2))
	{
		lcd_buf[x+y*240] = color24;
		SetPixel (hdc, x, y, color24) ;
	}
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

CELL doLcdDrawChar(MicroVM* vm, CELL* params)
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
			
			if((x+t>=lcd_x1&&x+t<lcd_x2)&&(y+pos>=lcd_y1&&y+pos<lcd_y2))
			{
				lcd_buf[x+t+(y+pos)*240] = color24;
				SetPixel (hdc, x+t, y+pos, color24) ;
			}
		   }
	          temp>>=1; 
	      }
	}
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

CELL doLcdFill(MicroVM* vm, CELL* params)
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
		if(countX>=lcd_x1&&countX<lcd_x2)
		{
			for(countY = y1;countY<y2;countY++)
			{
				if(countY>=lcd_y1&&countY<lcd_y2)
				{
					SetPixel (hdc, countX, countY, color24) ;
					lcd_buf[countX+countY*240] = color24;
				}
			}
		}
	}
	ReleaseDC (Global_hwnd,hdc) ;
  	return res;
}

CELL doLcdSetRectArea(MicroVM* vm, CELL* params)
{
  	int x1 = params[0].ival;
	int y1 = params[1].ival;
	int x2 = params[2].ival;
	int y2 = params[3].ival;

	CELL res;	
	lcd_x1 = x1;
	lcd_y1 = y1;
	lcd_x2 = x2;
	lcd_y2 = y2;
  	return res;
}

//void doTouchInit(char[] name,char[] mode)
CELL doTouchInit(MicroVM* vm, CELL* params)
{
  	CELL res;
	//Touch_Init();
	//Pen_Int_Set(0);
	return res;
}

//int doIsTouch()
CELL doIsTouch(MicroVM* vm, CELL* params)
{
  	CELL res;
	int i=20;

	if(isTouch == 0)
		res.ival = 0;
	else
		res.ival = 1;
	return res;
}

//int doTouchX()
CELL doTouchX(MicroVM* vm, CELL* params)
{
  	CELL res;
	//res.ival = Pen_Point.X0;
	res.ival = TouchX;
	return res;
}

//int doTouchY()
CELL doTouchY(MicroVM* vm, CELL* params)
{
  	CELL res;
	//res.ival = Pen_Point.Y0;
	res.ival = TouchY;
	return res;
}
