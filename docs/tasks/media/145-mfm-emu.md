# Conteneur physique MFM EMU

État : bloc terminé ; compilation Debug, contrôle natif ciblé, catalogues et audit réussis. Programme et binaires temporaires supprimés. Voir `docs/validation/media/mfm-emu.md`.

- [x] Structure et signal
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Constants.h`, `Header.h`, `Header.cpp` : révisions, chaînes natives, octets annexes, géométrie physique et adresses de piste.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Signal.h` et `Signal.cpp` : lecture temporelle et fenêtres de transitions dans les mots de cellules little-endian, conservation des cellules non traversées.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Container.h`, `Container.cpp`, `GeneratedData.cpp` : source bornée, validation, remplacement immuable de pistes, copie, pistes vierges et génération formatée à la demande.
- [x] Représentation et adaptateurs
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Representation.h`, `Representation.cpp`, `Adapter.h`, `Reader.cpp`, `Writer.cpp`, `Creation.cpp` : accès physique/logique Adaptec, vue blocs, lecture/écriture atomique, création vierge et ADFS depuis le formateur existant.
  - [x] Modifier les diagnostics MediaEngine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/mfm-emu.json` et `Languages/<langue>/MediaEngine/mfm-emu.json` : erreurs traduites ; compléter les invariants communs si nécessaire.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les fichiers et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-mfm-emu/CMakeLists.txt` et `main.cpp` : en-têtes/révisions, signal et fenêtres, conservation native, source vierge/formatée et accès logique, publication et validations des adresses.
  - [x] Supprimer `build/verification-mfm-emu/` avec ses données synthétiques et binaires après consignation.
