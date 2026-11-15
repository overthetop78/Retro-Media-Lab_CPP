# ISO et sélection commune des writers

- [x] Format ISO
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Iso/Constants.h`, `Reader.h`, `Reader.cpp`, `Writer.h` et `Writer.cpp` : porter les extensions ISO/GCM, l’alignement 2048, la piste continue et ses métadonnées, le contrôle de longueur réelle et le profil d’écriture C# ; réutiliser le modèle optique et le writer atomique.
- [x] Registre des writers
  - [x] Modifier `src/MediaEngine/src/Images/Writers.cpp` : utiliser la primitive ordinale partagée pour le contrat SupportsTarget, comme le comparateur C#, y compris les identifiants non ASCII.
  - [x] Créer `src/MediaEngine/src/Images/WriterRegistry.h` et `WriterRegistry.cpp` : conserver les writers et leur ordre, refuser les doublons, normaliser les extensions, séparer cibles annoncées et compatibilité des documents, résoudre le premier writer compatible comme en C#.
- [x] Intégration
  - [x] Modifier les diagnostics MFS (`Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`), `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer les sources et les erreurs typées/traduisibles.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/iso-writers.json` et `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/iso-writers.json` : générer avec Argos puis relire les validations ISO et du registre.
- [x] Contrôle ciblé
  - [x] Modifier `src/MediaEngine/src/Formats/Optical/Iso/Writer.cpp` : refuser aussi les secteurs de prégap stocké, omis dans le prédicat C#, puisqu’une sortie ISO ne peut pas les conserver ; vérifier cette divergence volontaire dans le banc ciblé.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger le nom technique ISO dans les traductions des formats.
  - [x] Modifier `src/Translations/Catalogs/Languages/ar-SA/MediaFileSystems/gamecube-fst.json` : conserver l’invariant ISO dans GameCubeNotIso, altéré par la traduction précédente et détecté par le contrôle commun.
  - [x] Créer `build/verification-iso-writers/CMakeLists.txt` et `main.cpp` : un contrôle synthétique ISO → document → exploration, écriture atomique, sélection ordonnée et refus des pertes de structure ; ne pas rejouer les suites antérieures.
  - [x] Supprimer `build/verification-iso-writers/` : retirer le banc et ses images temporaires après conservation du résultat.
