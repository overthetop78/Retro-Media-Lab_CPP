# Conteneur physique MFM EMU

État du point de sauvegarde : huit fichiers sources créés, portage en cours.
Les diagnostics, les adaptateurs et le raccordement à la compilation restent à réaliser ; ces sources ne sont pas encore compilées ni validées.
Les cases restent ouvertes jusqu'à la réalisation et la vérification du bloc.

- [ ] Structure et signal
  - [ ] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Constants.h`, `Header.h`, `Header.cpp` : révisions, chaînes natives, octets annexes, géométrie physique et adresses de piste.
  - [ ] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Signal.h` et `Signal.cpp` : lecture temporelle et fenêtres de transitions dans les mots de cellules little-endian, conservation des cellules non traversées.
  - [ ] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Container.h`, `Container.cpp`, `GeneratedData.cpp` : source bornée, validation, remplacement immuable de pistes, copie, pistes vierges et génération formatée à la demande.
- [ ] Représentation et adaptateurs
  - [ ] Créer `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Representation.h`, `Representation.cpp`, `Adapter.h`, `Reader.cpp`, `Writer.cpp`, `Creation.cpp` : accès physique/logique Adaptec, vue blocs, lecture/écriture atomique, création vierge et ADFS depuis le formateur existant.
  - [ ] Modifier les diagnostics MediaEngine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/mfm-emu.json` et `Languages/<langue>/MediaEngine/mfm-emu.json` : erreurs traduites ; compléter les invariants communs si nécessaire.
  - [ ] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les fichiers et constantes.
- [ ] Contrôle ciblé
  - [ ] Créer `build/verification-mfm-emu/CMakeLists.txt` et `main.cpp` : en-têtes/révisions, signal et fenêtres, conservation native, source vierge/formatée et accès logique, publication et validations des adresses.
  - [ ] Supprimer `build/verification-mfm-emu/` avec ses données synthétiques et binaires après consignation.
