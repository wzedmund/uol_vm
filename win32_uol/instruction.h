// Copyright (c) 2013 Zhe Wang
// wzedmund@gmail.com

// version 1.0.3
// date 17/02/2015
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#define	Nop	0
#define	LoadBit32Local	1
#define	LoadBit64Local	2
#define	StoreBit32Local	3
#define	StoreBit64Local	4
#define	LoadBit32Param	5
#define	LoadBit64Param	6
#define	StoreBit32Param	7
#define	StoreBit64Param	8
#define	LoadBit32Data	9
#define	LoadBit64Data	10
#define	StoreBit32Data	11
#define	StoreBit64Data	12
#define	LoadBit32Code	 13
#define	LoadBit64Code	 14
#define	StoreBit32Code	15
#define	StoreBit64Code	16
#define	LoadBit8Local	17
#define	LoadBit8Param	 18
#define	LoadBit8Data	19
#define	LoadBit8Code	20
#define	StoreBit8Local	21
#define	StoreBit8Param	22
#define	StoreBit8Data	23
#define	StoreBit8Code	24
#define	LoadBit16Local	25
#define	LoadBit16Param	26
#define	LoadBit16Data	27
#define	LoadBit16Code	28
#define	StoreBit16Local	29
#define	StoreBit16Param	30
#define	StoreBit16Data	31
#define	StoreBit16Code	32
#define	LoadBit8LocalWide	33
#define	LoadBit16LocalWide	34
#define	LoadBit32LocalWide	35
#define	LoadBit64LocalWide	36
#define	StoreBit8LocalWide	37
#define	StoreBit16LocalWide	38
#define	StoreBit32LocalWide	39
#define	StoreBit64LocalWide	40
#define	LoadBit8DataWide        41
#define	LoadBit16DataWide	42
#define	LoadBit32DataWide	43
#define	LoadBit64DataWide	44
#define	StoreBit8DataWide	       45
#define	StoreBit16DataWide	46
#define	StoreBit32DataWide	47
#define	StoreBit64DataWide	48
#define	LoadBit8Array	49
#define	StoreBit8Array	50
#define	LoadBit16Array	51
#define	StoreBit16Array	52
#define	LoadBit32Array	53
#define	StoreBit32Array	54
#define	LoadBit64Array	55
#define	StoreBit64Array	56
#define	LoadBit32Static	57
#define	StoreBit32Static	58
#define	LoadBit64Static	59
#define	StoreBit64Static	60
#define	LoadBit16Static	61
#define	StoreBit16Static	62
#define	LoadBit8Static	63
#define	StoreBit8Static	64
#define	Bit32Add	65
#define	Bit32Sub	66
#define	Bit32Mul	67
#define	Bit32Div	68
#define	Bit32And	69
#define	Bit32Or	70
#define	Bit32Xor	71
#define	Bit32Not	72
#define	Bit32Rem	73
#define	Bit32ShiftLeft	74
#define	Bit32ShiftRight  75
#define	Bit32Inc	76
#define	Bit32Dec	77
#define	Bit32Neg	78
#define	Bit64Add	79
#define	Bit64Sub	80
#define	Bit64Mul	81
#define	Bit64Div	82
#define	Bit64And	83
#define	Bit64Or	84
#define	Bit64Xor	85
#define	Bit64Not	86
#define	Bit64Rem	87
#define	Bit64ShiftLeft	88
#define	Bit64ShiftRight	89
#define	Bit64Inc	90
#define	Bit64Dec	91
#define	Bit64Neg	92
#define	FloatAdd	93
#define	FloatSub	94
#define	FloatMul	95
#define	FloatDiv	96
#define	FloatNeg	97
#define	DoubleAdd	98
#define	DoubleSub	99
#define	DoubleMul	100
#define	DoubleDiv	101
#define	DoubleNeg	102
#define	LongToInt	103
#define	FloatToInt	104
#define	DoubleToInt	105
#define	IntToLong	106
#define	FloatToLong	107
#define	DoubleToLong	108
#define	IntToFloat	109
#define	LongToFloat	110
#define	DoubleToFloat	111
#define	IntToDouble	112
#define	LongToDouble 113
#define	FloatToDouble	114
#define	ObjToObj	115
#define	ObjToClass	116

#define	Jump	117
#define	JumpZero	118
#define	JumpNotZero	119
#define	JumpIntEq	120
#define	JumpIntNotEq	121
#define	JumpIntGt	122
#define	JumpIntLt	123
#define	JumpIntGtEq	124
#define	JumpIntLtEq	125
#define	SJump	126
#define	SJumpZero	127
#define	SJumpNotZero	128
#define	SJumpIntEq	129
#define	SJumpIntNotEq	130
#define	SJumpIntGt	131
#define	SJumpIntLt	132
#define	SJumpIntGtEq	133
#define	SJumpIntLtEq	134
#define	Call	135
#define	CallNative	136
#define	CallAbstract	137
#define	CallInterrupt	138
#define	Return	139
#define	ReturnBit8	140
#define	ReturnBit16	141
#define	ReturnBit32	142
#define	ReturnBit64	143
#define	ReturnI	144
#define	Bit32Eq	145
#define	Bit32NotEq	146
#define	Bit32Gt	147
#define	Bit32GtEq	148
#define	Bit32Lt	149
#define	Bit32LtEq	150
#define	Bit64Eq	151
#define	Bit64NotEq	152
#define	Bit64Gt	153
#define	Bit64GtEq	154
#define	Bit64Lt	155
#define	Bit64LtEq	156
#define	FloatEq	157
#define	FloatNotEq	158
#define	FloatGt	159
#define	FloatGtEq	160
#define	FloatLt	161
#define	FloatLtEq	162
#define	DoubleEq	163
#define	DoubleNotEq	164
#define	DoubleGt	165
#define	DoubleGtEq	166
#define	DoubleLt	167
#define	DoubleLtEq	168

#define	New	169
#define	Free	170
#define	PushObject	171
#define	PushObjectCall	172
#define	PopObject	173

#define	NewBit8	174
#define	NewBit16	175
#define	NewBit32	176
#define	NewBit64	177

#define	Pop	178
#define	Dup	179
#define	Sup	180
#define	Sdn	181
#define	Sup2	182
#define	Sdn2	183

#define	LoadStringPool	184

#define	NewTrycatch	185
#define	FreeTrycatch	186
#define	Terminate	187

#define	GetAddr	188
#define	LoadModule	189
#define	StoreModule	190
#define	LoadData	191
#define	LoadArrayLen	192
#define       NewModule 193
#define       CallNativeWide 194
#define       CallNativeVoid 195		
#endif
