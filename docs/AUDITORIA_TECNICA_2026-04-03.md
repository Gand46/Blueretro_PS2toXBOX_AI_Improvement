# Auditoría técnica y validación (2026-04-03)

## Alcance

Se auditó el repositorio completo en tres dimensiones:

1. **Código firmware** (`firmware/*.ino`).
2. **Dependencias y cadena de build** (documentadas en `docs/DEPENDENCIES.md`).
3. **Documentación funcional/técnica** (`Readme.md` y `docs/*.md`).

## Metodología aplicada

### Validaciones ejecutadas

- inspección estática manual de ambos firmwares.
- revisión de consistencia entre documentos de arquitectura, puesta en marcha y plan de validación.
- barrido de marcadores de deuda técnica (`TODO`, `FIXME`, `HACK`, `XXX`).
- verificación de disponibilidad de herramientas de compilación/lint en el entorno actual.

### Resultado de herramientas en este entorno

- `arduino-cli`: **no disponible**.
- `cppcheck`: **no disponible**.
- `avr-g++`: **no disponible**.

> Conclusión: no se pudo cerrar una validación de compilación automática dentro de este contenedor; la auditoría se basa en revisión estática y coherencia técnica del código y docs.

---

## Hallazgos de código firmware

## 1) Fortalezas detectadas

- Buen manejo de estado de sesión (`haveController`, `caps`, reconexión periódica).
- Estrategia defensiva ante fallos de lectura (`MAX_CONSECUTIVE_READ_FAILURES`) antes de desconectar.
- Neutralización explícita de estado Xbox al perder enlace (`resetXboxState()`), mitigando botones “pegados”.
- Control de rumble desacoplado y con ventana de caducidad (`RUMBLE_HOLD_MS`) para reducir vibración residual.
- Mapeo de sticks con zona muerta y saturación (`mapPsxAxisToXbox`, `clampAxis`) para evitar overflow y drift menor.

## 2) Riesgos / deuda técnica vigente

- **Dependencia de bit-bang PS2 sin ACK efectivo**: sensible a ruido eléctrico y tolerancias temporales.
- **Subclase parcheada** de `PsxControllerBitBang` para acceder a estado interno (`forceRumbleTransport`), acoplando el código a detalles de implementación de librería.
- **Duplicación funcional** entre variante hardened y variante rápida; eleva costo de mantenimiento cuando se corrige un bug en una sola rama.
- **Constantes de tuning embebidas** en código sin perfil externo (p.ej. `RUMBLE_BIG_GAIN_PERCENT`, `PSX_ANALOG_DEADZONE`), lo que dificulta reproducir ajustes por hardware/mando.

## 3) Recomendaciones de optimización (priorizadas)

### Prioridad alta

1. **Unificar núcleo común** (lectura, reconexión, mapeo sticks, limpieza de estado) en un bloque compartido para ambas variantes.
2. **Centralizar parámetros de tuning** en un header común (por variante/perfil) para reducir divergencias.
3. **Agregar verificación de compilación en CI** (al menos build smoke test con `arduino-cli`) para detectar regresiones tempranas.

### Prioridad media

4. Exponer banderas de diagnóstico opcional por `#define` (contadores de fallos, reconexiones, estado rumble).
5. Añadir una pequeña capa de “debounce lógico” para transiciones de botones digitales si aparecen rebotes con ciertos mandos Bluetooth.
6. Documentar huella temporal esperada del loop (100 Hz) y budget por iteración para evitar degradación al añadir lógica.

### Prioridad baja

7. Crear presets por tipo de mando (DS3/DS4/Xbox One/Switch Pro) si se confirma comportamiento diferencial repetible.
8. Evaluar migración futura a lectura con soporte de sincronización más robusta (si el stack lo permite) para reducir sensibilidad a timing.

---

## Auditoría de dependencias

## Estado

Dependencias clave documentadas:

- `PsxNewLib`
- `DigitalIO`
- `OGXBOX-PAD`
- `BlueRetro` (firmware/hardware externo)

## Riesgos detectados

- Falta de **versionado/pinning explícito** en la documentación actual.
- Falta de una **matriz de compatibilidad verificada** (versión de BlueRetro vs comportamiento observado).
- Falta de instrucciones de **verificación post-instalación** reproducibles por comando.

## Recomendación mínima de hardening de dependencias

1. Registrar commit/tag exacto de librerías utilizadas en validación.
2. Mantener una tabla de compatibilidad “probado/no probado”.
3. Establecer un procedimiento de actualización segura:
   - actualizar una dependencia por vez,
   - ejecutar plan `docs/VALIDATION_PLAN.md`,
   - registrar resultado en changelog.

---

## Auditoría de documentación

## Fortalezas

- Estructura modular clara por tema.
- Flujo de onboarding correcto: hardware -> configuración BlueRetro -> firmware recomendado -> validación -> rumble.
- Riesgos reales de la arquitectura descritos de forma útil.

## Mejoras recomendadas

1. Añadir una **sección de “versiones validadas”** (BlueRetro, PsxNewLib, OGXBOX-PAD).
2. Incorporar un **checklist rápido de pre-flasheo** en `docs/GETTING_STARTED.md`.
3. Documentar explícitamente condiciones de prueba mínimas para aceptar cambios (criterio de salida de auditoría).

---

## Veredicto de auditoría

- **Estado general del código**: sólido para el objetivo del proyecto (integración BlueRetro -> PS2 -> Xbox OG).
- **Estado de dependencias**: funcional pero con trazabilidad insuficiente (sin pinning/versiones verificadas).
- **Estado de documentación**: bueno en arquitectura y operación; mejorable en control de versiones de dependencias.

## Dictamen

**Apto para uso experimental/controlado**, con recomendación de completar hardening de dependencias y automatizar compilación antes de considerarlo “baseline estable a largo plazo”.
