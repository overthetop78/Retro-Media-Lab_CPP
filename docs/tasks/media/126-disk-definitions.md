# Définitions de disquettes

- [x] Modèles et disposition
  - [x] Créer `src/MediaEngine/src/Formats/Definitions/DiskDefinitions.h`, `Layout.cpp` et `Constants.h` : modèles de définitions, sélection des pistes avec priorité C#, tailles répétées, numéros Apple et disposition des secteurs.
- [x] Catalogue embarqué et lecture
  - [x] Copier les 35 fichiers `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Definitions/*.cfg` vers `src/MediaEngine/src/Formats/Definitions/Data/` sans modifier leur contenu ni leurs notices.
  - [x] Créer `src/MediaEngine/src/Formats/Definitions/Reader.cpp` : charger les imports et définitions embarqués, lister les identifiants d'un fichier externe avec prévention des cycles, exposer les dimensions et les définitions compatibles brut selon les identifiants réservés fournis par le catalogue.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer `src/Translations/Catalogs/Constants/MediaEngine/disk-definitions.json` et `Languages/<langue>/MediaEngine/disk-definitions.json` : erreurs de fichier, cycle, syntaxe et valeur traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler modèles/lecteur et embarquer les CFG dans MediaEngine.dll.
  - [x] Modifier `scripts/quality/audit_literals.py` : déclarer `Formats/Definitions/Constants.h` comme fichier de constantes contrôlé.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-disk-definitions/CMakeLists.txt` et `main.cpp` : charger le catalogue embarqué, vérifier priorité des pistes, tailles répétées et numéros physiques ; contrôler imports externes et cycle avec fichiers temporaires.
  - [x] Supprimer `build/verification-disk-definitions/` avec ses fichiers CFG et binaires temporaires après consignation.
