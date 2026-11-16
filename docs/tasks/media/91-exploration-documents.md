# Exploration des documents reconnus

- [x] Contrats et composition
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileSystemIds.h` : reprendre tous les identifiants C# de `Definitions/FileSystemIds.cs`, et modifier `src/MediaFileSystems/src/Exploration/Composition/CatalogConstants.h` pour les réutiliser.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MediaFileSystemReader.h`, `VolumeDetectors.h`, `MediaExplorer.h` : contrats génériques, résultats d’exploration et registre de détecteurs ; modifier `SectorReader.h` pour son adaptation au contrat générique existant.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileSystemModels.h` et `src/MediaFileSystems/src/Exploration/EntryAnalysis.cpp` : fournir des vues analysées possédées sans recopier les contenus ni modifier le volume lu.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Sources.h` et `src/MediaFileSystems/src/Sources.cpp` : permettre au pont de représentation par blocs de conserver le nombre logique déclaré.
- [x] Services portés depuis le C#
  - [x] Créer `src/MediaFileSystems/src/Exploration/Volumes/Detectors.cpp`, `Registry.cpp` : conserver l’ordre séquentiel/optique/GPT/MBR/volume entier, les volumes déjà connus, les replis de longueur et les exceptions précises.
  - [x] Créer `src/MediaFileSystems/src/Exploration/MediaExplorer.cpp` : ordre des lecteurs avec priorité au système indiqué, corruption conservée sans absorber E/S ou annulation, enrichissement séparé et premier fichier en parcours préordre.
  - [x] Modifier les diagnostics MFS, `src/MediaFileSystems/CMakeLists.txt` et `scripts/quality/audit_literals.py`, et créer les catalogues `MediaFileSystems/media-exploration.json` : intégrer services et constantes, traduire les nouveaux diagnostics.
- [x] Raccordement final réalisé
  - [x] Modifier la composition publique de l’explorateur pour ajouter les neuf lecteurs non sectoriels C# après leur portage ; ne pas publier un faux catalogue par défaut réduit.

Vérification : compilation et contrôles ciblés utiles uniquement, sans batteries de comparaison par composant conformément à la dernière consigne utilisateur.

- [x] Contrôle ciblé du raccordement
  - [x] Modifier `build/verification-volume-contract/main.cpp` : vérifier un unique parcours document → détection → lecteur → analyse, le registre de 21 lecteurs et le partage du contenu sans relancer les comparaisons de composants.
  - [x] Supprimer `build/verification-volume-contract/` après ce contrôle et conservation des résultats utiles.
