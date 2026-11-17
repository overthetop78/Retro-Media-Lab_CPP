# Raccordement des pages App

Référence C# consultée en lecture seule : `F:/GW GUI/src/RetroMediaLab.App/Theme/Base/Views/Controls/Explorer/Sections/ExplorerSection.Navigation.cs` et `ExplorerSection.Media.cs`.

L'explorateur présente les dossiers avant les fichiers, navigue par arbre et double-clic, affiche le détail de la sélection, distingue les volumes et les sessions optiques. Les catalogues `Resources/<langue>/Explorer.resx` contiennent déjà les traductions à reprendre. Les états et résultats doivent survivre à la destruction d'une présentation de thème.

Contrats C++ disponibles : `MediaEngine/MediaExploration.h` (`analyzeMediaOpeningAsync`, résultat disque ou média), `ExplorationInfo.h` (volumes et entrées possédées), `DiskExploration.h` (sélection d'interprétation). App n'utilise aucune API MediaFileSystems ou MediaAnalysis.

État initial vérifié : `WorkspaceTabs.cpp` crée sept QWidget vides ; les cinq pages demandées restent donc à implémenter. Ce document ne constitue pas une validation de leur fonctionnement.

## Premier raccordement de l’explorateur

Session conservée dans WorkspaceState ; ouverture asynchrone, annulation et attente de fin à la destruction dans Infrastructure ; navigation et présentation dans la vue du thème. Les volumes et sessions optiques, les dossiers, les fichiers, les dates locales et les types classifiés sont raccordés au résultat public. Les 30 catalogues Explorer (Constants + 29 langues) ont été repris des RESX C# sans nouvelle traduction automatique. TranslationService peut désormais résoudre explicitement le domaine des libellés média.

Validation : compilation Release de la session, de la vue et des 30 catalogues réussie ; contrôle des littéraux réussi (1613 sources) ; frontières de bibliothèques contrôlées sans violation. Aucun codec inchangé n’a été retesté. La vérification à l’écran et le raccordement à la lecture physique restent ouverts, tout comme les quatre autres pages : ce raccordement ne clôture pas l’objectif.

## Visualisation : premier raccordement et essai réel

Références C# supplémentaires : `ViewModels/Visualization/TrackSegment.cs`, `Presenters/Visualization/SectorMediaInspectorPresenter.cs`, `ScpInspectorPresenter.cs`, `Theme/Base/Views/Controls/Visualization/` et `Resources/<langue>/Visualizer.resx`. Le présentateur C# distingue notamment données présentes, absence, défaut d’intégrité et secteur dégradé : la distinction dégradée complète reste à raccorder.

La session Infrastructure prépare les descripteurs sur un worker, conserve le document et les sélections hors du thème, annule les opérations devenues obsolètes et attend leur terminaison à la destruction. La vue dessine les faces/pistes/secteurs ; les flux utilisent des moyennes angulaires calculées à partir des intervalles réels et le choix de révolution. Le signal audio utilise les minima/maxima des échantillons par colonne de pixels. Les autres représentations disposent d’une vue des éléments décrits par MediaEngine et d’une inspection à la demande ; leurs inspections spécialisées restent à compléter.

Compilation Release du bloc et des 30 catalogues réussie. Audit des littéraux : 1620 sources, PASS. Déploiement local par cmake --install effectué.

Essai Windows avec la version installée : ouverture de `F:/Retro/AMX Art 2.1.ssd` en lecture seule ; face circulaire et table des secteurs affichées ; bouton + passant de 100 % à 120 % avec agrandissement réel ; clic sur la piste 24 modifiant la sélection et les octets affichés. Passage à l’explorateur : même image ouverte, système Acorn DFS, capacité 100 Kio, 11 entrées, types de fichiers et libellés français affichés. Fenêtre de vérification fermée normalement et absence de fenêtre RML constatée ensuite.

Cet essai ne valide ni les trois pages encore vides (Lecture, Écriture, Conversion), ni l’ensemble des représentations, ni les changements de langue/thème en cours de session. L’objectif global demeure ouvert.
