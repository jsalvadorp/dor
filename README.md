# Dor
Dor es un lenguaje funcional, estrictamente tipado.

Por el momento tiene un type-checker basico.

## Instrucciones
Compilacion

    make

Ejecucion

    ./bin/dorc <samples/analysis.dor

La salida es un "dump" del arbol semantico generado.

## TODO

* Muy importante: usar correctamente line y column, para marcar los errores de sintaxis exactamente donde ocurren. Por el momento no se transfiere correctamente la linea de los tokens al AST ni al arbol semantico, y la columna ni siquiera se calcula.

* Reemplazar los asserts por una forma mas robusta de fallar y reportar errores.

## Trabajando en

Por el momento se esta escribiendo el analizador semantico, que convierte el arbol de sintaxis en uno anotado que se pueda compilar y checar en tipado. Se tiene una version algo cruda de la inferencia de tipos; falta que se puedan cuantificar las variables de tipos. Una vez hecho eso, implementar polimorfismo parametrico sera muy sencillo
