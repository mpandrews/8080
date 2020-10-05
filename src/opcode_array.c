#include "../include/cpu.h"
#include "../include/opcode_decls.h"

#include <stdlib.h>

// The array of opcode function pointers!

/*
 * Register field key:
 * 	000 B
 * 	001 C
 * 	010 D
 * 	011 E
 * 	100 H
 * 	101 L
 * 	110 Memory reference through HL
 * 	111 A
 * Flag key:
 * 	000 !Zero
 * 	001 Zero
 * 	010 !Carry
 * 	011 C
 * 	100 !Parity (Odd)
 * 	101 Parity (Even)
 * 	110 !Sign (Positive)
 * 	111 Sign (Negative
 * Register Pair Key:
 * 	00 BC
 * 	01 DE
 * 	10 HL
 * 	11 Stack Pointer, or PSW when push/popping.
 */

/*
 * This is the actual array of opcode functions.  It's a global,
 * with an extern declaration in the accompanying header file.
 */

int (*const opcodes[256])(uint8_t opcode, struct cpu_state* cpu) = {
		nop,	 // 0x00	NOP
		lxi,	 // 0x01	LXI	B
		stax,	 // 0x02	STAX	B
		inx,	 // 0x03	INX	B
		inr,	 // 0x04	INR	B
		dcr,	 // 0x05	DCR	B
		mvi,	 // 0x06	MVI	B
		rlc,	 // 0x07	RLC
		nop,	 // 0x08	NOP
		dad,	 // 0x09	DAD	B
		ldax,	 // 0x0A	LDAX	B
		dcx,	 // 0x0B	DCX	B
		inr,	 // 0x0C	INR	C
		dcr,	 // 0x0D	DCR	C
		mvi,	 // 0x0E	MVI	C
		rrc,	 // 0x0F	RRC
		nop,	 // 0x10	NOP
		lxi,	 // 0x11	LXI	D
		stax,	 // 0x12	STAX	D
		inx,	 // 0x13	INX	D
		inr,	 // 0x14	INR	D
		dcr,	 // 0x15	DCR	D
		mvi,	 // 0x16	MVI	D
		ral,	 // 0x17	RAL
		nop,	 // 0x18	NOP
		dad,	 // 0x19	DAD	D
		ldax,	 // 0x1A	LDAX	D
		dcx,	 // 0x1A	DCX	D
		inr,	 // 0x1C	INR	E
		dcr,	 // 0x1D	DCR	E
		mvi,	 // 0x1E	MVI	E
		rar,	 // 0x1F	RAR
		nop,	 // 0x20	NOP
		lxi,	 // 0x21	LXI	H
		shld,	 // 0x22	SHLD
		inx,	 // 0x23	INX	H
		inr,	 // 0x24	INR	H
		dcr,	 // 0x25	DCR	H
		mvi,	 // 0x26	MVI	H
		daa,	 // 0x27	DAA
		nop,	 // 0x28	NOP
		dad,	 // 0x29	DAD	H
		lhld,	 // 0x2A	LHLD
		dcx,	 // 0x2B	DCX	H
		inr,	 // 0x2C	INR	L
		dcr,	 // 0x2D	DCR	L
		mvi,	 // 0x2E	MVI	L
		cma,	 // 0x2F	CMA
		nop,	 // 0x30	NOP
		lxi,	 // 0x31	LXI 	SP
		sta,	 // 0x32	STA
		inx,	 // 0x33	INX 	SP
		inr,	 // 0x34	INR	M
		dcr,	 // 0x35	DCR	M
		mvi,	 // 0x36	MVI	M
		stc,	 // 0x37	STC
		nop,	 // 0x38	NOP
		dad,	 // 0x39	DAD	SP
		lda,	 // 0x3A	LDA
		dcx,	 // 0x3B	DCX	SP
		inr,	 // 0x3C	INR	A
		dcr,	 // 0x3D	DCR	A
		mvi,	 // 0x3E	MVI	A
		cmc,	 // 0x3F	CMC
		mov,	 // 0x40	MOV	B,B
		mov,	 // 0x41	MOV	B,C
		mov,	 // 0x42	MOV	B,D
		mov,	 // 0x43	MOV	B,E
		mov,	 // 0x44	MOV	B,H
		mov,	 // 0x45	MOV	B,L
		mov,	 // 0x46	MOV	B,M
		mov,	 // 0x47	MOV	B,A
		mov,	 // 0x48	MOV	C,B
		mov,	 // 0x49	MOV	C,C
		mov,	 // 0x4A	MOV	C,D
		mov,	 // 0x4B	MOV	C,E
		mov,	 // 0x4C	MOV	C,H
		mov,	 // 0x4D	MOV	C,L
		mov,	 // 0x4E	MOV	C,M
		mov,	 // 0x4F	MOV	C,A
		mov,	 // 0x50	MOV	D,B
		mov,	 // 0x51	MOV	D,C
		mov,	 // 0x52	MOV	D,D
		mov,	 // 0x53	MOV	D,E
		mov,	 // 0x54	MOV	D,H
		mov,	 // 0x55	MOV	D,L
		mov,	 // 0x56	MOV	D,M
		mov,	 // 0x57	MOV	D,A
		mov,	 // 0x58	MOV	E,B
		mov,	 // 0x59	MOV	E,C
		mov,	 // 0x5A	MOV	E,D
		mov,	 // 0x5B	MOV	E,E
		mov,	 // 0x5C	MOV	E,H
		mov,	 // 0x5D	MOV	E,L
		mov,	 // 0x5E	MOV	E,M
		mov,	 // 0x5F	MOV	E,A
		mov,	 // 0x60	MOV	H,B
		mov,	 // 0x61	MOV	H,C
		mov,	 // 0x62	MOV	H,D
		mov,	 // 0x63	MOV	H,E
		mov,	 // 0x64	MOV	H,H
		mov,	 // 0x65	MOV	H,L
		mov,	 // 0x66	MOV	H,M
		mov,	 // 0x67	MOV	H,A
		mov,	 // 0x68	MOV	L,B
		mov,	 // 0x69	MOV	L,C
		mov,	 // 0x6A	MOV	L,D
		mov,	 // 0x6B	MOV	L,E
		mov,	 // 0x6C	MOV	L,H
		mov,	 // 0x6D	MOV	L,L
		mov,	 // 0x6E	MOV	L,M
		mov,	 // 0x6F	MOV	L,A
		mov,	 // 0x70	MOV	M,B
		mov,	 // 0x71	MOV	M,C
		mov,	 // 0x72	MOV	M,D
		mov,	 // 0x73	MOV	M,E
		mov,	 // 0x74	MOV	M,H
		mov,	 // 0x75	MOV	M,L
		hlt,	 // 0x76	HLT
		mov,	 // 0x77	MOV	M,A
		mov,	 // 0x78	MOV	A,B
		mov,	 // 0x79	MOV	A,C
		mov,	 // 0x7A	MOV	A,D
		mov,	 // 0x7B	MOV	A,E
		mov,	 // 0x7C	MOV	A,H
		mov,	 // 0x7D	MOV	A,L
		mov,	 // 0x7E	MOV	A,M
		mov,	 // 0x7F	MOV	A,A
		add,	 // 0x80	ADD	B
		add,	 // 0x81	ADD	C
		add,	 // 0x82	ADD	D
		add,	 // 0x83	ADD	E
		add,	 // 0x84	ADD	H
		add,	 // 0x85	ADD	L
		add,	 // 0x86	ADD	M
		add,	 // 0x87	ADD	A
		adc,	 // 0x88	ADC	B
		adc,	 // 0x89 	ADC	C
		adc,	 // 0x8A 	ADC	D
		adc,	 // 0x8B 	ADC	E
		adc,	 // 0x8C 	ADC	H
		adc,	 // 0x8D 	ADC	L
		adc,	 // 0x8E 	ADC	M
		adc,	 // 0x8F 	ADC	A
		sub,	 // 0x90	SUB	B
		sub,	 // 0x91	SUB	C
		sub,	 // 0x92	SUB	D
		sub,	 // 0x93	SUB	E
		sub,	 // 0x94	SUB	H
		sub,	 // 0x95	SUB	L
		sub,	 // 0x96	SUB	M
		sub,	 // 0x97	SUB	A
		sbb,	 // 0x98	SBB	B
		sbb,	 // 0x99	SBB	C
		sbb,	 // 0x9A	SBB	D
		sbb,	 // 0x9B	SBB	E
		sbb,	 // 0x9C	SBB	H
		sbb,	 // 0x9D	SBB	L
		sbb,	 // 0x9E	SBB	M
		sbb,	 // 0x9F	SBB	A
		ana,	 // 0xA0	ANA	B
		ana,	 // 0xA1	ANA	C
		ana,	 // 0xA2	ANA	D
		ana,	 // 0xA3	ANA	E
		ana,	 // 0xA4	ANA	H
		ana,	 // 0xA5	ANA	L
		ana,	 // 0xA6	ANA	M
		ana,	 // 0xA7	ANA	A
		xra,	 // 0xA8	XRA	B
		xra,	 // 0xA9	XRA	C
		xra,	 // 0xAA	XRA	D
		xra,	 // 0xAB	XRA	E
		xra,	 // 0xAC	XRA	H
		xra,	 // 0xAD	XRA	L
		xra,	 // 0xAE	XRA	M
		xra,	 // 0xAF	XRA	A
		ora,	 // 0xB0	ORA	B
		ora,	 // 0xB1	ORA	C
		ora,	 // 0xB2	ORA	D
		ora,	 // 0xB3	ORA	E
		ora,	 // 0xB4	ORA	H
		ora,	 // 0xB5	ORA	L
		ora,	 // 0xB6	ORA	M
		ora,	 // 0xB7	ORA	A
		cmp,	 // 0xB8	CMP	B
		cmp,	 // 0xB9	CMP	C
		cmp,	 // 0xBA	CMP	D
		cmp,	 // 0xBB	CMP	E
		cmp,	 // 0xBC	CMP	H
		cmp,	 // 0xBD	CMP	L
		cmp,	 // 0xBE	CMP	M
		cmp,	 // 0xBF	CMP	A
		retcond, // 0xC0	RNZ
		pop,	 // 0xC1	POP	B
		jcond,	 // 0xC2	JNZ
		jmp,	 // 0xC3	JMP
		ccond,	 // 0xC4	CNZ
		push,	 // 0xC5	PUSH	B
		adi,	 // 0xC6	ADI
		rst,	 // 0xC7	RST	0
		retcond, // 0xC8	RZ
		ret,	 // 0xC9	RET
		jcond,	 // 0xCA	JZ
		jmp,	 // 0xCB	JMP
		ccond,	 // 0xCC	CZ
		call,	 // 0xCD	CALL
		aci,	 // 0xCE	ACI
		rst,	 // 0xCF	RST	1
		retcond, // 0xD0	RNC
		pop,	 // 0xD1	POP	D
		jcond,	 // 0xD2	JNC
		out,	 // 0xD3	OUT
		ccond,	 // 0xD4	CNC
		push,	 // 0xD5	PUSH	D
		sui,	 // 0xD6	SUI
		rst,	 // 0xD7	RST	2
		retcond, // 0xD8	RC
		ret,	 // 0xD9	RET
		jcond,	 // 0xDA	JC
		in,	 // 0xDB	IN
		ccond,	 // 0xDC	CC
		call,	 // 0xDD	CALL
		sbi,	 // 0xDE	SBI
		rst,	 // 0xDF	RST	3
		retcond, // 0xE0	RPO
		pop,	 // 0xE1	POP	H
		jcond,	 // 0xE2	JPO
		xthl,	 // 0xE3	XTHL
		ccond,	 // 0xE4	CPO
		push,	 // 0xE5	PUSH	H
		ani,	 // 0xE6	ANI
		rst,	 // 0xE7	RST	4
		retcond, // 0xE8	RPE
		pchl,	 // 0xE9	PCHL
		jcond,	 // 0xEA	JPE
		xchg,	 // 0xEB	XCHG
		ccond,	 // 0xEC	CPE
		call,	 // 0xED	CALL
		xri,	 // 0xEE	XRI
		rst,	 // 0xEF	RST	5
		retcond, // 0xF0	RP
		pop,	 // 0xF1	POP	PSW
		jcond,	 // 0xF2	JP
		di,	 // 0xF3	DI
		ccond,	 // 0xF4	CP
		push,	 // 0xF5	PUSH	PSW
		ori,	 // 0xF6	ORI
		rst,	 // 0xF7	RST	6
		retcond, // 0xF8	RM
		sphl,	 // 0xF9	SPHL
		jcond,	 // 0xFA	JM
		ei,	 // 0xFB	EI
		ccond,	 // 0xFC	CM
		call,	 // 0xFD	CALL
		ccond,	 // 0xFE	CPI
		rst,	 // 0xFF	RST	7
};
