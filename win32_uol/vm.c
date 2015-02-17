// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#include <windows.h>
#include "vm.h"
#include "instruction.h"

error_t vmInit(MicroVM* vm)
{
	int addr;
	int len;
	int i;
	int strLen;
	int index;
	
	Obj* strPool = NULL;
	Array* array = NULL;
	uint8_t *pStr;
	
	//get address index of opcode
	len  = *(uint32_t*)vm->pOpcodeAddr;
	//get offset address of string pool·
	addr = *(uint32_t*)(vm->pOpcodeAddr+VM_STRPOOL_ADDR_OFFSET);
	pStr = vm->pOpcodeAddr+addr;

	//return error when opcode size is zero
	if(vm->codeSize==0)
	{
		return VM_BAD_CODE_SIZE_ERR;
	}
	
	//initialize string pool
	if(len>0)
	{
		strPool = (Obj*)malloc(len*sizeof(Obj));
		if (strPool == NULL)
		{
			printf("Cannot malloc string pool segments\r\n");
			return VM_MALLOC_STRING_POOL_ERR;
		}
		for(i=0;i<len;)
		{
			 strLen = *(int32_t*)pStr;
			 pStr+=4;
			 (strPool+i)->var = (CELL*)malloc(2*sizeof(CELL));
			 (strPool+i)->classId = vm->stringClassId;
			 *(int*)((strPool+i)->var+1)= strLen;
			 (strPool+i)->var->addr = (Array*)malloc(sizeof(Array));
			 array = (strPool+i)->var->addr;
			 array->addr = (uint8_t*)malloc(strLen*sizeof(char));
			 array->len = strLen;
			 for(index = 0;index<strLen;index++)
			 {
				 *((int8_t*)array->addr + index) = pStr[index];
			 }
			 pStr=pStr+strLen;
			 i++;
		}
	}
	vm->pStrPool = strPool;
	//get size of static memory
	len  =   *(uint32_t*)(vm->pOpcodeAddr+VM_STATIC_LEN_OFFSET);
	vm->staticSize = len;
	printf("static memory size: %d bytes\r\n",len*4);
	if(len>0)
	{
		vm->pStaticAddr = (CELL*)malloc(len*4);

		if(vm->pStaticAddr == NULL)
		{
			printf("Cannot malloc static memory segments\r\n");
			return VM_MALLOC_STATIC_MEM_ERR;
		}
		memset(vm->pStaticAddr,0,len*sizeof(int));
	}
	return VM_OK;
}

MicroVM* intVm;
int prio = 1;

int task_res;

