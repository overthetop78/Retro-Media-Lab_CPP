# Validation des politiques sectorielles — 15 novembre 2026

Le banc compile 42 fichiers C# originaux en lecture seule : politiques, registre, contrat, modèles réels SectorImage/SectorBlock/TrackSector, géométries et cadences. Les identifiants de format nécessaires sont extraits des déclarations originales (57 liaisons). Les espaces de noms vides du banc ne servent qu’à satisfaire des using inutilisés ; aucun algorithme de politique ou de géométrie n’y est remplacé.

Les 59 sources compilées ou consultées pour les constantes/global using correspondent à l’inventaire. Empreinte du manifeste : `c0e250497f374c3b96a8c421a54068402abef91a063f07f099a90458c2f781ab`.

2 688 cas comparent correspondance, choix du codec, adresses converties, cadence et durée de révolution, attributs de piste et tous les champs des TrackSector produits. Sont inclus : limites des zones Apple/Commodore, cylindres Lisa invalides, géométries autour de 80 pistes et du seuil Lisa, blocs courts/longs, 1581 incomplet ou désordonné, tags et CRC, intégrités contradictoires, densités JFD nulles/zéro/mélangées, format JFD sensible à la casse, identifiants de politiques invalides, U+FEFF et les paires de casse Unicode générées à partir des lettres disponibles dans Unicode Python. Ce dernier échantillon n’est pas une preuve de toutes les séquences UTF-16 invalides ou des futures versions Unicode.

Les diagnostics d’erreur sont comparés par clé. Les exceptions C# de densité JFD absente ou nulle sont rapprochées du diagnostic C++ traduit PolicyJfdDensity ; les nouvelles gardes natives pour bloc nul et dépassement 32 bits sont contrôlées séparément. Les contrôles C++ vérifient aussi priorité de résolution, copie du registre, propriété après destruction, indépendance des onze instances par défaut, annulation des transformations, conservation de U+FEFF dans le modèle et le constructeur ISO. Les neuf textes des 29 DLL de langues sont comparés aux catalogues, soit 261 textes.

- Construction complète et déploiement Debug/Release réussis.
- Chaque configuration : **2 985 contrôles, 0 différence, 2 688 cas, 261 textes**.
- Audits : **560 fichiers C++**, **548 fichiers pour les frontières**, **0 violation** ; git diff --check réussi.
- Correction supplémentaire : validation des identifiants U+FEFF dans les images sectorielles et leur reconstruction ISO.
- Correction de compilation : inclusion explicite de QList pour la comparaison Unicode ; le banc utilise aussi les global using et dépendances réelles nécessaires aux sources C#.
- Les traductions Argos ont été relues puis corrigées, notamment le vocabulaire technique des secteurs, les invariants Lisa FileWare/JFD, les mentions de null et l’entier signé 32 bits.
- Banc et journaux temporaires retirés après validation ; aucun test ajouté au workflow GitHub.

Ces contrôles portent sur la reconstruction sectorielle et ses contrats ; ils ne valident pas encore l’orchestration d’une image complète ni l’application entière.
