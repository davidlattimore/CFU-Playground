#include <stdio.h>
#include <generated/csr.h>

#include "base.h"
#include "riscv.h"

__attribute__((optimize("align-functions=32"))) int check_post_cfu_instruction3(
    int input) {
  register int output;
  int tmp;
  asm volatile(
      // Some padding. The alignment of the CFU instruction seems to matter.
      "nop \n\t"
      "nop \n\t"
      "nop \n\t"
      "nop \n\t"
      "nop \n\t"
      "nop \n\t"

      // Set up an input to our CFU instruction.
      "li a6, 0; \n\t"

      // These four instructions seem to need to be stores.
      "sw	%[input],%[tmp] \n\t"
      "sw	%[input],%[tmp] \n\t"
      "sw	%[input],%[tmp] \n\t"
      "sw	%[input],%[tmp] \n\t"

      // This can be any instruction.
      "nop \n\t"

      // Call CFU function 3.
      ".word ((CUSTOM0) | (regnum_a7 << 7) | (regnum_a6 << 15) | (regnum_a6 << "
      "20) | (3 << 12) | (5 << 25)) \n\t"

      // The value stored into %[output] is wrong and is the result of CFU
      // function 0, which we didn't call.
      "mv	%[output],%[input] \n\t"

      // Changing the following nop to repeat the line above, or swapping it
      // with the line above "fixes" the bug.
      "nop \n\t"

      "nop \n\t"
      "nop \n\t"
      : [output] "=r"(output), [tmp] "=m"(tmp)
      : [input] "r"(input)
      : "a6", "a7"

  );
  return output;
}

int main(void) {
  init_runtime();

  sim_trace_enable_write(1);

  for (int i = 0; i < 100; i++) {
    asm volatile("nop");
  }

  int v1 = check_post_cfu_instruction3(42);

  // For some reason if we don't continue tracing for a while, the trace file
  // ends up invalid.
  for (int i = 0; i < 50000; i++) {
    asm volatile("nop");
  }

  sim_trace_enable_write(0);

  // This should print the value 42, but when the bug is occurring, it prints
  // 99, which is the result of CFU instruction 0, which we never invoked.
  printf("\nv1: %d\n", v1);

  sim_finish_finish_write(1);

  return (0);
}
