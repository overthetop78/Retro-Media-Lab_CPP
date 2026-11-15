# Pistes AED 6200P MFM

- [x] Champs et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/Aed6200p/Constants.h`, `Mfm.h`, `Fields.cpp` : identité, marques C0–C3, champs de taille faible/fort et contrats du codec.
  - [x] Créer `src/MediaEngine/src/Codecs/Aed6200p/Encoder.cpp`, `Decoder.cpp` : CRC CCITT, émissions normale/supprimée, fenêtre de recherche, structures incomplètes et non appariées, métadonnées et ordre C#.
- [x] Construction et langues
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` et `scripts/quality/audit_literals.py` : sources compilées, diagnostic de champs hors limites et constantes nommées.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger AED 6200P.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/aed6200p-mfm.json` et ses 29 catalogues de langues : limites, CRC, marques et structures ; Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-aed6200p/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison des sources C# originales, champs, pistes et flux, quatre marques, tailles, fenêtre de recherche, erreurs, annulation, registre et langues Debug/Release.
  - [x] Supprimer `build/verification-aed6200p/` et les journaux associés après vérification : retrait du banc et des artefacts temporaires.
