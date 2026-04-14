#include "memory.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
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