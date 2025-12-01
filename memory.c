#include "memory.h"
#include <math.h>
#include <string.h>

void administrador_memoria_inicio(MemoryManager *mm, int tamanoFisico,
                                  int tamanoDePagina) {
  mm->tamanoFisico = tamanoFisico;
  mm->tamanoDePagina = tamanoDePagina;

  // Memoria virtual: entre 1.5 y 4.5 veces la fisica
  float factor = 1.5 + ((float)rand() / RAND_MAX) * 3.0;
  mm->tamanoVirtual = (int)(tamanoFisico * factor);

  mm->numeroDeFrames = tamanoFisico / tamanoDePagina;
  mm->numero_slots_swap = (mm->tamanoVirtual - tamanoFisico) / tamanoDePagina;

  mm->contador_paginas = 0;
  mm->contador_procesos = 0;
  mm->id_siguinte_proceso = 1;
  mm->page_faults = 0;

  // Inicializar marcos y slots
  mm->ram_frames = (bool *)calloc(mm->numeroDeFrames, sizeof(bool));
  mm->swap_slots = (bool *)calloc(mm->numero_slots_swap, sizeof(bool));

  for (int i = 0; i < MAX_PROCESSES; i++) {
    mm->procesos[i].activo = false;
    mm->procesos[i].pagina_indices = NULL;
  }

  printf("=== Sistema de Paginacion Inicializado ===\n");
  printf("Memoria Fisica (RAM): %d MB\n", mm->tamanoFisico);
  printf("Memoria Virtual: %d MB (factor: %.2f)\n", mm->tamanoVirtual, factor);
  printf("Tamano de pagina: %d MB\n", mm->tamanoDePagina);
  printf("Marcos en RAM: %d\n", mm->numeroDeFrames);
  printf("Slots en Swap: %d\n", mm->numero_slots_swap);
  printf("==========================================\n\n");
}

void administrador_memoria_limpieza(MemoryManager *mm) {
  for (int i = 0; i < mm->contador_procesos; i++) {
    if (mm->procesos[i].pagina_indices != NULL) {
      free(mm->procesos[i].pagina_indices);
    }
  }
  free(mm->ram_frames);
  free(mm->swap_slots);
}

int find_free_frame(MemoryManager *mm) {
  for (int i = 0; i < mm->numeroDeFrames; i++) {
    if (!mm->ram_frames[i])
      return i;
  }
  return -1;
}

int find_free_swap_slot(MemoryManager *mm) {
  for (int i = 0; i < mm->numero_slots_swap; i++) {
    if (!mm->swap_slots[i])
      return i;
  }
  return -1;
}

