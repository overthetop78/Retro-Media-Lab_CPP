# Conteneur HDD brut

- [x] Format, lecture et écriture
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Raw/Constants.h`, `Format.h`, `Reader.h`, `Reader.cpp`, `Writer.h` et `Writer.cpp` : contraintes 512/4096, détection GPT puis MBR ou choix explicite, document par plages de blocs et écriture atomique de la représentation complète, sans dépendance au système de fichiers hôte.
  - [x] Modifier les diagnostics Engine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/hdd-raw.json` et `Languages/<langue>/MediaEngine/hdd-raw.json` : traduire les rejets de format, longueur modifiée et écriture incompatible.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : compiler le lecteur/writer et déclarer leurs constantes ; régénérer les projets Visual C++.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-hdd-raw/CMakeLists.txt` et `main.cpp` : vérifier choix explicite, sondes GPT 4096/512 et MBR, refus de données sans indice, lecture/écriture complète et refus d’une longueur modifiée.
  - [x] Supprimer `build/verification-hdd-raw/` avec les médias synthétiques et les sorties après consignation.
