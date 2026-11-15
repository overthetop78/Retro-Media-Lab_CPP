# Intégration des portages Apple parallèles

- [x] Intégration commune
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Commodore/Dos/Sector.cpp` : typer explicitement la borne positive int32 lors de sa comparaison avec le nombre de cylindres uint32, sans changer la garde existante, pour retirer C4018.
  - [x] Modifier `src/MediaEngine/src/Formats/TwoImg/TwoImg.h`, les `DiagnosticKeys.h` Engine/MFS et `scripts/quality/audit_literals.py` : conserver les valeurs implicites de l’enum public, placer les constantes sur leurs lignes propres et contrôler les invariants isolés des catalogues selon le contrat Constants/Languages existant.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h`, `src/MediaEngine/src/Formats/FormatIds.h`, `src/MediaFileSystems/src/FileSystems/Apple/Dos/Constants.h` : centraliser les quatorze identifiants Apple/Macintosh partagés du C# dans MFS et réutiliser ces constantes dans Engine et Apple DOS sans changer leurs valeurs.
  - [x] Modifier `scripts/quality/audit_literals.py` : reconnaître les valeurs numériques des membres d’enums comme constantes nommées, sans exempter les autres valeurs brutes des fichiers publics.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `src/MediaFileSystems/CMakeLists.txt`, `scripts/quality/audit_literals.py` et les diagnostics/catalogues décrits dans les tâches 68, 69 et 70 : intégrer les sources et diagnostics des trois agents, puis compiler les cibles affectées et les catalogues révisés.
- [x] Nettoyage
  - [x] Supprimer `build/apple-parallel-build.log`, `build/apple-parallel-build-incremental.log`, `build/apple-parallel-build-final.log`, `build/apple-containers-translations.log`, `build/apple-dos-address-translation.log` après enregistrement des résultats des blocs 67 à 71.

- [x] Support des tableaux de constantes Macintosh
  - [x] Modifier `scripts/quality/audit_literals.py` : reconnaître les initialisateurs multilignes d’une déclaration inline constexpr dans les seuls headers de constantes, sans autoriser de littéraux dans les autres déclarations.
