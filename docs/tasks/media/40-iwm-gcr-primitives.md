# Primitives Apple IWM GCR

- [x] Transformations communes Macintosh/Lisa
  - [x] Créer `src/MediaEngine/src/Codecs/AppleIwm/Constants.h`, `Codec.h`, `Internal.h`, `Encode.cpp`, `Decode.cpp` : bloc de 524 octets, regroupement 6-and-2 et dernier groupe incomplet, accumulateurs avec retenues, quatre valeurs de checksum, vérification des tailles avant accès et annulation.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation et diagnostics/constantes nommés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/iwm-gcr-primitives.json` et les 29 catalogues : erreurs de taille taguée et nombre de symboles, génération Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : ajouter le nom technique IWM à la liste commune d’invariants.
- [x] Validation
  - [x] Créer `build/verification-iwm-primitives/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources C# originales, tags/données, retenues, motifs et groupes limites, valeurs numériques brutes, tailles incorrectes, checksum reconstruit, textes des DLL et annulation Debug/Release.
  - [x] Supprimer `build/verification-iwm-primitives/` et les journaux de référence/comparaison : retrait du banc et des artefacts après validation.

AppleIwmGcrCodec sera porté intégralement ; les paramètres et codecs de piste Macintosh/Lisa seront traités ensuite. AppleIwmGcrFormat restera partiel jusqu’à cette étape.
