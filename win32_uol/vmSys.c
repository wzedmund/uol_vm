// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015
#include "vm.h"
#include <windows.h>

static Obj strObj;
static CELL strVar[2];
static Array strArray;
static char strBuf[32];

CELL Sys_PrintStream_printByte(MicroVM* vm, CELL* params)
{
  uint8_t  pu8Buf  = (uint8_t )params[0].ival;
  CELL res;	
  printf("%c",pu8Buf);
  //fflush(stdout);
  return res;
}

CELL Sys_System_malloc(MicroVM* vm, CELL* params)
{
 	uint32_t  size  = (uint32_t )params[0].ival;
 	CELL res;	
 	void* mem; 
  	mem = (void*)malloc(size);
	res.addr = mem;
 	return res;
}

CELL Sys_System_free(MicroVM* vm, CELL* params)
{
 	uint8_t*  mem  = params[0].addr;
 	CELL res;	 
  	if(mem != NULL) free(mem);
 	return res;
}

CELL Sys_System_copy(MicroVM* vm, CELL* params)
{
 	uint8_t* src = params[0].addr;
 	int32_t  srcOff = params[1].ival;
 	uint8_t* dest = params[2].addr;
 	int32_t  destOff = params[3].ival;
 	int32_t  num = params[4].ival;
 	CELL res;	 
	if (num > 0)
    		memmove(dest+destOff, src+srcOff, num);
 	return res;
}

CELL Sys_System_int2Str(MicroVM* vm, CELL* params)
{
 	int32_t i = params[0].ival;
 	CELL res;
	memset(strBuf,0,32);
	sprintf(strBuf, "%d", i);
	strObj.classId = vm->stringClassId;
	strArray.addr = strBuf;
	strArray.len = sizeof(strBuf);
	strVar[0].addr = &strArray;
	strObj.var = (CELL*)strVar;
	res.addr = &strObj;
  	return res;
}

CELL Sys_System_hex2Str(MicroVM* vm, CELL* params)
{
 	int32_t i = params[0].ival;
 	CELL res;
	sprintf(strBuf, "%x", i);
	strObj.classId = vm->stringClassId;
	strArray.addr = strBuf;
	strArray.len = sizeof(strBuf);
	strVar[0].addr = &strArray;
	strObj.var = (CELL*)strVar;
	res.addr = &strObj;
  	return res;
}



CELL Sys_System_long2Str(MicroVM* vm, CELL* params)
{
 	int64_t i = *(int64_t*)params;
 	CELL res;
	#ifdef _WIN32
	  sprintf(strBuf, "%I64d", i);
	#else
	  sprintf(strBuf, "%lld", i);
	#endif
	strObj.classId = vm->stringClassId;
	strArray.addr = strBuf;
	strArray.len = sizeof(strBuf);
	strVar[0].addr = &strArray;
	strObj.var = (CELL*)strVar;
	res.addr = &strObj;
  	return res;
}

CELL Sys_System_longHex2Str(MicroVM* vm, CELL* params)
{
 	int64_t i = *(int64_t*)params;
 	CELL res;
	#ifdef _WIN32
	  sprintf(strBuf, "%I64x", i);
	#else
	  sprintf(strBuf, "%llx", i);
	#endif
	strObj.classId = vm->stringClassId;
	strArray.addr = strBuf;
	strArray.len = sizeof(strBuf);
	strVar[0].addr = &strArray;
	strObj.var = (CELL*)strVar;
	res.addr = &strObj;
  	return res;
}

CELL Sys_System_float2Str(MicroVM* vm, CELL* params)
{
 	float f = params[0].fval;
 	CELL res;
	sprintf(strBuf, "%f", f);
	strObj.classId = vm->stringClassId;
	strArray.addr = strBuf;
	strArray.len = sizeof(strBuf);
	strVar[0].addr = &strArray;
	strObj.var = (CELL*)strVar;
	res.addr = &strObj;
  	return res;
}

CELL Sys_System_double2Str(MicroVM* vm, CELL* params)
{
 	double d = *(double*)params;
 	CELL res;
	sprintf(strBuf, "%lf", d);
	strObj.classId = vm->stringClassId;
	strArray.addr = strBuf;
	strArray.len = sizeof(strBuf);
	strVar[0].addr = &strArray;
	strObj.var = (CELL*)strVar;
	res.addr = &strObj;
  	return res;
}

CELL Sys_System_compareBytes(MicroVM* vm, CELL* params)
{
	uint8_t* a    = params[0].addr;
	int32_t  aoff = params[1].ival;
	uint8_t* b    = params[2].addr;
	int32_t  boff = params[3].ival;
	int32_t  len  = params[4].ival;
	int i;
	CELL res;
	a = a + aoff;
	b = b + boff;
	for (i=0; i<len; ++i)
	{          
		int ai = a[i];
		int bi = b[i];
		if (ai != bi) 
		{
			if(ai<bi) res.ival = -1;
			else res.ival = 1;
		}
	}    
	res.ival = 0;                      
	return res;
}

CELL Sys_System_sleep(MicroVM* vm, CELL* params)
{
	uint32_t time = params[0].ival;
	CELL res;
	Sleep(time);                      
	return res;
}

CELL sys_System_printStackIndex(MicroVM* vm, CELL* params)
{
 	int32_t i = params[0].ival;
 	CELL res;
	printf("%x\n", vm->sp->ival);
  	return res;
}

