# Validation reconstruction SCP Commodore GCR — 15 novembre 2026

Le banc compile cinquante sources C# originales sans modification de leur dépôt ; leurs empreintes correspondent à l’inventaire. La branche privée ReadGcr est invoquée directement pour comparer son contrat sans prétendre valider le routage 1581/900. Les modèles SCP/secteurs et le registre de décodage sont des adaptateurs de données/dispatch ; les algorithmes de reconstruction, sélection, fenêtres, décodeur/encodeur et géométries sont les sources originales.

439 cas comprennent géométries 1541/1571 standard/étendues, demandes absentes/correctes/majuscules/inconnues, pistes 0..255 sélectionnées et frontières, numéros négatifs ou hors zones, données absentes/vides/de tailles variées, captures nulles, piste physique de face un sans révolution, ordre non trié des adresses et candidats rejetés après inférence. Les 27 combinaisons de trois états d’intégrité exercent priorité et stabilité sur trois révolutions. Les candidats perdants non nuls et les secteurs hors géométrie qui influencent l’extension sont couverts.

59 fixtures utilisent réellement le décodeur GCR, à partir de flux produits par l’encodeur C# original : pistes logiques 1..80 sélectionnées, demandes de formats contradictoires, données nulles/non nulles, plusieurs secteurs et révolutions, coupes de piste, face physique vide et progression. Les sorties comparent format, dimensions, capacité/nombre logique et tous les blocs dans l’ordre, avec adresse, taille/SHA-256, intégrité nullable et révolution. Les notifications réelles comparent coordonnées et compteurs.

57 captures compatibles sont ensuite écrites en SCP puis relues/reconstruites en C++ ; le résultat reste identique au résultat déjà comparé au C#. Les cas sans révolution ou rejetés avant production ne sont pas présentés comme des allers-retours réussis. L’annulation initiale, l’annulation depuis la progression et les exceptions du rappel sont contrôlées. Les 29 textes sont chargés depuis les DLL et comparés aux catalogues relus.

- Compilation complète et déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 005 contrôles, aucune différence**, 380 cas valides, 59 refusés, 59 fixtures de flux GCR et 57 allers-retours SCP.
- Audit des littéraux : **359 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `13a16ba51185f99c962078d5c35518c76fa77b3f8f4c36c6a3cd7ca06036be6a`.

Le banc `build/verification-commodore-scp-gcr/`, ses fixtures/références/binaires et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. La branche 1581 MFM et les conversions ne sont pas validées par ces contrôles.
