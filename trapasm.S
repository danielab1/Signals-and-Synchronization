#include "mmu.h"
#include "syscall.h"
#include "traps.h"
  # vectors.S sends all traps here.
.globl alltraps

alltraps:
  # Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data segments.
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  pushl %esp
  call trap
  addl $4, %esp

  # Return falls through to trapret...
.globl trapret
trapret:

  pushl $end_sigret_caller-sigret_caller
  pushl $sigret_caller
  call check_pending_signals
  addl $8, %esp
  
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret


sigret_caller:
  movl $SYS_sigret, %eax
  int $T_SYSCALL
end_sigret_caller: