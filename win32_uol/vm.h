// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "errcode.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//VM Marco
#define STACK_SIZE 2048
 
#define VM_STRPOOL_LEN_OFFSET 0
#define VM_STRPOOL_ADDR_OFFSET 4
#define VM_STATIC_LEN_OFFSET 8
#define VM_NUM_OBJECTS_OFFSET 12
#define VM_NUM_ARRAYS_OFFSET 14
#define VM_ENDIAN_OFFSET 16
#define VM_DEBUG_OFFSET 17
#define VM_STACK_OFFSET 18	
#define VM_STRING_CLASS_TYPE_OFFSET 22

#define THREAD_STACK_SIZE 512
#define VM_HEAD_SIZE 26

#define CAST_BIT8     0x000000FF
#define CAST_BIT16    0x0000FFFF

//basic memory unit
typedef union
{
	int32_t ival;
	float   fval;
	void*   addr;
}CELL;

struct 	MICROVM;
typedef int	 error_t;
typedef CELL (*NativeMethod)(struct MICROVM* vm, CELL* params);
typedef int64_t (*NativeMethodWide)(struct MICROVM* vm, CELL* params);

//object structure
typedef struct OBJ
{
	CELL* parent;
	CELL* var;
	int32_t classId;
	CELL* data;
}Obj;

//try-catch structure
typedef struct TRYCATCH
{
	CELL* sp;
	CELL* prevTry;
	int32_t addr;
}Trycatch;

//array structure
typedef struct ARRAY
{
	void* addr;
	uint32_t len;
}Array;

//invoke structure
typedef struct INVOKE
{
	Obj* data;
	CELL* prevLink;
}Invoke;

//vm structure
typedef struct MICROVM
{
	uint8_t* pOpcodeAddr;  //opcode address
	uint32_t pcOffset;     //pc offset
	uint32_t codeSize;	   //opcode size
	uint32_t stackSize;    //local stack size
	uint16_t stackOffset;  //stack offset address
	uint32_t staticSize;   //static memory size
	CELL* pParamAddr;      //parameter pointer
	CELL* pStackAddr;      //local stack pointer
	CELL* pLocalAddr;	   //local memory pointer
	Obj* pDataAddr;	       //data memory pointer
	Obj* pStrPool;         //string pool pointer
	CELL* pStaticAddr;     //static memory pointer
	Trycatch* errSp;	   //try-catch stack pointer
	NativeMethod** nativeTable; //native map pointer
	uint32_t priority;     //thread priority    
	uint8_t* name;         //thread name
	CELL* handle;		   //thread handle
	uint8_t* pc;           //program counter
	uint32_t stringClassId;//string class id
}MicroVM;


int isNativeValid(int id1, int id2);

error_t vmStart(MicroVM* vm);

error_t vmInit(MicroVM* vm);

error_t vmRun(MicroVM* vm);

int vmLoadFile(MicroVM* vm);
#endif
