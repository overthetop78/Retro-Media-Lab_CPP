# Injection des migrations dans les images vierges

- [x] Service et capacités
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Migration.h` et `src/Migration/Constants.h` ; créer `src/Migration/Capabilities.cpp`, `Internal.h` et `Service.cpp` : capacités C# des sept destinations, politiques de noms existantes, validation et consentement avant écriture, image vierge complète, géométrie et adresses conservées.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AppleSos.h` et `src/FileSystems/Apple/Sos/Writer.cpp` : réutiliser l'écrivain ProDOS puis ajouter le marqueur SOS sans modifier le profil ni les adresses.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `src/Translations/Catalogs/Constants/MediaFileSystems/migration-injection.json` et `Languages/*/MediaFileSystems/migration-injection.json` : erreurs d'injection traduisibles.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler les composants ajoutés.
- [x] Vérification ciblée
  - [x] Créer `build/verification-migration-injection/CMakeLists.txt` et `main.cpp` : injection et relecture des sept destinations, perte de métadonnées acceptée explicitement, adresses et capacité conservées, rejet des images non vierges ou incomplètes, annulation et paramètres invalides.
  - [x] Supprimer `build/verification-migration-injection/` et ses sorties après utilisation.
