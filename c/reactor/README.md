Reactor
=======

Reactor es software libre, licenciado bajo la GNU GPL.

Copyright (c) 2016 Francisco Moya Fernández

**Se permite la copia y distribución de este archivo, con o sin
modificaciones, siempre que se mantenga la nota de copyright y esta
advertencia.**

-------------------------------------------------------------------

INTRODUCCIÓN
============

Reactor es una pequeña biblioteca para programar sistemas reactivos
utilizando el *patrón reactor*.  No tiene dependencias externas al
margen de la biblioteca estándar pero utiliza algunas características
de GNU libc.  Está concebida como herramienta docente para el [Taller
de Raspberry Pi][] impartido en la Escuela de Ingeniería Industrial de
Toledo, de la Universidad de Castilla-La Mancha.

  [Taller de Raspberry Pi]: https://sites.google.com/site/tallerraspberrypi/


Disposición del código
----------------------

El código de la biblioteca está disponible en la carpeta `reactor`. En
la carpeta `test` se encuentra un conjunto de pequeñas pruebas que
puede ilustrar el uso de cada módulo.  Una descripción más detallada
puede encontrarse en la [documentación del
taller](https://www.gitbook.com/book/franciscomoya/taller-de-raspberry-pi).


OBTENER LA ÚLTIMA VERSIÓN
=========================

Esta biblioteca se mantiene junto al software de apoyo del taller en
un [repositorio GitHub][].  Por favor, ten en cuenta que no hacemos
liberaciones periódicas de esta biblioteca.  Lo más parecido que
tenemos a una *release* es el objetivo `dist` del *makefile*.

  [repositorio GitHub]: https://github.com/FranciscoMoya/rpi-src

Puedes obtener una copia de solo lectura utilizando GIT:

``` git clone git@github.com:FranciscoMoya/rpi-src.git
```


Compilar el código
------------------

Normalmente todo lo que es necesario hacer para compilar la biblioteca
es ejecutar `make`.  Los `makefile` incluídos en la biblioteca
compilan el código como una biblioteca estática.  Mira el `makefile`
de la carpeta `test` para ver cómo se construyen ejecutables empleando
*Reactor*.



REPORTAR ERRORES
================

Si crees que has encontrado un error, por favor, comprueba que estás
utilizando la última versión.  Prepara un ejemplo tan pequeño como sea
posible que muestre el error y envía la información a
francisco.moya@uclm.es.
