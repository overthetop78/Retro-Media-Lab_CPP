# Catalogue des enregistrements mécaniques

- [x] Contrats et sélection
  - [x] Créer temporairement `build/verification-mechanical-catalog/import_catalog.py` : convertir les définitions C# strictement, vérifier les chemins et copier les FLAC de production sans modifier la source.
  - [x] Créer `src/MediaEngine/src/Audio/Mechanical/RecordedSounds.h`, `RecordedSounds.cpp`, `Constants.h` et `CatalogData.h` : profils, échantillons et opérations C#, ordre des catégories, sélection stable par sens/distance/cylindre et repli vers le profil de base.
  - [x] Créer `src/MediaEngine/Assets/MechanicalAudio/**/*.flac` par copie binaire des fichiers homologues du dépôt C# : banque sonore de production complète, chemins relatifs conservés.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation des modèles, constantes et installation de la banque dans `Assets/MechanicalAudio`.
- [x] Vérification ciblée
  - [x] Créer `build/verification-mechanical-catalog/CMakeLists.txt` et `main.cpp` : ordre et nombre des profils, identifiants uniques, chemins des fichiers, priorité sens/distance/cylindre, stabilité des égalités et repli de profil.
  - [x] Supprimer `build/verification-mechanical-catalog/` et ses sorties après utilisation.
