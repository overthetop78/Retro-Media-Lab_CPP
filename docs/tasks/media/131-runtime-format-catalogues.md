# Catalogues de formats dynamiques et capacités Greaseweazle

- [x] Contrats et libellés techniques
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/ImageFormat.h` : permettre les noms techniques invariants découverts à l'exécution et exposer la fusion des formats, sans figer les traductions des entrées existantes.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/GwFormatCapabilities.h` : capacités annoncées par l'aide Greaseweazle, analyse du texte fourni et adaptation d'un catalogue.
- [x] Implémentation
  - [x] Créer `src/MediaEngine/src/Formats/Catalog/Runtime.cpp` : fusionner seulement les extensions/compatibilités des formats déjà connus et ajouter les nouveaux dans l'ordre source ; résoudre les noms par clé ou invariant technique.
  - [x] Créer `src/MediaEngine/src/Formats/Catalog/Greaseweazle.cpp`, modifier `Catalog/Constants.h` : parser les sections de l'aide, appliquer les capacités physiques, créer les formats découverts et leurs noms techniques, reprendre les préférences d'extensions et les cas bitcell.
  - [x] Modifier `src/MediaEngine/src/Formats/Definitions/Reader.cpp` : partager les alias de format Greaseweazle avec la résolution des dimensions.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler les deux implémentations.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-runtime-format-catalogues/CMakeLists.txt` et `main.cpp` : vérifier sections d'aide, capacités inconnues/connues, conservation des présentations, alias, formats découverts et préférences bitcell.
  - [x] Supprimer `build/verification-runtime-format-catalogues/` et ses binaires temporaires après consignation.
