#ifndef OPCODE_SIZE
#define OPCODE_SIZE

#include <stdint.h>

static inline int get_opcode_size(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x01: // LXI B
	case 0x11: // LXI D
	case 0x21: // LXI H
	case 0x22: // SHLD
	case 0x2a: // LHDL
	case 0x31: // LXI SP
	case 0x32: // STA
	case 0x3a: // LDA
	case 0xc2: // JNZ
	case 0xc3: // JMP
	case 0xc4: // CNZ
	case 0xca: // JZ
	case 0xcb: // JMP
	case 0xcc: // CZ
	case 0xcd: // CALL
	case 0xd2: // JNC
	case 0xd4: // CNC
	case 0xda: // JC
	case 0xdc: // CC
	case 0xdd: // CALL
	case 0xe2: // JPO
	case 0xe4: // CPO
	case 0xea: // JPE
	case 0xec: // CPE
	case 0xed: // CALL
	case 0xf2: // JP
	case 0xf4: // CP
	case 0xfa: // JM
	case 0xfc: // CM
	case 0xfd: // CALL
		return 3;
	case 0x06: // MVI B
	case 0x0e: // MVI C
	case 0x16: // MVI D
	case 0x1e: // MVI E
	case 0x26: // MVI H
	case 0x2e: // MVI L
	case 0x36: // MVI M
	case 0x3e: // MVI A
	case 0xc6: // ADI
	case 0xce: // ACI
	case 0xd3: // OUT
	case 0xd6: // SUI
	case 0xdb: // IN
	case 0xde: // SBI
	case 0xe6: // ANI
	case 0xee: // XRI
	case 0xf6: // ORI
	case 0xfe: // CPI
		return 2;
	default: return 1;
	}
} //__attribute__((pure))

#endif
