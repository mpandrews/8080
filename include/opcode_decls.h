#ifndef OPCODE_DECLS
#define OPCODE_DECLS

#include "cpu.h"

#include <stdint.h>

// Individual opcodes are listed here in the same order they appear in the
// 8080 manual.

// Special placeholder function, to catch unimplemented
// opcodes without segfaulting. Definition is in other_opcodes.c
int placeholder(const uint8_t*, struct cpu_state*);

// DATA TRANSFER GROUP

int mov(const uint8_t*, struct cpu_state*);
int mvi(const uint8_t*, struct cpu_state*);
int lxi(const uint8_t*, struct cpu_state*);
int lda(const uint8_t*, struct cpu_state*);
int sta(const uint8_t*, struct cpu_state*);
int lhld(const uint8_t*, struct cpu_state*);
int shld(const uint8_t*, struct cpu_state*);
int ldax(const uint8_t*, struct cpu_state*);
int stax(const uint8_t*, struct cpu_state*);
int xchg(const uint8_t*, struct cpu_state*);

// ARITHMETIC GROUP

int add_adc(const uint8_t*, struct cpu_state*);
int adi(const uint8_t*, struct cpu_state*);
int aci(const uint8_t*, struct cpu_state*);
int sub_sbb(const uint8_t*, struct cpu_state*);
int sui_sbi(const uint8_t*, struct cpu_state*);
int sbi(const uint8_t*, struct cpu_state*);
int inr(const uint8_t*, struct cpu_state*);
int dcr(const uint8_t*, struct cpu_state*);
int inx_dcx(const uint8_t*, struct cpu_state*);
int dad(const uint8_t*, struct cpu_state*);
int daa(const uint8_t*, struct cpu_state*);

// LOGICAL GROUP

int ana(const uint8_t*, struct cpu_state*);
int ani(const uint8_t*, struct cpu_state*);
int xra(const uint8_t*, struct cpu_state*);
int xri(const uint8_t*, struct cpu_state*);
int ora(const uint8_t*, struct cpu_state*);
int ori(const uint8_t*, struct cpu_state*);
int cmp(const uint8_t*, struct cpu_state*);
int cpi(const uint8_t*, struct cpu_state*);
int rlc(const uint8_t*, struct cpu_state*);
int rrc(const uint8_t*, struct cpu_state*);
int ral(const uint8_t*, struct cpu_state*);
int rar(const uint8_t*, struct cpu_state*);
int cma(const uint8_t*, struct cpu_state*);
int cmc(const uint8_t*, struct cpu_state*);
int stc(const uint8_t*, struct cpu_state*);

// BRANCH GROUP

int jmp(const uint8_t*, struct cpu_state*);
int jcond(const uint8_t*, struct cpu_state*);
int call(const uint8_t*, struct cpu_state*);
int ccond(const uint8_t*, struct cpu_state*);
int ret(const uint8_t*, struct cpu_state*);
int retcond(const uint8_t*, struct cpu_state*);
int rst(const uint8_t*, struct cpu_state*);
int pchl(const uint8_t*, struct cpu_state*);

// STACK, I/O, AND MACHINE CONTROL (OTHER) GROUP

int push(const uint8_t*, struct cpu_state*);
int pop(const uint8_t*, struct cpu_state*);
int xthl(const uint8_t*, struct cpu_state*);
int sphl(const uint8_t*, struct cpu_state*);
int ei(const uint8_t*, struct cpu_state*);
int di(const uint8_t*, struct cpu_state*);
int hlt(const uint8_t*, struct cpu_state*);
int nop(const uint8_t*, struct cpu_state*);

#endif
