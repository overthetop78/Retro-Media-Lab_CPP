# Validation Arburg — 15 novembre 2026

Le banc compile 36 fichiers C# originaux en lecture seule. Leurs SHA-256 correspondent à l’inventaire et restent inchangés en fin de vérification. Empreinte du manifeste : `9c7ed92d48ed574c8b3040581d92adf45d1db0ee0b92f25321745f22a2820b86`.

Les **1 209 cas** comprennent 272 calculs de checksum, 256 encodages système, 395 lectures après encodage système, 40 constructions de bloc avec checksum, seize contrôles de checksum, quatorze reconstructions privées BuildBlock, 109 pistes encodées, 98 flux construits, quatre lectures directes système et cinq suites de cellules. Les transformations élémentaires couvrent les 256 valeurs d’octet. Les sommes couvrent des données jusqu’à 65 536 octets ; les blocs couvrent remplissage, tailles utiles/complètes, checksum fourni remplacé et erreurs de capacité.

Les pistes et flux comparent les deux variantes et leurs ordres physiques DS, SD, DSD et SDS, plusieurs durées de cellule, plusieurs préfixes, les gaps, les checksum corrects/incorrects, les marques tronquées et les cellules altérées. Les attributs système positifs/négatifs non nuls activent bien le système ; l’attribut de piste n’impose pas la variante aux secteurs de données. Les métadonnées ignorées restent ignorées.

La référence retourne 184 secteurs : 136 à intégrité vraie, 24 fausse et 24 indéterminée. Les 184 structures comprennent 96 données et 88 en-têtes système. Les secteurs ont une taille physique mais une charge utile sans checksum. Les comparaisons vérifient toutes les cellules, l’index, tous les intervalles uint32 little-endian, la durée de cellule, la confiance, les octets, les secteurs et toutes leurs métadonnées, puis les structures/types/positions/longueurs/descriptions et leur ordre.

Les 37 refus C# sont comparés par diagnostic : vingt tailles de charge utile, deux cylindres, deux faces, une piste vide, une durée de cellule nulle, une durée d’index nulle, un compte négatif système, huit capacités de bloc et une taille utile négative. Les erreurs d’indexation/allocation pour les arguments invalides deviennent des diagnostics explicites. Trois contrôles C++ supplémentaires vérifient le départ négatif, le départ int64 maximal et le compte int64 maximal sans allocation excessive ; ces bornes n’existaient pas comme telles dans le contrat C#.

Huit contrôles vérifient l’annulation du checksum, de sa validation, de la construction/reconstruction de bloc, des deux transformations système, du décodage et de l’encodage des pistes. Un contrôle passe une piste de données par le registre automatique explicite et compare charge utile et intégrité Checksum. Les 174 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **1 395 contrôles, 0 différence inattendue, 1 209 cas, 174 textes**.
- Audit des littéraux : **520 fichiers C++, 29 langues**, réussi.
- `git diff --check` : réussi.

Le banc `build/verification-arburg/` et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. La validation porte sur le codec ; le portage global reste en cours.
