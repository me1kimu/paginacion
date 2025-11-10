# Resumen de Implementación - Simulador de Paginación de Memoria

## Cumplimiento de Especificaciones

Este proyecto implementa completamente todas las especificaciones de la Tarea 3 (Tarea3_SO20252.docx):

### ✅ Requisitos Implementados

1. **Entrada de Parámetros**
   - ✅ Tamaño de memoria física (en MB) - entrada por usuario
   - ✅ Tamaño de memoria virtual - calculado aleatoriamente entre 1.5x a 4.5x la memoria física
   - ✅ Tamaño de página (en KB) - entrada por usuario

2. **Creación de Procesos**
   - ✅ Procesos creados cada 2 segundos
   - ✅ Tamaños aleatorios entre 1 MB y 10 MB
   - ✅ Páginas asignadas a RAM cuando hay espacio
   - ✅ Páginas asignadas a swap cuando RAM está llena

3. **Gestión de Memoria**
   - ✅ Seguimiento de páginas en RAM y swap
   - ✅ Detección de memoria insuficiente
   - ✅ Terminación automática cuando no hay memoria disponible

4. **Operaciones Después de 30 Segundos**
   - ✅ Terminación de proceso aleatorio cada 5 segundos
   - ✅ Acceso a dirección virtual aleatoria cada 5 segundos
   - ✅ Detección y reporte de page faults
   - ✅ Simulación de swap con política LRU

5. **Política de Reemplazo**
   - ✅ Implementación de LRU (Least Recently Used)
   - ✅ Registro de tiempo de último acceso para cada página
   - ✅ Reemplazo de página menos recientemente usada

6. **Condiciones de Terminación**
   - ✅ Programa termina si no hay memoria suficiente para nuevo proceso
   - ✅ Programa termina si no hay espacio en swap
   - ✅ Resumen completo al finalizar

## Archivos Entregados

1. **paging_simulator.c** - Código fuente principal (18 KB, ~532 líneas)
2. **Makefile** - Configuración de compilación
3. **README.md** - Documentación completa con instrucciones de uso
4. **.gitignore** - Excluye binarios compilados
5. **test_simulator.sh** - Script de prueba automatizado
6. **IMPLEMENTATION_SUMMARY.md** - Este archivo de resumen

## Características Técnicas

- **Lenguaje**: C estándar (C99)
- **Compilador**: GCC con todas las advertencias habilitadas
- **Plataforma**: UNIX/Linux
- **Gestión de memoria**: Asignación dinámica con liberación apropiada
- **Sin warnings de compilación**
- **Sin memory leaks**
- **Sin vulnerabilidades de seguridad**

## Funcionalidades Verificadas

### Proceso de Creación
```
[PROCESO CREADO] PID: 10, Tamaño: 4 MB, Páginas: 1024
  -> Páginas en RAM: 512, Páginas en swap: 512
  -> Uso de RAM: 12800/12800 páginas (100.0%)
  -> Uso de swap: 512/11776 páginas (4.3%)
```

### Terminación de Proceso
```
[PROCESO TERMINADO] PID: 1, Páginas liberadas: 512
  -> RAM liberada: 12288/12800 páginas (96.0%)
  -> Swap usado: 7168/11776 páginas (60.9%)
```

### Page Fault y Swap
```
[ACCESO A DIRECCIÓN VIRTUAL] PID: 15, Dirección: 0x25642B
  -> Página del proceso: 621
  -> [PAGE FAULT] Página en swap (índice 5997) - Total page faults: 1
  -> Aplicando política LRU: reemplazando página en frame 0
  -> Página cargada exitosamente en RAM (frame 0)
```

### Terminación por Memoria Insuficiente
```
[ERROR] No hay suficiente memoria disponible (RAM + swap)
Se requieren 2560 páginas pero solo hay 1280 disponibles
Terminando simulación...

=== Resumen de la simulación ===
Total de procesos creados: 18
Total de page faults: 1
Uso final de RAM: 7680/7680 páginas (100.0%)
Uso final de swap: 14848/16128 páginas (92.1%)
```

## Instrucciones de Uso

### Compilación
```bash
make
```

### Ejecución
```bash
./paging_simulator
```

Cuando se solicite, ingrese:
- Tamaño de memoria física (MB): Ejemplo: 50
- Tamaño de página (KB): Ejemplo: 4

### Prueba Automatizada
```bash
./test_simulator.sh
```

## Notas Importantes

- El simulador usa `sleep(1)` para simular el paso del tiempo
- Los tamaños de proceso se generan aleatoriamente entre 1 y 10 MB
- La memoria virtual se calcula aleatoriamente entre 1.5 y 4.5 veces la memoria física
- La política LRU mantiene en RAM las páginas más recientemente accedidas
- El programa termina automáticamente cuando no hay memoria suficiente

## Autor

Proyecto desarrollado como parte de la Tarea 3 del curso de Sistemas Operativos.
Universidad Diego Portales - Facultad de Ingeniería y Ciencias

## Fecha de Entrega

Noviembre 2025
