# Dependencias y entorno

## Hardware objetivo

- **ATmega32u4 a 16 MHz**  
  Ejemplos: Pro Micro 5V/16MHz, Arduino Micro, Arduino Leonardo.

## Librerías principales

### 1. PsxNewLib

Usada para leer el bus PSX/PS2.

Responsabilidades:
- inicialización del dispositivo PS2
- lectura de botones y sticks
- negociación de modo analógico
- soporte de pressure buttons y rumble cuando la cadena lo permite

### 2. DigitalIO

Dependencia de bajo nivel requerida por `PsxNewLib` en varios entornos AVR.

### 3. OGXBOX-PAD

Usada para presentar el estado final hacia la Xbox original.

## Dependencias de hardware no Arduino

- **BlueRetro**
- **level shifter 4 canales**
- **regulación a 3.3 V**
- cableado hacia puerto/cable de control de Xbox OG

## Entorno sugerido de compilación

- Arduino IDE reciente o `arduino-cli`
- placa basada en ATmega32u4 a 16 MHz
- verificar que las librerías se instalen desde sus repositorios oficiales o equivalentes confiables

## Observación sobre BlueRetro

BlueRetro usa distintas familias de firmware según el hardware:

- **HW1**: adaptadores externos típicos
- **HW2**: instalaciones internas o hardware con soporte extra de gestión de energía/detección

Asegúrate de usar el firmware correcto para tu placa BlueRetro antes de empezar a depurar el lado Pro Micro.
