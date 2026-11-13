# Dépôt source Retro Media Lab

Consultation initiale : 13 novembre 2026, avec les outils GitHub, en lecture seule.

## Périmètre

- Source C# : https://github.com/overthetop78/Retro-Media-Lab
- Branche par défaut constatée : `main`.
- Le dépôt source ne doit pas être modifié. Les modifications du portage sont limitées à `Retro-Media-Lab_CPP`.
- Cette consultation vérifie l'accès et repère la structure ; elle ne constitue pas un audit du code ni un début de migration.

## Informations consultées

Le [README](https://github.com/overthetop78/Retro-Media-Lab/blob/main/README.md) présente une application Windows en C#/.NET 10 et WPF pour les opérations Greaseweazle, le traitement et l'exploration des images disque et l'émulation intégrée.

SHA du README consulté : `27d0c919ce2eceb0169508b97ed90ac28658023e`.

La [racine](https://github.com/overthetop78/Retro-Media-Lab) contient notamment `RetroMediaLab.sln`, `docs/`, `scripts/`, `sdk/`, `src/`, `tests/`, `module-registry/`, `installer/`, `licenses/` et `wiki/`.

Dossiers constatés dans [src/](https://github.com/overthetop78/Retro-Media-Lab/tree/main/src) :

- `RetroMediaLab.App`
- `RetroMediaLab.Chips`
- `RetroMediaLab.Emulation.Acorn`
- `RetroMediaLab.Emulation.Amstrad`
- `RetroMediaLab.Emulation.Apple`
- `RetroMediaLab.Emulation.Atari`
- `RetroMediaLab.Emulation.Bandai`
- `RetroMediaLab.Emulation.Coleco`
- `RetroMediaLab.Emulation.Commodore`
- `RetroMediaLab.Emulation.Epoch`
- `RetroMediaLab.Emulation.Example`
- `RetroMediaLab.Emulation.Fairchild`
- `RetroMediaLab.Emulation.Mattel`
- `RetroMediaLab.Emulation.Microsoft`
- `RetroMediaLab.Emulation.Nec`
- `RetroMediaLab.Emulation.Nintendo`
- `RetroMediaLab.Emulation.Panasonic`
- `RetroMediaLab.Emulation.Philips`
- `RetroMediaLab.Emulation.Sega`
- `RetroMediaLab.Emulation.Sharp`
- `RetroMediaLab.Emulation.Sinclair`
- `RetroMediaLab.Emulation.Snk`
- `RetroMediaLab.Emulation.Sony`
- `RetroMediaLab.Emulation.Thomson`
- `RetroMediaLab.Emulation.Watara`
- `RetroMediaLab.Emulation.Wenquxing`
- `RetroMediaLab.Emulation`
- `RetroMediaLab.Infrastructure`
- `RetroMediaLab.Launcher`
- `RetroMediaLab.MediaAnalysis`
- `RetroMediaLab.MediaEngine`
- `RetroMediaLab.MediaFileSystems`
- `RetroMediaLab.Updater`
- `RetroMediaLab.Updates`
- `RetroMediaLab.VideoPresentation`
