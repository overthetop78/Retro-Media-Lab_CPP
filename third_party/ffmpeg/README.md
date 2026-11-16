# FFmpeg embarqué

Version Windows x64 fixée par `package.json`. L’archive et l’exécutable sont vérifiés par SHA-256. Les binaires restent dans le cache `build/dependencies/ffmpeg/` et sont intégrés à la distribution par `scripts/local-building/build.ps1` ; ils ne sont pas ajoutés à Git.

Préparation séparée :

```powershell
./scripts/local-building/prepare-ffmpeg.ps1
```

Une installation correspondant exactement au manifeste peut alimenter le cache sans téléchargement : `-PackageRoot <dossier contenant bin/ffmpeg.exe, LICENSE et README.txt>`.

Le logiciel distribué contient `lib/FFmpeg/ffmpeg.exe`, la licence GPL et la provenance. Il fonctionne sans installation FFmpeg sur le PC utilisateur et sans recherche dans son PATH. Les bibliothèques du backend Qt sont placées dans le même dossier par windeployqt ; l’exécutable choisi est statique et indépendant de ces DLL.

Le service public `RetroMediaLab::MediaEngine::AudioConversionService` pilote la conversion audio ; il ne contient aucun contrôle GUI. Il accepte un chemin explicite pour les intégrations et, par défaut, utilise l’exécutable embarqué. Les autres usages futurs peuvent réutiliser cet exécutable.

Sources : [FFmpeg](https://ffmpeg.org/ffmpeg.html), [construction Gyan](https://www.gyan.dev/ffmpeg/builds/), [manifeste WinGet](https://raw.githubusercontent.com/microsoft/winget-pkgs/master/manifests/g/Gyan/FFmpeg/9.0.2/Gyan.FFmpeg.installer.yaml). La licence et le README de la construction sont copiés sans modification à côté de l’exécutable.
