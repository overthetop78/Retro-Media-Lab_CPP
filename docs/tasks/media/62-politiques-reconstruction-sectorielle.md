# Politiques de reconstruction sectorielle

- [x] Contrats et politiques
  - [x] Créer `src/MediaEngine/src/Reconstruction/Sector/Policy.h`, `Policy.cpp`, `Policies.h`, `Matching.cpp`, `Apple.cpp`, `Commodore.cpp`, `OtherFamilies.cpp`, `Constants.h` : copie fidèle des métadonnées, découpage/regroupement Apple/RX02/1581, adressage Atari/Lisa, contrôles JFD, profils rares et politiques exactes/par préfixe ; cadences zonées conservées.
  - [x] Créer `src/MediaEngine/src/Reconstruction/Sector/Registry.h`, `Registry.cpp`, `src/MediaEngine/src/Composition/SectorPolicies.h`, `SectorPolicies.cpp` : catalogue ordonné complet et résolution du premier résultat, collections injectées et propriété des politiques.
  - [x] Modifier `src/MediaEngine/src/Formats/FormatIds.h`, `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : identifiants nécessaires, intégration et diagnostics nommés, sans dépendance GUI/matériel.
  - [x] Modifier `src/MediaEngine/src/Primitives/Identifiers.h`, `IdentifierConstants.h` : comparaison ordinale des clés personnalisées Unicode des politiques exactes/par préfixe, sans assimilation ASCII/Unicode et avec cas de référence C#.
  - [x] Modifier `src/MediaEngine/src/Models/Sectors/SectorImages.cpp`, `src/MediaEngine/src/Reconstruction/Iso/Builder.cpp` : réutiliser la validation d’identifiant conservant U+FEFF pour les images construites et reconstruite par les politiques.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/sector-policies.json` et ses catalogues de langues : diagnostics des contrats, des profils absents et des valeurs invalides ; génération Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-sector-policies/Reference.csproj`, `Reference.cs`, `FormatConstants.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les politiques C# originales avec leurs modèles sectoriels, toutes les familles, métadonnées, géométries, frontières zonées, erreurs et propriétés ; vérification Debug/Release.
  - [x] Supprimer `build/verification-sector-policies/` et ses journaux : retirer les tests et artefacts temporaires après validation.
