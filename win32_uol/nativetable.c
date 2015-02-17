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
};


NativeMethod* nativeTable[] = 
{
  Natives0, 	
};

//check if native id is valid
int isNativeValid(int id1, int id2)
{
  switch(id1)
  {
    case 0:
      if (id2 >= 12) return 0;
      else return Natives0[id2] != NULL;
		
    default:
       return 0;
  }
}

