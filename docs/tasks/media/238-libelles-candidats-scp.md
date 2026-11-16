# Libellés des candidats SCP

- [x] Reprendre les noms du C# dans la progression automatique
  - [x] Créer `src/MediaEngine/src/Reconstruction/Scp/NamesConstants.h` : noms techniques invariants et motifs d'identifiants du C#.
  - [x] Modifier `src/MediaEngine/src/Reconstruction/Scp/Candidates.h` et `Candidates.cpp` : exposer le nom du candidat avec la priorité et le repli de famille du C#.
  - [x] Modifier `src/MediaEngine/src/Exploration/Scp/Service.cpp` : fournir le nom lisible pour le début et la fin d'un candidat.
  - [x] Modifier `RetroMediaLab.vcxproj` et `.filters` : référencer l'en-tête.
- [x] Vérifier le choix des noms
  - [x] Créer `build/verification-scp-names/CMakeLists.txt` et `main.cpp` : vérifier les correspondances exactes, les motifs insensibles à la casse et les cinq replis de famille sans décoder de capture.
  - [x] Supprimer `build/verification-scp-names/` après utilisation.

- [x] Intégrer le fichier de constantes au contrôle existant
  - [x] Modifier `scripts/quality/audit_literals.py` : déclarer `NamesConstants.h` parmi les fichiers de constantes autorisés.
