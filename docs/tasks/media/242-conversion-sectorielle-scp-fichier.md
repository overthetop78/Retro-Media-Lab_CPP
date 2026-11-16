# Conversion d'un fichier sectoriel vers SCP

- [x] Porter la façade fichier C# sur les convertisseurs existants
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/FluxConversion.h` : exposer le choix de cible SCP et la conversion asynchrone depuis un chemin.
  - [x] Créer `src/MediaEngine/src/Images/Conversion/ScpFile.cpp` : reconnaître par le service commun, exiger une représentation sectorielle, déléguer à `Scp::SectorConverter` et conserver les gardes RAII d'activité.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer la façade.
- [x] Vérifier uniquement le raccordement fichier
  - [x] Créer `build/verification-scp-file/CMakeLists.txt` et `main.cpp` : sélecteur de cible, conversion d'un petit fichier sectoriel temporaire, en-tête SCP et annulation, sans comparaison des codecs déjà validés.
  - [x] Supprimer `build/verification-scp-file/` et ses médias temporaires après utilisation.
