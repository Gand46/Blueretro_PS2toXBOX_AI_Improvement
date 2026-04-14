# Evaluación ACK PS2 y viabilidad de migración de librería (GitHub) — 2026-04-03

## Objetivo

Evaluar si conviene:

1. implementar soporte real de línea **ACK** en la cadena actual, o
2. migrar a una librería/controlador más completo para PS2.

La evaluación se basó en repositorios de GitHub y revisión de código relevante.

## Proyectos analizados (GitHub)

## 1) SukkoPera/PsxNewLib

- Repo: https://github.com/SukkoPera/PsxNewLib
- `master`: `334c3f64327e798150a1d9b24cf685883b2762b4`
- `devel`: `a69a65245478bc027150a199c099c780ba7dcb3e`

### Hallazgo clave

En `README.md` de `master`, el propio autor indica explícitamente que la versión actual **no usa ACK** y que la rama `devel` sí avanza en esa dirección.

Además, en `devel` existe implementación concreta para ACK vía interrupciones:

- `src/PsxDriverHwSpiWithAck.h`
- `src/PsxDriver.h` (flujo que espera ACK por byte con timeout)

Esto confirma que **sí hay base real para ACK**, pero en rama de desarrollo.

## 2) madsci1016/Arduino-PS2X

- Repo: https://github.com/madsci1016/Arduino-PS2X
- `master`: `b4a7f3c6691301c3bbf5f0c2f82662d23fec8aa2`

### Hallazgo clave

El código se apoya intensivamente en delays y recalibración temporal (`read_delay`, `delayMicroseconds`, `delay`) en lugar de una ruta robusta de sincronización con ACK por byte.

Conclusión práctica: como baseline histórico funciona, pero **no mejora claramente** la robustez de ACK para este proyecto frente a continuar con PsxNewLib.

---

## Viabilidad técnica para este proyecto

## Opción A — Mantener `PsxNewLib master` (estado actual)

**Ventajas**

- Menor riesgo inmediato.
- Se mantiene compatibilidad con firmware actual.

**Desventajas**

- Sigue sin ACK efectivo.
- Mayor sensibilidad a timing eléctrico y variación por mando.

## Opción B — Migrar a `PsxNewLib devel` con ACK (recomendada como PoC)

**Ventajas**

- Ruta técnica explícita para ACK real.
- Diseño más alineado con sincronización por byte y timeout.

**Riesgos**

- Rama no estable (`devel`), posible cambio de API.
- Requiere adaptar firmware actual al nuevo modelo de driver.
- En AVR, ACK por PCINT exige cuidado (latencia/ruido/ISR compartidas).

## Opción C — Migrar a `Arduino-PS2X`

**Ventajas**

- Muy conocido y ampliamente usado en proyectos legacy.

**Desventajas**

- Enfoque más orientado a delays que a ACK robusto.
- Coste de migración sin ganancia clara para la problemática objetivo.

---

## Dictamen

La vía más viable y con mejor relación costo/beneficio es:

1. **No cambiar de familia de librería** (mantener PsxNewLib).
2. Ejecutar **PoC controlada con `PsxNewLib devel` + ACK**.
3. Promover a rama principal solo si supera criterios de estabilidad del proyecto.

---

## Plan propuesto de implementación (faseado)

## Fase 1 — Preparación de hardware

- Cablear línea ACK desde BlueRetro/PS2 a un pin del ATmega32u4 apto para PCINT.
- Mantener pull-up de 1 kΩ a 3.3 V del lado PS2.
- No retirar la ruta actual (rollback simple).

## Fase 2 — Rama experimental

- Crear rama `feature/ack-poc`.
- Integrar `PsxNewLib devel` en entorno de build reproducible.
- Adaptar solo el firmware `PS2toXBOX_blueretro_hardened.ino` primero.

## Fase 3 — Validación técnica

- Ejecutar `docs/VALIDATION_PLAN.md` completo en dos modos:
  - `Accessories=None`
  - `Accessories=Rumble`
- Añadir dos métricas nuevas:
  - tasa de lecturas fallidas por minuto,
  - reconexiones forzadas por hora.

## Fase 4 — Go/No-Go

Promover ACK a baseline solo si:

- no aumenta inputs fantasmas,
- no empeora reconexión,
- mantiene o mejora la estabilidad con rumble,
- no incrementa uso de CPU/latencia perceptible.

---

## Modificaciones recomendadas en este repo (derivadas de esta evaluación)

1. Reservar en documentación de cableado una línea dedicada para ACK (aunque la rama actual no la consuma todavía).
2. Añadir sección de “Matriz ACK PoC” en el plan de validación.
3. Mantener pinning de commit de librería para separar claramente `master` vs `devel` durante ensayos.

---

## Estado de implementación en este repositorio

Se implementó una primera versión funcional en:

- `firmware/PS2toXBOX_blueretro_ack_poc.ino`

Características de la implementación:

- usa `PsxDriverHwSpiWithAck<PIN_PS2_ATT, PIN_PS2_ACK>`
- mantiene mapeo digital conservador para botones
- conserva sticks analógicos
- añade reconexión y limpieza de estado Xbox
- deja rumble fuera de alcance en esta PoC para aislar validación de ACK