int crear_proceso(MemoryManager *mm, float tamano_min, float tamano_max) {
  if (mm->contador_procesos >= MAX_PROCESSES) {
    printf("ERROR: LImite de procesos alcanzado\n");
    return -1;
  }

  // Tamano aleatorio del proceso
  float tamano =
      tamano_min + ((float)rand() / RAND_MAX) * (tamano_max - tamano_min);
  int numero_pagina = (int)ceil(tamano / mm->tamanoDePagina);

  // Verificamos si hay suficiente memoria virtual
  int ram_disponible = 0, swap_disponible = 0;
  for (int i = 0; i < mm->numeroDeFrames; i++) {
    if (!mm->ram_frames[i])
      ram_disponible++;
  }
  for (int i = 0; i < mm->numero_slots_swap; i++) {
    if (!mm->swap_slots[i])
      swap_disponible++;
  }

  if (ram_disponible + swap_disponible < numero_pagina) {
    printf("ERROR: No hay suficiente memoria disponible (RAM + Swap)\n");
    printf("Programa terminado por falta de memoria.\n");
    return -2;
  }

  // Creamos eel proceso
  int proc_indice = mm->contador_procesos;
  Process *proc = &mm->procesos[proc_indice];
  proc->id = mm->id_siguinte_proceso++;
  proc->tamano = tamano;
  proc->numero_pagina = numero_pagina;
  proc->activo = true;
  proc->pagina_indices = (int *)malloc(numero_pagina * sizeof(int));

  printf("[NUEVO PROCESO] PID: %d | Tamano: %.2f MB | Paginas: %d\n", proc->id,
         tamano, numero_pagina);

  // Asignamos las paginas
  for (int i = 0; i < numero_pagina; i++) {
    Page *pagina = &mm->pages[mm->contador_paginas];
    pagina->idProceso = proc->id;
    pagina->NumeroPagina = i;
    pagina->ULTIMO_ACCESO = time(NULL);

    // Intentamos asignar en RAM primero chatisimo
    int frame = find_free_frame(mm);
    if (frame != -1) {
      pagina->en_ram = true;
      pagina->numero_Frame = frame;
      mm->ram_frames[frame] = true;
      printf("  Pagina %d -> RAM (Marco %d)\n", i, frame);
    } else {
      // Asignamos en swap
      int slot = find_free_swap_slot(mm);
      if (slot == -1) {
        printf("ERROR: No hay espacio en SWAP\n");
        free(proc->pagina_indices);
        proc->pagina_indices = NULL;
        return -2;
      }
      pagina->en_ram = false;
      pagina->numero_Frame = slot;
      mm->swap_slots[slot] = true;
      printf("  Pagina %d -> SWAP (Slot %d)\n", i, slot);
    }

    proc->pagina_indices[i] = mm->contador_paginas;
    mm->contador_paginas++;
  }

  mm->contador_procesos++;
  printf("\n");
  return proc->id;
}

void terminar_proceso(MemoryManager *mm, int idProceso) {
  Process *proc = NULL;

  // Buscar el proceso
  for (int i = 0; i < mm->contador_procesos; i++) {
    if (mm->procesos[i].activo && mm->procesos[i].id == idProceso) {
      proc = &mm->procesos[i];
      break;
    }
  }

  if (proc == NULL) {
    printf("[TERMINAR] Proceso %d no encontrado o ya terminado\n", idProceso);
    return;
  }

  printf("[TERMINAR PROCESO] PID: %d | Liberando %d paginas\n", idProceso,
         proc->numero_pagina);

  // Liberaramos todas las paginas del proceso
  for (int i = 0; i < proc->numero_pagina; i++) {
    int page_idx = proc->pagina_indices[i];
    Page *pagina = &mm->pages[page_idx];

    if (pagina->en_ram) {
      mm->ram_frames[pagina->numero_Frame] = false;
    } else {
      mm->swap_slots[pagina->numero_Frame] = false;
    }
  }

  free(proc->pagina_indices);
  proc->pagina_indices = NULL;
  proc->activo = false;
  printf("\n");
}

int buscar_victima_pagina(MemoryManager *mm) {
  // Aplicacion de LRU
  time_t tiempo_mas_antiguo = time(NULL);
  int victima = -1;

  for (int i = 0; i < mm->contador_paginas; i++) {
    if (mm->pages[i].en_ram &&
        mm->pages[i].ULTIMO_ACCESO < tiempo_mas_antiguo) {
      // Verificamos que el proceso esté activo
      bool esta_activo = false;
      for (int j = 0; j < mm->contador_procesos; j++) {
        if (mm->procesos[j].activo &&
            mm->procesos[j].id == mm->pages[i].idProceso) {
          esta_activo = true;
          break;
        }
      }
      if (esta_activo) {
        tiempo_mas_antiguo = mm->pages[i].ULTIMO_ACCESO;
        victima = i;
      }
    }
  }

  return victima;
}

