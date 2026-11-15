# Conteneurs Amstrad et Famicom Disk System

- [x] Lecteurs de conteneurs
  - [x] Créer `src/MediaEngine/src/Formats/Cartridge/Readers.h`, `Constants.h`, `Cpr.cpp`, `Rom.cpp` et `Fds.cpp` : reprendre capacités/sondes/lecture des lecteurs C# CPR, ROM et FDS, construire les documents et leurs plages sans recopier l'image entière ni remplir les banques manquantes.
  - [x] Créer `src/MediaEngine/src/Formats/Cartridge/Internal.h` : partager extraction bornée, métadonnées numériques et construction des libellés traduisibles.
- [x] Écriture
  - [x] Créer `src/MediaEngine/src/Formats/Cartridge/Writers.cpp` : reprendre validation/ordre des banques CPR, continuité des ROM et validation des faces FDS ; réutiliser `writeAtomicImage` et les copies de plages.
- [x] Intégration
  - [x] Modifier les diagnostics Engine, `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer les formats et leurs constantes sans logique dans App.
  - [x] Créer les catalogues `MediaEngine/amstrad-fds.json` dans `src/Translations/Catalogs/Constants/` et `Languages/<langue>/` : traduire erreurs, avertissements et libellés virtuels ; conserver les invariants isolés uniquement dans Constants.

Compilation commune en fin de bloc et contrôle ciblé du chemin conteneur → document → exploration. Ne pas créer de nouvelle batterie de comparaisons C#.

- [x] Sélection des cibles d’écriture
  - [x] Modifier `src/MediaEngine/src/Images/Writers.h` et créer `Writers.cpp`, puis modifier `src/MediaEngine/CMakeLists.txt` : reprendre SupportsTarget du contrat C#, indépendamment de la compatibilité du document, avec la comparaison d’identifiants partagée.

- [x] Contrat commun des writers
  - [x] Créer `src/MediaEngine/src/Images/Writers.h` et modifier `src/MediaEngine/src/Formats/Cartridge/Readers.h` : porter les capacités/identifiants des writers et fournir l'adaptateur polymorphe CPR/ROM/FDS, avec conservation du document jusqu'à la fin de l'opération asynchrone.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger CPR, FDS, RIFF et AMSDOS avant génération des messages.
- [x] Contrôle ciblé de raccordement
  - [x] Créer `build/verification-cartridge-documents/CMakeLists.txt` et `main.cpp` : un contrôle en mémoire des trois documents, de leur exploration, des writers atomiques et du nettoyage mécanique avant/après acquisition ; aucune ancienne suite rejouée.
  - [x] Supprimer `build/verification-cartridge-documents/` : retirer le banc, ses exécutables et les images temporaires après consignation du résultat.


- [ ] Relecture linguistique restante
  - [ ] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaEngine/amstrad-fds.json` et `MediaFileSystems/media-document.json` : relire les langues autres que le français et corriger les contresens éventuels ; les clés, paramètres et invariants sont déjà contrôlés automatiquement.
