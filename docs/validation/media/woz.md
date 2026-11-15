# Validation WOZ — 15 novembre 2026

Le banc compile **100 sources C# originales**, reprenant la liste de dépendances du banc NIB existant et ajoutant les huit fichiers WOZ. Les véritables codecs, sélecteurs, builders, packer et modèles C# sont utilisés. Les empreintes correspondent à l’inventaire ; manifeste : `ab8a11ca840439e9635fd4b489201ba8da2bdcecf26307dcc9aebe3056d8e48a`.

**194 comparaisons ciblées, zéro différence** :

- 156 cas de CRC, chunks ASCII, octets complets du writer WOZ1, limites 1/40 pistes et 53 184 bits, lectures WOZ1/WOZ2, mutations d’en-tête/CRC/chunks/TMAP/descripteurs, données tronquées et pistes vides.
- 22 cas de reconstruction partagée Apple II/RWTS18 : secteurs absents ou invalides, tailles, bornes, préférence d’intégrité, doublons, pistes et adresses. Ils couvrent l’extraction des fonctions utilisées par NIB ; les 62 anciens cas NIB ne sont pas rejoués.
- Huit fixtures contenant de véritables secteurs encodés Apple II et RWTS18, lus par WOZ1, WOZ2 et NIB. Les fixtures sont fabriquées avec les encodeurs C++ déjà validés, mais les résultats sont comparés au décodage C# réel : quatre secteurs standards, douze secteurs RWTS18 sur deux pistes et repli standard vide sur une seule piste RWTS18.
- Huit nouveaux cas de concurrence entre descripteurs WOZ1/WOZ2 : candidat le plus riche en première ou seconde position et égalité de scores avec des charges différentes. Le contenu retenu confirme la stabilité du premier candidat à score égal.

Les sorties comparent l’intégralité des octets sérialisés par SHA-256, puis formats, géométries, adresses de chaque bloc, intégrité et contenu sectoriel. Les erreurs sont normalisées par acceptation/refus ; ces comparaisons ne certifient pas une identité des types d’exceptions .NET et C++.

**Trois contrôles natifs supplémentaires** vérifient l’écriture par chemin identique aux octets du builder, la propagation de l’annulation et la conservation de la destination préexistante. Le répertoire et les fichiers temporaires sont libérés par RAII même si un contrôle échoue.

La compilation des bibliothèques Debug est coordonnée avec les autres agents et réussit sans avertissement C++. Les nouveaux catalogues sont générés une seule fois par Argos, puis relus/corrigés ; paramètres et invariants sont contrôlés. La validation globale des frontières et littéraux est réalisée au niveau du projet.

Le banc et ses artefacts temporaires ont été supprimés après le feu vert final de l’agent principal. Aucun test n’est ajouté au workflow GitHub. Aucun défaut de production n’a été trouvé dans les nouveaux parcours WOZ ; aucun parcours inchangé n’a été relancé sans raison.
