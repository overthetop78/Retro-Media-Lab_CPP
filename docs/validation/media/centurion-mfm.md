# Validation Centurion MFM — 15 novembre 2026

Le banc compile 36 fichiers C# originaux en lecture seule. Les SHA-256 correspondent à l’inventaire et restent inchangés en fin de validation. Empreinte du manifeste : `607903f6c9226ba0c1531a014ff9aa18a02e45cc77bb562db211bdaed3598537`.

Les **612 cas** comprennent 128 constructions d’en-tête, 57 constructions de champ de données par invocation des méthodes privées originales, 61 encodages complets, 361 flux construits et cinq suites de cellules. Ils couvrent les blocs vides et partiels, les frontières 255/256/257 et 511/512/513, les tailles maximales 65 279/65 280 et le refus à 65 281 octets, les identités limites, les durées, plusieurs secteurs et les métadonnées ignorées.

Les flux construits croisent CRC vrais/faux, données absentes, clés 0/1/2/127/255, longueurs déclarées de zéro à 65 535, charges utiles plus courtes, préfixes, troncatures, mutations de cellules, distances de recherche autour de 400 et présence d’un autre en-tête avant les données. Le résultat C# produit 407 secteurs : 81 à intégrité vraie, 137 fausse et 189 indéterminée. Les 1 599 structures comprennent 412 en-têtes et 1 187 données, dans leur ordre original.

La comparaison porte sur toutes les cellules, l’index, tous les intervalles uint32 little-endian, la durée de cellule, la confiance, les octets, les secteurs et toutes leurs métadonnées, les structures/types/positions/longueurs/descriptions. Les 13 refus sont comparés par diagnostic : six limites Centurion, deux cylindres, deux faces, une piste vide, une durée de cellule nulle et une durée d’index nulle. Une correction du banc a été nécessaire pour reconnaître le libellé C# « Index duration » ; aucune correction de production n’a été nécessaire après cette comparaison.

Quatre contrôles d’annulation couvrent construction d’en-tête, champ de données, encodage et décodage. Le registre explicite est contrôlé, puis son décodage automatique vérifie cylindre 255, secteur 254, face logique zéro, intégrité et complément d’une charge de 257 à 512 octets. Les 203 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **821 contrôles, 0 différence, 612 cas, 203 textes**.
- Audit des littéraux : **509 fichiers C++, 29 langues**, réussi.
- `git diff --check` : réussi.

Le banc `build/verification-centurion/` et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Cette validation concerne le codec ; elle ne déclare pas terminé le portage global.
