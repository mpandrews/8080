#include "../include/cpu.h"
#include "../include/opcodes.h"

//The array of opcode function pointers!

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

int (* const opcodes[256]) (uint8_t opcode, struct cpu_state* cpu) =
{
	NULL, //0x00	NOP
	NULL, //0x01	LXI	B
	NULL, //0x02	STAX	B
	NULL, //0x03	INX	B
	NULL, //0x04	INR	B
	NULL, //0x05	DCR	B
	NULL, //0x06	MVI	B
	NULL, //0x07	RLC
	NULL, //0x08	NOP
	NULL, //0x09	DAD	B
	NULL, //0x0A	LDAX	B
	NULL, //0x0B	DCX	B
	NULL, //0x0C	INR	C
	NULL, //0x0D	DCR	C
	mvi, //0x0E	MVI	C
	NULL, //0x0F	RRC
	NULL, //0x10	NOP
	NULL, //0x11	LXI	D
	NULL, //0x12	STAX	D
	NULL, //0x13	INX	D
	NULL, //0x14	INR	D
	NULL, //0x15	DCR	D
	mvi, //0x16	MVI	D
	NULL, //0x17	RAL
	NULL, //0x18	NOP
	NULL, //0x19	DAD	D
	NULL, //0x1A	LDAX	D
	NULL, //0x1A	DCX	D
	NULL, //0x1C	INR	E
	NULL, //0x1D	DCR	E
	mvi, //0x1E	MVI	E
	NULL, //0x1F	RAR
	NULL, //0x20	NOP
	NULL, //0x21	LXI	H
	NULL, //0x22	SHLD
	NULL, //0x23	INX	H
	NULL, //0x24	INR	H
	NULL, //0x25	DCR	H
	mvi, //0x26	MVI	H
	NULL, //0x27	DAA
	NULL, //0x28	NOP
	NULL, //0x29	DAD	H
	NULL, //0x2A	LHLD
	NULL, //0x2B	DCX	H
	NULL, //0x2C	INR	L
	NULL, //0x2D	DCR	L
	mvi, //0x2E	MVI	L
	NULL, //0x2F	CMA
	NULL, //0x30	NOP
	NULL, //0x31	LXI 	SP
	NULL, //0x32	STA
	NULL, //0x33	INX 	SP
	NULL, //0x34	INR	M
	NULL, //0x35	DCR	M
	mvi, //0x36	MVI	M
	NULL, //0x37	STC
	NULL, //0x38	NOP
	NULL, //0x39	DAD	SP
	NULL, //0x3A	LDA
	NULL, //0x3B	DCX	SP
	NULL, //0x3C	INR	A
	NULL, //0x3D	DCR	A
	mvi, //0x3E	MVI	A
	NULL, //0x3F	CMC
	mov, //0x40	MOV	B,B
	mov, //0x41	MOV	B,C
	mov, //0x42	MOV	B,D
	mov, //0x43	MOV	B,E
	mov, //0x44	MOV	B,H
	mov, //0x45	MOV	B,L
	mov, //0x46	MOV	B,M
	mov, //0x47	MOV	B,A
	mov, //0x48	MOV	C,B
	mov, //0x49	MOV	C,C
	mov, //0x4A	MOV	C,D
	mov, //0x4B	MOV	C,E
	mov, //0x4C	MOV	C,H
	mov, //0x4D	MOV	C,L
	mov, //0x4E	MOV	C,M
	mov, //0x4F	MOV	C,A
	mov, //0x50	MOV	D,B
	mov, //0x51	MOV	D,C
	mov, //0x52	MOV	D,D
	mov, //0x53	MOV	D,E
	mov, //0x54	MOV	D,H
	mov, //0x55	MOV	D,L
	mov, //0x56	MOV	D,M
	mov, //0x57	MOV	D,A
	mov, //0x58	MOV	E,B
	mov, //0x59	MOV	E,C
	mov, //0x5A	MOV	E,D
	mov, //0x5B	MOV	E,E
	mov, //0x5C	MOV	E,H
	mov, //0x5D	MOV	E,L
	mov, //0x5E	MOV	E,M
	mov, //0x5F	MOV	E,A
	mov, //0x60	MOV	H,B
	mov, //0x61	MOV	H,C
	mov, //0x62	MOV	H,D
	mov, //0x63	MOV	H,E
	mov, //0x64	MOV	H,H
	mov, //0x65	MOV	H,L
	mov, //0x66	MOV	H,M
	mov, //0x67	MOV	H,A
	mov, //0x68	MOV	L,B
	mov, //0x69	MOV	L,C
	mov, //0x6A	MOV	L,D
	mov, //0x6B	MOV	L,E
	mov, //0x6C	MOV	L,H
	mov, //0x6D	MOV	L,L
	mov, //0x6E	MOV	L,M
	mov, //0x6F	MOV	L,A
	mov, //0x70	MOV	M,B
	mov, //0x71	MOV	M,C
	mov, //0x72	MOV	M,D
	mov, //0x73	MOV	M,E
	mov, //0x74	MOV	M,H
	mov, //0x75	MOV	M,L
	NULL, //0x76	HLT
	mov, //0x77	MOV	M,A
	mov, //0x78	MOV	A,B
	mov, //0x79	MOV	A,C
	mov, //0x7A	MOV	A,D
	mov, //0x7B	MOV	A,E
	mov, //0x7C	MOV	A,H
	mov, //0x7D	MOV	A,L
	mov, //0x7E	MOV	A,M
	mov, //0x7F	MOV	A,A
	NULL, //0x80	ADD	B
	NULL, //0x81	ADD	C
	NULL, //0x82	ADD	D
	NULL, //0x83	ADD	E
	NULL, //0x84	ADD	H
	NULL, //0x85	ADD	L
	NULL, //0x86	ADD	M
	NULL, //0x87	ADD	A
	NULL, //0x88	ADC	B
	NULL, //0x89 	ADC	C
	NULL, //0x8A 	ADC	D
	NULL, //0x8B 	ADC	E
	NULL, //0x8C 	ADC	H
	NULL, //0x8D 	ADC	L
	NULL, //0x8E 	ADC	M
	NULL, //0x8F 	ADC	A
	NULL, //0x90	SUB	B
	NULL, //0x91	SUB	C
	NULL, //0x92	SUB	D
	NULL, //0x93	SUB	E
	NULL, //0x94	SUB	H
	NULL, //0x95	SUB	L
	NULL, //0x96	SUB	M
	NULL, //0x97	SUB	A
	NULL, //0x98	SBB	B
	NULL, //0x99	SBB	C
	NULL, //0x9A	SBB	D
	NULL, //0x9B	SBB	E
	NULL, //0x9C	SBB	H
	NULL, //0x9D	SBB	L
	NULL, //0x9E	SBB	M
	NULL, //0x9F	SBB	A
	NULL, //0xA0	ANA	B
	NULL, //0xA1	ANA	C
	NULL, //0xA2	ANA	D
	NULL, //0xA3	ANA	E
	NULL, //0xA4	ANA	H
	NULL, //0xA5	ANA	L
	NULL, //0xA6	ANA	M
	NULL, //0xA7	ANA	A
	NULL, //0xA8	XRA	B
	NULL, //0xA9	XRA	C
	NULL, //0xAA	XRA	D
	NULL, //0xAB	XRA	E
	NULL, //0xAC	XRA	H
	NULL, //0xAD	XRA	L
	NULL, //0xAE	XRA	M
	NULL, //0xAF	XRA	A
	NULL, //0xB0	ORA	B
	NULL, //0xB1	ORA	C
	NULL, //0xB2	ORA	D
	NULL, //0xB3	ORA	E
	NULL, //0xB4	ORA	H
	NULL, //0xB5	ORA	L
	NULL, //0xB6	ORA	M
	NULL, //0xB7	ORA	A
	NULL, //0xB8	CMP	B
	NULL, //0xB9	CMP	C
	NULL, //0xBA	CMP	D
	NULL, //0xBB	CMP	E
	NULL, //0xBC	CMP	H
	NULL, //0xBD	CMP	L
	NULL, //0xBE	CMP	M
	NULL, //0xBF	CMP	A
	NULL, //0xC0	RNZ
	NULL, //0xC1	POP	B
	NULL, //0xC2	JNZ
	NULL, //0xC3	JMP
	NULL, //0xC4	CNZ
	NULL, //0xC5	PUSH	B
	NULL, //0xC6	ADI
	NULL, //0xC7	RST	0
	NULL, //0xC8	RZ
	NULL, //0xC9	RET
	NULL, //0xCA	JZ
	NULL, //0xCB	JMP
	NULL, //0xCC	CZ
	NULL, //0xCD	CALL
	NULL, //0xCE	ACI
	NULL, //0xCF	RST	1
	NULL, //0xD0	RNC
	NULL, //0xD1	POP	D
	NULL, //0xD2	JNC
	NULL, //0xD3	OUT
	NULL, //0xD4	CNC
	NULL, //0xD5	PUSH	D
	NULL, //0xD6	SUI
	NULL, //0xD7	RST	2
	NULL, //0xD8	RC
	NULL, //0xD9	RET
	NULL, //0xDA	JC
	NULL, //0xDB	IN
	NULL, //0xDC	CC
	NULL, //0xDD	CALL
	NULL, //0xDE	SBI
	NULL, //0xDF	RST	3
	NULL, //0xE0	RPO
	NULL, //0xE1	POP	H
	NULL, //0xE2	JPO
	NULL, //0xE3	XTHL
	NULL, //0xE4	CPO
	NULL, //0xE5	PUSH	H
	NULL, //0xE6	ANI
	NULL, //0xE7	RST	4
	NULL, //0xE8	RPE
	NULL, //0xE9	PCHL
	NULL, //0xEA	JPE
	NULL, //0xEB	XCHG
	NULL, //0xEC	CPE
	NULL, //0xED	CALL
	NULL, //0xEE	XRI
	NULL, //0xEF	RST	5
	NULL, //0xF0	RP
	NULL, //0xF1	POP	PSW
	NULL, //0xF2	JP
	NULL, //0xF3	DI
	NULL, //0xF4	CP
	NULL, //0xF5	PUSH	PSW
	NULL, //0xF6	ORI
	NULL, //0xF7	RST	6
	NULL, //0xF8	RM
	NULL, //0xF9	SPHL
	NULL, //0xFA	JM
	NULL, //0xFB	EI
	NULL, //0xFC	CM
	NULL, //0xFD	CALL
	NULL, //0xFE	CPI
	NULL, //0xFF	RST	7
};
