# Sources et partitions — validation

Date : 15 novembre 2026. Ce bloc ne termine pas MediaEngine ni MediaFileSystems.

## Vérifications exécutées

- Compilation et installation Debug/Release de MediaEngine.dll et MediaFileSystems.dll ; aucune erreur ni avertissement dans les deux compilations finales.
- 323 images/cas donnent les mêmes volumes et diagnostics structurés que les détecteurs C# originaux : MBR primaire, EBR et chaînes, GPT 512/4096, CRC, GUID, noms Unicode, défauts de bornes, chevauchements, ordre des partitions, signature absente, table vide, chaînes cycliques et limite de 4 096 EBR. Cas aléatoires déterministes supplémentaires ; détecteurs isolés et sélection automatique.
- Le banc C# compile directement les détecteurs, contrats et fonctions d’origine par liens en lecture seule. Le banc C++ appelle les DLL construites. Les diagnostics C++ à clés/arguments sont rendus avec les phrases C# d’origine pour la comparaison ; la formulation anglaise des catalogues de production a été précisée pour la traduction (« storage medium »).
- Un 324e cas confirme une correction : un tableau GPT de 65 664 octets à LBA 0 dans une image de 65 536 octets passait le contrôle de capacité C# à cause de la division entière signée. Le C# lève ensuite ArgumentOutOfRangeException ; le C++ renvoie le diagnostic GptArrayOutside sans lire hors limites.
- Sources : mémoire copiée et indépendante de l’entrée, lectures exactes dépassant la taille du chunk interne, plages vides à la fin, offsets extrêmes, bloc nul/trop grand, chemin fichier Unicode, fichier tronqué, fermeture effective du flux après erreur et suppression du fichier alors que la source existe encore.
- Annulation avant lecture et pendant l’examen GPT ; une erreur d’accès injectée pendant la lecture de l’entête secondaire reste une exception d’accès et n’est pas transformée en simple erreur de contenu.
- 1 044 résolutions de diagnostic (36 clés × 29 langues) via TranslationService et les DLL de langues installées : égalité exacte avec les JSON correspondants. Présence, paramètres et invariants contrôlés pour tous les fichiers. Relecture et corrections de libellés documentées.
- Audit des constantes et catalogues et git diff --check réussis. Les contrats/fonctions repris restent rangés par responsabilité ; App n’est ni modifié ni relié directement à MFS/MediaAnalysis.

## Preuves de référence

- SHA-256 de `cases.json` temporaire : `2d0e78311dbe5fd51879fb78a808d9281833761035ee83038a25c845ae5b53c1`.
- SHA-256 de `csharp.json` temporaire : `5c53b0b176ebfa113b55fe7e2e7663bfbcd1b88a8170563521697bc3e227d8e7`.
- SHA-256 de `cpp.json` temporaire : `35b7b2fe30071a6a3f20afb7fcaa0d4008e8e3012abe4679eb7d2b85c3e71cc6`.

12 sources C# de ce bloc ont été revérifiées identiques à leurs empreintes d’inventaire.

Le banc, ses exécutables, fixtures générées et artefacts .NET/C++ sont supprimés après validation. Aucun disque physique n’est lu ou écrit ; aucun média permanent n’est ajouté. Aucun test n’est intégré aux workflows GitHub.

## Limites et suite

Ce bloc porte le trajet des sources à longueur connue vers les volumes par blocs. Il ne valide pas encore les documents complets, flux, pistes/secteurs dégradés, sessions optiques ou données séquentielles. La taille maximale d’un bloc est fixée à 1 MiB avant allocation ; les tableaux GPT sont plafonnés à 128 MiB comme en C#.

La liste des systèmes de fichiers, la classification reliée à leur exploration, les formats d’images, l’écriture/conversion/migration, la visualisation et les services d’opérations restent à porter. Les lectures de ce bloc sont synchrones et annulables ; le service appelant devra les exécuter dans le contexte de travail adapté, pas sur le thread graphique.

Référence détaillée : [sources et partitions C#](../../references/retro-media-lab/media/sources-et-partitions.md).
