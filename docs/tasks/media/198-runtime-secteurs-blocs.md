# Disques runtime sectoriels et blocs

- [x] Contrat public et état runtime
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/HardDiskGeometry.h` et modifier `src/MediaEngine/src/Models/Blocks/BlockImage.h` : déplacer la déclaration de géométrie existante vers l’API publique sans changer son implémentation.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/RuntimeDisk.h`, `src/MediaEngine/src/Runtime/DiskState.h`, `Disk.cpp` et `Sectors.cpp` : ouvrir les images décodées secteur/bloc avec les compositions existantes, exposer des copies des secteurs et de leur intégrité, superposer les modifications, conserver la propriété des mécaniques et terminer les accès en RAII.
  - [x] Créer `src/MediaEngine/src/Runtime/Persistence.cpp` et `Metadata.cpp` : reconstruire les représentations modifiées sans matérialiser un HDD entier, écrire puis rouvrir avant d’effacer les changements, mise à jour DSC et préparation de géométrie Acorn, fermeture explicite et nettoyage sur exception.
  - [x] Modifier `src/MediaEngine/src/Formats/HardDisk/Acorn/Descriptor.h`, `Descriptor.cpp` et `Writer.cpp` : partager le décodage strict du descripteur Base64 entre runtime et écrivain.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/runtime-disk.json` et `Languages/<langue>/MediaEngine/runtime-disk.json` : erreurs runtime traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler les modèles publics et le runtime secteur/bloc.
- [x] Vérification ciblée
  - [x] Créer `build/verification-runtime-disk/CMakeLists.txt`, `main.cpp` et de petites images temporaires : secteurs et blocs réels, copies et persistance, protection écriture, échec de flush avec changements conservés, annulation, DSC et changement de géométrie, mécaniques possédées/empruntées et fermeture sur erreur.
  - [x] Supprimer `build/verification-runtime-disk/` après utilisation avec les images et exécutables temporaires.