void pagina_swap(MemoryManager *mm, int indiceDePagina) {
  Page *pagina = &mm->pages[indiceDePagina];

  if (pagina->en_ram) {
    printf("ERROR: La pagina ya esta en RAM\n");
    return;
  }

  // Buscamos la victima usando LRU
  int victima = buscar_victima_pagina(mm);
  if (victima == -1) {
    printf("ERROR: No se encontró pagina víctima\n");
    return;
  }

  Page *pagina_victim = &mm->pages[victima];
  int frame_libre = pagina_victim->numero_Frame;
  int swapLibre = pagina->numero_Frame;

  printf("  [SWAP] Reemplazando pagina (P%d:Pag%d) con (P%d:Pag%d)\n",
         pagina_victim->idProceso, pagina_victim->NumeroPagina,
         pagina->idProceso, pagina->NumeroPagina);

  // Movemos la vvctima a swap
  pagina_victim->en_ram = false;
  pagina_victim->numero_Frame = swapLibre;

  // Movemos pgina requerida a RAM
  pagina->en_ram = true;
  pagina->numero_Frame = frame_libre;
  pagina->ULTIMO_ACCESO = time(NULL);
}

void acceso_memoria_virtual(MemoryManager *mm, int direccionVirtual) {
  int numeroPagina = direccionVirtual / mm->tamanoDePagina;
  int offset = direccionVirtual % mm->tamanoDePagina;

  printf("[ACCESO] Direccion virtual: %d (Pagina: %d, Offset: %d)\n",
         direccionVirtual, numeroPagina, offset);

  // Buscamos la pagina en la memoria
  Page *pagina_OBJETIVO = NULL;
  for (int i = 0; i < mm->contador_paginas; i++) {
    // Verificamos que el proceso esté activo
    bool esta_activo = false;
    for (int j = 0; j < mm->contador_procesos; j++) {
      if (mm->procesos[j].activo &&
          mm->procesos[j].id == mm->pages[i].idProceso) {
        esta_activo = true;
        break;
      }
    }

    if (esta_activo && mm->pages[i].NumeroPagina == numeroPagina) {
      pagina_OBJETIVO = &mm->pages[i];
      break;
    }
  }

  if (pagina_OBJETIVO == NULL) {
    printf("  Resultado: Pagina no encontrada en el sistema\n\n");
    return;
  }

  if (pagina_OBJETIVO->en_ram) {
    printf("  Resultado: Pagina encontrada en RAM (Marco %d)\n",
           pagina_OBJETIVO->numero_Frame);
    pagina_OBJETIVO->ULTIMO_ACCESO = time(NULL);
  } else {
    printf("  Resultado: PAGE FAULT - Pagina en SWAP (Slot %d)\n",
           pagina_OBJETIVO->numero_Frame);
    mm->page_faults++;
    pagina_swap(mm, pagina_OBJETIVO - mm->pages);
    printf("  Pagina ahora en RAM (Marco %d)\n", pagina_OBJETIVO->numero_Frame);
  }
  printf("\n");
}

void imprimir_estadp_memoria(MemoryManager *mm) {
  int ram_usada = 0, swap_utilizado = 0;
  int procesoActivo = 0;

  for (int i = 0; i < mm->numeroDeFrames; i++) {
    if (mm->ram_frames[i])
      ram_usada++;
  }
  for (int i = 0; i < mm->numero_slots_swap; i++) {
    if (mm->swap_slots[i])
      swap_utilizado++;
  }
  for (int i = 0; i < mm->contador_procesos; i++) {
    if (mm->procesos[i].activo)
      procesoActivo++;
  }

  printf("╔══════════════════════════════════════════╗\n");
  printf("║       ESTADO DE LA MEMORIA               ║\n");
  printf("╠══════════════════════════════════════════╣\n");
  printf("║ RAM: %d/%d marcos (%d%%)                 \n", ram_usada,
         mm->numeroDeFrames,
         mm->numeroDeFrames > 0 ? (ram_usada * 100 / mm->numeroDeFrames) : 0);
  printf("║ SWAP: %d/%d slots (%d%%)                 \n", swap_utilizado,
         mm->numero_slots_swap,
         mm->numero_slots_swap > 0
             ? (swap_utilizado * 100 / mm->numero_slots_swap)
             : 0);
  printf("║ Procesos activos: %d                     \n", procesoActivo);
  printf("║ Page Faults totales: %d                  \n", mm->page_faults);
  printf("╚══════════════════════════════════════════╝\n\n");
}