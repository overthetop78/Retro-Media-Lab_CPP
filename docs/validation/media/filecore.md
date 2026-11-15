# Validation FileCore — 15 novembre 2026

Builds de production et bancs **Debug et Release réussis**, chacun avec **8 594 contrôles, zéro différence**. Le banc compile directement les onze sources C# FileCore, leur interface et le codec de noms ADFS depuis le dépôt source en lecture seule ; les adaptateurs fournissent uniquement une image sectorielle mémoire et ses blocs.

## Périmètre comparé

Les 4 245 cas de référence comprennent :

- 2 121 lectures de champs de bits, plages valides et invalides, masques de différentes largeurs ; 800 recherches de bits positionnés.
- 483 décalages signés, incluant décalages invalides et dépassements de capacité Int32.
- 400 DiscRecords valides ou altérés, tailles tronquées et capacités insuffisantes ; 256 séquences de noms couvrant les 256 valeurs d'octet et les deux codecs ; neuf adresses indirectes.
- 56 anciennes cartes avec tailles de champs variées, capacités, options d'entrelacement et sondes d'adresses/offsets. Les noms, racines, espace libre et listes de positions résolues sont comparés.
- 120 images de nouvelles cartes, avec une à quatre zones et trois tailles de partage. Les scénarios comprennent les deux copies valides, première ou seconde copie corrompue, deux copies invalides, terminateur manquant, contrôle XOR invalide, DiscRecord invalide, taille sectorielle incompatible, fichier tronqué et bloc d'amorçage incorrect. Parmi elles, 39 cartes sont acceptées ; les quatre nombres de zones ont des cas positifs. Les adresses résolues, le nom issu de la carte, la racine et l'espace libre sont comparés.

Les listes de positions sont comparées intégralement ; les totaux de contrôles comptent une comparaison de liste comme un contrôle, pas comme des images supplémentaires. Les exceptions C# sont comparées comme des échecs ; les exceptions C++ doivent porter une clé de diagnostic.

## Contrôles C++ complémentaires

Sept champs de 32 bits non alignés sont lus correctement, alors que la fenêtre fixe C# ne peut les accueillir. Les cartes restent utilisables après destruction de l'image source. Annulation des anciennes/nouvelles cartes, bornes des offsets extrêmes et validation des décalages d'un DiscRecord construit directement sont vérifiées.

Les 87 textes FileCore (trois clés dans 29 langues) sont chargés depuis les DLL installées et comparés aux catalogues relus. Argos a servi à leur génération, puis les traductions incorrectes ou laissées en anglais ont été corrigées, notamment les formulations de dépassement numérique et le chinois traditionnel. Aucun modèle n'a été installé. L'audit des littéraux passe sur 243 fichiers C++ ; `git diff --check` passe.

SHA-256 du manifeste de résultats C# : `703cb8977e08a597fe23cfd4e92dba71592a812b7534fa9685dfd7ccece1c0b2`.

Le banc, les 120 petites images synthétiques, les fichiers de référence et leurs artefacts étaient regroupés dans `build/verification-filecore/`, puis supprimés après validation. Aucun fichier matériel utilisateur ni modification du workflow GitHub. Les 13 empreintes C# contrôlées restent identiques à l'inventaire.

Ces vérifications couvrent FileCore ; elles ne prouvent pas le portage du parcours ADFS, de ses conteneurs ou de la façade applicative, qui restent à faire.
