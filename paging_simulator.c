#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PROCESSES 100
#define MIN_PROCESS_SIZE 1  // MB
#define MAX_PROCESS_SIZE 10 // MB

// Structure to represent a page
typedef struct {
    int process_id;
    int page_number;
    unsigned long last_access_time; // For LRU
    bool in_ram;
} Page;

// Structure to represent a process
typedef struct {
    int pid;
    int size_mb;
    int num_pages;
    int *page_indices; // Indices in RAM or swap
    bool active;
} Process;

// Global variables
int physical_memory_mb;
int virtual_memory_mb;
int page_size_kb;
int num_physical_pages;
int num_virtual_pages;
Page *ram_pages;
Page *swap_pages;
Process processes[MAX_PROCESSES];
int num_processes = 0;
int next_pid = 1;
int ram_used = 0;
int swap_used = 0;
unsigned long access_counter = 0;
int page_faults = 0;

// Function prototypes
void initialize_memory();
void create_process();
void terminate_random_process();
void access_virtual_address();
int find_lru_page();
void swap_page_to_disk(int ram_index);
bool load_page_to_ram(int swap_index);
void print_memory_status();
void cleanup();

int main() {
    printf("=== Simulador de Paginación de Memoria ===\n\n");
    
    // Input: Physical memory size
    printf("Ingrese el tamaño de la memoria física (en MB): ");
    if (scanf("%d", &physical_memory_mb) != 1 || physical_memory_mb <= 0) {
        fprintf(stderr, "Error: Tamaño de memoria física inválido\n");
        return 1;
    }
    
    // Input: Page size
    printf("Ingrese el tamaño de cada página (en KB): ");
    if (scanf("%d", &page_size_kb) != 1 || page_size_kb <= 0) {
        fprintf(stderr, "Error: Tamaño de página inválido\n");
        return 1;
    }
    
    // Calculate virtual memory (1.5 to 4.5 times physical memory)
    srand(time(NULL));
    double multiplier = 1.5 + ((double)rand() / RAND_MAX) * 3.0; // 1.5 to 4.5
    virtual_memory_mb = (int)(physical_memory_mb * multiplier);
    
    printf("\nMemoria física: %d MB\n", physical_memory_mb);
    printf("Memoria virtual: %d MB (%.2fx memoria física)\n", virtual_memory_mb, multiplier);
    printf("Tamaño de página: %d KB\n\n", page_size_kb);
    
    initialize_memory();
    
    time_t start_time = time(NULL);
    time_t last_process_creation = start_time;
    time_t last_process_termination = start_time;
    time_t last_address_access = start_time;
    
    printf("Iniciando simulación...\n");
    printf("Los procesos comenzarán a crearse cada 2 segundos.\n");
    printf("Después de 30 segundos, se eliminarán procesos y accederán direcciones virtuales cada 5 segundos.\n\n");
    
    // Main simulation loop
    while (true) {
        time_t current_time = time(NULL);
        int elapsed = current_time - start_time;
        
        // Create process every 2 seconds
        if (current_time - last_process_creation >= 2) {
            create_process();
            last_process_creation = current_time;
        }
        
        // After 30 seconds, start periodic operations
        if (elapsed >= 30) {
            // Terminate random process every 5 seconds
            if (current_time - last_process_termination >= 5) {
                terminate_random_process();
                last_process_termination = current_time;
            }
            
            // Access virtual address every 5 seconds
            if (current_time - last_address_access >= 5) {
                access_virtual_address();
                last_address_access = current_time;
            }
        }
        
        sleep(1);
    }
    
    cleanup();
    return 0;
}

