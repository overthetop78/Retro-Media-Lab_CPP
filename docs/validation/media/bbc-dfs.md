# Validation BBC DFS — 15 novembre 2026

Les builds de production Debug et Release réussissent après la centralisation du décodeur de catalogue. Le banc exécuté avec les DLL de chaque configuration rapporte **493 324 contrôles, zéro différence** dans chacune. Ce total inclut des contrôles par secteur ; il ne représente pas autant d'images distinctes.

## Comparaison avec les sources C#

- 198 images générées : quatre profils, catalogues connus et structurels, Atom/JFD, deux faces, noms/verrouillage/adresses, ponctuation, fichiers vides, secteurs absents ou courts et refus des alias invalides.
- Comparaison des volumes, entrées, ordre, capacités, espace libre, avertissements, empreintes des contenus et huit identifiants issus du classifieur.
- 72 sondes avec format demandé ; 40 géométries observées et 24 recherches exactes de profil.
- 256 cas de champs compactés et 256 cas de décodage des octets de noms.
- Contrôles des octets et adresses sectorielles : numérotation depuis zéro, face enregistrée zéro, complétude des blocs ; vérification que les valeurs par défaut IBM/MSX restent inchangées.
- Quatre sorties writer comparées octet par octet au C#, puis relues et explorées.

Le projet de référence compile les sources C# originales liées en lecture seule. Les adaptateurs ne fournissent que les contrats et transports nécessaires au banc ; les algorithmes de lecture/écriture ne sont pas réimplémentés dans la référence. Les 13 empreintes de sources concernées restent celles de l'inventaire.

Empreinte SHA-256 du manifeste final : `4b50040c23b40bcc17aba0bbb3edb66a4efbc80685b7a68c4e088a256b72d4e9`.

## Fiabilité et traductions

Le banc couvre annulation, bornes des décodeurs, intégrité invalide, contexte de face séparé, remplissage positionnel des secteurs absents et bloc court. Une écriture incomplète ou annulée préserve la destination et abandonne le fichier temporaire. Le formatage des adresses est contrôlé avec une locale globale qui introduirait sinon des groupements de chiffres ; la restauration de locale utilise RAII.

Les sept nouvelles clés de diagnostic donnent 203 résolutions vérifiées via les bibliothèques de langues réelles (29 langues). Argos a généré les catalogues, suivis d'une relecture et de corrections ; les paramètres et invariants sont contrôlés. L'audit des littéraux passe sur 238 fichiers C++, ainsi que le contrôle des catalogues de tous les domaines médias. `git diff --check` passe.

Le banc et ses médias sont regroupés sous `build/verification-dfs/`, supprimés après utilisation. Aucun test matériel ni ajout au workflow GitHub. Ces résultats ne valident pas les familles restantes, les registres de reconnaissance ni la façade publique, encore à porter.
