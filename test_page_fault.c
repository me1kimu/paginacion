#include "memory.h"
#include <assert.h>
#include <stdio.h>

int main() {
  printf("=== TEST: Provoke Page Fault ===\n");

  MemoryManager mm;
  administrador_memoria_inicio(&mm, 16, 4);

  printf("Creating Process 1 (8MB)...\n");
  int pid1 = crear_proceso(&mm, 8, 8);
  assert(pid1 != -1);

  printf("Creating Process 2 (8MB)...\n");
  int pid2 = crear_proceso(&mm, 8, 8);
  assert(pid2 != -1);


  printf("Creating Process 3 (4MB) - Should go to SWAP...\n");
  int pid3 = crear_proceso(&mm, 4, 4);
  assert(pid3 != -1);


  Page *p3_page0 = &mm.pages[4];
  assert(p3_page0->idProceso == pid3);
  assert(p3_page0->en_ram == false); // Should be in swap
  printf("Verified: Process 3 Page 0 is in SWAP.\n");


  printf("Terminating Process 1 and 2 to isolate Process 3...\n");
  terminar_proceso(&mm, pid1);
  terminar_proceso(&mm, pid2);

  assert(p3_page0->en_ram == false);

  int initial_faults = mm.page_faults;
  printf("Initial Page Faults: %d\n", initial_faults);

  printf("Accessing Virtual Address 0 (Page 0 of Process 3)...\n");
  acceso_memoria_virtual(&mm, 0);

  int final_faults = mm.page_faults;
  printf("Final Page Faults: %d\n", final_faults);

  if (final_faults > initial_faults) {
    printf("TEST PASSED: Page fault triggered.\n");
  } else {
    printf("TEST FAILED: Page fault NOT triggered.\n");
    return 1;
  }

  administrador_memoria_limpieza(&mm);
  return 0;
}
