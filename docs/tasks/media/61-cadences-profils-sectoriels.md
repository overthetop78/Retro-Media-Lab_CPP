# Cadences standard et profils sectoriels rares

- [x] Catalogues de reconstruction
  - [x] Créer `src/MediaEngine/src/Formats/FormatIds.h` : préfixes et identifiants requis par les cadences, en réutilisant les identifiants déjà partagés avec MediaFileSystems.
  - [x] Créer `src/MediaEngine/src/Encoding/TimingConstants.h`, `Timings.h`, `Timings.cpp`, `RareProfiles.h`, `RareProfiles.cpp` : cadences FM/MFM par famille et format, durée 300/360 RPM, résolution des quatorze profils rares avec leurs identifiants de codecs existants.
  - [x] Créer `src/MediaEngine/src/Primitives/IdentifierConstants.h` et modifier `Primitives/Identifiers.h`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py` : comparaison des clés ASCII sans assimilation de caractères Unicode, conservation du contenu UTF-8, intégration des sources et déclaration des en-têtes de constantes.
- [x] Vérification
  - [x] Créer `build/verification-sector-timings/Reference.csproj`, `Reference.cs`, `FormatConstants.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : compiler les catalogues C# originaux avec leurs constantes extraites des sources, comparer toutes les branches et profils, variantes de casse, préfixes inconnus et Unicode, puis vérifier Debug/Release.
  - [x] Supprimer `build/verification-sector-timings/` et ses journaux : retirer le banc et les artefacts temporaires après validation.
