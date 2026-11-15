# Conteneurs WOZ

- [x] Lecture et écriture
  - [x] Créer `src/MediaEngine/src/Formats/Apple/DecodedTracks.h`, `DecodedTracks.cpp`, `TrackSelectionConstants.h`, modifier `src/MediaEngine/src/Formats/Nib/Reader.cpp` : extraire la sélection/reconstruction Apple déjà portée et la partager avec WOZ, en préservant les parcours NIB.
  - [x] Créer `src/MediaEngine/src/Formats/Woz/Constants.h`, `Woz.h`, `Reader.cpp`, `Writer.cpp` : porter WOZ1/WOZ2, contrôles de chunks et CRC, sélection des descripteurs, sérialisation WOZ1, réutilisation des codecs et reconstruction Apple partagée.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, les diagnostics MediaEngine ; créer `src/Translations/Catalogs/Constants/MediaEngine/woz.json` et catalogues `Languages/` : intégrer les sources et erreurs traduites (coordonné par l’agent principal).
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/woz.json` : relire/corriger les 29 catalogues générés et vérifier paramètres et invariants techniques.
- [x] Vérification ciblée
  - [x] Créer `build/verification-woz/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer aux classes C# originales CRC, chunks, octets WOZ1, lectures WOZ1/2, refus et reconstruction de pistes ; vérifier les empreintes des sources ; contrôler uniquement les parcours NIB affectés par l’extraction partagée.
  - [x] Supprimer `build/verification-woz/` et les artefacts temporaires après validation.
