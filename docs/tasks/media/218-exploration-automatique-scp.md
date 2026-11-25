# Exploration automatique SCP

- [x] Inspection et classement
  - [x] Créer `src/MediaEngine/src/Exploration/Scp/Inspection.h`, `Inspection.cpp`, `Ranking.cpp` et `Constants.h` : inspection des systèmes de fichiers, normalisation et interprétations supplémentaires, classement stable par preuve, géométrie compatible et conservation des rejets.
- [x] Sonde et orchestration
  - [x] Créer `src/MediaEngine/src/Exploration/Scp/Probe.cpp` et `Service.cpp` : échantillonnage de 24 pistes, cinq familles et décodeurs ordonnés, exécution bornée et nettoyage RAII, événements de progression, normalisation puis classement des candidats.
  - [x] Modifier `src/MediaEngine/src/Composition/Runtime.h`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : partager lecteurs, registres et service d'inspection.
  - [x] Modifier `src/MediaEngine/src/Reconstruction/Scp/Composition.h` et `Composition.cpp` : exposer l'opération de décodage possédée par la composition pour éviter un second registre.
- [x] Raccordement des conversions
  - [x] Modifier `src/MediaEngine/src/Images/Conversion/Fat12Facade.cpp` et `Fat12Conversion.h` : explorer le chemin avec les politiques communes, détecter les interprétations hybrides et convertir seulement après validation.
- [x] Validation ciblée
  - [x] Créer `build/verification-scp-inspection/CMakeLists.txt` et `main.cpp` : ordres, égalités, rejets et événements avec candidats injectés, échantillonnage et normalisations, conversion FAT12 par chemin ; compiler le bloc assemblé.
  - [x] Supprimer `build/verification-scp-inspection/` et ses artefacts temporaires après utilisation.
- [x] Vérifier et présenter les catalogues des SCP triformats
  - [x] Créer puis supprimer `build/verification-scp-triformat/` et ses artefacts de comparaison des catalogues C# et C++.
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/ProjectionFormats.cpp` et `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp` pour afficher qu’un volume sectoriel sans catalogue ne possède pas de système de fichiers reconnu.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/StatusBar.cpp` pour donner priorité à la progression du lecteur physique devant la présentation simulée.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/DiskExplorationTypes.h`, `src/MediaEngine/src/Exploration/Disk/Explorer.h`, `Explorer.cpp` et `Projection.cpp` pour dériver la présence d’un catalogue du résultat réel de l’exploration.
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/ProjectionFormats.cpp` et `src/Infrastructure/Media/MigrationSession.cpp` pour réserver l’identifiant de système de fichiers aux catalogues trouvés.

Comparaison des moteurs C# et C++ Release sur deux captures. Sur CrazyCarsIII_Gen4_Tri_Format.scp, le catalogue AmigaDOS expose 27 entrées. Atari ST et IBM PC exposent chacun 612 secteurs et aucun catalogue. Dans le C#, le premier secteur MFM disponible porte le numéro 54. Le fichier CrazyCarsIII_Gen4_Tri_Format.st du 11 septembre 2026 contient lui aussi 54 secteurs initiaux marqués BAD SECTOR. Sur Tilt N°105 - Septembre 1992.scp, les deux moteurs reconnaissent les trois catalogues et trouvent 12 entrées pour Atari ST et pour IBM PC.

Le C++ dérive désormais le système de fichiers du catalogue effectivement trouvé. Les vues sectorielles sans catalogue conservent leur géométrie et affichent la mention traduite correspondante. La barre d’état donne priorité aux événements du lecteur physique. Les 75 ms de la Visualisation concernent le défilement lors de l’ouverture d’une image.

Le fichier de la première capture C#, [PC-AMIGA-ST] Génératiion 4  N°5 - Crazy Cars III.scp dans F:/Retro/A Trier, mesure 39 609 620 octets. Il est distinct de CrazyCarsIII_Gen4_Tri_Format.scp dans G:/Retro/Disquettes, qui mesure 41 101 424 octets. Le moteur C++ obtient 5 et 27 entrées AmigaDOS respectivement, et 612 secteurs IBM/ST sans catalogue sur chacune. Le contrôle indépendant des marques MFM et des CRC sur les trois révolutions trouve des synchronisations Amiga mais aucun en-tête IBM/ST valide sur les pistes physiques 0 à 5 des deux fichiers. Sur la capture de G:, les 612 en-têtes IBM/ST valides occupent les cylindres 3 à 36 et portent les adresses de ces cylindres. Aucun secteur d’amorçage IBM/ST ou paramètre de volume FAT12 valide n’a été retrouvé ailleurs dans cette capture.

