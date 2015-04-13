#include "vm.h"
#include "font.h"
#include <windows.h>

#define RGB24_TO_RGB16(color) ((((color) >> 19) & 0x1f) << 11) \
                                            |((((color) >> 10) & 0x3f) << 5) \
                                            |(((color) >> 3) & 0x1f)

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

void vm_draw_point(int x,int y,int cx1,int cy1,int cx2,int cy2,int c)
{
	HDC hdc;
	BYTE rgbBuf[3];
	DWORD color24;
	rgb565Torgb24(rgbBuf,c);
	color24 = RGB (rgbBuf[2],rgbBuf[1],rgbBuf[0]);
	hdc = GetDC (Global_hwnd) ;
	if((x>=cx1&&x<=cx2)&&(y>=cy1&&y<=cy2))
	{
		lcd_buf[x+y*240] = color24;
		SetPixel (hdc, x, y, color24);
	}
	ReleaseDC (Global_hwnd,hdc) ;
}

//void setPoint(int x, int y, int color)
CELL graphic_Graphic_setPoint(MicroVM* vm, CELL* params)
{
  	int x0 = params[0].ival;
	int y0 = params[1].ival;
	int cx0 = params[2].ival;
	int cy0 = params[3].ival;
	int cx1 = params[4].ival;
	int cy1 = params[5].ival;
	int color = params[6].ival;
	CELL res;
	vm_draw_point(x0,y0,cx0,cy0,cx1,cy1,color);
  	return res;
}

CELL graphic_Graphic_getChar(MicroVM* vm, CELL* params)
{
	int x = params[0].ival;
	int y = params[1].ival;
	CELL res;
	res.ival = asc2_1608[(unsigned char)x-' '][y];
}

//void drawChar(int x, int y, int c, int font, int color)
CELL graphic_Graphic_drawChar(MicroVM* vm, CELL* params)
{
  	int x = params[0].ival;
	int y = params[1].ival;
	int cx0 = params[2].ival;
	int cy0 = params[3].ival;
	int cx1 = params[4].ival;
	int cy1 = params[5].ival;
	int c = params[6].ival;
	int f = params[7].ival;
	int color = params[8].ival;
	int pos,size = f;
	char temp,t;
	CELL res;
	c=c-' ';
	for(pos=0;pos<size;pos++)
	{
		if(size==12)temp=asc2_1206[c][pos];//调用1206字体
		else temp=asc2_1608[c][pos];		 //调用1608字体
		for(t=0;t<size/2;t++)
	    {                 
	       if(temp&0x01) 
		   {
				vm_draw_point (x+t, y+pos, cx0,cy0,cx1,cy1, color) ;
		   }
	       temp>>=1; 
	    }
	}
  	return res;
}

