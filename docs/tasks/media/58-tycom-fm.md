# Pistes TYCOM FM

- [x] Champs et pistes
  - [x] Modifier `src/MediaEngine/src/Models/Flux/BitReader.h`, `BitReader.cpp` : ajouter une lecture FM double largeur bornée et annulable réutilisable par les codecs, sans modifier les lectures MFM existantes.
  - [x] Créer `src/MediaEngine/src/Codecs/Tycom/Constants.h`, `Fm.h`, `Fields.cpp` : définitions TYCOM, CRC CCITT partagé, lecture FM double largeur et recherche des quatre marques avec arrêt au nouvel en-tête.
  - [x] Créer `src/MediaEngine/src/Codecs/Tycom/Encoder.cpp`, `Decoder.cpp` : sélection normale/supprimée, rejet des en-têtes CRC incorrects, structures tronquées/non appariées, tri stable et secteurs.
- [x] Construction et langues
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : sources regroupées, diagnostics et constantes nommées.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger TYCOM.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/tycom-fm.json` et ses 29 catalogues : plages, taille, CRC et structures ; Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-tycom/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales des CRC, helpers, pistes, quatre marques, erreurs d’intégrité, bornes, progression, diagnostics, annulation, registre et langues Debug/Release.
  - [x] Supprimer `build/verification-tycom/` et ses journaux après vérification : retrait du banc et de ses artefacts temporaires.
