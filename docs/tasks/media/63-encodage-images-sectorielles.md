# Encodage des images sectorielles et gardes associées

- [x] Orchestration des pistes
  - [x] Créer `src/MediaEngine/src/Encoding/SectorImageEncoder.h`, `SectorImageEncoder.cpp` : porter SectorImageTrackEncoder, sa sélection de politique, l’ordre physique enregistré, le regroupement cylindre/face, les deux modes d’encodage physique et leurs unités ; annulation et erreurs traduites.
  - [x] Modifier `src/MediaEngine/src/Encoding/Constants.h`, `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : constantes et raccordement des diagnostics d’image logique, politique absente, absence de piste et conversion de durée hors limites.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/sector-image-encoding.json` et ses catalogues de langues : diagnostics d’encodage des images ; génération Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger l’unité technique ticks afin d’éviter sa traduction comme parasite dans les messages de durée.
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/flux-core.json` : conserver l’unité ticks dans le diagnostic historique de dépassement d’intervalle, conformément à son ajout aux invariants communs.
- [x] Gardes révélées pendant le raccordement
  - [x] Créer `src/MediaFileSystems/src/Primitives/Identifiers.h`, modifier `src/MediaFileSystems/src/Exploration/EntryAnalysis.cpp`, `src/MediaFileSystems/src/FileSystems/Fat/Fat12/Writer.cpp`, `src/MediaFileSystems/src/FileSystems/Amiga/Dos/Codec.cpp` : vérifier les règles C# puis conserver U+FEFF dans les validations UTF-8 encore concernées, avec un helper privé au composant.
  - [x] Modifier `src/MediaEngine/src/Formats/Commodore900/FluxReader.cpp` : corriger la comparaison signé/non signé des numéros de secteur après le contrôle de négativité, sans modifier le filtrage des secteurs.
  - [x] Modifier `src/MediaEngine/src/Reconstruction/Sector/Policy.h`, `Policy.cpp`, `OtherFamilies.cpp` et `Encoding/SectorImageEncoder.cpp` : lire les blocs disponibles avec leurs métadonnées sans imposer le contrôle de taille réservé à GetBlock dans le C#, y compris pour JFD ; conserver le diagnostic de bloc absent.
- [x] Vérification
  - [x] Créer `build/verification-sector-image-encoding/Reference.csproj`, `Reference.cs`, `FormatConstants.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer le regroupement et la sélection C# originaux, l’ordre des pistes et secteurs, les erreurs, les durées physiques et les secteurs produits ; vérifier les gardes UTF-8 et le filtre Commodore 900 dans leurs composants réels, en Debug/Release.
  - [x] Supprimer `build/verification-sector-image-encoding/` et ses journaux : retirer le banc et les artefacts temporaires après validation.
