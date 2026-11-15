# Validation lecture SCP — 15 novembre 2026

## Comparaison

Le banc compile 47 sources C# originales, sans modification du dépôt C#, et vérifie leurs empreintes contre l’inventaire. Les dépendances de composition non exercées sont des contrats de banc minimaux, explicitement séparés dans `Contracts.cs`. Les valeurs attendues de parsing, fenêtres et reconstruction viennent des algorithmes originaux.

**2 781 cas** : en-têtes/signatures, champs d’en-tête parcourus sur les 256 valeurs, limites de pistes/révolutions/largeurs/têtes/résolution, fichiers tronqués, offsets et tailles proches des limites int32/uint32, mots de flux big endian, marqueurs zéro successifs jusqu’au débordement, checksum Writable/incorrect, fenêtres vides/simples/raccordées et débordement d’index tardif. Les listes d’intégrité explorent les combinaisons vrai/inconnu/faux jusqu’à cinq candidats.

La reconstruction couvre les frontières de zones Commodore 900, les deux faces, secteurs valides et hors limites, cylindres incohérents, pistes ignorées, secteurs répétés, checksums faux puis vrais et égalités conservant le premier. Une fixture effectue le trajet conteneur SCP complet → parsing → fenêtres → GCR → image sectorielle.

Les champs d’en-tête, octets de drapeaux, tailles, intervalles, origines, numéros de révolutions, états de checksum, blocs, adresses, charges utiles, intégrité, provenance et progression sont comparés. Les données sont vérifiées par SHA-256. Les valeurs finies de durée/RPM utilisent une tolérance relative de 1e-12 ; les infinis sont conservés. Les rejets sont comparés par catégorie d’exception C#, pas par texte anglais.

223 lectures valides sont rejouées depuis une source qui refuse toute lecture hors limites ou supérieure à 65 536 octets. S’ajoutent reconnaissance à la limite de seize octets, résolution invalide, annulation préalable de lecture/fenêtres/reconstruction, annulation déclenchée pendant la lecture et depuis le rappel de progression avant la seconde piste. Les dix-sept textes des 29 langues sont chargés depuis les DLL installées et comparés aux catalogues relus, soit **493 textes**.

## Résultats

- Compilation complète et déploiement Debug et Release avec MSVC x64/Qt 6.12.
- Dans chaque configuration : **3 733 contrôles, aucune différence**, 1 043 cas acceptés et 1 738 rejetés.
- Audit des littéraux : 321 fichiers C++, 29 langues ; paramètres/invariants de diagnostics vérifiés, y compris les clés MediaEngine nouvellement couvertes par le contrôle des déclarations.
- SHA-256 du manifeste C# : `835c9e1d8d10afe2421fefe3b1b49a0547935a79ac0ef3964db97c72f6725fc7`.

Le banc `build/verification-scp-reader/` et ses journaux temporaires sont supprimés après validation. Aucun média utilisateur ni test matériel, aucun ajout au workflow GitHub. Ces contrôles ne valident pas encore les autres codecs SCP, l’écriture du conteneur ou les registres/documents applicatifs.
