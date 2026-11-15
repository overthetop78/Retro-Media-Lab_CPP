# Primitives Apple II GCR et lecture synchronisée Apple

- [x] Lecture commune
  - [x] Créer `src/MediaEngine/src/Codecs/Apple/BitLatch.h`, `BitLatch.cpp`, `Constants.h` : octets à bit de synchronisation, validation count/offset dans l’ordre source, curseur conservé sur lecture partielle, résultat possédé et annulation.
- [x] Transformations Apple II
  - [x] Créer `src/MediaEngine/src/Codecs/AppleII/Codec.h`, `Constants.h`, `Address.cpp`, `Internal.h`, `SixAndTwo.cpp`, `FiveAndThree.cpp` : tables/inverses, 4-and-4, checksum/adresse, encodage et décodage 6-and-2/5-and-3, checksum invalide distinct de symbole inconnu, positions finales.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, constantes et diagnostics traduisibles.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/appleii-gcr-primitives.json` et les 29 catalogues : erreurs de compte, position et taille de secteur, génération Argos puis relecture.
- [x] Protection des traductions
  - [x] Modifier `scripts/translations/translate.py` et `scripts/translations/ignores/common.txt` : protection des paramètres accolés, traitement des invariants sans chevauchement, protection des nombres et ajout des noms/protocoles Apple/Amiga et de leurs codages.
  - [x] Créer `build/verification-appleii-primitives/translation-check.py` : paramètres répétés/accolés, invariants imbriqués, limites de mots, nombres altérés et repli si les tokens sont déformés.
- [x] Validation
  - [x] Créer `build/verification-appleii-primitives/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources originales, matrice 4-and-4/tables, motifs de charge utile, symboles inconnus et checksums, troncatures/décalages/synchronisation, curseur partiel, erreurs, annulation et langues Debug/Release.
  - [x] Supprimer `build/verification-appleii-primitives/`, `build/appleii-primitives-reference.log`, `build/appleii-primitives-bench-debug.log`, `build/appleii-primitives-bench-release.log` : retrait du banc et des artefacts.

Le codec de primitives est porté intégralement ; le format reste partiel tant que ses paramètres de piste ne sont pas repris. Les classes de piste Encoder/Decoder et les associations adresse/données seront traitées ensuite, sans prétendre fournir un catalogue par défaut réduit.
