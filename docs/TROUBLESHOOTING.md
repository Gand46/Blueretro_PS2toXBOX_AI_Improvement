# Troubleshooting

## Síntoma: inputs fantasma o botones que se pulsan solos

### Posibles causas

- cableado débil en DAT/CLK
- masa común deficiente
- pull-ups demasiado altos
- variante de firmware demasiado agresiva
- configuración persistente rara en BlueRetro
- mando Bluetooth concreto con reportes problemáticos

### Qué hacer

1. volver a `PS2toXBOX_blueretro_hardened.ino`
2. desactivar rumble (`Accessories = None`)
3. revisar soldaduras y GND
4. confirmar pull-ups de 1 kΩ
5. probar otro mando Bluetooth

## Síntoma: vibración débil

### Posibles causas

- limitación del propio mando Bluetooth
- traducción de rumble de BlueRetro
- negociación PS2 parcial o no ideal

### Qué hacer

1. confirmar que BlueRetro está en `Accessories = Rumble`
2. comparar con otro mando Bluetooth
3. validar primero que la cadena sin rumble sea perfecta
4. no introducir más cambios a la vez

## Síntoma: vibración residual o permanente

### Posibles causas

- lógica de rumble mal retenida en el sketch
- estados inconsistentes durante reconfiguración en caliente

### Qué hacer

- usar la variante endurecida
- evitar builds experimentales previas
- reiniciar BlueRetro y el Pro Micro después de cambiar firmware

## Síntoma: botones lentos o arrastrados

### Posibles causas

- dependencia excesiva de pressure buttons
- polling demasiado conservador

### Qué hacer

- probar la variante rápida
- mantener la variante endurecida como referencia de estabilidad

## Síntoma: desconexiones intermitentes

### Posibles causas

- timing del bus PS2
- alimentación inestable
- problema del mando Bluetooth
- problema propio de cierta release/configuración de BlueRetro

### Qué hacer

1. revisar alimentación 3.3 V
2. revisar level shifter
3. probar sin rumble
4. probar otro mando Bluetooth
5. documentar exactamente la versión BlueRetro y el hardware HW1/HW2
