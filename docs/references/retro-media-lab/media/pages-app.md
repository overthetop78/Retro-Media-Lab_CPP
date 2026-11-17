# Raccordement des pages App

Référence C# consultée en lecture seule : `F:/GW GUI/src/RetroMediaLab.App/Theme/Base/Views/Controls/Explorer/Sections/ExplorerSection.Navigation.cs` et `ExplorerSection.Media.cs`.

L'explorateur présente les dossiers avant les fichiers, navigue par arbre et double-clic, affiche le détail de la sélection, distingue les volumes et les sessions optiques. Les catalogues `Resources/<langue>/Explorer.resx` contiennent déjà les traductions à reprendre. Les états et résultats doivent survivre à la destruction d'une présentation de thème.

Contrats C++ disponibles : `MediaEngine/MediaExploration.h` (`analyzeMediaOpeningAsync`, résultat disque ou média), `ExplorationInfo.h` (volumes et entrées possédées), `DiskExploration.h` (sélection d'interprétation). App n'utilise aucune API MediaFileSystems ou MediaAnalysis.

État initial vérifié : `WorkspaceTabs.cpp` crée sept QWidget vides ; les cinq pages demandées restent donc à implémenter. Ce document ne constitue pas une validation de leur fonctionnement.

## Premier raccordement de l’explorateur

Session conservée dans WorkspaceState ; ouverture asynchrone, annulation et attente de fin à la destruction dans Infrastructure ; navigation et présentation dans la vue du thème. Les volumes et sessions optiques, les dossiers, les fichiers, les dates locales et les types classifiés sont raccordés au résultat public. Les 30 catalogues Explorer (Constants + 29 langues) ont été repris des RESX C# sans nouvelle traduction automatique. TranslationService peut désormais résoudre explicitement le domaine des libellés média.

Validation : compilation Release de la session, de la vue et des 30 catalogues réussie ; contrôle des littéraux réussi (1613 sources) ; frontières de bibliothèques contrôlées sans violation. Aucun codec inchangé n’a été retesté. La vérification à l’écran et le raccordement à la lecture physique restent ouverts, tout comme les quatre autres pages : ce raccordement ne clôture pas l’objectif.
