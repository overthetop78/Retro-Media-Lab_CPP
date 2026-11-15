# Pistes Membrain MFM

- [x] Adresses, champs et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/Membrain/Constants.h`, `Mfm.h`, `Address.cpp` : adresse compactée, masques, marques et contrats internes du codec.
  - [x] Créer `src/MediaEngine/src/Codecs/Membrain/Fields.cpp` : lecture d’en-tête, recherche bornée des marques F8–FB et lecture des blocs avec CRC IBM initialisé à zéro.
  - [x] Créer `src/MediaEngine/src/Codecs/Membrain/Encoder.cpp`, `Decoder.cpp` : pistes de 512 octets, intégrité, progression, données tronquées/non appariées et ordre C# des structures.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compiler les sources, déclarer les erreurs de taille/adresse et les constantes nommées.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Membrain.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/membrain-mfm.json` et ses 29 catalogues : limites, marques, CRC et structures ; Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-membrain/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparer C# original, adresses compactées/décompactées, helpers de lecture, quatre marques, pistes et flux, bornes, CRC, progression, diagnostics, annulation, registre et langues en Debug/Release.
  - [x] Supprimer `build/verification-membrain/` et ses journaux après vérification : retrait du banc et des artefacts temporaires.