- [x] Vérifier les possibilités de récupération dans le seul SCP CrazyCarsIII_Gen4_Tri_Format
  - [x] Créer puis supprimer `build/verification-crazycars-fat` pour rechercher le secteur d’amorçage et les tables FAT dans les charges utiles de cette capture.

Vérification limitée à CrazyCarsIII_Gen4_Tri_Format.scp. Les 612 secteurs MFM disponibles couvrent les blocs 54 à 665. Aucun des 612 secteurs MFM et des 1 012 charges utiles Amiga de 512 octets ne contient de BPB valide à leur début. Les trois séquences ressemblant à un BPB dans FILE1.BIN déclarent zéro secteur par cluster et une géométrie de deux secteurs par piste. Les seules séquences de trois secteurs MFM avec des valeurs dans les plages FAT12 attendues correspondent à des zones de remplissage, avec 713 fins de chaîne et aucun lien ni cluster libre. La recherche par signatures dans les charges utiles MFM n’a identifié ni en-tête DOS MZ, ni binaire Atari TOS, ni entrée courte cohérente parmi les entrées de fichier ordinaires testées. Aucun catalogue FAT12 n’a pu être reconstruit à partir des données vérifiées.

## Présentations et fichiers de Joystick_40.scp

- [x] Moteur
  - [x] Modifier `src/MediaEngine/src/Exploration/Scp/Service.cpp` pour départager les catalogues PC/ST avec les indices du média.
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/Projection.h`, `Projection.cpp`, `ProjectionFormats.cpp` et `src/MediaEngine/src/Images/ExplorationMapper.h` et `ExplorationMapper.cpp` pour projeter les catalogues et leurs fichiers analysés une fois par format.
- [x] Explorateur
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp`, `ExplorerPage.h` et `MediaPageConstants.h` pour sélectionner une présentation réelle et afficher ses icônes, types et détails.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ClassificationPanel.cpp`, `ClassificationPanel.h` et `ClassificationConstants.h` pour marquer les choix reconnus dans le popup.
  - [x] Modifier `src/Infrastructure/Media/ExplorerSession.cpp` et `ExplorerSession.h` pour essayer une présentation supplémentaire en conservant le résultat courant en cas de rejet.
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/Explorer.cpp` et `Explorer.h` pour essayer le décodeur demandé sur les flux lorsqu’il manque dans les présentations détectées.
- [x] Vérification
  - [x] Créer puis supprimer `build/verification-joystick40` pour vérifier les catalogues et les signatures de cette capture SCP en Release.

Le build Release et la sonde sur la seule capture `Joystick_40.scp` confirment deux présentations détectées, AmigaDOS et IBM PC. Les deux lectures FAT12 automatiques PC et Atari utilisaient les mêmes secteurs et le secteur d’amorçage MS-DOS 5.0. L’essai forcé Atari reste possible et ne rajoute pas Atari aux formats détectés. Le catalogue Amiga contient `blastar` avec la signature Hunk `00 00 03 F3`. Le catalogue IBM PC contient `INSTALL.EXE` et `SETUP.EXE` avec la signature DOS `4D 5A`. Les trois entrées ont le type exécutable natif dans leur présentation. La géométrie du popup n’a pas été vérifiée dans une fenêtre interactive.

## Lecture forcée sans catalogue

- [x] Moteur
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/Explorer.cpp` pour conserver les secteurs décodés sans catalogue dans la présentation forcée.
- [x] Explorateur
  - [x] Modifier `src/Infrastructure/Media/ExplorerSession.cpp` pour appliquer le choix manuel après la lecture.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp` pour afficher la présentation forcée et vider la liste si aucun catalogue n’est disponible.
- [x] Vérification
  - [x] Créer puis supprimer `build/verification-forced-explorer` pour vérifier les lectures forcées de cette capture.

Vérifié en Release via la session Explorateur sur `Joystick_40.scp`. Le choix IBM PC 1,44 Mio produit 927 secteurs valides et un catalogue FAT. Le choix Apple II ProDOS 140 Kio reste sélectionné sans secteur exploitable ni erreur. Les détections Amiga et IBM PC restent disponibles, et le passage en automatique annule le choix forcé.
