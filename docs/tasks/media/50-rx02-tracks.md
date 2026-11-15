# Pistes DEC RX01/RX02

- [x] Encodage et décodage des pistes
  - [x] Modifier `src/MediaEngine/src/Codecs/DecRx02/Constants.h`, `Rx02.h` : compléter les clés/paramètres de CRC et les contrats d’encodeur/décodeur de piste.
  - [x] Créer `src/MediaEngine/src/Codecs/DecRx02/Encoder.cpp`, `Decoder.cpp`, `Descriptions.h`, `Descriptions.cpp` : en-têtes FM, données FM/M²FM normales/supprimées, CRC, appairage, rejets de taille et descriptions localisables.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compiler les sources et déclarer les diagnostics d’adresse et de code de taille.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/rx02-track.json` et ses 29 catalogues : erreurs et descriptions d’en-têtes, données et marques non classées ; Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-rx02-tracks/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison C# des cellules/révolutions/résultats de pistes FM/M²FM, six marques, CRC, décalage de phase, tailles, troncatures, bornes de recherche, erreurs, langues, registre et annulation Debug/Release.
  - [x] Supprimer `build/verification-rx02-tracks/` et les journaux associés après vérification : retrait du banc et de ses artefacts temporaires.
