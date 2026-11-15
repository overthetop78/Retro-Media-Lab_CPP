# Validation écriture SCP — 15 novembre 2026

## Référence

Le banc compile 28 sources C# originales, dont `ScpWriter`, `ScpReader` et `ScpEncodedTrackFluxService`, sans modifier le dépôt C#. Leurs SHA-256 correspondent à l’inventaire. Les dépendances de composition non exercées sont des contrats de banc minimaux ; les algorithmes comparés sont originaux.

3 765 cas comparent les octets du fichier par SHA-256 et, pour les sorties de moins de 100 000 octets, intégralement. La lecture C# et C++ des résultats compare drapeaux/checksum, taille, ordre et adresses des pistes, index, nombres de mots et intervalles. Les rejets sont comparés par catégorie d’exception C#.

La couverture comprend les 256 valeurs des drapeaux, largeur, têtes, résolution, version et type de disque ; bornes de révolution et de pistes ; doublons et géométrie incohérente ; pistes absentes, désordonnées et vides ; intervalles proches de 65 536 et uint32, reports successifs, dépassement du report et intervalle nul. Une piste dépasse la taille du bloc de sortie. Les métadonnées de checksum, origine et nombre déclaré sont volontairement incohérentes pour vérifier qu’elles sont recalculées ou ignorées comme dans la source.

Les conversions synthétiques couvrent toutes les résolutions SCP, les arrondis, les instants confondus, les intervalles nuls, la liste vide, uint32 et les cumuls qui dépassent sa plage. Les adresses couvrent les bornes int32, les valeurs négatives et les débordements intermédiaires du calcul cylindre × 2 + face.

## Contrôles C++ supplémentaires

- Sortie limitée à 65 536 octets par appel.
- Surcharge fichier comparée aux octets C# ; destination sentinelle conservée sur validation invalide et annulation préalable.
- Publication atomique avec le writer SCP comme producteur : annulation et exception injectées après la première émission ; destination inchangée et liste des fichiers temporaires identique avant/après.
- Absence de rappel et annulation de conversion synthétique : diagnostics codés.
- Une piste qui nécessiterait plusieurs milliards de mots est rejetée par le plan de taille avant tout appel de sortie et sans allocation du tableau de mots.
- **16 allers-retours** encodeur GCR Commodore 900 → révolution synthétique → writer SCP → lecteur SCP → reconstruction sectorielle, sur les frontières de toutes les zones et les deux faces : checksum, octets de chaque secteur, intégrité et provenance vérifiés. Ces contrôles ne sont pas présentés comme des mesures matérielles.
- Six clés × 29 langues, soit **174 textes**, chargées depuis les DLL installées et comparées aux catalogues relus.

## Résultats

- Construction complète et déploiement Debug et Release avec MSVC x64/Qt 6.12.
- Dans chaque configuration : **4 197 contrôles, aucune différence**, 3 004 cas acceptés et 761 rejetés.
- Audit des littéraux : 324 fichiers C++, 29 langues ; diagnostics et placeholders contrôlés.
- SHA-256 des résultats C# : `ef1b6ad6b1642b55a4fc1585d58a04ad3e0670e992f2c0d9d77172713316f3ac`.

Le banc `build/verification-scp-writer/`, son fichier sentinelle/de sortie et ses journaux temporaires sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. Les sorties réellement supérieures à plusieurs gigaoctets ne sont pas fabriquées ; les tests de dépassement restent bornés.
