# Charge utile Apple RWTS18

- [x] Primitives des trois pages
  - [x] Créer `src/MediaEngine/src/Codecs/AppleRwts18/Constants.h`, `Codec.h`, `Payload.cpp` : regroupement des trois pages de 256 octets, symboles 6-and-2, checksum, conservation des valeurs brutes du décodeur et validation des longueurs avant accès.
- [x] Compilation et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, constantes et erreurs de taille traduisibles.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/rwts18-payload.json` et ses 29 catalogues : erreurs de taille et de nombre de symboles, génération Argos et relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger le nom technique RWTS18 dans toutes les langues.
- [x] Validation
  - [x] Créer `build/verification-rwts18-payload/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison des sources C# originales, motifs des trois pages, symboles et checksum, valeurs brutes hors 6 bits, longueurs et annulation, textes des DLL Debug/Release.
  - [x] Supprimer `build/verification-rwts18-payload/` et les journaux de référence/comparaison : retrait du banc et des artefacts après vérification.

Les primitives RWTS18 sont isolées avant les codecs de piste. AppleRwts18Format restera partiel ; encodeur/décodeur de piste, ordre physique et recherche des champs seront portés dans le bloc suivant.
