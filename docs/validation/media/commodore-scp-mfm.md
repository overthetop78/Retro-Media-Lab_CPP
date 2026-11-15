# Validation reconstruction SCP Commodore 1581 — 15 novembre 2026

51 sources C# originales sont compilées sans modifier leur dépôt et leurs empreintes correspondent à l’inventaire. La branche privée Read1581 est invoquée directement. Les modèles SCP/secteurs et le registre sont des adaptateurs de données et de dispatch ; les algorithmes de fenêtres, géométrie 1581, reconstruction, sélection d’intégrité et encodage/décodage ISO MFM sont les sources originales. Les branches GCR/900 et le routage général ne sont pas présentés comme validés par ce banc.

527 cas couvrent coordonnées physiques int32 extrêmes et frontières, secteur négatif/zéro/1/10/11/extrême, payload absent/vide/de tailles proches de 256/512, états d’intégrité, les 27 combinaisons de trois candidats, stabilité des égalités, ordre non trié des clés, remplacement des données, révolutions absentes/vides et CH enregistrés contradictoires avec les coordonnées physiques.

46 captures utilisent réellement l’encodeur puis le décodeur MFM : cylindres 0/1/39/79, les deux faces, un/trois/dix secteurs, intégrités d’en-tête et de données combinées, numéros hors géométrie, rotations de cellules et plusieurs pistes/révolutions. Les sorties comparent format, géométrie, capacité et nombre logique, tous les blocs dans l’ordre avec adresse/longueur/SHA-256, intégrité nullable et révolution. La progression compare compteurs et coordonnées. Les deux diagnostics de rejet sont comparés séparément.

39 captures valides sont aussi écrites en SCP, relues et reconstruites en C++ ; leurs blocs restent identiques aux résultats comparés au C#. Les cas sans révolution ou sans payload ne sont pas comptés comme des allers-retours réussis. L’annulation initiale du lecteur, de l’accumulateur et de sa finalisation, l’annulation depuis la progression et les exceptions de ce rappel sont contrôlées.

Les deux textes des 29 langues sont chargés depuis les DLL et comparés aux catalogues relus, soit 58 contrôles. Le banc Ninja reprend le préfixe d’inclusion MSVC français du projet.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **627 contrôles, aucune différence**, 527 cas (92 valides, 335 sans secteur décodé, 100 sans secteur utilisable), 46 captures MFM, 39 allers-retours SCP.
- Audit : **377 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `932f2f2cb479500e817dbb6d3a76bfcc08a89b89f9ec475bbb5ef42c3d701e6f`.

Le banc `build/verification-commodore-scp-mfm/`, ses références/binaires et journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub.
