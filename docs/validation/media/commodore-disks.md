# Validation géométries et conteneurs Commodore — 15 novembre 2026

## Références et couverture

25 sources C# originales sont compilées sans modifier le dépôt source ; toutes leurs empreintes correspondent à l’inventaire. Les algorithmes de géométrie, dispositions, lecture/builders et writer D81 sont originaux. Les contrats de composition, valeurs sectorielles et sortie mémoire du writer sont des adaptateurs minimaux de banc.

20 640 cas de référence : 5 804 conversions 1571, 4 545 adresses inverses 1541, 3 318 conversions de face, 3 200 adresses et 3 200 conversions de blocs 1581, 294 conversions physiques 1581, 129 reconnaissances, 45 nombres de secteurs, 39 lectures de conteneurs, 27 recherches de disposition, 11 nombres de blocs par face, huit adresses inverses 1571, sept conversions de cylindre, sept validations de writer et six écritures.

Toutes les adresses logiques valides sont exercées ; les conversions physiques 1581 couvrent des coordonnées sélectionnées et leurs limites. S’ajoutent les frontières de zones, les numérotations, deux faces, pistes étendues, dimensions invalides, valeurs int32 et débordements non vérifiés conservés par les helpers. Les neuf dispositions et leurs longueurs adjacentes sont testées, avec identifiants absents, corrects, en majuscules et incorrects.

Les lectures comparent format, dimensions, capacité utile, nombre et ordre des blocs, adresses, tailles, intégrité, diagnostic brut, provenance et SHA-256 de toutes les données. Les cartes parcourent les 256 valeurs d’octet, y compris les codes non nommés. Les noms de dispositions sont résolus en français depuis les DLL et comparés aux noms C#.

Le writer D81 compare les sorties valides par SHA-256 et longueur ; ses validations couvrent identifiant différent/en majuscules, bloc absent/court et taille de bloc modifiée. Les refus sont comparés par catégorie d’exception, sans imposer les anciens textes français aux diagnostics traduits.

## Fiabilité C++

40 images sur source vérifient les lectures différées : aucune lecture initiale de payload, une lecture initiale seulement pour la carte d’erreurs, puis accès bornés au secteur. Les neuf dispositions vérifient conservation de la source après libération par l’appelant, arrêt anticipé de l’énumération sans lecture, bornes de bloc, annulation et destruction du dernier propriétaire.

Une source défaillante lors du chargement de carte ne reste pas retenue après l’exception. Une erreur de lecture sectorielle est propagée et la vue reste utilisable après rétablissement de la source. Lecture/reconnaissance annulées ne touchent pas la source. Le fichier destination sentinelle reste intact lors d’un bloc D81 manquant ou d’une annulation ; la liste des fichiers avant/après confirme l’absence de temporaire abandonné.

Les dix clés des 29 langues sont chargées depuis les DLL installées et comparées aux catalogues relus, soit 290 textes.

## Résultats

- Compilation complète et déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **21 189 contrôles, aucune différence**, 20 089 cas acceptés, 551 rejetés, deux écritures D81 valides comparées.
- Audit des littéraux : 332 fichiers C++, 29 langues ; paramètres et invariants des diagnostics vérifiés.
- SHA-256 du manifeste C# : `888fbe1d3eee63d4d5be57470ed0d707f966b2db3f4dab7e2627ea9076ec19b7`.

Le banc `build/verification-commodore-disks/`, les fichiers D81/sentinelles qu’il produit et ses journaux temporaires sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. Le système de fichiers Commodore DOS et les trajets SCP des autres machines ne sont pas déclarés validés par ces contrôles.
