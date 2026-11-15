# Conteneur Apple 2IMG et dépendance NIB

- [x] Lecture et écriture
  - [x] Créer `src/MediaEngine/src/Formats/TwoImg/Constants.h`, `TwoImg.h`, `Reader.cpp`, `Writer.cpp` : disposition, signature/version, bornage de charge utile, routage DOS/ProDOS/NIB et écriture atomique 2IMG.
  - [x] Créer `src/MediaEngine/src/Formats/Nib/Constants.h`, `Nib.h`, `Reader.cpp` : décodage Apple II/RWTS18 des pistes fixes et reconstruction des secteurs, sans lecteur factice.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/twoimg.json` et ses homologues dans `Languages/` : intégration assurée par l’agent parent, compilation et diagnostics traduits.
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/twoimg.json` : relire les 29 catalogues générés, corriger troncature, offset et entier signé, préserver 2IMG et les paramètres.
- [x] Vérification ciblée
  - [x] Créer `build/verification-twoimg/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les sources C# originales, les octets 2IMG écrits, les champs et plages invalides et les données/adresses issues des charges DOS/ProDOS/NIB.
  - [x] Supprimer `build/verification-twoimg/` après validation et retirer les journaux temporaires.
