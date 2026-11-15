# Lecteurs Atari complémentaires et registre sectoriel complet

- [ ] Lecteurs Atari sans catalogue classique
  - [ ] Créer `src/MediaFileSystems/src/FileSystems/Atari/KFile/Constants.h`, `Reader.cpp`, `src/MediaFileSystems/src/FileSystems/Atari/BootDisk/Constants.h`, `Reader.cpp`, `src/MediaFileSystems/src/FileSystems/Atari/ClkGraphicsLibrary/Constants.h`, `Reader.cpp` : reconnaissance KBoot/boot complet/CLK, métadonnées et contenu CLK à partir des sources C# réelles, sans noms inventés.
- [ ] Contrats sectoriels et erreurs structurées
  - [ ] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/SectorReader.h`, `SectorRegistry.h` et `src/MediaFileSystems/src/Exploration/Composition/Constants.h`, `Internal.h`, `Registry.cpp`, `Identity.cpp` : contrats de lecteurs, matches/échecs/rapports/candidats possédés, sélection ordonnée et identité Unicode fidèle.
  - [ ] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp` et créer `src/Translations/Catalogs/Constants/MediaFileSystems/sector-registry.json` : catégorie publique ErrorKind vérifiée selon les factories C#, erreurs de configuration et Atari traduisibles ; ne capturer que les corruptions de Read, jamais les erreurs de sonde, IO, argument ou annulation.
- [ ] Composition complète des 21 lecteurs
  - [ ] Créer `src/MediaFileSystems/src/Exploration/Composition/Catalog.cpp`, `CatalogConstants.h` : publier identifiants exacts et tous CatalogFormatIds des 21 lecteurs C# dans leur ordre initial, réutiliser les fonctions déjà portées et les identifiants centraux.
  - [ ] Modifier CMake MFS, audit des constantes et catalogues de langues : intégrer les nouvelles sources et traductions, actions partagées du parent ; intégrer l’adaptation whole-volume aux documents génériques fournis par le bloc88.
- [ ] Validation ciblée
  - [ ] Créer `build/verification-sector-registry/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer Atari complémentaires, découverte/ordre/metadonnées21, sélection/failures/injection de catégories, première interprétation secondaire et identité Unicode, sans rejouer les 21 bancs de lecture.
  - [ ] Supprimer `build/verification-sector-registry/` et ses artefacts après validation.

- [x] Catégories des dépassements CP/M (agent apple_filesystems)
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Cpm/Directory.cpp` : typer explicitement ErrorKind::Overflow à la garde de multiplication checked BlockCount×BlockSize dans flatten, sans modifier les conditions ni le diagnostic.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Cpm/Content.cpp` : typer explicitement ErrorKind::Overflow à la garde checked origine+allocation×taille dans offset, sans modifier les conditions ni le diagnostic ; ne pas reclasser les limites de flux append/zeros en dépassements arithmétiques.

- [ ] Fidélité des catégories et adaptateur document
  - [ ] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h` et `src/MediaFileSystems/src/Diagnostics.cpp` : distinguer ArgumentOutOfRange d’InvalidArgument et catégoriser exhaustivement chaque diagnostic selon les exceptions C# ; conserver les surcharges de catégorie explicite.
  - [ ] Modifier `src/MediaFileSystems/src/FileSystems/Amiga/Dos/Directory.cpp`, `Recovery.cpp`, `src/MediaFileSystems/src/FileSystems/Commodore/Coherent/Directory.cpp`, `src/MediaFileSystems/src/FileSystems/Acorn/BbcDfs/Sides.cpp`, `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Directory.cpp`, `BlockView.cpp` : filtrer les catégories exactes capturées en C#, sans absorber IO, arguments ou annulations non prévues.
  - [ ] Modifier `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Codec.cpp` : typer les deux bornes numériques write16/write24 en ArgumentOutOfRange, en conservant le diagnostic et toutes les conditions.
  - [ ] Créer `src/MediaFileSystems/src/Exploration/Composition/Document.cpp` : adapter le contrat sectoriel aux interfaces ImageDocument/SectorRepresentation et au VolumeDescriptor validé du bloc88, limité au volume entier comme en C#.

- [ ] Reconnaissance Atari partagée
  - [ ] Créer `src/MediaFileSystems/src/FileSystems/Atari/FormatRecognition.h`, `FormatRecognition.cpp` : réutiliser les neuf formats AtariDOS existants et fournir les seuls contrats internes des trois nouveaux lecteurs ; aucune modification des lecteurs DOS/SpartaDOS.

> Les batteries natives prévues dans ce document sont abandonnées sur consigne utilisateur. Ne pas les relancer. Le banc temporaire a été retiré après conservation du statut réel dans `docs/validation/media/bancs-prepares-retires.md`.
