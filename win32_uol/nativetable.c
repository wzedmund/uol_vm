// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#include "vm.h"

CELL Sys_PrintStream_printByte(MicroVM* vm, CELL* params);
CELL Sys_System_malloc(MicroVM* vm, CELL* params);
CELL Sys_System_free(MicroVM* vm, CELL* params);
CELL Sys_System_copy(MicroVM* vm, CELL* params);
CELL Sys_System_int2Str(MicroVM* vm, CELL* params);
CELL Sys_System_hex2Str(MicroVM* vm, CELL* params);
CELL Sys_System_long2Str(MicroVM* vm, CELL* params);
CELL Sys_System_longHex2Str(MicroVM* vm, CELL* params);
CELL Sys_System_float2Str(MicroVM* vm, CELL* params);
CELL Sys_System_double2Str(MicroVM* vm, CELL* params);
CELL Sys_System_compareBytes(MicroVM* vm, CELL* params);
CELL Sys_System_sleep(MicroVM* vm, CELL* params);
CELL sys_System_printStackIndex(MicroVM* vm, CELL* params);

NativeMethod Natives0[] = 
{
	Sys_PrintStream_printByte,
	Sys_System_malloc,
	Sys_System_free,
	Sys_System_copy,
	Sys_System_int2Str,
	Sys_System_hex2Str,
	Sys_System_long2Str,
	Sys_System_longHex2Str,
	Sys_System_float2Str,
	Sys_System_double2Str,
	Sys_System_compareBytes,
	Sys_System_sleep,
	sys_System_printStackIndex,
};

NativeMethod Natives1[] = 
{
	Sys_PrintStream_printByte,
};

//void onOff(bool b)
CELL graphic_Graphic_onOff(MicroVM* vm, CELL* params);

//void clear(int color)
CELL graphic_Graphic_clear(MicroVM* vm, CELL* params);
//int getWidth()
CELL graphic_Graphic_getWidth(MicroVM* vm, CELL* params);
//int getHeight()
CELL graphic_Graphic_getHeight(MicroVM* vm, CELL* params);
//int getPoint(int x, int y)
CELL graphic_Graphic_getPoint(MicroVM* vm, CELL* params);
//void setPoint(int x, int y, int color)
CELL graphic_Graphic_setPoint(MicroVM* vm, CELL* params);

//void drawChar(int x, int y, int c, int font, int color)
CELL graphic_Graphic_drawChar(MicroVM* vm, CELL* params);

//void fill(int x1, int y1, int x2, int y2, int color)
CELL graphic_Graphic_fill(MicroVM* vm, CELL* params);

//int isTouch()
CELL graphic_TouchDispatcher_isTouch(MicroVM* vm, CELL* params);

//int touchX()
CELL graphic_TouchDispatcher_touchX(MicroVM* vm, CELL* params);
//int touchY()
CELL graphic_TouchDispatcher_touchY(MicroVM* vm, CELL* params);

NativeMethod Natives2[] = 
{
	graphic_Graphic_getWidth,
	graphic_Graphic_getHeight,
	graphic_Graphic_onOff,
	graphic_Graphic_setPoint,
	graphic_Graphic_getPoint,
	graphic_Graphic_fill,
	graphic_Graphic_drawChar,
	graphic_Graphic_clear,
	graphic_TouchDispatcher_isTouch,
	graphic_TouchDispatcher_touchX,
	graphic_TouchDispatcher_touchY,
};

NativeMethod* nativeTable[] = 
{
  Natives0, 
  Natives1,
  Natives2,	
};

//check if native id is valid
int isNativeValid(int id1, int id2)
{
  switch(id1)
  {
    case 0:
      if (id2 >= 13) return 0;
      else return Natives0[id2] != NULL;
	case 1:
      if (id2 >= 0) return 0;
      else return Natives1[id2] != NULL;
	case 2:
      if (id2 >= 11) return 0;
      else return Natives2[id2] != NULL;	
    default:
       return 0;
  }
}

