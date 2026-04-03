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

## Recomendación de trazabilidad (auditoría 2026-04-03)

Para reducir regresiones al actualizar componentes, se recomienda mantener una tabla de control con:

- nombre de dependencia
- versión/tag/commit validado
- fecha de validación
- hardware usado (BlueRetro HW1/HW2, mando probado)
- resultado (OK / observaciones)

Y aplicar la regla:

1. actualizar una sola dependencia por iteración,
2. ejecutar `docs/VALIDATION_PLAN.md`,
3. registrar resultado en `CHANGELOG.md`.

## Validación práctica de entorno

Se documentó una ejecución real de instalación y compilación en:

- `docs/ENV_SETUP_AND_BUILD_VALIDATION.md`

Incluye comandos ejecutados, commits de dependencias usadas y resultado de build de ambos firmwares.


## Evaluación de ACK y migración de librería

Análisis técnico comparativo en:

- `docs/ACK_STRATEGY_AND_LIBRARY_EVALUATION.md`


## Dependencias obligatorias para ACK PoC

Para `firmware/PS2toXBOX_blueretro_ack_poc.ino` se validó la siguiente matriz exacta:

- `PsxNewLib` rama `devel` commit `a69a65245478bc027150a199c099c780ba7dcb3e`
- `DigitalIO` commit `00fa53d`
- `OGXBOX-PAD` commit `5160fd1`
- toolchain:
  - `arduino-builder 1.3.25`
  - `avr-gcc 7.3.0`

### Instalación de librerías (referencia)

```bash
git clone --depth 1 --branch devel https://github.com/SukkoPera/PsxNewLib.git
git clone --depth 1 https://github.com/greiman/DigitalIO.git
git clone --depth 1 https://github.com/eolvera85/OGXBOX-PAD.git
```

### Nota crítica

La PoC ACK **no** usa `PsxControllerBitBang` del `master`; depende del driver `PsxDriverHwSpiWithAck` presente en `devel`.
