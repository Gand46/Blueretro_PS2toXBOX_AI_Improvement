# Plan de validación

## Objetivo

Validar no solo que “funciona”, sino que funciona de forma reproducible y con una UX aceptable.

## Matriz mínima de pruebas

### A. Pruebas base sin rumble

1. arranque en frío
2. detección del mando al primer intento
3. botón A/B/X/Y
4. D-pad en cuatro direcciones
5. stick izquierdo centrado y recorrido completo
6. stick derecho centrado y recorrido completo
7. Start / Back / L3 / R3 si aplica al mapeo

### B. Pruebas de reconexión

1. apagar el mando y volver a encenderlo
2. reiniciar BlueRetro
3. desconectar alimentación de BlueRetro y restaurarla
4. comprobar que el estado Xbox vuelva a neutro y no queden botones pegados

### C. Pruebas con rumble

1. activar `Accessories = Rumble`
2. validar que no aparezca vibración residual
3. validar que no se congelen las pulsaciones bajo vibración
4. comparar con más de un mando Bluetooth si el rumble es muy débil

### D. Pruebas de estabilidad prolongada

1. 10 a 15 minutos de juego continuo
2. cambios rápidos de dirección
3. pulsaciones repetitivas
4. entradas simultáneas stick + botones + trigger

## Qué registrar en cada prueba

- firmware usado
- versión BlueRetro
- si BlueRetro es HW1 o HW2
- mando Bluetooth usado
- si `Accessories` estaba en `None` o `Rumble`
- síntoma observado
- si el problema desaparece al volver a la variante endurecida

## Criterio de aprobación recomendado

Aprobar una build solo si:

- no hay inputs espontáneos
- no quedan botones pegados al desconectar
- no hay arrastre molesto de pulsaciones
- el rumble no provoca pérdida de lectura
