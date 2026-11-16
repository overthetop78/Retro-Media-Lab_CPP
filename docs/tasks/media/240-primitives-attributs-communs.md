# Primitives et attributs communs

- [x] Centraliser les attributs d'encodage et l'écriture 24 bits
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/TrackEncodingAttributeKeys.h` : les six clés C# et la construction des clés de tag.
  - [x] Modifier `src/MediaEngine/src/Codecs/AppleII/Constants.h`, `AppleIwm/Constants.h`, `Commodore/Commodore1541Constants.h`, `Iso/FmConstants.h` et `Iso/MfmConstants.h` : référencer les clés communes.
  - [x] Modifier `src/MediaFileSystems/src/Primitives/Binary.h` : ajouter une écriture little endian 24 bits vérifiant la destination avant modification.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Formatter.cpp` et `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Codec.cpp` : réutiliser cette primitive en conservant les validations propres aux formats.
  - [x] Modifier `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le contrat et ses constantes.
- [x] Vérifier le bloc commun
  - [x] Créer `build/verification-common-encoding/CMakeLists.txt` et `main.cpp` : clés, tags et écriture 24 bits, limites de destination et absence d'écriture partielle ; sans retester les codecs.
  - [x] Supprimer `build/verification-common-encoding/` après utilisation.
