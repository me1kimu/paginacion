#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PROCESSES 100
#define MAX_PAGES 10000

// Estructura para representar una pagina
typedef struct {
    int idProceso;      // ID del proceso al que pertenece
    int NumeroPagina;     // No de pagina dentro del proceso
    bool en_ram;         // true si está en RAM, false si está en swap
    int numero_Frame;    // Numero de marco (si está en RAM) o posicion en swap
    time_t ULTIMO_ACCESO;  // Para implementar LRU
} Page;

// Estructura para representar un proceso
typedef struct {
    int id;
    float tamano;            // Tamano en MB
    int numero_pagina;       // Numero de paginas que ocupa
    int *pagina_indices;   // Índices de las páginas en el arreglo global
    bool activo;
} Process;

// Estructura para gestionar la memoria
typedef struct {
    int tamanoFisico;   // Tamano de RAM en MB
    int tamanoVirtual;    // Tamano de memoria virtual en MB
    int tamanoDePagina;       // Tamano de pagina en MB
    int numeroDeFrames;      // Numero de marcos en RAM
    int numero_slots_swap;  // Numero de slots en swap
    
    Page pages[MAX_PAGES];
    int contador_paginas;
    
    Process procesos[MAX_PROCESSES];
    int contador_procesos;
    int id_siguinte_proceso;
    
    bool *ram_frames;    // true si el marco esta ocupado
    bool *swap_slots;    // true si el slot está ocupado
    
    int page_faults;
} MemoryManager;

// Funciones principales
void administrador_memoria_inicio(MemoryManager *mm, int tamamo_fisico, int tamanoDePagina);
void administrador_memoria_limpieza(MemoryManager *mm);
int crear_proceso(MemoryManager *mm, float tamano_min, float tamano_max);
void terminar_proceso(MemoryManager *mm, int proceso_id);
void acceso_memoria_virtual(MemoryManager *mm, int direccion_virtual);
void pagina_swap(MemoryManager *mm, int indice_pagina);
int buscar_victima_pagina(MemoryManager *mm);
void imprimir_estadp_memoria(MemoryManager *mm);

#endif