//void fill(int x1, int y1, int x2, int y2, int color)
CELL graphic_Graphic_fill(MicroVM* vm, CELL* params)
{
	int xx0 = params[0].ival;
	int yy0 = params[1].ival;
	int xx1 = params[2].ival;
	int yy1 = params[3].ival;
	int cx0 = params[4].ival;
	int cy0 = params[5].ival;
	int cx1 = params[6].ival;
	int cy1 = params[7].ival;
	int c = params[8].ival;
	int sx,sy,ex,ey;
	CELL res;	
	HDC hdc;
	RECT rect;
	HBRUSH hBrush;
	BYTE rgbBuf[3];
	int countX,countY;
	DWORD color24;

	if(xx0<=cx0) 	sx = cx0;
	else if(xx0>=cx1)sx = cx1;
	else        	sx = xx0;
	
	if(xx1>=cx1) 	ex = cx1;
	else if(xx1<=cx0)ex = cx0;
	else        	ex = xx1;
	
	if(yy0<=cy0) 	sy = cy0;
	else if(yy0>=cy1)sy = cy1;
	else        	sy = yy0;
	
	if(yy1>=cy1) 	ey = cy1;
	else if(yy1<=cy0)ey = cy0;
	else        	ey = yy1;

	rgb565Torgb24(rgbBuf,c);
	color24 = RGB (rgbBuf[2],rgbBuf[1],rgbBuf[0]);
    SetRect(&rect, sx, sy, ex+1, ey+1);
	hBrush = CreateSolidBrush(color24);
	hdc = GetDC (Global_hwnd) ;

	for(countX = sx;countX<=ex;countX++)
	{
		for(countY = sy;countY<=ey;countY++)
		{
			lcd_buf[countX+countY*240] = color24;
		}
	}
	FillRect(hdc, &rect, hBrush);
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


//int getColor(int c)
CELL graphic_Graphic_getColor(MicroVM* vm, CELL* params)
{
 	int c = params[0].ival; 	
	CELL res;
	res.ival = RGB24_TO_RGB16(c);
	return res;
}


void vm_draw_line(int x1, int y1, int x2,int y2,int cx1, int cy1, int cx2,int cy2,int color)
{
	int t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //????×?±ê??á? 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //éè??μ￥2?·??ò 
	else if(delta_x==0)incx=0;//′1?±?? 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//?????? 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //??è??ù±???á?×?±ê?á 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//?-??ê?3? 
	{  
		vm_draw_point(uRow,uCol,cx1,cy1,cx2,cy2,color);//?-μ? 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

//void drawLine(int x0,int y0,int x1,int y1,int cx0,int cy0,int cx1,int cy1,int color)
CELL graphic_Graphic_drawLine(MicroVM* vm, CELL* params)
{
  	int x0 = params[0].ival;
	int y0 = params[1].ival;
	int x1 = params[2].ival;
	int y1 = params[3].ival;
	int cx0 = params[4].ival;
	int cy0 = params[5].ival;
	int cx1 = params[6].ival;
	int cy1 = params[7].ival;
	int color = params[8].ival;
	CELL res;
	vm_draw_line(x0,y0,x1,y1,cx0,cy0,cx1,cy1,color);
  	return res;
}

//void fillCircle(int x0,int y0,int x1,int y1,int cx0,int cy0,int cx1,int cy1,int color)
CELL graphic_Graphic_fillCircle(MicroVM* vm, CELL* params)
{
  	int xc = params[0].ival;
	int yc = params[1].ival;
	int cx1 = params[2].ival;
	int cy1 = params[3].ival;
	int cx2 = params[4].ival;
	int cy2 = params[5].ival;
	int r = params[6].ival;
	int color = params[7].ival;
	CELL res;
	int x = 0, y = r, yi, d;
	d = 3 - 2 * r;
 	if (xc + r < 0  || yc + r < 0) return res;
	while (x <= y) 
	{
        for (yi = x; yi <= y; yi ++)
		{
			vm_draw_point(xc + x, yc + yi,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc - x, yc + yi,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc + x, yc - yi,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc - x, yc - yi,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc + yi, yc + x,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc - yi, yc + x,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc + yi, yc - x,cx1,cy1,cx2,cy2,color);
			vm_draw_point(xc - yi, yc - x,cx1,cy1,cx2,cy2,color);
		}

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y --;
        }
        x++;
    }
  	return res;
}

//void drawLine(int x0,int y0,int x1,int y1,int cx0,int cy0,int cx1,int cy1,int color)
CELL graphic_Graphic_drawRect(MicroVM* vm, CELL* params)
{
  	int x1 = params[0].ival;
	int y1 = params[1].ival;
	int x2 = params[2].ival;
	int y2 = params[3].ival;
	int cx1 = params[4].ival;
	int cy1 = params[5].ival;
	int cx2 = params[6].ival;
	int cy2 = params[7].ival;
	int color = params[8].ival;

	CELL res;
	vm_draw_line(x1,y1,x2,y1,cx1,cy1,cx2,cy2,color);
	vm_draw_line(x1,y1,x1,y2,cx1,cy1,cx2,cy2,color);
	vm_draw_line(x1,y2,x2,y2,cx1,cy1,cx2,cy2,color);
	vm_draw_line(x2,y1,x2,y2,cx1,cy1,cx2,cy2,color);
  	return res;
}

CELL graphic_Graphic_drawCircle(MicroVM* vm, CELL* params)
{
  	int x0 = params[0].ival;
	int y0 = params[1].ival;
	int cx1 = params[2].ival;
	int cy1 = params[3].ival;
	int cx2 = params[4].ival;
	int cy2 = params[5].ival;
	int r = params[6].ival;
	int color = params[7].ival;
	CELL res;
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //
	while(a<=b)
	{
		vm_draw_point(x0+a,y0-b,cx1,cy1,cx2,cy2,color);             //5
 		vm_draw_point(x0+b,y0-a,cx1,cy1,cx2,cy2,color);             //0           
		vm_draw_point(x0+b,y0+a,cx1,cy1,cx2,cy2,color);             //4               
		vm_draw_point(x0+a,y0+b,cx1,cy1,cx2,cy2,color);             //6 
		vm_draw_point(x0-a,y0+b,cx1,cy1,cx2,cy2,color);             //1       
 		vm_draw_point(x0-b,y0+a,cx1,cy1,cx2,cy2,color);             
		vm_draw_point(x0-a,y0-b,cx1,cy1,cx2,cy2,color);             //2             
  		vm_draw_point(x0-b,y0-a,cx1,cy1,cx2,cy2,color);             //7     	         
		a++;    
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
  	return res;
}
