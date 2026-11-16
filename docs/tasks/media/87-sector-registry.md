# Lecteurs Atari complémentaires et registre sectoriel complet

- [x] Lecteurs Atari sans catalogue classique
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Atari/FormatRecognition.h`, `FormatRecognition.cpp` : reconnaissance KBoot/boot complet/CLK, métadonnées et contenu CLK à partir des sources C# réelles, sans noms inventés.
- [x] Contrats sectoriels et erreurs structurées
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/SectorReader.h`, `SectorRegistry.h` et `src/MediaFileSystems/src/Exploration/Composition/Constants.h`, `Internal.h`, `Registry.cpp`, `Identity.cpp` : contrats de lecteurs, matches/échecs/rapports/candidats possédés, sélection ordonnée et identité Unicode fidèle.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp` et créer `src/Translations/Catalogs/Constants/MediaFileSystems/sector-registry.json` : catégorie publique ErrorKind vérifiée selon les factories C#, erreurs de configuration et Atari traduisibles ; ne capturer que les corruptions de Read, jamais les erreurs de sonde, IO, argument ou annulation.
- [x] Composition complète des 21 lecteurs
  - [x] Créer `src/MediaFileSystems/src/Exploration/Composition/Catalog.cpp`, `CatalogConstants.h` : publier identifiants exacts et tous CatalogFormatIds des 21 lecteurs C# dans leur ordre initial, réutiliser les fonctions déjà portées et les identifiants centraux.
  - [x] Modifier CMake MFS, audit des constantes et catalogues de langues : intégrer les nouvelles sources et traductions, actions partagées du parent ; intégrer l’adaptation whole-volume aux documents génériques fournis par le bloc88.
## Batterie abandonnée sur instruction utilisateur
  - Action abandonnée : créer `build/verification-sector-registry/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer Atari complémentaires, découverte/ordre/metadonnées21, sélection/failures/injection de catégories, première interprétation secondaire et identité Unicode, sans rejouer les 21 bancs de lecture.
  - [x] Supprimer `build/verification-sector-registry/` et ses artefacts après validation.

- [x] Catégories des dépassements CP/M (agent apple_filesystems)
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Cpm/Directory.cpp` : typer explicitement ErrorKind::Overflow à la garde de multiplication checked BlockCount×BlockSize dans flatten, sans modifier les conditions ni le diagnostic.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Cpm/Content.cpp` : typer explicitement ErrorKind::Overflow à la garde checked origine+allocation×taille dans offset, sans modifier les conditions ni le diagnostic ; ne pas reclasser les limites de flux append/zeros en dépassements arithmétiques.

- [x] Fidélité des catégories et adaptateur document
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h` et `src/MediaFileSystems/src/Diagnostics.cpp` : distinguer ArgumentOutOfRange d’InvalidArgument et catégoriser exhaustivement chaque diagnostic selon les exceptions C# ; conserver les surcharges de catégorie explicite.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Amiga/Dos/Directory.cpp`, `Recovery.cpp`, `src/MediaFileSystems/src/FileSystems/Commodore/Coherent/Directory.cpp`, `src/MediaFileSystems/src/FileSystems/Acorn/BbcDfs/Sides.cpp`, `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Directory.cpp`, `BlockView.cpp` : filtrer les catégories exactes capturées en C#, sans absorber IO, arguments ou annulations non prévues.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Codec.cpp` : typer les deux bornes numériques write16/write24 en ArgumentOutOfRange, en conservant le diagnostic et toutes les conditions.
  - [x] Créer `src/MediaFileSystems/src/Exploration/Composition/Document.cpp` : adapter le contrat sectoriel aux interfaces ImageDocument/SectorRepresentation et au VolumeDescriptor validé du bloc88, limité au volume entier comme en C#.

- [x] Reconnaissance Atari partagée
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Atari/FormatRecognition.h`, `FormatRecognition.cpp` : réutiliser les neuf formats AtariDOS existants et fournir les seuls contrats internes des trois nouveaux lecteurs ; aucune modification des lecteurs DOS/SpartaDOS.

> Les batteries natives prévues dans ce document sont abandonnées sur consigne utilisateur. Ne pas les relancer. Le banc temporaire a été retiré après conservation du statut réel dans `docs/validation/media/bancs-prepares-retires.md`.

Actualisation du 17 novembre 2026 : contrats, 21 adaptateurs ordonnés, catégories d’erreurs et filtres sont présents. Atari partage FormatRecognition.* au lieu des trois dossiers initialement envisagés. La batterie abandonnée n’est pas déclarée exécutée ; seules les actions de code et de suppression sont clôturées. Voir reconciliation-suivi-initial.md.