void initialize_memory() {
    // Calculate number of pages
    num_physical_pages = (physical_memory_mb * 1024) / page_size_kb;
    num_virtual_pages = (virtual_memory_mb * 1024) / page_size_kb;
    int num_swap_pages = num_virtual_pages - num_physical_pages;
    
    printf("Inicializando memoria...\n");
    printf("Páginas físicas (RAM): %d\n", num_physical_pages);
    printf("Páginas virtuales totales: %d\n", num_virtual_pages);
    printf("Páginas swap: %d\n\n", num_swap_pages);
    
    // Allocate RAM pages
    ram_pages = (Page *)malloc(num_physical_pages * sizeof(Page));
    if (!ram_pages) {
        fprintf(stderr, "Error: No se pudo asignar memoria para RAM\n");
        exit(1);
    }
    
    // Allocate swap pages
    swap_pages = (Page *)malloc(num_swap_pages * sizeof(Page));
    if (!swap_pages) {
        fprintf(stderr, "Error: No se pudo asignar memoria para swap\n");
        exit(1);
    }
    
    // Initialize pages
    for (int i = 0; i < num_physical_pages; i++) {
        ram_pages[i].process_id = -1;
        ram_pages[i].page_number = -1;
        ram_pages[i].last_access_time = 0;
        ram_pages[i].in_ram = false;
    }
    
    for (int i = 0; i < num_swap_pages; i++) {
        swap_pages[i].process_id = -1;
        swap_pages[i].page_number = -1;
        swap_pages[i].last_access_time = 0;
        swap_pages[i].in_ram = false;
    }
    
    // Initialize processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].active = false;
    }
}

void create_process() {
    if (num_processes >= MAX_PROCESSES) {
        printf("[ADVERTENCIA] Número máximo de procesos alcanzado\n");
        return;
    }
    
    // Generate random process size
    int process_size = MIN_PROCESS_SIZE + rand() % (MAX_PROCESS_SIZE - MIN_PROCESS_SIZE + 1);
    int num_pages = (process_size * 1024 + page_size_kb - 1) / page_size_kb; // Round up
    
    // Check if there's enough total memory (RAM + swap)
    int total_free_pages = (num_physical_pages - ram_used) + 
                          ((num_virtual_pages - num_physical_pages) - swap_used);
    
    if (total_free_pages < num_pages) {
        printf("\n[ERROR] No hay suficiente memoria disponible (RAM + swap)\n");
        printf("Se requieren %d páginas pero solo hay %d disponibles\n", num_pages, total_free_pages);
        printf("Terminando simulación...\n");
        cleanup();
        exit(0);
    }
    
    // Create process
    int pid = next_pid++;
    processes[num_processes].pid = pid;
    processes[num_processes].size_mb = process_size;
    processes[num_processes].num_pages = num_pages;
    processes[num_processes].active = true;
    processes[num_processes].page_indices = (int *)malloc(num_pages * sizeof(int));
    
    printf("\n[PROCESO CREADO] PID: %d, Tamaño: %d MB, Páginas: %d\n", pid, process_size, num_pages);
    
    // Allocate pages to RAM or swap
    int pages_in_ram = 0;
    int pages_in_swap = 0;
    
    for (int i = 0; i < num_pages; i++) {
        access_counter++;
        
        // Try to allocate in RAM first
        if (ram_used < num_physical_pages) {
            // Find free RAM page
            for (int j = 0; j < num_physical_pages; j++) {
                if (ram_pages[j].process_id == -1) {
                    ram_pages[j].process_id = pid;
                    ram_pages[j].page_number = i;
                    ram_pages[j].last_access_time = access_counter;
                    ram_pages[j].in_ram = true;
                    processes[num_processes].page_indices[i] = j;
                    ram_used++;
                    pages_in_ram++;
                    break;
                }
            }
        } else {
            // Allocate in swap
            int num_swap_pages = num_virtual_pages - num_physical_pages;
            if (swap_used < num_swap_pages) {
                for (int j = 0; j < num_swap_pages; j++) {
                    if (swap_pages[j].process_id == -1) {
                        swap_pages[j].process_id = pid;
                        swap_pages[j].page_number = i;
                        swap_pages[j].last_access_time = access_counter;
                        swap_pages[j].in_ram = false;
                        processes[num_processes].page_indices[i] = -(j + 1); // Negative indicates swap
                        swap_used++;
                        pages_in_swap++;
                        break;
                    }
                }
            }
        }
    }
    
    printf("  -> Páginas en RAM: %d, Páginas en swap: %d\n", pages_in_ram, pages_in_swap);
    printf("  -> Uso de RAM: %d/%d páginas (%.1f%%)\n", 
           ram_used, num_physical_pages, (ram_used * 100.0) / num_physical_pages);
    printf("  -> Uso de swap: %d/%d páginas (%.1f%%)\n", 
           swap_used, num_virtual_pages - num_physical_pages, 
           (swap_used * 100.0) / (num_virtual_pages - num_physical_pages));
    
    num_processes++;
}

