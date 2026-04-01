# Arquitectura del proyecto

## Objetivo

Convertir una salida **PSX/PS2 emulada por BlueRetro** en una salida compatible con **Xbox original**, usando un microcontrolador **ATmega32u4** como puente intermedio.

## Cadena completa

```text
[Control Bluetooth]
        │
        ▼
[BlueRetro]
  salida PSX/PS2 (DualShock 2 emulado)
        │
        ▼
[Level shifter 3.3 V <-> 5 V]
        │
        ▼
[ATmega32u4]
  PsxNewLib + PS2toXBOX + OGXBOX-PAD
        │
        ▼
[Xbox original]
```

## Responsabilidad de cada bloque

### BlueRetro

- recibe el control Bluetooth
- lo remapea a la salida de consola configurada
- en este caso emite un dispositivo **PSX/PS2**
- en `GamePad` emula un **DualShock 2**

### Level shifter

- adapta señales entre el dominio **3.3 V** del lado PS2 y el dominio **5 V** del Pro Micro típico
- ayuda a proteger el lado PS2/BlueRetro
- aporta el soporte adecuado para líneas open-collector si se acompaña de pull-ups correctos

### ATmega32u4

- lee el bus PS2 con `PsxNewLib`
- traduce la entrada al layout esperado por Xbox original
- presenta el dispositivo final a la consola mediante `OGXBOX-PAD`

### Xbox original

- recibe el estado final como si proviniera de un control compatible

## Por qué esta arquitectura existe

BlueRetro no entrega de forma nativa un puerto OG Xbox. En cambio, sí puede emular PSX/PS2. Como `PS2toXBOX` ya resuelve la traducción **PS2 -> Xbox OG**, la combinación permite reutilizar un camino ya probado.

## Riesgos principales de esta arquitectura

- timing del bus PS2 por bit-bang
- dependencia fuerte de nivel lógico y pull-ups
- comportamiento variable del rumble
- diferencia entre un **DualShock 2 real** y un **DualShock 2 emulado por BlueRetro**

## Decisión de diseño más importante

Para BlueRetro actual, el diseño más robusto es tratar la salida PS2 como una fuente **analógica en sticks** pero **conservadora en botones**. En otras palabras:

- sticks: analógicos
- cruceta / botones de cara / hombros: preferentemente digitales
- rumble: opcional y validado al final
