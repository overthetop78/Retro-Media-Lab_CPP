# Composition générale de reconnaissance

- [x] Cartouches console requises par le registre
  - [x] Créer `src/MediaEngine/src/Formats/Cartridge/ConsoleConstants.h` et `Console.cpp` ; modifier `Readers.h` : reprendre les extensions et systèmes du C#, banques possédées de 16 Kio, métadonnées et limite de banques sans transformation des octets.
- [x] Registre commun
  - [x] Créer `src/MediaEngine/src/Composition/Recognition.h` et `Recognition.cpp` : tous les lecteurs de la composition C# dans le même ordre, instance SCP partagée, union des extensions sans doublons ; conserver la reconnaissance dans MediaEngine.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/console-cartridge.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/console-cartridge.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation et constantes.
- [x] Contrôle de l'intégration
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/Raw/Reader.cpp` : reconnaître les identifiants RAW explicitement demandés sans sensibilité à la casse, conformément au C#.
  - [x] Créer `build/verification-recognition-composition/CMakeLists.txt` et `main.cpp` : ordre et présence des lecteurs, union des extensions, partage SCP, cartouches à banque partielle, sélection explicite et annulation ; reconnaissance de formats représentatifs sans reprise des suites de codecs.
  - [x] Supprimer `build/verification-recognition-composition/` et les sorties temporaires après utilisation.
