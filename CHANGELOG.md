# Changelog de esta variante documentada

## v1.0.2-env-setup

Instalación de toolchain y dependencias para validar compilación real de ambos firmwares.

### Incluye

- nuevo documento `docs/ENV_SETUP_AND_BUILD_VALIDATION.md`
- procedimiento reproducible de instalación de entorno
- registro de compilación exitosa para `hardened` y `respuesta_rapida`

## v1.0.1-docs-auditoria

Auditoría técnica integral del repositorio, con hallazgos y backlog de optimizaciones priorizadas.

### Incluye

- nuevo informe `docs/AUDITORIA_TECNICA_2026-04-03.md`
- veredicto técnico de código, dependencias y documentación
- recomendaciones de hardening y trazabilidad de dependencias

## v1.0.0-docs

Primera publicación estructurada de la integración documentada.

### Incluye

- `README.md` orientado a GitHub
- documentación modular en `docs/`
- firmware recomendado `PS2toXBOX_blueretro_hardened.ino`
- firmware opcional `PS2toXBOX_ajuste_respuesta_rapida.ino`
- copia del sketch base en `upstream/`

### Decisiones asentadas

- usar 3.3 V del lado PS2
- usar level shifter con pull-ups reforzados
- priorizar estabilidad sobre pressure buttons
- no usar renegociación en caliente como estrategia estándar
