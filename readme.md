1. Simulador de Gestión de Memoria Paginación y Swap
	1.1 Este proyecto es una simulación en C de un administrador de memoria de Sistema Operativo. Implementa un sistema de paginación que gestiona la memoria física (RAM) y utiliza un espacio de intercambio (Swap) cuando la RAM se llena, aplicando el algoritmo LRU 
	para el reemplazo de páginas.

2.Características
	2.1 Gestión de Memoria Virtual:Simula un espacio de direcciones mayor a la memoria física disponible.
	2.2 Paginación: Divide la memoria en marcos y páginas de tamaño fijo
	2.3 Swap: Mueve páginas de la RAM al disco (Swap) cuando no hay espacio físico.
	2.4 Algoritmo LRU: Reemplaza las páginas que llevan más tiempo sin usarse cuando ocurre un Page Fault.
	2.5 Simulación de Eventos: Genera procesos, accesos a memoria y terminación de procesos de forma dinámica.



3. Compilación y Ejecución
	3.1 El proyecto incluye un `Makefile` para automatizar la compilación.
	3.2 Compilar: Abrir la terminal en la carpeta del proyecto y ejecuta: make
	3.3 Esto generará el ejecutable simulador_paginacion.


4. Ejecutar
	4.1 Para iniciar la simulación se utiliza: make run
	4.2 También puede ser: ./simulador_paginacion

5. Limpiar (Opcional): Para borrar los archivos generados (.o y ejecutable): make clean

6. Uso del Simulador
	6.1 Al iniciar el programa, pide onfigurar los parámetros iniciales. Ejemplo recomendado para probar:
	6.2 Tamaño memoria física (MB): 100 (Ej. 100 MB de RAM).
	6.3 Tamaño de página (MB): 10 (Ej. Páginas de 10 MB).
	6.4 Tamaño mínimo de proceso: 20.
	6.5 Tamaño máximo de proceso: 50.
	6.6 NOTA: El sistema calcula automáticamente el tamaño del Swap basándose en un factor aleatorio (entre 1.5x y 4.5x la RAM).

7. ¿Cómo funciona la simulación? El programa corre un bucle principal que simula el paso del tiempo:
	7.1 Fase de Carga (0 - 30 segundos):
			El sistema crea un nuevo proceso cada 2 segundos.
			Intenta asignar páginas en RAM; si no cabe, usa Swap.
	7.2 Fase de Eventos Aleatorios (> 30 segundos):
			Además de crear procesos, ocurren dos eventos aleatorios cada 5 segundos:
			Terminación de procesos: Se libera la memoria de un proceso al azar.
	7.3 Acceso a memoria virtual: Se intenta leer una dirección de memoria específica.
			Si la página está en RAM → Acceso exitoso.
			Si la página está en Swap → Page Fault (Fallo de página). El sistema trae la página a RAM, moviendo otra a 
			Swap usando el algoritmo LRU.
	7.4 La simulación se detiene si la memoria (RAM + Swap) se llena completamente o si se alcanza el tiempo límite de seguridad.

8. Estructura del Proyecto
	8.1 main.c: Contiene el bucle principal de la simulación y la lógica de los eventos temporales.
	8.2 memory.c: Implementa la lógica del gestor de memoria (creación de procesos, paginación, LRU, Swap).
	8.3 memory.h: Archivo de cabecera con las estructuras de datos (Page, Process, MemoryManager).
	8.4 Makefile: Script para compilar y ejecutar el proyecto fácilmente.



