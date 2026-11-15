# Validation Commodore DOS — 15 novembre 2026

27 sources C# originales sont compilées sans modification de leur dépôt ; leurs empreintes correspondent à l’inventaire. Les adaptateurs remplacent les contrats de données, pas les algorithmes du lecteur.

3 221 cas de référence couvrent PETSCII (dont un parcours de toutes les unités UTF-16), les 256 types d’entrée, les 256 octets de nom, les cinq géométries D64/D71 standard/étendues et D81, coordonnées, lecture sectorielle, reconnaissance, répertoires, contenus, BAM, tailles, attributs, commentaires et avertissements. Les corruptions comprennent cycles, chaîne supérieure à la fenêtre de reconnaissance, absences, tailles incorrectes et compteurs extrêmes. Les cas invalides comparent les catégories d’échec.

Les contrôles C++ supplémentaires vérifient annulation avant et pendant lecture, absence d’accès à une source déjà annulée, propagation des erreurs de source et dix chemins MediaEngine → MediaFileSystems → MediaAnalysis, avec et sans extension PRG. Les 580 textes (20 clés × 29 langues) sont chargés depuis les DLL et comparés aux catalogues. Le booléen `freeSpaceKnown` est vérifié séparément : il devient faux sur BAM illisible, correction intentionnelle du défaut de valeur par défaut C#.

- Compilation complète et déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **4 146 contrôles, aucune différence**, 3 111 cas valides et 110 rejetés, dix chemins d’exploration/analyse.
- Audit des littéraux : **344 fichiers C++, 29 langues**, contrôle réussi.
- SHA-256 du manifeste de résultats C# : `4a263d35ecbdf4074a4c2f2212f96ca6da88e5e20ef6c6e9e132cc92fcaf9f5a`.

Le banc temporaire `build/verification-commodore-dos/` et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur ni test matériel, aucun ajout au workflow GitHub. Ces contrôles ne valident ni l’écriture Commodore DOS ni l’ensemble du portage des trois bibliothèques.
