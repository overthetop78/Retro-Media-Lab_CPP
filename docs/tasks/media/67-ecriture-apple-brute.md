# Ã‰criture Apple brute

- [x] Writers
  - [x] CrÃ©er `src/MediaEngine/src/Formats/Apple/RawWriter.cpp`, modifier `Raw.h`, `Constants.h`, `src/MediaEngine/src/Formats/FormatIds.h`, `src/MediaEngine/CMakeLists.txt` : porter les sorties Apple II DOS/ProDOS/SOS, les charges utiles 2IMG et Macintosh GCR/MFM, avec validation et Ã©criture atomique existante.
  - [x] Modifier les diagnostics MediaEngine, crÃ©er `src/Translations/Catalogs/Constants/MediaEngine/apple-writing.json` et ses homologues dans `Languages/` : traduire lâ€™erreur dâ€™adresse Macintosh ; rÃ©utiliser les diagnostics gÃ©nÃ©riques appropriÃ©s pour les autres refus.
- [x] VÃ©rification ciblÃ©e
  - [x] CrÃ©er `build/verification-apple-writing/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les octets des writers C# et C++, formats/extensions acceptÃ©s, gÃ©omÃ©tries, blocs absents ou mal dimensionnÃ©s et adresses Macintosh ; contrÃ´ler lâ€™Ã©criture par chemin et lâ€™annulation sans relancer les tests des lecteurs.
  - [x] Supprimer `build/verification-apple-writing/` et les journaux temporaires aprÃ¨s validation.