error_t vmRun(MicroVM* vm)
{
	register CELL*  sp;
	register CELL*  param;
	register Obj*  data = NULL;
	register CELL*  local;
	register uint8_t*  pc;
	register uint8_t*  pcBaseAddr;
	register Obj* strpool;
	register CELL* pStatic;
	register Trycatch* errSp;

	Obj* temp_obj;
	Obj* temp_obj1;
	CELL* temp_errSp;

	CELL* spHigh;
	CELL* spLow;
	CELL cell;
	CELL* temp_param;
	CELL* temp_local;
	Array* temp_array;
	Invoke* invoke;
	Invoke* tempInvoke;
	
	int64_t temp64_value;
	int32_t temp32_value;
	int16_t temp16_value;
	int32_t error;

	NativeMethod** nativeTable;
	NativeMethod native;
  
	CELL* pStackAddr;
	uint32_t stackSize;
	
	error = VM_OK;
	pStackAddr = vm->pStackAddr;
	stackSize  = vm->stackSize;
	data =vm->pDataAddr;
	
	pStatic = vm->pStaticAddr;
	sp=vm->pStackAddr;
	param=vm->pParamAddr;
	pc=vm->pOpcodeAddr+vm->pcOffset;
	pcBaseAddr=vm->pOpcodeAddr;
	local = sp;
	sp = sp + vm->stackOffset;
	nativeTable = vm->nativeTable;
	strpool=vm->pStrPool;
	spHigh = pStackAddr+(stackSize/4)-1;
	spLow  = pStackAddr;
	errSp = vm->errSp;
	
	invoke = malloc(sizeof(Invoke));
	invoke->data = data;

	while(1)
	{
	  	//check if stack overflows
		if((sp>=(spHigh)||sp<spLow))
		{
			error = VM_STACK_OVERFLOW_ERR;
			goto VM_ERROR;
		}
		switch(*pc)
	 	{
			case Nop:pc++;break;
			//locals
			case LoadBit32Local:++sp;sp->ival=*(int32_t*)(local+*(uint8_t*)(pc+1));pc=pc+2;break;
			case LoadBit64Local:++sp;*(int64_t*)sp=*(int64_t*)(local+*(uint8_t*)(pc+1));sp++;pc=pc+2;break;
			case LoadBit32LocalWide:++sp;sp->ival=*(int32_t*)(local+*(uint16_t*)(pc+1));pc=pc+3;break;
			case LoadBit64LocalWide:++sp;*(int64_t*)sp=*(int64_t*)(local+*(uint16_t*)(pc+1));sp++;pc=pc+3;break;
			case StoreBit32Local:*(int32_t*)(local+*(uint8_t*)(pc+1))=sp->ival;sp--;pc=pc+2;break;
			case StoreBit64Local:*(int64_t*)(local+*(uint8_t*)(pc+1))=*(int64_t*)(sp-1);sp=sp-2;pc=pc+2;break;
			case StoreBit32LocalWide:*(int32_t*)(local+*(uint16_t*)(pc+1))=sp->ival;sp--;pc=pc+3;break;
			case StoreBit64LocalWide:*(int64_t*)(local+*(uint16_t*)(pc+1))=*(int64_t*)(sp-1);sp=sp-2;pc=pc+3;break;
			//arguments
			case LoadBit32Param:sp++;sp->ival=*(int32_t*)(param+*(uint8_t*)(pc+1));pc=pc+2;break;
			case LoadBit64Param:sp++;*(int64_t*)sp=*(int64_t*)(param+*(uint8_t*)(pc+1));sp++;pc=pc+2;break;
			case StoreBit32Param:*(int32_t*)(param+*(uint8_t*)(pc+1))=sp->ival;sp--;pc=pc+2;break;
			case StoreBit64Param:*(int64_t*)(param+*(uint8_t*)(pc+1))=*(int64_t*)(sp-1);sp=sp-2;pc=pc+2;break;
			//class data members
			case LoadBit32Data:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;	
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				sp++;sp->ival=*(int32_t*)(data->var+*(uint8_t*)(pc+1));pc=pc+2;break;
			case LoadBit32DataWide:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;	
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				sp++;sp->ival=*(int32_t*)(data->var+*(uint16_t*)(pc+1));pc=pc+3;break;
			case LoadBit64Data:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;	
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				sp++;*(int64_t*)sp=*(int64_t*)(data+*(uint8_t*)(pc+1));sp++;pc=pc+2;break;
			case LoadBit64DataWide:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;	
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				sp++;*(int64_t*)sp=*(int64_t*)(data+*(uint16_t*)(pc+1));sp++;pc=pc+3;break;
			case StoreBit32Data:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				*(int32_t*)(data->var+*(uint8_t*)(pc+1))=sp->ival;sp--;pc=pc+2;break;
			case StoreBit32DataWide:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				*(int32_t*)(data->var+*(uint16_t*)(pc+1))=sp->ival;sp--;pc=pc+3;break;
			case StoreBit64Data:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				*(int64_t*)(data+*(uint8_t*)(pc+1))=*(int64_t*)(sp-1);sp=sp-2;pc=pc+2;break;
			case StoreBit64DataWide:
				if(data == NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;break;
					}
					else
						goto VM_ERROR;
				}
				*(int64_t*)(data+*(uint16_t*)(pc+1))=*(int64_t*)(sp-1);sp=sp-2;pc=pc+3;break;
			//constant
			case LoadBit32Code:sp++;sp->ival=*(int32_t*)(pc+1);pc=pc+5;break;
			case LoadBit64Code:sp++;*(int64_t*)sp=*(int64_t*)(pc+1);sp++;pc=pc+9;break;
			case StoreBit32Code:*(int32_t*)(pc+1)=sp->ival;sp--;pc=pc+5;break;
			case StoreBit64Code:*(int64_t*)(pc+1)=*(int64_t*)(sp-1);sp=sp-2;pc=pc+9;break;
			//arithmetic of 32-bit
			case Bit32Add:sp=sp-1;sp->ival=sp->ival + (sp+1)->ival;pc++;break;
			case Bit32Sub:sp=sp-1;sp->ival=sp->ival - (sp+1)->ival;pc++;break;
			case Bit32Mul:sp=sp-1;sp->ival=sp->ival * (sp+1)->ival;pc++;break;
			case Bit32Div:
				sp--;
				if((sp+1)->ival==0)
				{
					error = VM_DIVIDE_BY_ZERO_ERR;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = VM_DIVIDE_BY_ZERO_ERR;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				sp->ival=sp->ival / (sp+1)->ival;pc++;break;
			case Bit32And:sp=sp-1;sp->ival=sp->ival & (sp+1)->ival;pc++;break;
			case Bit32Or: sp=sp-1;sp->ival=sp->ival | (sp+1)->ival;pc++;break;
			case Bit32Xor:sp=sp-1;sp->ival=sp->ival ^ (sp+1)->ival;pc++;break;
			case Bit32Not:sp->ival= ~ sp->ival;pc++;break;
			case Bit32Rem:sp=sp-1;sp->ival=sp->ival % (sp+1)->ival;pc++;break;
			case Bit32ShiftLeft: sp=sp-1;sp->ival=sp->ival << (sp+1)->ival;pc++;break;
			case Bit32ShiftRight:sp=sp-1;sp->ival=sp->ival >> (sp+1)->ival;pc++;break;
			case Bit32Inc:sp->ival++;pc++;break;
			case Bit32Dec:sp->ival--;pc++;break;
			case Bit32Neg:sp->ival=-sp->ival;pc++;break;
			//arithmetic of 64-bit
			case Bit64Add:sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) + *(int64_t*)(sp+1);pc++;break;
			case Bit64Sub:sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) - *(int64_t*)(sp+1);pc++;break;
			case Bit64Mul:sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) * *(int64_t*)(sp+1);pc++;break;
			case Bit64Div:
				sp=sp-2;
				if(*(int64_t*)(sp+1)==0)
				{
					error = VM_DIVIDE_BY_ZERO_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				*(int64_t*)(sp-1)=*(int64_t*)(sp-1) / *(int64_t*)(sp+1);pc++;break;
			case Bit64And:sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) & *(int64_t*)(sp+1);pc++;break;
			case Bit64Or: sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) | *(int64_t*)(sp+1);pc++;break;
			case Bit64Xor:sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) ^ *(int64_t*)(sp+1);pc++;break;
			case Bit64Not:*(int64_t*)(sp-1)= ~ *(int64_t*)(sp-1);pc++;break;
			case Bit64Rem:       sp=sp-2;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) % *(int64_t*)(sp+1);pc++;break;
			case Bit64ShiftLeft: sp--;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) << sp->ival;pc++;break;
			case Bit64ShiftRight:sp--;*(int64_t*)(sp-1)=*(int64_t*)(sp-1) >> sp->ival;pc++;break;
			case Bit64Inc:(*(int64_t*)(sp-1))++;pc++;break;
			case Bit64Dec:(*(int64_t*)(sp-1))--;pc++;break;
			case Bit64Neg:*(int64_t*)(sp-1)=-(*(int64_t*)(sp-1));pc++;break;
			////arithmetic of floating point
			case FloatAdd:sp--;sp->fval=sp->fval + (sp+1)->fval;pc++;break;
			case FloatSub:sp--;sp->fval=sp->fval - (sp+1)->fval;pc++;break;
			case FloatMul:sp--;sp->fval=sp->fval * (sp+1)->fval;pc++;break;
			case FloatDiv:
				sp--;
				if((sp+1)->fval==0)
				{
					error = VM_DIVIDE_BY_ZERO_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				sp->fval=sp->fval / (sp+1)->fval;pc++;break;
			case FloatNeg:sp->fval=-(sp->fval);pc++;break;
			//arithmetic of double
			case DoubleAdd:sp=sp-2;*(double*)sp=*(double*)sp + *(double*)(sp+2);pc++;break;
			case DoubleSub:sp=sp-2;*(double*)sp=*(double*)sp - *(double*)(sp+2);pc++;break;
			case DoubleMul:sp=sp-2;*(double*)sp=*(double*)sp * *(double*)(sp+2);pc++;break;
			case DoubleDiv:
				if(*(double*)(sp+2)==0)
				{
					error = VM_DIVIDE_BY_ZERO_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				sp=sp-2;*(double*)sp=*(double*)sp / *(double*)(sp+2);pc++;break;
			case DoubleNeg:*(double*)sp=-(*(double*)sp);pc++;break;
			//cast to integer
			case LongToInt:  sp--;sp->ival=(int32_t)(*(int64_t*)sp);pc++;break;
			case FloatToInt:      sp->ival=(int32_t)(sp->fval);pc++;break;
			case DoubleToInt:sp--;sp->ival=(int32_t)(*(double*)sp);pc++;break;
			//cast to long type
			case IntToLong:   *(int64_t*)sp=(int64_t)(*(int32_t*)sp);sp++;pc++;break;
			case FloatToLong: *(int64_t*)sp=(int64_t)(sp->fval);sp++;pc++;break;
			case DoubleToLong:*(int64_t*)(sp-1)=(int64_t)(*(double*)(sp-1));pc++;break;
			//cast to float
			case IntToFloat:        sp->fval=(float)(sp->ival);pc++;break;
			case LongToFloat:  sp--;sp->fval=(float)(*(int64_t*)sp);pc++;break;
			case DoubleToFloat:sp--;sp->fval=(float)(*(double*)sp);pc++;break;
			//cast to double
			case IntToDouble:  *(double*)sp=(double)(sp->ival);sp++;pc++;break;
			case LongToDouble: *(double*)(sp-1)=(double)(*(int64_t*)(sp-1));pc++;break;
			case FloatToDouble:*(double*)sp=(double)(sp->fval);sp++;pc++;break;

			//jump
			case Jump:pc=pcBaseAddr+(*(uint32_t*)(pc+1));break;
			case JumpZero:    if(sp->ival==0) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-1;break;
			case JumpNotZero: if(sp->ival!=0) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-1;break;
			case JumpIntEq:   if((sp-1)->ival==sp->ival) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			case JumpIntNotEq:if((sp-1)->ival!=sp->ival) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			case JumpIntGt:   if((sp-1)->ival>sp->ival)  pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			case JumpIntLt:   if((sp-1)->ival<sp->ival)  pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			case JumpIntGtEq: if((sp-1)->ival>=sp->ival) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			case JumpIntLtEq: if((sp-1)->ival<=sp->ival) pc=pcBaseAddr+(*(uint32_t*)(pc+1));else {pc=pc+5;}sp=sp-2;break;
			//short jump
			case SJump:        pc=pc+(*(int8_t*)(pc+1));break;
			case SJumpZero:    if(sp->ival==0) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-1;break;
			case SJumpNotZero: if(sp->ival!=0) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-1;break;
			case SJumpIntEq:   if((sp-1)->ival==sp->ival) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			case SJumpIntNotEq:if((sp-1)->ival!=sp->ival) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			case SJumpIntGt:   if((sp-1)->ival>sp->ival)  pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			case SJumpIntLt:   if((sp-1)->ival<sp->ival)  pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			case SJumpIntGtEq: if((sp-1)->ival>=sp->ival) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			case SJumpIntLtEq: if((sp-1)->ival<=sp->ival) pc=pc+(*(int8_t*)(pc+1));else {pc=pc+2;}sp=sp-2;break;
			//method call
			case Call:
					  temp_param = param;
					  temp_local = local;
					  param = (sp-(*(pc+5))+1);
					  sp++;
					  local=sp;
					  sp=sp+(*(int16_t*)(pc+6));
					  sp->addr=pc+8;
					  sp++;
					  sp->ival=*(pc+5);
					  sp++;
					  sp->ival=*(int16_t*)(pc+6);	
					  sp++;
					  sp->addr = temp_param;
					  sp++;
					  sp->addr = temp_local;
					  pc=pcBaseAddr+*(uint32_t*)(pc+1);
					  break;
			//native call
			case CallNative:
							temp16_value = *(pc+3);
							if(!isNativeValid(*(pc+1),*(pc+2)))
							{
								error = VM_INVALID_NATIVE_ERR;
								if(errSp!=NULL)
								{
									sp++;
									sp->ival = VM_INVALID_NATIVE_ERR;
									pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							native = nativeTable[*(pc+1)][*(pc+2)];
							sp-=temp16_value-1;
							cell = native(vm,sp);
							*sp=cell;
							pc+=4;
							break;
			case CallNativeWide:
							temp16_value = *(pc+3);
							if(!isNativeValid(*(pc+1),*(pc+2)))
							{
								error = VM_INVALID_NATIVE_ERR;
								if(errSp!=NULL)
								{
									sp++;
									sp->ival = VM_INVALID_NATIVE_ERR;
									pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							native = nativeTable[*(pc+1)][*(pc+2)];
							sp-=temp16_value-1;
							temp64_value = ((NativeMethodWide)native)(vm,sp);
							*(int64_t*)sp = temp64_value;
							sp++;
							pc+=4;
							break;
			case CallNativeVoid:
							temp16_value = *(pc+3);
							if(!isNativeValid(*(pc+1),*(pc+2)))
							{
								error = VM_INVALID_NATIVE_ERR;
								if(errSp!=NULL)
								{
									sp++;
									sp->ival = VM_INVALID_NATIVE_ERR;
									pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							native = nativeTable[*(pc+1)][*(pc+2)];
							sp-=temp16_value-1;
							native(vm,sp);
							sp--;
							pc+=4;
							break;
			//method return
			case Return:
						local =(CELL*)sp->addr;
						sp--;
						param =(CELL*)sp->addr;
						sp--;
						pc=(uint8_t*)((sp-2)->addr);
				    		sp=sp-sp->ival-(sp-1)->ival-3;
						break;
			case ReturnBit8:
							temp32_value = sp->ival;
							sp--;
							local = (CELL*)sp->addr;
							sp--;
							param = (CELL*)sp->addr;
							sp--;
							pc=(uint8_t*)((sp-2)->addr);
							sp=sp-sp->ival-(sp-1)->ival-2;
							sp->ival = (uint8_t)temp32_value;
							break;
			case ReturnBit16:
							temp32_value = sp->ival;
							sp--;
							local = (CELL*)sp->addr;
							sp--;
							param = (CELL*)sp->addr;
							sp--;
							pc=(uint8_t*)((sp-2)->addr);
							sp=sp-sp->ival-(sp-1)->ival-2;
							sp->ival = (int16_t)temp32_value;
							break;
			case ReturnBit32:
							temp32_value = sp->ival;
							sp--;
							local = (CELL*)sp->addr;
							sp--;
							param = (CELL*)sp->addr;
							sp--;
							pc=(uint8_t*)((sp-2)->addr);
							sp=sp-sp->ival-(sp-1)->ival-2;
							sp->ival = temp32_value;
							break;
			case ReturnBit64:
							temp64_value = *(int64_t*)(sp-1);
							sp-=2;
							local = (CELL*)sp->addr;
							sp--;
							param = (CELL*)sp->addr;
							sp--;
							pc=(uint8_t*)((sp-2)->addr);
							sp=sp-sp->ival-(sp-1)->ival-2;
							*(int64_t*)sp=temp64_value;
							sp++;
							break;
		    //comparsion of 32-bit
			case Bit32Eq:--sp;sp->ival=sp->ival == (sp+1)->ival;pc++;break;
			case Bit32NotEq:--sp;sp->ival=sp->ival != (sp+1)->ival;pc++;break;
			case Bit32Gt:--sp;sp->ival=sp->ival > (sp+1)->ival;pc++;break;
			case Bit32GtEq:--sp;sp->ival=sp->ival >= (sp+1)->ival;pc++;break;
			case Bit32Lt:--sp;sp->ival=sp->ival < (sp+1)->ival;pc++;break;
			case Bit32LtEq:--sp;sp->ival=sp->ival <= (sp+1)->ival;pc++;break;

			//comparsion of 64-bit
			case Bit64Eq:   sp=sp-3;sp->ival=*(int64_t*)sp == *(int64_t*)(sp+2);pc++;break;
			case Bit64NotEq:sp=sp-3;sp->ival=*(int64_t*)sp != *(int64_t*)(sp+2);pc++;break;
			case Bit64Gt:   sp=sp-3;sp->ival=*(int64_t*)sp > *(int64_t*)(sp+2);pc++;break;
			case Bit64GtEq: sp=sp-3;sp->ival=*(int64_t*)sp >= *(int64_t*)(sp+2);pc++;break;
			case Bit64Lt:   sp=sp-3;sp->ival=*(int64_t*)sp < *(int64_t*)(sp+2);pc++;break;
			case Bit64LtEq: sp=sp-3;sp->ival=*(int64_t*)sp <= *(int64_t*)(sp+2);pc++;break;

			//comparsion of float type
			case FloatEq:   sp--;sp->ival=sp->ival == (sp+1)->ival;pc++;break;
			case FloatNotEq:sp--;sp->ival=sp->fval != (sp+1)->fval;pc++;break;
			case FloatGt:   sp--;sp->ival=sp->fval >  (sp+1)->fval;pc++;break;
			case FloatGtEq: sp--;sp->ival=sp->fval >= (sp+1)->fval;pc++;break;
			case FloatLt:   sp--;sp->ival=sp->fval <  (sp+1)->fval;pc++;break;
			case FloatLtEq: sp--;sp->ival=sp->fval <= (sp+1)->fval;pc++;break;

			//comparsion of double type
			case DoubleEq:   sp=sp-3;sp->ival=*(int64_t*)sp == *(int64_t*)(sp+2);pc++;break;
			case DoubleNotEq:sp=sp-3;sp->ival=*(double*)sp  != *(double*)(sp+2);pc++;break;
			case DoubleGt:   sp=sp-3;sp->ival=*(double*)sp  >  *(double*)(sp+2);pc++;break;
			case DoubleGtEq: sp=sp-3;sp->ival=*(double*)sp  >= *(double*)(sp+2);pc++;break;
			case DoubleLt:   sp=sp-3;sp->ival=*(double*)sp  <  *(double*)(sp+2);pc++;break;
			case DoubleLtEq: sp=sp-3;sp->ival=*(double*)sp  <= *(double*)(sp+2);pc++;break;
			//8-bit storage
			case LoadBit8Local: ++sp;sp->ival=(local+*(uint8_t*)(pc+1))->ival&CAST_BIT8;pc=pc+2;break;		
			case LoadBit8Param: ++sp;sp->ival=(param+*(uint8_t*)(pc+1))->ival&CAST_BIT8;pc=pc+2;break;		
			case LoadBit8Data:  ++sp;sp->ival=(data->var+*(uint8_t*)(pc+1))->ival&CAST_BIT8;pc=pc+2;break;		
			case LoadBit8Code:  ++sp;sp->ival=*(pc+1);pc=pc+2;break;		
			case StoreBit8Local:(local+*(uint8_t*)(pc+1))->ival=(sp->ival&CAST_BIT8);sp--;pc=pc+2;break;		
			case StoreBit8Param:(param+*(uint8_t*)(pc+1))->ival=(sp->ival&CAST_BIT8);sp--;pc=pc+2;break;	
			case StoreBit8Data:   (data->var+*(uint8_t*)(pc+1))->ival =(sp->ival&CAST_BIT8);sp--;pc=pc+2;break;		
			case StoreBit8Code: *(int8_t*)(*(uint16_t*)(pc+1)) = (sp->ival&CAST_BIT8);sp--;pc=pc+2;break;
			//16-bit storage		
			case LoadBit16Local: ++sp;sp->ival=(local+*(uint8_t*)(pc+1))->ival&CAST_BIT16;pc=pc+2;break;		
			case LoadBit16Param: ++sp;sp->ival=(param+*(uint8_t*)(pc+1))->ival&CAST_BIT16;pc=pc+2;break;		
			case LoadBit16Data:  ++sp;sp->ival=(data->var+*(uint8_t*)(pc+1))->ival&CAST_BIT16;pc=pc+2;break;		
			case LoadBit16Code:  ++sp;sp->ival=(*(uint16_t*)(pc+1))&CAST_BIT16;pc=pc+3;break;		
			case StoreBit16Local:(local+*(uint8_t*)(pc+1))->ival=(sp->ival&CAST_BIT16);sp--;pc=pc+2;break;		
			case StoreBit16Param:(param+*(uint8_t*)(pc+1))->ival=(sp->ival&CAST_BIT16);sp--;pc=pc+2;break;		
			case StoreBit16Data: (data->var+*(uint8_t*)(pc+1))->ival=(sp->ival&CAST_BIT16);sp--;pc=pc+2;break;		
			case StoreBit16Code: *(int16_t*)(*(uint16_t*)(pc+1))=(sp->ival&CAST_BIT16);sp--;pc=pc+3;break;
			//object instantiation
			case New:   sp++;
						temp32_value = *(uint32_t*)(pc+1);
						temp16_value =  *(uint32_t*)(pc+5);
						temp_obj=(Obj*)malloc(sizeof(Obj));
						if(temp_obj==NULL)
						{
							error = VM_MALLOC_NEW_ERR;
							if(errSp!=NULL)
							{
								sp++;
								sp->ival = error;
								pc = pcBaseAddr + errSp->addr;
								break;
							}
							else
								goto VM_ERROR;
						}
						memset(temp_obj,0,sizeof(Obj));
						temp_obj->classId = *(uint32_t*)(pc+6);
						temp_obj->data = (CELL*)data;
						if(temp32_value>0)
						{
							temp_obj->var = (CELL*)malloc(sizeof(int)*temp32_value);
							if(temp_obj->var==NULL)
							{
								error = VM_MALLOC_NEW_ERR;
								if(errSp!=NULL)
								{
									sp++;
									sp->ival = error;
									pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							memset(temp_obj->var,0,sizeof(int)*temp32_value);
						}
						if(temp16_value>0)
						{
							temp_obj->parent = (CELL*)malloc(sizeof(int)*temp16_value);
							if(temp_obj->parent==NULL)
							{
								error = VM_MALLOC_NEW_ERR;
								if(errSp!=NULL)
								{
									sp++;
									sp->ival = error;
									pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							memset(temp_obj->parent,0,sizeof(int)*temp16_value);
						}
						sp->addr = temp_obj;
						pc=pc+10;break;
				case NewModule:  
						sp++;
						temp32_value = *(uint32_t*)(pc+1);
						temp_obj=(Obj*)malloc(sizeof(Obj));
						if(temp_obj==NULL)
						{
							error = VM_MALLOC_NEW_ERR;
							if(errSp!=NULL)
							{
								sp++;
								sp->ival = error;
								pc = pcBaseAddr + errSp->addr;
								break;
							}
							else
								goto VM_ERROR;
						}
						memset(temp_obj,0,sizeof(Obj));
						temp_obj->classId = *(uint32_t*)(pc+5);
						temp_obj->data = (CELL*)data;
						if(temp32_value>0)
						{
							temp_obj->var = (CELL*)malloc(sizeof(int)*temp32_value);
							if(temp_obj->var==NULL)
							{
								error = VM_MALLOC_NEW_ERR;
								if(errSp!=NULL)
								{
									sp++;
									sp->ival = error;
									pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							memset(temp_obj->var,0,sizeof(int)*temp32_value);
						}
						sp->addr = temp_obj;
						pc=pc+9;break;
			//object deletion
			case Free:  
						temp_obj = (Obj*)sp->addr;
						if(temp_obj->var!=NULL) free(temp_obj->var);
						if(temp_obj->parent!=NULL)
							for(temp16_value=0;(temp_obj->parent+temp16_value)->addr!=NULL;temp16_value++)
							{
								temp_obj1 = (Obj*)((temp_obj->parent+temp16_value)->addr);
								if(temp_obj1->var!=NULL) free(temp_obj1->var);
								free(temp_obj1);
							}
						free(temp_obj);
						pc++;
						sp--;
						break;
			//object invoke
			case PushObject:
						if(sp->addr!=NULL)
						{
							tempInvoke = malloc(sizeof(Invoke));
							tempInvoke->prevLink = (CELL*)invoke;
							tempInvoke->data = (Obj*)sp->addr;
							data = (Obj*)sp->addr;
							invoke = tempInvoke;
							sp--;
						}
						else
						{
							error = VM_NULL_POINTER_ERR;
							if(errSp!=NULL)
							{
								sp++;
								sp->ival = error;
								pc = pcBaseAddr + errSp->addr;
								break;
							}
							else
								goto VM_ERROR;
						}
						pc++;
						break;
			case PushObjectCall:
							temp16_value = *(pc+1);
							if((sp-temp16_value)->addr!=NULL)
							{
								tempInvoke = malloc(sizeof(Invoke));
								tempInvoke->prevLink = (CELL*)invoke;
								data = (Obj*)(sp-temp16_value)->addr;
								tempInvoke->data = data;
								invoke = tempInvoke;
							}
							else
							{
								error = VM_NULL_POINTER_ERR;
								if(errSp!=NULL)
								{
									sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
									break;
								}
								else
									goto VM_ERROR;
							}
							pc=pc+2;
				      break;
			case PopObject: 
						if(data!=NULL)
						{
							tempInvoke = invoke;
							invoke = (Invoke*)tempInvoke->prevLink;
							data = invoke->data;
							free(tempInvoke);
						}
						else
						{
							error = VM_NULL_POINTER_ERR;
							if(errSp!=NULL)
							{
								sp++;
								sp->ival = error;
								pc = pcBaseAddr + errSp->addr;
								break;
							}
							else
								goto VM_ERROR;
						}
						pc++;
						break;
			//array initialization
			case NewBit8:   
							if(sp->ival>0)
							{
								temp_array = (Array*)malloc(sizeof(Array));
								if(temp_array == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;
										sp->ival = error;
										pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp32_value = sp->ival*sizeof(int8_t);
								temp_array->addr = (int8_t*)malloc(temp32_value);
								if(temp_array->addr == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;
										sp->ival = error;
										pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp_array->len = sp->ival;
								sp->addr = temp_array;
								memset(temp_array->addr,0,temp32_value);
							}
							else
								sp->ival = 0;
							pc++;
							break;
			case NewBit16:  
							if(sp->ival>0)
							{
								temp_array = (Array*)malloc(sizeof(Array));
								if(temp_array == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;
										sp->ival = error;
										pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp32_value = sp->ival*sizeof(int16_t);
								temp_array->addr = (int16_t*)malloc(temp32_value);
								if(temp_array->addr == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;
										sp->ival = error;
										pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp_array->len = sp->ival;
								sp->addr = temp_array;
								memset(temp_array->addr,0,temp32_value);
							}
							else
								sp->ival = 0;
							pc++;
							break;
			case NewBit32:  
							if(sp->ival>0)
							{
								temp_array = (Array*)malloc(sizeof(Array));
								if(temp_array == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;
										sp->ival = error;
										pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp32_value = sp->ival*sizeof(int32_t);
								temp_array->addr = (int32_t*)malloc(temp32_value);
								if(temp_array->addr == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;
										sp->ival = error;
										pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp_array->len = sp->ival;
								sp->addr = temp_array;
								memset(temp_array->addr,0,temp32_value);
							}
							else
								sp->ival = 0;
							pc++;
							break;
			case NewBit64:  
							if(sp->ival>0)
							{
								temp_array = (Array*)malloc(sizeof(Array));
								if(temp_array == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp32_value = sp->ival*sizeof(int64_t);
								temp_array->addr = (int64_t*)malloc(temp32_value);
								if(temp_array->addr == NULL)
								{
									error = VM_MALLOC_NEW_ERR;
									if(errSp!=NULL)
									{
										sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
										break;
									}
									else
										goto VM_ERROR;
								}
								temp_array->len = sp->ival;
								sp->addr = temp_array;
								memset(temp_array->addr,0,temp32_value);
							}
							else
								sp->ival = 0;
							pc++;
							break;
			//array storage
			case LoadBit8Array:  
				if((sp-1)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-1)->addr;
				temp32_value = (int32_t)(sp->ival);
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				(sp-1)->ival = *((int8_t*)temp_array->addr + temp32_value);pc++;sp--;break;
			case StoreBit8Array: 
				if((sp-2)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-2)->addr;
				temp32_value = (int32_t)((sp-1)->ival);
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				*((int8_t*)temp_array->addr + temp32_value)=sp->ival;pc++;sp=sp-3;break;
			case LoadBit16Array: 
				if((sp-1)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-1)->addr;
				temp32_value = (int32_t)(sp->ival);
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				(sp-1)->ival = *((int16_t*)temp_array->addr + temp32_value);pc++;sp--;break;
			case StoreBit16Array: 
				if((sp-2)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-2)->addr;
				temp32_value = (int32_t)((sp-1)->ival);
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				*((int16_t*)temp_array->addr + temp32_value)=sp->ival;pc++;sp=sp-3;break;
			case LoadBit32Array: 
				if((sp-1)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-1)->addr;
				temp32_value = (int32_t)(sp->ival);
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				(sp-1)->ival = *((int32_t*)temp_array->addr + temp32_value);pc++;sp--;break;
			case StoreBit32Array: 
				if((sp-2)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-2)->addr;
				temp32_value = (int32_t)((sp-1)->ival);
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				*((int32_t*)temp_array->addr + temp32_value)=sp->ival;pc++;sp=sp-3;break;
			case LoadBit64Array: 
				--sp; 
				if(sp->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = sp->addr;
				temp32_value = (sp+1)->ival;
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				*(int64_t*)sp = *(((int64_t*)temp_array->addr) + temp32_value); ++pc; ++sp; break;
			case StoreBit64Array:
				if((sp-3)->addr==NULL) 
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				temp_array = (sp-3)->addr;
				temp32_value = (sp-2)->ival;
				if(temp32_value>=temp_array->len)
				{
					error = VM_OUT_OF_BOUNDS;
					if(errSp!=NULL)
					{
						sp++;sp->ival = error;pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				*(((int64_t*)(temp_array->addr)) + temp32_value) = *(int64_t*)(sp-1); sp -= 4; ++pc; break;
			
			//string pool
			case LoadStringPool:
								temp32_value = *(uint32_t*)(pc+1);
								sp++;
								sp->addr = strpool + temp32_value;
								pc=pc+5;
								break;
			//static memory storage
			case LoadBit32Static: ++sp;sp->ival=*(int32_t*)(pStatic+*(uint32_t*)(pc+1));pc=pc+5;break;
			case StoreBit32Static:*(int32_t*)(pStatic+*(uint32_t*)(pc+1))=sp->ival;sp--;pc=pc+5;break;
			case LoadBit64Static: ++sp;sp->ival=*(int64_t*)(pStatic+*(uint32_t*)(pc+1));pc=pc+5;break;
			case StoreBit64Static:*(int64_t*)(pStatic+*(uint32_t*)(pc+1))=sp->ival;sp--;pc=pc+5;break;
			case LoadBit16Static: ++sp;sp->ival=(pStatic+*(uint32_t*)(pc+1))->ival&CAST_BIT16;pc=pc+5;break;
			case StoreBit16Static:(pStatic+*(uint32_t*)(pc+1))->ival=sp->ival&CAST_BIT16;sp--;pc=pc+5;break;
			case LoadBit8Static:  ++sp;sp->ival=(pStatic+*(uint32_t*)(pc+1))->ival&CAST_BIT8;pc=pc+5;break;
			case StoreBit8Static: (pStatic+*(uint32_t*)(pc+1))->ival=sp->ival&CAST_BIT8;sp--;pc=pc+5;break;
			case Pop: *(sp-1) = *sp;sp--;pc++;break;
			//stack manipulation
			case Dup:(sp+1)->ival = sp->ival;sp++;pc++;break;
			case Sup:sp++;pc++;break;
			case Sup2:sp+=2;pc++;break;
			case Sdn:sp--;pc++;break;
			case Sdn2:sp-=2;pc++;break;
			//get method physical address
			case GetAddr: sp++;sp->ival=*(uint32_t*)(pc+1);pc=pc+5;break;
			//cal abstract method
			case CallAbstract:
					if(data->data!=NULL)
					{
						tempInvoke = malloc(sizeof(Invoke));
						tempInvoke->prevLink = (CELL*)invoke;
						tempInvoke->data = data;
						data = (Obj*)(data->data);
						invoke = tempInvoke;
					}
					else
					{
						error = VM_NULL_POINTER_ERR;
						if(errSp!=NULL)
						{
							sp++;
							sp->ival = error;
							pc = pcBaseAddr + errSp->addr;
							break;
						}
						else
							goto VM_ERROR;
					}
					temp16_value = (uint16_t)(sp->ival);
					sp--;
					temp32_value = (uint32_t)(sp->ival);
					if(temp32_value==0) 
					{
					  	error = VM_INVALID_JUMP_ADDRESS;
						if(errSp!=NULL)
						{
							sp++;
							sp->ival = error;
							pc = pcBaseAddr + errSp->addr;
							break;
						}
						else
							goto VM_ERROR;
					}
					sp--;
					temp_param = param;
					temp_local = local;
					param = (sp-(*(pc+1))+1);
					sp++;
					local=sp;
					sp=sp+(temp16_value);
					sp->addr=pc+2;
					sp++;
					sp->ival=*(pc+1);
					sp++;
					sp->ival=temp16_value;	
					sp++;
					sp->addr = temp_param;
					sp++;
					sp->addr = temp_local;
					pc=pcBaseAddr+(uint32_t)(temp32_value);
					break;
			//load current data pointer
			case LoadData:++sp;sp->addr = data;pc++;break;
			//create try-catch stack
			case NewTrycatch:
				temp_errSp = (CELL*)errSp;
				errSp=(Trycatch*)malloc(sizeof(Trycatch));
				if(errSp==NULL)
				{
					error = VM_MALLOC_NEW_ERR;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = error;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				errSp->sp = sp;
				errSp->prevTry = (CELL*)temp_errSp;
				errSp->addr = *(int16_t*)(pc+1);
				pc+=3;
				break;
			//delete try-catch stack
			case FreeTrycatch:
				temp_errSp = (CELL*)errSp;
				sp = errSp->sp;
				errSp = (Trycatch*)errSp->prevTry;
				free(temp_errSp->addr);
				pc++;
				break;
			//terminate current vm
			case Terminate:
				error = sp->ival;
				if(errSp!=NULL)
				{
					sp++;
					sp->ival = error;
					pc = pcBaseAddr + errSp->addr;
					break;
				}
				else
					goto VM_ERROR;
			
			//call interrupt method. The way you call this method can be customized.
			case CallInterrupt:
				intVm = (MicroVM*)malloc(sizeof(MicroVM));
				intVm->pOpcodeAddr = vm->pOpcodeAddr;
				intVm->pDataAddr = data;
				intVm->pStaticAddr = vm->pStaticAddr;
				intVm->pStrPool = vm->pStrPool;
				intVm->nativeTable = vm->nativeTable;
				if(intVm->stackSize == 0)
					intVm->stackSize = THREAD_STACK_SIZE;
				intVm->pcOffset = *(uint32_t*)(pc+1);
				intVm->stackOffset = *(uint16_t*)(pc+5);
				intVm->pStackAddr = (CELL*)malloc(intVm->stackSize);
				if(intVm->pStackAddr==NULL)
				{
					error = VM_MALLOC_NEW_ERR;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = error;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				else
				{		
					intVm->handle = CreateThread(NULL, 512, vmRun, intVm, 0, NULL);
				}
				pc+=7;
				break;
			
			//return from interrupt method. This can be customized in your way.
			case ReturnI:
				free(vm->pStackAddr);
				free(invoke);
				CloseHandle(vm->handle);
				error = VM_EXIT;
				goto GOTO_VM_EXIT;
			//object casting
			case ObjToClass:
				if(sp->addr==NULL)
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = error;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				sp->ival = ((Obj*)sp->addr)->classId;
				pc++;
				break;
			case ObjToObj:
				if((sp-1)->addr==NULL)
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = error;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				if(((Obj*)(sp-1)->addr)->classId!=sp->ival)
				{
					error = VM_INVALID_OBJ_CAST;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = error;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				sp--;
				pc++;
				break;
			//get array length
			case LoadArrayLen:
				if(sp->addr == NULL)
				{
					error = VM_NULL_POINTER_ERR;
					if(errSp!=NULL)
					{
						sp++;
						sp->ival = error;
						pc = pcBaseAddr + errSp->addr;
						break;
					}
					else
						goto VM_ERROR;
				}
				sp->ival = ((Array*)sp->addr)->len;
				pc++;
				break;
			//module storage
			case LoadModule:
				++sp;
				sp->ival=*(int32_t*)(((Obj*) data)->parent+*(uint8_t*)(pc+1));
				pc+=2;break;
			case StoreModule:
				*(int32_t*)(((Obj*) data)->parent+ *(uint8_t*)(pc+1))=sp->ival;
				sp--;
				pc+=2;break;
			default:
				error = VM_UNKNOWN_OPCODE_ERR;
				goto VM_ERROR;
	   	}
	}
	VM_ERROR:
	printf("Error code = %d\r\n",error);
	printf("pc = %d\r\n",*pc);
	GOTO_VM_EXIT:
	free(invoke);
	if(vm->stackSize)
		free(vm->pStackAddr);
	if(vm->staticSize)
		free(vm->pStackAddr);
	free(vm);
	return error;
}

extern NativeMethod* nativeTable[];

//customize your way to load executable file
int vmLoadFile(MicroVM* vm)
{
	FILE *file;
	size_t result;
	long lSize;
	uint8_t *buffer;
	
	//load opcodes
	file = fopen("d:/UOL/compiler_future/UOL_Lib/output.bin","rb");
	if (file == NULL) 
	{
		printf("can't open file\r\n");
		return VM_FILE_OPEN_ERR;
	}

	//get opcode size
	fseek (file , 0 , SEEK_END);
	lSize = ftell (file);
	rewind (file);
	buffer = (uint8_t*) malloc (sizeof(uint8_t)*lSize);
	printf("File size is: %d\r\n",lSize);

	// allocate memory for opcodes
	buffer = (uint8_t*) malloc (sizeof(uint8_t)*lSize);
	if(buffer == NULL)
	{
		return VM_MALLOC_CODE_ERR;
	}

	// read file into memory
	result = fread (buffer,1,lSize,file);
	printf("code size = %d bytes\n",lSize);
	vm->codeSize = lSize;
	vm->pcOffset = VM_HEAD_SIZE;
	if (!result) 
	{
		return VM_FILE_READ_ERR;
	}
	//close file
	fclose(file);
	//initialize opcode address
	vm->pOpcodeAddr = buffer;
	return VM_OK;
}

int vmStart(MicroVM* vm)
{
	uint32_t vm_err;
	uint32_t vmStackSize;
	//load executable file
	vm_err = vmLoadFile(vm);
	//exit if error occurs
	if(vm_err!=VM_OK)
	{
		return vm_err;
	}
	//get stack size
	vmStackSize = *(uint32_t*)(vm->pOpcodeAddr+VM_STACK_OFFSET);

	//allocate memory for stack
	if(vmStackSize==0)
	{
		vm->pStackAddr = (CELL*)malloc(STACK_SIZE);
		vm->stackSize = STACK_SIZE;
		printf("stack size = %d bytes\r\n",STACK_SIZE);
	}
	else
	{
		vm->pStackAddr = (CELL*)malloc(vmStackSize);
		vm->stackSize = vmStackSize;
		printf("stack size = %d bytes\r\n",vmStackSize);
	}
	vm->stackOffset = 0;
	vm->stringClassId= *(uint32_t*)(vm->pOpcodeAddr+VM_STRING_CLASS_TYPE_OFFSET);
	//return stack allocation error
	if (vm->pStackAddr == NULL)
	{
		return VM_MALLOC_STACK_ERR;
	}
	//get native table
	vm->nativeTable = nativeTable;

	//initialize vm
	vm_err = vmInit(vm);
	if(vm_err!=VM_OK)
	{
		return vm_err;
	}
	return vm_err;
}



