#ifndef __FSTATE_H_INCLUDED__
#define __FSTATE_H_INCLUDED__

typedef char ext_regs_t[512];

void asmutils_load_ext_regs(ext_regs_t regs);
void asmutils_store_ext_regs(ext_regs_t regs);

#endif