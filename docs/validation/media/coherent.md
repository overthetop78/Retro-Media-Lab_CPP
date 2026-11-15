# Validation COHERENT — 15 novembre 2026

Les quinze fichiers C# originaux ont été compilés sans modification avec des adaptateurs minimaux de modèles/interfaces. Les quinze empreintes sont identiques à l’inventaire. Les images sont synthétiques et temporaires.

## Comparaison C# / C++

2 462 cas : 1 029 entiers canoniques, 773 pointeurs 24 bits, 319 scénarios d’images, 256 noms ASCII, 64 modes, 16 validations de superblocs et cinq dates. Les codecs parcourent toutes les valeurs d’octet à chaque position, les longueurs insuffisantes et les frontières uint32.

Les 247 volumes lisibles sont comparés en profondeur : identifiant, nom/capacité/espace libre, dates, ordre des entrées, types, tailles, commentaires/attributs/références, validité, SHA-256 du contenu, arborescence et diagnostics. Les scénarios restants vérifient les erreurs attendues. Couverture des zones d’inodes, entrées hors zone, secteurs absents/courts, indicateurs d’intégrité ignorés comme dans la source, cycles et références répétées, types inconnus, trous, fichiers utilisant les trois niveaux indirects et pointeurs canoniques hors plage.

Les diagnostics codés C++ sont projetés vers les libellés C# pour comparer paramètres, ordre et causes imbriquées. Les traductions sont vérifiées séparément : 18 clés × 29 langues, soit 522 textes chargés depuis les DLL installées et comparés aux catalogues relus.

Des contrôles propres au portage vérifient l’annulation, les paramètres de commentaire, la profondeur maximale sur 70 répertoires imbriqués et l’accès paresseux en cache : aucun accès au secteur inutilisé et une seule lecture par bloc utile.

## Résultats

- Compilation complète et déploiement Debug et Release réussis avec MSVC x64/Qt 6.12.
- Par configuration : **7 912 contrôles, aucune différence, 247 volumes lisibles et 522 textes traduits**.
- Audit des littéraux : 284 fichiers C++ et 29 langues ; `git diff --check` passe.
- SHA-256 du manifeste de résultats C# : `a1edd4547286e9cc2f37f53b8fc03adb35dc5efe94a15137343344dbcee3ec63`.

Le banc `build/verification-coherent/`, les fixtures, références, projets et fichiers compilés sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout aux workflows GitHub. Le conteneur brut et les codecs Commodore 900 ne sont pas couverts par ces résultats.
