# CPCEMU DSK et EDSK

- [ ] Modèles et primitive commune
  - [ ] Créer `src/MediaEngine/src/Formats/CpcDsk/CpcDsk.h`, `Constants.h`, `Internal.h` : porter les contrats de conteneur/pistes/secteurs et le schéma CPCEMU source, en réutilisant SectorImage MFS et les identifiants centraux.
  - [ ] Créer `src/MediaEngine/src/Models/Sectors/UniformLayout.h` et `UniformLayout.cpp` : porter le helper commun C# UniformSectorImageLayout.TryCreate, conserver les emplacements manquants et réutiliser sparseSectorImage sans nouvelle représentation parallèle.
- [ ] Lecture, construction et écriture
  - [ ] Créer `src/MediaEngine/src/Formats/CpcDsk/Reader.cpp`, `Builder.cpp`, `Writer.cpp` : porter reconnaissance, Standard/Extended DSK, tables/descripteurs, données stockées/réplicas/statuts, construction depuis SectorImage avec géométries Acorn/BBC existantes et écriture atomique commune.
  - [ ] Modifier les fichiers partagés Engine de compilation/diagnostics/audit et créer les catalogues `Constants/MediaEngine/cpc-dsk.json`, `Languages/*/MediaEngine/cpc-dsk.json` : intégrer les sources et diagnostics traduits spécialisés pour éviter les raisons fixes anglaises opaques (agent parent).
  - [ ] Modifier les catalogues `src/Translations/Catalogs/Languages/*/MediaEngine/cpc-dsk.json` : relire/corriger Argos et contrôler paramètres/invariants selon partage coordonné avec le parent.
- [ ] Vérification ciblée
  - [ ] Créer `build/verification-cpc-dsk/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les sources C# originales sur conteneurs Standard/Extended, pistes absentes, tailles variables/réplicas, CHRN et statuts, reconstruction uniforme/dense, builders et writer octet pour octet, plus annulation/écriture atomique bornée ; sans rejouer les blocs précédents.
  - [ ] Supprimer `build/verification-cpc-dsk/` et ses artefacts après validation et intégration de l’inventaire par le parent.

- [ ] Finaliser la compilation
  - [x] Modifier `src/MediaEngine/src/Formats/CpcDsk/Writer.cpp` : renommer le paramètre de position de piste pour supprimer le masquage par l’index local des secteurs, sans modifier les données écrites.

- [ ] Corriger les traductions signalées
  - [x] Modifier `src/Translations/Catalogs/Languages/zh-Hans/MediaEngine/cpc-dsk.json`, `Languages/zh-Hant/MediaEngine/cpc-dsk.json` et `Languages/ar-SA/MediaEngine/cpc-dsk.json` : préserver DSK et le paramètre de numéro de piste.

> Les batteries natives prévues dans ce document sont abandonnées sur consigne utilisateur. Ne pas les relancer. Le banc temporaire a été retiré après conservation du statut réel dans `docs/validation/media/bancs-prepares-retires.md`.
