# Hardware y cableado

## Principio eléctrico básico

El lado **PS2/BlueRetro** debe tratarse como un dispositivo de **3.3 V**. Si tu ATmega32u4 trabaja a 5 V, necesitas adaptación de nivel.

## Pinout lógico usado por el firmware

```text
ATmega32u4     Señal PS2
-----------    ----------
D2             DAT
D3             CMD
D4             ATT
D5             CLK
3.3V           VCC lado PS2/BlueRetro
GND            GND común
```

## Cableado recomendado

```text
Pro Micro / Leonardo / Micro        Level shifter            BlueRetro (PS2)
------------------------------      -------------------      ----------------
D3  ----------------------------->  HV1 <-> LV1  ----------> CMD
D4  ----------------------------->  HV2 <-> LV2  ----------> ATT
D5  ----------------------------->  HV3 <-> LV3  ----------> CLK
D2  <---------------------------->  HV4 <-> LV4  <---------> DAT
5V  ----------------------------->  HV
3.3V regulator ------------------>  LV ---------------------> VCC PS2 side
GND ----------------------------->  GND --------------------> GND
```

## ACK opcional

Aunque la rama estable de `PsxNewLib` no lo usa todavía, conviene dejar previsto el pin **ACK** para una futura evolución del firmware o para pruebas experimentales.

Sugerencia:

```text
BlueRetro/PS2 ACK ----> pin libre ATmega32u4 (ej. D6)
                         + pull-up a 3.3 V
```

## Pull-ups

Este punto es crítico.

Los level shifters tipo BSS138 suelen traer pull-ups de **10 kΩ**, que en esta aplicación son demasiado débiles para una buena compatibilidad. Para este proyecto conviene dejar **1 kΩ** en el lado PS2.

### Formas de hacerlo

- reemplazar los 10 kΩ por 1 kΩ
- o poner 1 kΩ en paralelo a cada línea usada del lado PS2

## Alimentación

### Lado PS2 / BlueRetro

- 3.3 V

### Lado Pro Micro 5V/16MHz

- 5 V

### Si usas un ATmega32u4 a 3.3 V

- el level shifter podría omitirse
- aun así los pull-ups fuertes siguen siendo recomendables

## Rumble y motor power

### Control PS2 físico real

En un control PS2 físico, el pin de **Motor Power** importa para alimentar correctamente los motores.

### BlueRetro emulando PS2

En BlueRetro el comportamiento de rumble depende sobre todo de:

- configuración de BlueRetro
- traducción del dispositivo Bluetooth
- negociación PS2 de rumble
- lógica del sketch del Pro Micro

Por eso el rumble debe considerarse una función **de validación tardía**, no de primera puesta en marcha.

## Lado Xbox original

Desde el ATmega32u4 hacia la consola, el proyecto usa el stack de `OGXBOX-PAD`. Mantén buena integridad física en:

- D+
- D-
- 5 V
- GND

## Buenas prácticas físicas

- masa común sólida
- señales PS2 cortas
- soldaduras limpias en DAT y CLK
- desacoplo adecuado entre BlueRetro y la lógica auxiliar
- evitar cableados largos y sueltos durante pruebas de timing
