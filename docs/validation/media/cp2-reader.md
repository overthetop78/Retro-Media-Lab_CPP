# Validation ciblée du lecteur CP2

Le 16 novembre 2026, compilation Debug MediaEngine réussie avec MSVC 19.44 (`build/cp2-build.log`, `build/cp2-final-build.log`) ; les 30 DLL Constants/langues sont construites (`build/cp2-catalogs.log`).

Un seul programme natif temporaire a généré deux groupes CP2 en mémoire. Il vérifie le tri angulaire, la consommation d'un secteur de 128 octets ignoré dans la reconstruction, l'exclusion d'une adresse synthétique C/H=6 sans données, la priorité au premier duplicata de secteur, la géométrie observée 2×2×3 et son identifiant `ibm.6`, les secteurs manquants remplis de zéros, l'absence de métadonnées de piste inventées, les onze profils déclarés et la sélection par le registre.

Les rejets ciblés couvrent signature, longueur de description, descripteur tronqué, données tronquées, nombre de descripteurs excessif, absence de secteurs, géométrie à trois têtes, identifiant demandé incompatible et annulation.

Résultat : `PASS CP2 chained groups/angular order/synthetic records/size consumption/first duplicate/zero fill/registry/rejections`, dans `build/cp2-smoke.log`. Aucun média utilisateur ni ancienne suite de codecs n'a été utilisé.

Les catalogues anglais et les 28 traductions Argos ont été relus ; 139 messages ont été corrigés, notamment les offsets, pistes, descripteurs et paramètres. Les invariants CP2 et SNATCH-IT sont protégés dans la liste commune. L'audit statique passe sur 996 sources C++ et 29 langues, dans `build/cp2-audit.log` ; cela ne prouve pas l'achèvement des services globaux de MediaEngine.

Le projet de vérification, ses sources et ses binaires temporaires sont supprimés après consignation ; les journaux restent dans build, ignoré de Git.
