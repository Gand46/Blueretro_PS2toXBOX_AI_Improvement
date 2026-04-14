# Puesta en marcha rápida

## 1. Hardware mínimo

- placa con **ATmega32u4 a 16 MHz**
- acceso a cable de control de **Xbox original**
- **BlueRetro** con firmware adecuado para tu hardware (HW1 o HW2)
- adaptador **level shifter 4 canales**
- regulación de **3.3 V** para el lado PS2
- resistencias o adaptación para dejar **1 kΩ** de pull-up del lado PS2

## 2. Configuración inicial recomendada de BlueRetro

- Sistema: **PSX/PS2**
- Mode: **GamePad**
- Accessories: **None**
- sin remapeos especiales
- sin pruebas avanzadas de rumble todavía

## 3. Primer firmware a cargar

Empieza con:

- `firmware/PS2toXBOX_blueretro_hardened.ino`

## 4. Primeras pruebas

Haz estas pruebas antes de activar rumble:

1. detección al encender
2. botones de cara
3. cruceta
4. sticks centrados sin drift
5. reconexión tras apagar y volver a conectar el mando Bluetooth
6. reconexión tras reiniciar BlueRetro

## 5. Activación de rumble

Solo después de que la cadena anterior esté estable:

- cambia BlueRetro a `Accessories = Rumble`
- vuelve a probar
- confirma que no aparezcan inputs fantasmas, congelamiento de botones o vibración residual

## 6. Variante de respuesta rápida

Si la variante endurecida funciona bien pero sientes los botones lentos:

- prueba `firmware/PS2toXBOX_ajuste_respuesta_rapida.ino`
- documenta qué mando Bluetooth y qué firmware BlueRetro estabas usando


## 7. PoC ACK (experimental)

Si quieres validar ACK real:

- usa `firmware/PS2toXBOX_blueretro_ack_poc.ino`
- cablea ATT + ACK y SPI hardware del 32u4
- instala `PsxNewLib` en rama `devel` (no `master`)
- ejecuta bloque **E. Pruebas ACK (PoC)** de `docs/VALIDATION_PLAN.md`
