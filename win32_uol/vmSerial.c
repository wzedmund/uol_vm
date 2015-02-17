#include "vm.h"


CELL doSerialInit(MicroVM* vm, CELL* params)
{
  CELL res;	
  res.ival = 1;
  return res;
}

CELL doSerialClose(MicroVM* vm, CELL* params)
{
  //int port = params[0].ival;
  CELL res;	
  res.ival = 1;
  return res;
}

CELL doSerialRead(MicroVM* vm, CELL* params)
{
  CELL res;	
  return res;
}

CELL doSerialWrite(MicroVM* vm, CELL* params)
{
  //int port = params[0].ival;
  uint8_t c = (uint8_t)params[1].ival;
  CELL res;	
  printf("%c",c);
  return res;
}

CELL doSerialReadBytes(MicroVM* vm, CELL* params)
{
      //int port = params[0].ival;
	Array* data = params[1].addr;
	uint8_t* array = data->addr;
	uint32_t off = params[2].ival;
	uint32_t len = params[3].ival;
       int i;
	CELL res;	
  return res;
}

CELL doSerialWriteBytes(MicroVM* vm, CELL* params)
{
  //int port = params[0].ival;
	Array* array = params[1].addr;
	uint8_t* data = array->addr;
	uint32_t off = params[2].ival;
	uint32_t len = params[3].ival;
  int i;
	CELL res;	
  return res;
}
