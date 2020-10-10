#ifndef OPCODE_DECLS
#define OPCODE_DECLS

#include "cpu.h"

#include <stdint.h>

// Individual opcodes are listed here in the same order they appear in the
// 8080 manual.

// Special placeholder function, to catch unimplemented
// opcodes without segfaulting. Definition is in other_opcodes.c
int placeholder(uint8_t, struct cpu_state*);

// DATA TRANSFER GROUP

int mov(uint8_t, struct cpu_state*);
int mvi(uint8_t, struct cpu_state*);
int lxi(uint8_t, struct cpu_state*);
int lda(uint8_t, struct cpu_state*);
int sta(uint8_t, struct cpu_state*);
int lhld(uint8_t, struct cpu_state*);
int shld(uint8_t, struct cpu_state*);
int ldax(uint8_t, struct cpu_state*);
int stax(uint8_t, struct cpu_state*);
int xchg(uint8_t, struct cpu_state*);

// ARITHMETIC GROUP

int add_adc(uint8_t, struct cpu_state*);
int adi(uint8_t, struct cpu_state*);
int aci(uint8_t, struct cpu_state*);
int sub(uint8_t, struct cpu_state*);
int sui(uint8_t, struct cpu_state*);
int sbb(uint8_t, struct cpu_state*);
int sbi(uint8_t, struct cpu_state*);
int inr(uint8_t, struct cpu_state*);
int dcr(uint8_t, struct cpu_state*);
int inx(uint8_t, struct cpu_state*);
int dcx(uint8_t, struct cpu_state*);
int dad(uint8_t, struct cpu_state*);
int daa(uint8_t, struct cpu_state*);

// LOGICAL GROUP

int ana(uint8_t, struct cpu_state*);
int ani(uint8_t, struct cpu_state*);
int xra(uint8_t, struct cpu_state*);
int xri(uint8_t, struct cpu_state*);
int ora(uint8_t, struct cpu_state*);
int ori(uint8_t, struct cpu_state*);
int cmp(uint8_t, struct cpu_state*);
int cpi(uint8_t, struct cpu_state*);
int rlc(uint8_t, struct cpu_state*);
int rrc(uint8_t, struct cpu_state*);
int ral(uint8_t, struct cpu_state*);
int rar(uint8_t, struct cpu_state*);
int cma(uint8_t, struct cpu_state*);
int cmc(uint8_t, struct cpu_state*);
int stc(uint8_t, struct cpu_state*);

// BRANCH GROUP

int jmp(uint8_t, struct cpu_state*);
int jcond(uint8_t, struct cpu_state*);
int call(uint8_t, struct cpu_state*);
int ccond(uint8_t, struct cpu_state*);
int ret(uint8_t, struct cpu_state*);
int retcond(uint8_t, struct cpu_state*);
int rst(uint8_t, struct cpu_state*);
int pchl(uint8_t, struct cpu_state*);

// STACK, I/O, AND MACHINE CONTROL (OTHER) GROUP

int push(uint8_t, struct cpu_state*);
int pop(uint8_t, struct cpu_state*);
int xthl(uint8_t, struct cpu_state*);
int sphl(uint8_t, struct cpu_state*);
int in(uint8_t, struct cpu_state*);
int out(uint8_t, struct cpu_state*);
int ei(uint8_t, struct cpu_state*);
int di(uint8_t, struct cpu_state*);
int hlt(uint8_t, struct cpu_state*);
int nop(uint8_t, struct cpu_state*);

#endif
