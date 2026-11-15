# Images brutes IBM/MSX et écriture atomique

- [x] Contrats et diagnostics
  - [x] Déplacer les identifiants partagés de `MediaFileSystems/src/FileSystems/Fat/Fat12/FormatCatalog.h` dans `MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h` et adapter le catalogue : une seule définition pour le lecteur FAT12 et les formats Engine.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/Diagnostics.cpp` : erreurs propres au moteur à codes/paramètres, domaine de traduction MediaEngine.
  - [x] Créer `src/MediaEngine/src/Formats/Raw/RawImages.h`, `Constants.h`, `Geometry.cpp` : catalogues IBM/MSX, détection BPB/capacité/descripteur, reconnaissance MSX et contrats internes sans dépendance App.
- [x] Lecture et écriture
  - [x] Créer `src/MediaEngine/src/Formats/Raw/Reader.cpp` : sources bornées vers images sectorielles paresseuses IBM/MSX et exploration FAT12 réelle.
  - [x] Créer `src/MediaEngine/src/Sources/AtomicImageWriter.h`, `AtomicImageWriter.cpp`, `AtomicConstants.h` : écriture dans un fichier temporaire voisin et remplacement atomique, nettoyage RAII sur exception et annulation.
  - [x] Créer `src/MediaEngine/src/Formats/Raw/Writer.cpp` : validation géométrique, secteurs complets dans l’ordre logique, profils IBM/MSX et IBM scan, aucune écriture matérielle.
- [x] Primitives partagées
  - [x] Créer `src/MediaEngine/src/Models/Sectors/LinearSectorWriter.h` et `.cpp` ; modifier `SectorConstants.h` et `Formats/Raw/Writer.cpp` : extraire la validation et l’écriture linéaire communes, utilisables par les prochaines familles sans copier le code ; modifier `Formats/Raw/Reader.cpp` pour vérifier aussi l’annulation après lecture du secteur d’amorçage.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py` ; régénérer les projets Visual C++ : compiler les nouvelles sources, recenser les constantes et vérifier automatiquement les clés/paramètres des catalogues de diagnostics médias.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/raw.json` et les équivalents des 29 langues : diagnostics traduits avec Argos et relus, paramètres préservés.
- [x] Validation
  - [x] Créer `build/verification-raw/` : banc C#/C++ regroupé pour catalogues, géométries, profils ambigus, secteurs/écriture, trajets fichier/FAT12/analyse, annulation et préservation de destination en cas d’échec.
  - [x] Supprimer `build/verification-raw/` et ses artefacts après validation.