void terminate_random_process() {
    // Count active processes
    int active_count = 0;
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].active) {
            active_count++;
        }
    }
    
    if (active_count == 0) {
        printf("\n[INFO] No hay procesos activos para terminar\n");
        return;
    }
    
    // Select random active process
    int random_index = rand() % active_count;
    int current_active = 0;
    int selected_process = -1;
    
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].active) {
            if (current_active == random_index) {
                selected_process = i;
                break;
            }
            current_active++;
        }
    }
    
    if (selected_process == -1) {
        return;
    }
    
    Process *proc = &processes[selected_process];
    printf("\n[PROCESO TERMINADO] PID: %d, Páginas liberadas: %d\n", proc->pid, proc->num_pages);
    
    // Free pages
    for (int i = 0; i < proc->num_pages; i++) {
        int page_index = proc->page_indices[i];
        if (page_index >= 0) {
            // Page in RAM
            ram_pages[page_index].process_id = -1;
            ram_pages[page_index].page_number = -1;
            ram_used--;
        } else {
            // Page in swap
            int swap_index = -(page_index + 1);
            swap_pages[swap_index].process_id = -1;
            swap_pages[swap_index].page_number = -1;
            swap_used--;
        }
    }
    
    free(proc->page_indices);
    proc->active = false;
    
    printf("  -> RAM liberada: %d/%d páginas (%.1f%%)\n", 
           ram_used, num_physical_pages, (ram_used * 100.0) / num_physical_pages);
    printf("  -> Swap usado: %d/%d páginas (%.1f%%)\n", 
           swap_used, num_virtual_pages - num_physical_pages,
           (swap_used * 100.0) / (num_virtual_pages - num_physical_pages));
}

void access_virtual_address() {
    // Generate random virtual address
    unsigned long virtual_address = rand() % (virtual_memory_mb * 1024 * 1024);
    int page_number = virtual_address / (page_size_kb * 1024);
    int offset = virtual_address % (page_size_kb * 1024);
    
    printf("\n[ACCESO A DIRECCIÓN VIRTUAL] 0x%lX\n", virtual_address);
    printf("  -> Página virtual: %d, Offset: %d\n", page_number, offset);
    
    access_counter++;
    
    // Check if page is in RAM
    bool found_in_ram = false;
    for (int i = 0; i < num_physical_pages; i++) {
        if (ram_pages[i].process_id != -1) {
            // For simplicity, we check if this is a valid page
            ram_pages[i].last_access_time = access_counter;
            if (i == page_number % num_physical_pages && ram_pages[i].process_id != -1) {
                found_in_ram = true;
                printf("  -> [ACCESO EXITOSO] Página encontrada en RAM (frame %d)\n", i);
                break;
            }
        }
    }
    
    if (!found_in_ram) {
        // Page fault!
        page_faults++;
        printf("  -> [PAGE FAULT] Página no encontrada en RAM (Total page faults: %d)\n", page_faults);
        
        // Simulate swap process
        int num_swap_pages = num_virtual_pages - num_physical_pages;
        int swap_page_index = page_number % num_swap_pages;
        
        // Check if page exists in swap
        bool found_in_swap = false;
        for (int i = 0; i < num_swap_pages; i++) {
            if (swap_pages[i].process_id != -1 && i == swap_page_index) {
                found_in_swap = true;
                printf("  -> Página encontrada en swap, iniciando swap...\n");
                
                // Find LRU page in RAM to replace
                if (ram_used >= num_physical_pages) {
                    int lru_index = find_lru_page();
                    printf("  -> Aplicando política LRU: reemplazando página en frame %d\n", lru_index);
                    swap_page_to_disk(lru_index);
                }
                
                // Load page from swap to RAM
                if (load_page_to_ram(i)) {
                    printf("  -> Página cargada exitosamente en RAM\n");
                } else {
                    printf("  -> [ERROR] No se pudo cargar la página en RAM\n");
                }
                break;
            }
        }
        
        if (!found_in_swap) {
            printf("  -> [ADVERTENCIA] Página no encontrada en swap (dirección inválida)\n");
        }
    }
}

