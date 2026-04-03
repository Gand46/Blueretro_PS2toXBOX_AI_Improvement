# Firmware recomendado y variantes

## 1. Variante recomendada

### `PS2toXBOX_blueretro_hardened.ino`

Usa esta variante como firmware principal del repositorio.

#### Prioridades

- estabilidad
- menos inputs fantasmas
- sin renegociación en caliente
- manejo conservador del rumble

#### Cuándo usarla

- primer montaje
- validación inicial
- documentación pública
- pruebas con más de un mando Bluetooth

## 2. Variante opcional

### `PS2toXBOX_ajuste_respuesta_rapida.ino`

Variante enfocada a mejorar sensación de respuesta en pulsaciones.

#### Prioridades

- menor percepción de “botón arrastrado”
- respuesta más inmediata
- mantener sticks analógicos

#### Riesgo

Puede ser más sensible al mando o a la configuración BlueRetro que la variante endurecida.

## 3. Variante que no debe publicarse como principal

Cualquier build experimental que:

- renegocie rumble o capacidades en pleno runtime
- insista en pressure buttons como camino principal para todo
- haya mostrado inputs espontáneos en pruebas

## 5. Estrategia de publicación sugerida

Si subes esto a GitHub:

- marca la variante endurecida como `recommended`
- deja la rápida como `optional`
- documenta claramente que el comportamiento puede variar según el mando Bluetooth


## 6. Variante ACK (PoC experimental)

### `PS2toXBOX_blueretro_ack_poc.ino`

Variante experimental para validar sincronización por ACK usando `PsxNewLib` rama `devel` y driver `PsxDriverHwSpiWithAck`.

#### Requisitos

- wiring HW SPI del ATmega32u4 (líneas SPI nativas por ICSP)
- pin ATT dedicado
- pin ACK dedicado
- librería `PsxNewLib` rama `devel`

#### Limitaciones actuales

- enfocada a estabilidad de lectura (no incluye manejo de rumble en esta PoC)
- no debe reemplazar la variante `hardened` hasta completar pruebas E del plan de validación
