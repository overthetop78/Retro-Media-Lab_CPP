# Retro Media Lab — C++

Ossature graphique Windows : sept pages de travail vides, menus, console repliable et préférences de langue/apparence à chaud. Le thème RetroMediaLab propose Clair et Sombre ; les 29 langues du C# sont présentes. Les fonctions métier et l'émulation ne sont pas encore portées.

Compiler depuis PowerShell :

```powershell
./scripts/local-building/build.ps1 -Configuration Release
```

Le script détecte le kit MSVC Qt sous C:/Qt. Un autre emplacement peut être fourni avec `-QtRoot` ou `RETROMEDIALAB_QT_DIR`.

Exécutable : `build/Release/Retro Media Lab/retromedialab.exe`.

- [Compilation et distribution locale](docs/development/build.md)
- [Traductions](docs/development/translations.md)
- [Architecture du socle](docs/design/app/socle-graphique.md)
- [Validation du premier socle](docs/validation/app/socle-graphique.md)
- [Validation de l'ossature et des préférences](docs/validation/app/ossature-et-preferences.md)
- [Captures C# de référence](docs/references/retro-media-lab/captures/README.md)

Le dépôt C# source reste en lecture seule. Windows est la plateforme actuellement compilée et vérifiée ; les adaptations et la distribution Linux/macOS restent à valider.
