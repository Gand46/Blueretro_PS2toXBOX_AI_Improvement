# Compatibilidad BlueRetro actual

## Estado del upstream

BlueRetro quedó archivado en GitHub en diciembre de 2025. La última release estable publicada quedó marcada como **v25.04**, y también existe una **pre-release v25.10-beta**.

## Configuración PSX/PS2 relevante

Para este proyecto importa específicamente la sección **PSX/PS2** del manual de BlueRetro:

- `Mode = GamePad` emula un **DualShock 2**
- `Accessories = None` emula un **DualShock 2 sin rumble**
- `Accessories = Rumble` emula un **DualShock 2 con rumble**
- el **multitap PS2 no está soportado**

## Qué significa esto para este proyecto

### Lo bueno

- BlueRetro sí expone una ruta oficialmente documentada que encaja con `PS2toXBOX`
- no estamos forzando un modo raro o undocumented
- la emulación base de DualShock 2 existe y es usable

### Lo delicado

- BlueRetro no es un DualShock 2 físico real
- la calidad del rumble y de algunos reportes puede variar según el mando Bluetooth usado
- los cambios recientes de BlueRetro muestran que rumble e inputs glitcheados han sido áreas activas de corrección

## Hallazgos útiles de releases/issues

### Relevantes para esta integración

- en **1.9.2** BlueRetro añadió soporte para **PS2/3 pressure buttons**
- en **24.10** rehizo el rumble para soportar intensidad variable y doble motor
- en **24.10** también corrigió un caso específico de **PS2 RE4 rumble**
- en **24.10** ignoró reportes inválidos de controles PS3 que generaban **glitched inputs**
- en **25.01** corrigió **rumble update delay**

## Qué sí se puede corregir desde el Pro Micro

- no renegociar capacidades en caliente
- elegir mejor qué entradas se tratan como analógicas y cuáles como digitales
- resetear a neutro al desconectar
- endurecer la reconexión
- evitar que una mala lectura ocasional destruya la sesión entera

## Qué no se arregla mágicamente desde el Pro Micro

- reportes defectuosos originados por cierto mando Bluetooth
- diferencias internas de compatibilidad entre modelos de control
- bugs de BlueRetro específicos de cierta versión o cierto sistema

## Configuración recomendada de partida

```text
Sistema:      PSX/PS2
Mode:         GamePad
Accessories:  None
Preset:       Default / sin remapeos especiales
```

Luego:

```text
Accessories:  Rumble
```

## Orden correcto de pruebas

1. estabilidad sin rumble
2. botones y sticks
3. reconexión
4. activación de rumble
5. comparación entre mandos Bluetooth si aparece un síntoma raro
