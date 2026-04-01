# Esquemas en texto

## 1. Esquema funcional completo

```text
[Control Bluetooth]
        │
        │  enlace inalámbrico
        ▼
[BlueRetro]
  Sistema: PSX/PS2
  Modo:    GamePad
  Salida:  DualShock 2 emulado
        │
        │  bus PS2 a 3.3 V
        ▼
[Level shifter 4 canales]
  HV = 5 V
  LV = 3.3 V
  GND común
  Pull-ups lado PS2 = 1 kΩ
        │
        ▼
[ATmega32u4]
  D2 = DAT
  D3 = CMD
  D4 = ATT
  D5 = CLK
  firmware PS2toXBOX + OGXBOX-PAD
        │
        │  interfaz OG Xbox
        ▼
[Xbox original]
```

## 2. Esquema de conexiones señal por señal

```text
ATmega32u4      Level Shifter            BlueRetro / Salida PS2
-----------     ------------------       ----------------------
D3 -----------  HV1 <-> LV1  ----------  CMD
D4 -----------  HV2 <-> LV2  ----------  ATT
D5 -----------  HV3 <-> LV3  ----------  CLK
D2 <--------->  HV4 <-> LV4  <-------->  DAT
5V -----------  HV
3.3V ---------  LV --------------------  VCC
GND ----------  GND -------------------  GND
```

## 3. ACK opcional para expansión futura

```text
BlueRetro ACK -------------------------> D6 (opcional)
                                         |
                                         +-- pull-up 1 kΩ a 3.3 V
```

## 4. Esquema de alimentación recomendado

```text
Fuente principal sistema
   │
   ├── 5 V  ---------------------------> Pro Micro / ATmega32u4
   │
   └── Regulador 3.3 V ----------------> BlueRetro lado PS2
                                         Level shifter LV
```

## 5. Modificaciones clave frente al proyecto original

```text
Proyecto original:
  PS2 físico -> Pro Micro -> Xbox OG

Variante documentada:
  Control BT -> BlueRetro -> PS2 emulado -> level shifter -> Pro Micro -> Xbox OG
```

## 6. Esquema mental para depurar

```text
Si falla pulsación:
  revisar primero firmware variante elegida
  luego configuración BlueRetro
  luego cableado DAT/CLK/ATT/CMD
  luego pull-ups

Si falla rumble:
  revisar si Accessories = Rumble
  revisar estabilidad base sin rumble
  revisar que el problema no aparezca solo con cierto mando Bluetooth

Si hay inputs fantasmas:
  revisar cableado
  revisar masa común
  revisar si la variante de firmware es demasiado agresiva
  revisar si BlueRetro tiene config previa persistente
```
