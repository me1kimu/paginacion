#!/bin/bash
# Test script to demonstrate the paging simulator

echo "=== Test del Simulador de Paginación ==="
echo ""
echo "Este script ejecutará el simulador por 36 segundos para demostrar todas las funcionalidades."
echo ""
echo "Características a demostrar:"
echo "- Creación de procesos cada 2 segundos"
echo "- Asignación de páginas a RAM y swap"
echo "- Después de 30 segundos: terminación de procesos cada 5 segundos"
echo "- Después de 30 segundos: acceso a direcciones virtuales cada 5 segundos"
echo "- Page faults cuando se acceden páginas en swap"
echo "- Política de reemplazo LRU"
echo ""
echo "Presione Enter para continuar..."
read

echo "Ejecutando simulador con 50 MB de RAM y páginas de 4 KB..."
echo ""

# Run simulator for 36 seconds
printf "50\n4\n" | timeout -s INT 36 ./paging_simulator

echo ""
echo "=== Test completado ==="
