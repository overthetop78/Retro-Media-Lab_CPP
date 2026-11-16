# Exploration des banques de cartouches

- [x] Contrats communs
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h` : ajouter les identifiants exacts utilisés par les deux lecteurs depuis `MediaImageFormatIds.cs`.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CartridgeReaders.h` et `src/MediaFileSystems/src/FileSystems/Cartridge/Constants.h` : déclarer les deux lecteurs et leurs constantes communes.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileSystemModels.h` : conserver les diagnostics du document et la recette de traduction du nom du système de fichiers.
- [x] Lecteurs fidèles au C#
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Cartridge/Readers.cpp` : reprendre les banques Amstrad et console, les validations, les noms synthétiques et les adresses ; conserver les erreurs de lecture et vérifier le débordement du stockage console après la lecture.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h` et `src/MediaFileSystems/src/Diagnostics.cpp` : ajouter les diagnostics traduisibles des cartouches.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt` et `scripts/quality/audit_literals.py` : compiler les lecteurs et déclarer leur fichier de constantes.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/cartridge-readers.json` et les versions dans `Languages/<langue>/MediaFileSystems/` : erreurs et nom Amstrad traduisibles, sans texte utilisateur dans le code.

Validation : compilation incrémentale et examen ciblé des contrats ; aucune nouvelle batterie de comparaisons.

- [x] Relecture linguistique restante
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/` du présent bloc : corriger les contresens restants après lecture des langues non encore relues.

Relecture des trois catalogues dans les 29 langues effectuée le 17 novembre 2026 ; corrections et limites dans `docs/validation/media/relecture-cartouches-fds-gamecube.md`.
