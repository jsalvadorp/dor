# Dor
Dor es un lenguaje funcional, estrictamente tipado.

Por el momento tiene un type-checker con inferencia de tipos: soporta polimorfismo parametrico (sin constraints/interfaces), definicion de tipos algebraicos.

Tambien tiene un generador de codigo basico, capaz de generar codigo para estatutos condicionales, llamadas, pattern matching y expresiones aritmeticas. 

La maquina virtual es capaz de correr los binarios, aunque sin recoleccion de memoria automatica aun.

## Instrucciones
Compilacion

Para compilar el compilador y la máquina

    make
    cd dorvm
    make
    cd ..

    
Para remover binarios y archivos objeto, tanto en el directorio de proyecto como en dorvm/
    make clean
    cd dorvm
    make clean
    cd ..

Compilacion y ejecucion del programa ejemplo 

    ./bin/dorc tests/pruebas_finales.dor
    ./dorvm/bin/dorvm a.out


La salida del compilador es un "dump" del arbol semantico generado y del codigo binario. La salida de la VM es la salida del programa. El binario generado siempre se llama a.out.

## TODO

* Muy importante: usar correctamente line y column, para marcar los errores de sintaxis exactamente donde ocurren. Por el momento no se transfiere correctamente la linea de los tokens al AST ni al arbol semantico, y la columna ni siquiera se calcula.

* Reemplazar los asserts por una forma mas robusta de fallar y reportar errores.

