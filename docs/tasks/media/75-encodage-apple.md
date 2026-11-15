# Encodage des pistes Apple et écriture NIB

- [x] Encodage et empaquetage partagés
  - [x] Créer `src/MediaEngine/src/Encoding/BitPacker.h`, `BitPacker.cpp`, `PackingConstants.h`, modifier `src/MediaEngine/src/Encoding/Bits.cpp`, `src/MediaEngine/src/Formats/Woz/Writer.cpp` : porter le packer MSB C# complet et mutualiser les parcours d’écriture existants, y compris remplissage et limites.
  - [x] Créer `src/MediaEngine/src/Formats/Apple/TrackEncoding.h`, `TrackEncoding.cpp`, `TrackEncodingConstants.h` : porter les services d’encodage standard/RWTS18 avec registre possédé/injecté, géométries, erreurs, secteurs physiques DOS et découpage ProDOS.
  - [x] Créer `src/MediaEngine/src/Formats/Nib/Writer.cpp`, modifier `Nib.h` : empaqueter les pistes fixes NIB et écrire par le service atomique existant.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, diagnostics MediaEngine et catalogues de traduction : intégrer les sources, constantes et erreurs (coordonné par l’agent principal).
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/apple-track-encoding.json` : relire les 29 catalogues Argos, corriger les sens techniques et préserver les paramètres.
- [x] Vérification ciblée
  - [x] Créer `build/verification-apple-track-encoding/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json`, `sources.json` : comparer les sources C# des services/packer/writer NIB, l’ordre ProDOS, limites, secteurs manquants/dupliqués, annulation, registres injectés et seuls parcours WOZ/FM/MFM affectés par la mutualisation.
  - [x] Supprimer `build/verification-apple-track-encoding/` et les artefacts temporaires après validation.
