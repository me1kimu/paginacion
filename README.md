# Simulador de Paginación de Memoria

Este proyecto implementa un simulador de paginación de memoria que demuestra cómo los sistemas operativos gestionan la memoria utilizando paginación y swap.

## Descripción

El simulador implementa las siguientes características:

1. **Gestión de Memoria**: Simula memoria física (RAM) y memoria virtual (swap)
2. **Creación de Procesos**: Crea procesos aleatorios cada 2 segundos
3. **Asignación de Páginas**: Asigna páginas a RAM o swap según disponibilidad
4. **Reemplazo de Páginas**: Utiliza la política LRU (Least Recently Used) para reemplazo
5. **Simulación de Page Faults**: Simula accesos a direcciones virtuales y maneja page faults
6. **Terminación de Procesos**: Después de 30 segundos, termina procesos aleatorios cada 5 segundos

## Requisitos

- Sistema operativo UNIX/Linux
- Compilador GCC
- Make (opcional, pero recomendado)

## Compilación

### Usando Make:
```bash
make
```

### Sin Make:
```bash
gcc -Wall -Wextra -std=c99 -O2 -o paging_simulator paging_simulator.c
```

## Ejecución

```bash
./paging_simulator
```

El programa solicitará los siguientes parámetros de entrada:
1. **Tamaño de la memoria física** (en MB): Ejemplo: 100
2. **Tamaño de página** (en KB): Ejemplo: 4

La memoria virtual se calculará automáticamente como un valor aleatorio entre 1.5 y 4.5 veces la memoria física.

## Comportamiento del Simulador

### Fase 1: Primeros 30 segundos
- Se crean procesos aleatorios cada 2 segundos
- Cada proceso tiene un tamaño entre 1 y 10 MB
- Las páginas se asignan a RAM si hay espacio disponible
- Si la RAM está llena, las páginas se asignan a swap
- El programa termina si no hay espacio en swap

### Fase 2: Después de 30 segundos
Además de la creación de procesos, se realizan las siguientes operaciones cada 5 segundos:

1. **Terminación de Procesos**: Se termina un proceso aleatorio y se liberan sus páginas
2. **Acceso a Direcciones Virtuales**: 
   - Se genera una dirección virtual aleatoria
   - Si la página está en RAM, el acceso es exitoso
   - Si la página no está en RAM, se genera un **page fault**
   - En caso de page fault, se aplica la política de reemplazo LRU

## Política de Reemplazo: LRU (Least Recently Used)

El simulador utiliza la política LRU para reemplazar páginas:
- Cada página tiene un contador de tiempo de último acceso
- Cuando se necesita reemplazar una página, se selecciona la que tiene el tiempo de acceso más antiguo
- Esta política minimiza los page faults al mantener en RAM las páginas más recientemente utilizadas

## Ejemplo de Salida

```
=== Simulador de Paginación de Memoria ===

Ingrese el tamaño de la memoria física (en MB): 100
Ingrese el tamaño de cada página (en KB): 4

Memoria física: 100 MB
Memoria virtual: 324 MB (3.24x memoria física)
Tamaño de página: 4 KB

Inicializando memoria...
Páginas físicas (RAM): 25600
Páginas virtuales totales: 82944
Páginas swap: 57344

Iniciando simulación...
Los procesos comenzarán a crearse cada 2 segundos.
Después de 30 segundos, se eliminarán procesos y accederán direcciones virtuales cada 5 segundos.

[PROCESO CREADO] PID: 1, Tamaño: 7 MB, Páginas: 1792
  -> Páginas en RAM: 1792, Páginas en swap: 0
  -> Uso de RAM: 1792/25600 páginas (7.0%)
  -> Uso de swap: 0/57344 páginas (0.0%)

[PROCESO CREADO] PID: 2, Tamaño: 3 MB, Páginas: 768
  -> Páginas en RAM: 768, Páginas en swap: 0
  -> Uso de RAM: 2560/25600 páginas (10.0%)
  -> Uso de swap: 0/57344 páginas (0.0%)

...

[PROCESO TERMINADO] PID: 5, Páginas liberadas: 1280
  -> RAM liberada: 18432/25600 páginas (72.0%)
  -> Swap usado: 1024/57344 páginas (1.8%)

[ACCESO A DIRECCIÓN VIRTUAL] 0x4A3B2C1
  -> Página virtual: 1234, Offset: 3009
  -> [PAGE FAULT] Página no encontrada en RAM (Total page faults: 1)
  -> Página encontrada en swap, iniciando swap...
  -> Aplicando política LRU: reemplazando página en frame 5678
  -> Página cargada exitosamente en RAM
```

## Estructura del Código

El programa está organizado en las siguientes secciones:

- **Estructuras de Datos**:
  - `Page`: Representa una página con información del proceso, número de página y tiempo de acceso
  - `Process`: Representa un proceso con PID, tamaño y páginas asignadas

- **Funciones Principales**:
  - `initialize_memory()`: Inicializa las estructuras de RAM y swap
  - `create_process()`: Crea un nuevo proceso y asigna sus páginas
  - `terminate_random_process()`: Termina un proceso aleatorio
  - `access_virtual_address()`: Simula el acceso a una dirección virtual
  - `find_lru_page()`: Encuentra la página menos recientemente usada
  - `swap_page_to_disk()`: Mueve una página de RAM a swap
  - `load_page_to_ram()`: Carga una página de swap a RAM

## Condiciones de Terminación

El simulador terminará automáticamente en los siguientes casos:
1. No hay suficiente memoria disponible (RAM + swap) para crear un nuevo proceso
2. No hay espacio disponible en swap cuando se intenta hacer swap de una página

Al terminar, se muestra un resumen con:
- Total de procesos creados
- Total de page faults
- Uso final de RAM y swap

## Limpieza

Para eliminar el ejecutable compilado:
```bash
make clean
```

## Notas Técnicas

- El programa utiliza `sleep(1)` para simular el paso del tiempo
- Los tamaños de proceso se generan aleatoriamente entre 1 y 10 MB
- La memoria virtual se calcula aleatoriamente entre 1.5 y 4.5 veces la memoria física
- El simulador utiliza aritmética entera para evitar problemas de precisión en floating point

## Autor

Proyecto desarrollado como parte de la Tarea 3 del curso de Sistemas Operativos.
