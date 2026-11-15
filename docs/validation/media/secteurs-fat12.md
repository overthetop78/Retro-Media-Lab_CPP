# Secteurs et FAT12 — validation

Date : 15 novembre 2026. Bloc 03 du portage complet ; il ne termine pas MediaEngine/MediaFileSystems.

## Vérifications

- Compilations et installations Debug/Release réussies sans erreur ni avertissement du compilateur C++.
- 173 images générées comparées au C# : reconnaissance, label, capacité/espace libre, ordre et arborescence, tailles, clusters, attributs, dates, validité, SHA-256 des contenus, nombre de diagnostics et huit identifiants de classification pour chaque entrée. Aucun écart.
- Dix sorties du writer C++ strictement identiques octet par octet au C# (SHA-256 complets), avec réouverture : 160/180/320/360/720/1200/1440/1680/2880 KiB, dont une construction MSX distincte. Fichiers vides, contenu sur plusieurs clusters et sous-répertoires imbriqués.
- Les 28 identifiants du catalogue sont examinés ; dispositions IBM historiques/Apricot et variantes DOS Plus/JFD 1024 également construites séparément.
- Secteurs absents/tronqués à chaque position des structures et débuts de données ; FAT secondaire, en-tête nul admissible, chaînes libres/réservées/cycliques/hors plage, tailles déclarées excessives, attributs, marqueurs supprimés/fin, labels BPB, dates invalides et compte de secteurs 32 bits.
- 262 144 combinaisons de date/heure FAT : même résultat/null que le décodeur C#. Comparaison groupée par SHA-256 des valeurs calendaires. Quinze cas supplémentaires comparent la détection BPB : bornes, dimensions, longueur exacte, total 32 bits, structure tronquée.
- Contrôles C++ spécifiques : exceptions à codes, annulation lecture/écriture/exploration/énumération, secteurs doublons/hors bornes, dimensions/capacité, taille stricte, ordre d’énumération, copie indépendante des octets et métadonnées physiques, reclassification sans perte de ces métadonnées, préférence de copie FAT fiable, intégrité invalide explicite, cycle de répertoires, budget global contre liens croisés, noms doublons, fichiers avec enfants, dates invalides, disque plein et géométries rejetées. Les géométries faisant osciller le calcul de taille du C# sont vérifiées uniquement en C++, sans faire tourner le C# dans une boucle infinie.
- Source fichier → image sectorielle paresseuse → FAT12 → MediaAnalysis exécutée via les DLL ; domaine de traduction MediaAnalysis conservé pour les entrées.
- Régressions ciblées MBR et GPT après partage des primitives binaires : partitions, offsets, GUID/nom et CRC des entêtes/tableaux.
- 667 résolutions réelles via TranslationService et les DLL de langues installées (23 clés × 29 langues) égales aux JSON. Paramètres, invariants et absence de marqueurs temporaires contrôlés dans les 29 fichiers. Relecture et corrections des résultats Argos.
- Le banc regroupe 1 197 contrôles, dont la comparaison groupée des 262 144 dates, en plus des détails par image. Même résultat en exécution Debug et Release, aucun écart final. Les sources C# sont compilées par liens en lecture seule ; seul le contrat de branchement IFileSystemReader est réduit dans le banc pour ne pas importer les services de documents hors de ce bloc. Aucun algorithme de référence n’est réécrit.

## Empreintes

- Manifest complet des 173 images et cas BPB/classification : `250e9a58fb3f1965c06fb7b61dcfa4bc874d22c102ffc531f1701baad08a9169`.
- SHA-256 des valeurs calendaires des 262 144 combinaisons : `7dcdfa9bd9edb3c6f2a2e1ab1711ae5e29c4a45072a0fce5585cbcb30a6ae4f1`.

43 fichiers sources C# concernés ont conservé leur empreinte d’inventaire. Audit des littéraux et `git diff --check` réussis. Aucun matériel physique n’est lu ou écrit. Les fichiers et exécutables temporaires sont supprimés après validation, sans ajout de test au workflow GitHub.

## Limites

Ce contrôle porte le sous-ensemble sectoriel FAT12, pas l’ensemble des images/flux ou des services du projet. L’observation WithDataAccess et la façade publique MediaEngine restent à porter. Les différences volontaires de fiabilité sont décrites dans [la référence du bloc](../../references/retro-media-lab/media/secteurs-fat12.md).
