/*
 * SIMULADOR DE PAGINACION DE MEMORIA (COMBINED)
 *
 * Este archivo contiene todo el código fuente del simulador de paginación,
 * incluyendo las definiciones de estructuras, la implementación del gestor
 * de memoria, la lógica principal de simulación y las pruebas unitarias.
 *
 * Para compilar: gcc -o simulador combined.c -lm
 * Para ejecutar simulacion: ./simulador
 * Para ejecutar pruebas: ./simulador --test
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// ==========================================
// SECCION: memory.h (Definiciones)
// ==========================================

#define MAX_PROCESSES 100
#define MAX_PAGES 10000

// Estructura para representar una pagina
typedef struct {
  int idProceso;        // ID del proceso al que pertenece
  int NumeroPagina;     // No de pagina dentro del proceso
  bool en_ram;          // true si está en RAM, false si está en swap
  int numero_Frame;     // Numero de marco (si está en RAM) o posicion en swap
  time_t ULTIMO_ACCESO; // Para implementar LRU
} Page;

// Estructura para representar un proceso
typedef struct {
  int id;
  float tamano;        // Tamano en MB
  int numero_pagina;   // Numero de paginas que ocupa
  int *pagina_indices; // Índices de las páginas en el arreglo global
  bool activo;
} Process;

// Estructura para gestionar la memoria
typedef struct {
  int tamanoFisico;      // Tamano de RAM en MB
  int tamanoVirtual;     // Tamano de memoria virtual en MB
  int tamanoDePagina;    // Tamano de pagina en MB
  int numeroDeFrames;    // Numero de marcos en RAM
  int numero_slots_swap; // Numero de slots en swap

  Page pages[MAX_PAGES];
  int contador_paginas;

  Process procesos[MAX_PROCESSES];
  int contador_procesos;
  int id_siguinte_proceso;

  bool *ram_frames; // true si el marco esta ocupado
  bool *swap_slots; // true si el slot está ocupado

  int page_faults;
} MemoryManager;

// Prototipos de funciones
void administrador_memoria_inicio(MemoryManager *mm, int tamamo_fisico,
                                  int tamanoDePagina);
void administrador_memoria_limpieza(MemoryManager *mm);
int crear_proceso(MemoryManager *mm, float tamano_min, float tamano_max);
void terminar_proceso(MemoryManager *mm, int proceso_id);
void acceso_memoria_virtual(MemoryManager *mm, int direccion_virtual);
void pagina_swap(MemoryManager *mm, int indice_pagina);
int buscar_victima_pagina(MemoryManager *mm);
void imprimir_estadp_memoria(MemoryManager *mm);

// ==========================================
// SECCION: memory.c (Implementacion)
// ==========================================

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

  // Primero intentamos encontrar un marco libre en RAM
  int frame_libre = find_free_frame(mm);

  if (frame_libre != -1) {
    // Hay espacio en RAM, movemos directamente
    int swap_slot = pagina->numero_Frame;

    pagina->en_ram = true;
    pagina->numero_Frame = frame_libre;
    mm->ram_frames[frame_libre] = true;

    // Liberamos el slot de swap
    mm->swap_slots[swap_slot] = false;

    pagina->ULTIMO_ACCESO = time(NULL);
    return;
  }

  // Buscamos la victima usando LRU
  int victima = buscar_victima_pagina(mm);
  if (victima == -1) {
    printf("ERROR: No se encontró pagina víctima\n");
    return;
  }

  Page *pagina_victim = &mm->pages[victima];
  frame_libre = pagina_victim->numero_Frame;
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

// ==========================================
// SECCION: test_page_fault.c (Modificado)
// ==========================================

int run_test_page_fault() {
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

// ==========================================
// SECCION: main.c (Modificado)
// ==========================================

int main(int argc, char *argv[]) {
  // Verificar si se solicita ejecutar el test
  if (argc > 1 && strcmp(argv[1], "--test") == 0) {
    return run_test_page_fault();
  }

  int tamanoFisico, tamanoPagina;
  float tamano_min_proceso, tamano_max_proceso;

  // Solicitar parametros al usuario
  printf(" SIMULADOR DE PAGINACION DE MEMORIA\n");

  printf("Ingresa el tamano de la memoria fisica (MB): ");
  if (scanf("%d", &tamanoFisico) != 1 || tamanoFisico <= 0) {
    printf("Error: Tamano de memoria inválido\n");
    return 1;
  }

  printf("Ingresa el tamano de cada pagina (MB): ");
  if (scanf("%d", &tamanoPagina) != 1 || tamanoPagina <= 0 ||
      tamanoPagina > tamanoFisico) {
    printf("Error: Tamano de pagina invalido\n");
    return 1;
  }

  printf("Ingresa el tamano minimo de proceso (MB): ");
  if (scanf("%f", &tamano_min_proceso) != 1 || tamano_min_proceso <= 0) {
    printf("Error: Tamano minimo inválido\n");
    return 1;
  }

  printf("Ingresa el tamano maximo de proceso (MB): ");
  if (scanf("%f", &tamano_max_proceso) != 1 ||
      tamano_max_proceso < tamano_min_proceso) {
    printf("Error: Tamano maximo invalido\n");
    return 1;
  }

  printf("\n");

  // Inicializar generador de numeros aleatorios
  srand(time(NULL));

  // Inicializar el gestor de memoria
  MemoryManager mm;
  administrador_memoria_inicio(&mm, tamanoFisico, tamanoPagina);

  time_t comienzo_tiempo = time(NULL);
  time_t creacion_ultimo_proceso = comienzo_tiempo;
  time_t ultima_terminacion_proceso = comienzo_tiempo;
  time_t ultimo_acceso_memoria = comienzo_tiempo;

  bool corriendo_simulacion = true;
  int iteracion = 0;

  printf("Iniciando simulacion...\n");
  printf("(Los primeros 30 segundos solo se crearan procesos)\n\n");

  while (corriendo_simulacion) {
    time_t tiempo_actual = time(NULL);
    int transcurrido = tiempo_actual - comienzo_tiempo;

    // Crear proceso cada 2 segundos
    if (tiempo_actual - creacion_ultimo_proceso >= 2) {
      int resultado =
          crear_proceso(&mm, tamano_min_proceso, tamano_max_proceso);
      if (resultado == -2) {
        // No hay memoria disponible, se termina la simulacion
        corriendo_simulacion = false;
        break;
      }
      creacion_ultimo_proceso = tiempo_actual;
      imprimir_estadp_memoria(&mm);
    }

    // Despues de 30 segundos, comenzar eventos periodicos cada 5 segundos
    // Evento 1: Terminar un proceso aleatorio cada 5 segundos
    if (transcurrido >= 30 && tiempo_actual - ultima_terminacion_proceso >= 5) {
      printf("\n--- Evento: Terminacion de Proceso (Tiempo: %d s) ---\n",
             transcurrido);

      int contador_activo = 0;
      int active_pids[MAX_PROCESSES];
      for (int i = 0; i < mm.contador_procesos; i++) {
        if (mm.procesos[i].activo) {
          active_pids[contador_activo++] = mm.procesos[i].id;
        }
      }

      if (contador_activo > 0) {
        int random_pid = active_pids[rand() % contador_activo];
        terminar_proceso(&mm, random_pid);
      } else {
        printf("[INFO] No hay procesos activos para terminar\n\n");
      }

      ultima_terminacion_proceso = tiempo_actual;
      imprimir_estadp_memoria(&mm);
    }

    // Evento 2: Acceder a una direccion virtual aleatoria cada 5 segundos
    if (transcurrido >= 30 && tiempo_actual - ultimo_acceso_memoria >= 5) {
      printf("\n--- Evento: Acceso a Memoria Virtual (Tiempo: %d s) ---\n",
             transcurrido);

      int direccion_virtual_max = mm.tamanoVirtual;
      int direccionAleatoria = rand() % direccion_virtual_max;
      acceso_memoria_virtual(&mm, direccionAleatoria);

      ultimo_acceso_memoria = tiempo_actual;
      imprimir_estadp_memoria(&mm);

      printf("Presione Ctrl+C para detener la simulacion\n");
    }

    // Verificar si hay memoria disponible
    bool tieneMemoria = false;
    for (int i = 0; i < mm.numeroDeFrames; i++) {
      if (!mm.ram_frames[i]) {
        tieneMemoria = true;
        break;
      }
    }
    if (!tieneMemoria) {
      for (int i = 0; i < mm.numero_slots_swap; i++) {
        if (!mm.swap_slots[i]) {
          tieneMemoria = true;
          break;
        }
      }
    }

    if (!tieneMemoria) {
      printf(
          "\nMEMORIA LLENA, no hay espacio disponible en la RAM ni en SWAP.\n");
      printf("Finalizando simulacion...\n");
      corriendo_simulacion = false;
    }

    sleep(1);
    iteracion++;

    // El lomite de seguridad, ello para evitar simulaciones infinitas en
    // pruebas
    if (iteracion > 300) { // 5 minutos máximo
      printf("\n El Limite de tiempo fue alcanzado,  se finalizo la "
             "simulacion...\n");
      corriendo_simulacion = false;
    }
  }

  printf("\n SIMULACION FINALIZADA\n");
  imprimir_estadp_memoria(&mm);

  administrador_memoria_limpieza(&mm);

  return 0;
}
