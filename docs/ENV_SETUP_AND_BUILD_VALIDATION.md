# Preparación de entorno y validación de build (2026-04-03)

Este documento deja un procedimiento reproducible para **descargar, instalar y validar** el entorno de pruebas del proyecto.

## 1) Paquetes del sistema instalados

Se instaló el stack de compilación AVR/Arduino desde Ubuntu:

- `arduino`
- `arduino-builder`
- `arduino-core-avr`
- `gcc-avr`
- `avr-libc`
- `avrdude`

Comando usado:

```bash
apt-get update -y
apt-get install -y arduino-builder arduino-core-avr arduino
```

## 2) Dependencias del proyecto descargadas e instaladas

Se instalaron las dependencias en el entorno local:

- `PsxNewLib` (commit `334c3f6`)
- `DigitalIO` (commit `00fa53d`)
- `OGXBOX-PAD` (commit `5160fd1`)

Comandos usados:

```bash
mkdir -p /root/Arduino/libraries
cd /root/Arduino/libraries
git clone --depth 1 https://github.com/SukkoPera/PsxNewLib.git
git clone --depth 1 https://github.com/greiman/DigitalIO.git
git clone --depth 1 https://github.com/eolvera85/OGXBOX-PAD.git
```

## 3) Registro del core OGXBOX-PAD para compilación

`OGXBOX-PAD` se integra como plataforma hardware personalizada para que el include `OGXBOX.h` resuelva desde el core.

Comandos usados:

```bash
mkdir -p /root/Arduino/hardware/OGXBOX-PAD
ln -s /root/Arduino/vendor/OGXBOX-PAD /root/Arduino/hardware/OGXBOX-PAD/avr
```

## 4) Validación de compilación ejecutada

Para evitar colisiones por múltiples `.ino` en el mismo directorio, cada firmware se compiló como sketch independiente temporal.

### 4.1 Firmware hardened

```bash
arduino-builder -compile -logger=machine \
  -hardware /usr/share/arduino/hardware \
  -hardware /root/Arduino/hardware \
  -tools /usr/share/arduino/tools \
  -tools /usr/share/arduino/hardware/tools/avr \
  -tools /usr/bin \
  -libraries /root/Arduino/libraries \
  -fqbn=OGXBOX-PAD:avr:leonardo \
  -build-path /tmp/arduino-build-hardened \
  /tmp/sketch_hardened/sketch_hardened.ino
```

Resultado:

- **Compila OK**
- Program storage: **9068 bytes (31%)** de 28672
- RAM global: **485 bytes (18%)** de 2560

### 4.2 Firmware respuesta rápida

```bash
arduino-builder -compile -logger=machine \
  -hardware /usr/share/arduino/hardware \
  -hardware /root/Arduino/hardware \
  -tools /usr/share/arduino/tools \
  -tools /usr/share/arduino/hardware/tools/avr \
  -tools /usr/bin \
  -libraries /root/Arduino/libraries \
  -fqbn=OGXBOX-PAD:avr:leonardo \
  -build-path /tmp/arduino-build-rapida \
  /tmp/sketch_rapida/sketch_rapida.ino
```

Resultado:

- **Compila OK**
- Program storage: **9600 bytes (33%)** de 28672
- RAM global: **488 bytes (19%)** de 2560

## 5) Herramientas confirmadas en entorno

- `arduino-builder`: 1.3.25
- `avr-gcc`: 7.3.0

## 6) Observación técnica

Durante compilación aparece advertencia de plataforma OGXBOX-PAD (`platform.txt` con campo legado `compiler.path`), pero no bloquea el build.
