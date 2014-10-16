# Dor
Dor es un lenguaje funcional, estrictamente tipado.

Por el momento tiene un type-checker con inferencia de tipos: soporta polimorfismo parametrico (sin constraints/interfaces), definicion de tipos algebraicos.

Tambien tiene un generador de codigo basico, capaz de generar codigo para estatutos condicionales, llamadas y expresiones aritmeticas. 

## Instrucciones
Compilacion

Para compilar (solo los cpp que han cambiado), se usa

    make
    
Para remover binarios y archivos objeto,
    make clean

Ejecucion

    ./bin/dorc <samples/semantica/correcto.dor 

La salida es un "dump" del arbol semantico generado y del codigo binario.

## TODO

* Muy importante: usar correctamente line y column, para marcar los errores de sintaxis exactamente donde ocurren. Por el momento no se transfiere correctamente la linea de los tokens al AST ni al arbol semantico, y la columna ni siquiera se calcula.

* Reemplazar los asserts por una forma mas robusta de fallar y reportar errores.

