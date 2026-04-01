# Cambios realizados sobre el proyecto original

## Base de referencia

La base auditada fue el sketch original del proyecto `PS2toXBOX`.

## Objetivo de los cambios

Adaptar el sketch a un escenario que el upstream ya identificaba como sensible:

- **BlueRetro emulando PSX/PS2**
- pressure buttons no siempre ideales
- rumble potencialmente problemático

## Cambios de criterio aplicados

### 1. Estabilidad de conexión

- no marcar sesión válida hasta que la inicialización realmente complete
- no propagar estados basura al perder el mando
- resetear el estado Xbox a neutro al desconectar

### 2. Reconfiguración en caliente

Se descartó como estrategia base.

Motivo:
- la cadena BlueRetro + PS2 bit-bang es sensible a estados de configuración intermedios
- renegociar rumble/capacidades durante runtime indujo comportamientos extraños en pruebas previas

### 3. Mapeo de entradas

Se decidió mantener:
- **sticks analógicos**

Y priorizar ruta digital para:
- cruceta
- botones de cara
- hombros
- según variante, también triggers

Motivo:
- con BlueRetro, pressure buttons pueden sentirse lentos o producir transiciones poco limpias

### 4. Rumble

Se descartaron variantes que:
- dejaban el rumble “enganchado” más tiempo del debido
- intentaban renegociar rumble de forma demasiado agresiva

La variante endurecida trata el rumble como función secundaria y conservadora.

### 5. Manejo de errores de lectura

Se evitó colapsar toda la sesión por una sola lectura fallida. El objetivo es tolerar pequeñas irregularidades sin provocar una desconexión total inmediata.

## Resultado práctico

Después de la auditoría, la conclusión fue esta:

- la variante más publicable como base es **PS2toXBOX_blueretro_hardened.ino**
- la variante rápida debe tratarse como opcional
- las variantes intermedias experimentales no deben publicarse como firmware principal

## Qué no se tocó deliberadamente

- no se reescribió el proyecto para SPI hardware
- no se cambió la librería de salida Xbox
- no se alteró el enfoque general del upstream más de lo necesario

## Por qué se mantuvo esa prudencia

Porque el objetivo no era crear un proyecto totalmente nuevo, sino dejar una rama de trabajo estable, trazable y defendible para una integración concreta con BlueRetro.