int find_lru_page() {
    int lru_index = 0;
    unsigned long min_time = ram_pages[0].last_access_time;
    
    for (int i = 1; i < num_physical_pages; i++) {
        if (ram_pages[i].process_id != -1 && ram_pages[i].last_access_time < min_time) {
            min_time = ram_pages[i].last_access_time;
            lru_index = i;
        }
    }
    
    return lru_index;
}

void swap_page_to_disk(int ram_index) {
    if (ram_pages[ram_index].process_id == -1) {
        return;
    }
    
    int num_swap_pages = num_virtual_pages - num_physical_pages;
    
    // Find free swap space
    for (int i = 0; i < num_swap_pages; i++) {
        if (swap_pages[i].process_id == -1) {
            // Move page to swap
            swap_pages[i] = ram_pages[ram_index];
            swap_pages[i].in_ram = false;
            
            // Clear RAM page
            ram_pages[ram_index].process_id = -1;
            ram_pages[ram_index].page_number = -1;
            ram_pages[ram_index].last_access_time = 0;
            
            ram_used--;
            swap_used++;
            return;
        }
    }
    
    // No swap space available
    printf("\n[ERROR CRÍTICO] No hay espacio disponible en swap\n");
    printf("Terminando simulación...\n");
    cleanup();
    exit(0);
}

bool load_page_to_ram(int swap_index) {
    if (ram_used >= num_physical_pages) {
        return false; // No space in RAM
    }
    
    // Find free RAM page
    for (int i = 0; i < num_physical_pages; i++) {
        if (ram_pages[i].process_id == -1) {
            // Move page from swap to RAM
            ram_pages[i] = swap_pages[swap_index];
            ram_pages[i].in_ram = true;
            ram_pages[i].last_access_time = access_counter;
            
            // Clear swap page
            swap_pages[swap_index].process_id = -1;
            swap_pages[swap_index].page_number = -1;
            swap_pages[swap_index].last_access_time = 0;
            
            ram_used++;
            swap_used--;
            return true;
        }
    }
    
    return false;
}

void cleanup() {
    printf("\n=== Resumen de la simulación ===\n");
    printf("Total de procesos creados: %d\n", next_pid - 1);
    printf("Total de page faults: %d\n", page_faults);
    printf("Uso final de RAM: %d/%d páginas (%.1f%%)\n", 
           ram_used, num_physical_pages, (ram_used * 100.0) / num_physical_pages);
    printf("Uso final de swap: %d/%d páginas (%.1f%%)\n", 
           swap_used, num_virtual_pages - num_physical_pages,
           (swap_used * 100.0) / (num_virtual_pages - num_physical_pages));
    
    free(ram_pages);
    free(swap_pages);
    
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].page_indices != NULL) {
            free(processes[i].page_indices);
        }
    }
}
