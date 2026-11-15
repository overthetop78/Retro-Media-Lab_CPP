# HFE versions 1 et 3

- [x] Conteneur et conversions protégées
  - [x] Créer `src/MediaEngine/src/Models/Flux/Conversion.h`, `Conversion.cpp`, `ConversionConstants.h` : partager les conversions fidèles cellules/flux et la projection des pistes protégées utilisées par HFE.
  - [x] Créer `src/MediaEngine/src/Formats/Hfe/Constants.h`, `Hfe.h`, `Models.cpp` : constantes format/layout/métadonnées, modèles possédés, packer LSB HFE et conversion vers les pistes protégées existantes.
  - [x] Créer `src/MediaEngine/src/Formats/Hfe/Reader.cpp` : lecture HFE v1/v3, limites, entrelacement des faces, opcodes de débit/index/skip/random et timing exact.
  - [x] Créer `src/MediaEngine/src/Formats/Hfe/Writer.cpp` : entêtes/table/entrelacement v1, validation, encodages et écritures atomiques.
  - [x] Créer `src/MediaEngine/src/Formats/Hfe/ProtectedImage.cpp`, `V3Writer.cpp` : adaptateurs de pistes protégées, conversion des cellules/flux, résolution SCP/HFE, sérialisation v3 et repli fidèle du writer.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, diagnostics et allowlist des constantes ; créer les catalogues HFE : intégration coordonnée par l’agent principal.
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/hfe.json` : relire/corriger les traductions générées et préserver paramètres et invariants.
- [x] Vérification ciblée
  - [x] Créer `build/verification-hfe/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt` et cas/manifestes : comparer les véritables sources C# pour packer, reader, writer v1/v3 et adaptateurs, vérifier limites/erreurs/timing/opcodes et écriture atomique.
  - [x] Supprimer `build/verification-hfe/` et ses artefacts temporaires après validation.
