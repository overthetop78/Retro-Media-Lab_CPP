# Relecture des diagnostics de blocs et documents

17 novembre 2026. Les 34 messages de MediaFileSystems/media-document.json et block-representation.json ont été lus dans les 29 langues (986 valeurs). 370 messages corrigés dans 53 fichiers de 27 langues. Les catalogues Argos existants sont conservés quand leur sens convient ; les changements sont des corrections de relecture.

Corrections de sens : décalage de lecture, nombres d’octets, positivité numérique, null, plages manquantes, plages non stockées, interdiction de chevauchement et limites des entiers signés. La lecture bornée était devenue illimitée en turc ; une plage manquante était annoncée présente en coréen ; l’obligation de non-chevauchement était affaiblie en hébreu. Ces erreurs concernaient les textes générés, pas les conditions du code C# ou C++. Les caractères traditionnels chinois sont rétablis dans zh-Hant.

Vérifications réussies : mêmes clés et paramètres que Constants ; audit des littéraux (1 607 fichiers C++, 29 langues) ; compilation commune Release des 27 DLL de langues modifiées, puis installation dans lib/Languages. Journaux : build/media-block-document-catalogs-release.log et build/media-block-document-catalogs-install.log. Aucun codec, algorithme natif ou test de format modifié ou relancé.

La relecture amstrad-fds.json reste ouverte dans les tâches 98/99 ; celle des systèmes optiques reste dans la tâche 95.
