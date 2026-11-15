# Système de fichiers Macintosh MFS

- [x] Lecteur et composants
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Primitives.h`, `Primitives.cpp`, `Constants.h`, `MacRomanTable.h` : reprendre les primitives communes Macintosh, la table MacRoman issue du fournisseur de codepage C# et l’époque 1904 pour réutilisation par HFS.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MacMfs.h`, `src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Mfs/Constants.h`, `Internal.h`, `Allocation.cpp`, `Fork.cpp`, `Directory.cpp`, `Reader.cpp` : porter carte 12 bits, chaînes d’allocation, lectures positionnelles, forks, répertoire plat, métadonnées et lecteur MFS avec les contrats existants.
  - [x] Modifier les fichiers partagés MFS de compilation, diagnostics et constantes et créer les catalogues de traduction MFS : intégrer les sources et primitives Macintosh communes proposées au parent (modifications partagées assurées par l’agent parent).
  - [x] Modifier `src/Translations/Catalogs/Languages/{ar-SA,cs-CZ,da-DK,de-DE,el-GR,es-ES,fi-FI,fr-FR,he-IL,hu-HU,id-ID,it-IT,ja-JP,ko-KR}/MediaFileSystems/macintosh-mfs.json` : relire et corriger les 14 premières langues (agent parallèle apple_twoimg).
  - [x] Modifier `src/Translations/Catalogs/Languages/{nb-NO,nl-NL,pl-PL,pt-BR,pt-PT,ro-RO,ru-RU,sv-SE,th-TH,tr-TR,uk-UA,vi-VN,zh-Hans,zh-Hant}/MediaFileSystems/macintosh-mfs.json` : corriger les 14 dernières langues et vérifier les paramètres/invariants ; contrôler en-US sans dupliquer les invariants isolés.
  - [x] Modifier `Mfs/Internal.h`, `Fork.cpp`, `Directory.cpp`, `Reader.cpp`, `Constants.h` et les diagnostics/catalogues partagés : remplacer les descriptifs fixes français des arguments par des contextes internes data/resource/directory/volume et des clés traduites spécialisées, sans changer la lecture des octets ou les conditions d’acceptation.
  - [x] Modifier les 29 catalogues `src/Translations/Catalogs/Languages/*/MediaFileSystems/macintosh-mfs-context.json` : relire/corriger les 14 premières langues avec apple_twoimg, les 14 dernières avec apple_filesystems et contrôler en-US, les paramètres et les invariants (source Constants intégrée par le parent).
- [x] Adaptations communes Macintosh pour HFS
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Primitives.h` et `Mfs/Internal.h` : déplacer ForkKind dans le namespace commun Macintosh et conserver un alias interne MFS, sans modifier ses valeurs.
  - [x] Modifier `Macintosh/Primitives.h`, `Primitives.cpp`, `Constants.h`, `Mfs/Directory.cpp` et `Mfs/Constants.h` : déplacer les helpers fileType et lessName communs aux lecteurs C# MFS/HFS, conserver les algorithmes et remplacer les appels MFS sans dépendance HFS vers MFS.
- [x] Correction Finder ciblée
  - [x] Modifier `Macintosh/Primitives.cpp` et `Constants.h` : reprendre IsNullOrWhiteSpace après ASCII.Trim, afin que les types Finder entièrement composés d’espaces ASCII utilisent le commentaire de repli ; préserver les types mixtes.
- [x] Vérification ciblée
  - [x] Créer `build/verification-mac-mfs/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json`, `contexts.json` : comparer les sources C# originales sur les cartes et forks, volumes MFS et erreurs structurelles, noms MacRoman et dates, sans relancer le banc Apple DOS ; vérifier les 14 sites/arguments spécialisés et les deux annulations sans répéter les 63 contenus déjà passés.
  - [x] Supprimer `build/verification-mac-mfs/` et ses artefacts après validation.
