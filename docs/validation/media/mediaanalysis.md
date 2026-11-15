# MediaAnalysis — validation du portage

Date : 15 novembre 2026. Ce compte rendu concerne uniquement MediaAnalysis, pas l’ensemble du portage MediaEngine/MediaFileSystems.

## Composant livré

- `lib/MediaAnalysis.dll`, construite et installée en Debug et Release ; API C++20 dans `include/RetroMediaLab/MediaAnalysis`, sans types Qt exposés.
- QtCore est une dépendance privée utilisée pour le décodage UTF-8, les espaces Unicode et les conversions de casse ; aucun GUI ni accès matériel, aucune API Windows dans ce composant.
- Neuf enums, contrats de signature/règle/résultat, recherche et classification ; contenu absent distinct d’un fichier vide ; métadonnées conservées dans le contrat d’entrée comme en C# (le classifieur C# courant ne les utilise pas).
- 16 catalogues, 531 règles et 72 signatures : ordre, familles, priorités, extensions, groupes d’octets, positions et longueurs comparés intégralement au C#.
- 25 clés de présentation ; 24 textes par langue et un invariant ROM résolu depuis Constants ; domaine MediaAnalysis dans les DLL de langues existantes.

## Comparaison exécutée

Le banc temporaire compile directement les 37 fichiers C# originaux par liens en lecture seule, avec .NET 10, et appelle la DLL C++ construite avec MSVC. Les deux programmes reçoivent les mêmes cas. Les structures complètes retournées sont comparées : famille, extension normalisée, catégorie, clé, exécution, icône, format, encodage, aperçu et indices des règles trouvées par priorité.

- 12062 cas comparés : 1 catalogue complet, 8061 classifications/recherches/normalisations et 4000 tests directs de signatures. Résultat : **aucune différence**.

Cas couverts : toutes les règles avec contenu construit, absent, vide, tronqué, inversé et extension mise en majuscules/entourée d’espaces ; familles voisines ; dossiers/liens/fichiers/inconnus ; métadonnées natives ; seuil texte et limite de 512 octets ; caractères latins et cas Unicode sélectionnés ; positions fixes et recherches ordonnées dans les deux sens, groupes vides, positions invalides et position maximale int32. Générateur aléatoire déterministe pour les signatures (graine 74193).

Les comparaisons ont détecté 448 écarts dus uniquement à la normalisation de U+0130 ; ils ont été corrigés avant la comparaison finale étendue. La parité ne prétend pas prouver tous les comportements possibles de toutes les versions Unicode ; les cas effectivement exécutés sont décrits ci-dessus.

203 résolutions de clés supplémentaires exécutées via TranslationService : sept clés représentatives pour chacune des 29 langues, y compris le repli invariant ROM. Structure, présence et paramètres des 25 clés vérifiés pour tous les catalogues ; corrections de libellés documentées dans la référence C#.

## Reproductibilité et nettoyage

- SHA-256 du fichier temporaire `cases.json` : `376d84380673dd075bdadabc5705cdb3353ab9ad98fbb1b35e6753be821e1313`.
- SHA-256 du fichier temporaire `csharp.json` : `ae66bc16542305973df5fd72b4c29411bd0c507eee65deed52313ac3bb3a647d`.
- SHA-256 du fichier temporaire `cpp.json` : `ee5a9dda95fd426ff238410b788f66b4426b569ff4dd6371c910f2ec4dde5bd8`.

Le banc et ses artefacts sont supprimés après validation conformément à config.toml. Aucun média externe, fichier volumineux permanent, accès matériel ou changement de workflow GitHub. Les sources C# ont été vérifiées identiques à leurs empreintes d’inventaire. Audit des constantes/catalogues et git diff --check exécutés.

## Suite de l’objectif

MediaEngine (1 083 sources C#) et MediaFileSystems (372 sources C#) restent à porter. La réussite de ce bloc ne valide ni l’exploration d’un média, ni sa conversion, ni ses opérations matérielles.
