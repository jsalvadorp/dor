# Dor
Dor es un lenguaje funcional, estrictamente tipado.

Por el momento solo funciona el lexer y el parser.

## Instrucciones
Compilacion

    make

Ejecucion

    ./bin/dorc <samples/correcto.dor

La salida es un "dump" del arbol sintactico generado.

## TODO

* usar correctamente line y column, para marcar los errores de sintaxis exactamente donde ocurren.

* reemplazar los asserts por una forma mas robusta de fallar y reportar errores.

## Trabajando en

Por el momento se esta escribiendo el analizador semantico, que convierte el arbol de sintaxis en uno anotado que se pueda compilar y checar en tipado.
