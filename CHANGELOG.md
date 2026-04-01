# Changelog de esta variante documentada

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
