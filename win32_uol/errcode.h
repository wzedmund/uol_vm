// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015

#ifndef ERRCODE_H
#define ERRCODE_H

#define VM_OK 0
#define VM_FILE_OPEN_ERR 1
#define VM_FILE_READ_ERR 2
#define VM_MALLOC_CODE_ERR      		3
#define VM_MALLOC_STACK_ERR					4	
#define VM_MALLOC_INVOKE_ERR				5
#define VM_MALLOC_STRING_POOL_ERR		6
#define VM_MALLOC_STATIC_MEM_ERR		7
#define VM_MALLOC_NEW_ERR						8

#define VM_BAD_CODE_SIZE_ERR					9
#define VM_BAD_STRING_POOL_SIZE_ERR		10
#define VM_BAD_STATIC_MEM_SIZE_ERR		11

#define VM_UNKNOWN_OPCODE_ERR 				12
#define VM_INVALID_NATIVE_ERR					13
#define VM_NULL_POINTER_ERR						14
#define VM_STACK_OVERFLOW_ERR					15
#define VM_INVALID_PARAM_LEN_ERR			16

#define VM_UNKNOWN_ERR                17
#define VM_OUT_OF_BOUNDS              18
#define VM_DIVIDE_BY_ZERO_ERR		  		19
#define VM_EXIT  					  					20
#define VM_INVALID_JUMP_ADDRESS 			21
#define VM_INVALID_OBJ_CAST 					22

#endif
