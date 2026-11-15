# Validation du codec Data General 2F — 15 novembre 2026

Le banc compile trente-six fichiers C# originaux inchangés. Leurs SHA-256 correspondent à l’inventaire. L’empreinte du manifeste est `937a6d63080529b2c9c84290720936541f1b926df7bf9b79543ffcdb896c020e`.

Les 1 327 cas comprennent 777 checksums, 307 encodages et 243 décodages de flux. Les checksums couvrent tous les octets isolés, des séquences uniformes/déterministes et les longueurs zéro, un, deux, trois, quatre, 511, 512, 513 et 4 096, avec l’itération terminale nulle source.

Les pistes encodent les 128 cylindres et deux faces, contrôlent tailles, numéros, durées et coordonnées hors plage, attributs/métadonnées ignorés et secteurs multiples dans l’ordre fourni. Toutes les cellules et tous les intervalles uint32 little-endian ainsi que l’index sont comparés. Les décodages vérifient la cellule estimée, confiance, octets, secteurs et toutes leurs métadonnées, structures/type/position/longueur/description et ordre.

Les flux construits vérifient les distances 31, 32, 33, 255, 256 et 257 cellules entre fin de synchronisation d’en-tête et synchronisation des données, cylindres/faces masqués, deux bits sectoriels bas ignorés, numéros supérieurs à sept rejetés, checksums faux, bits d’horloge et de données altérés, troncatures, synchronisations seules/adjacentes/internes et concaténation de pistes. Les synchronisations des données reconnues et celles de leur charge utile sont sautées après une paire complète, conformément au C#.

La référence produit 1 290 résultats et 37 refus : 32 ArgumentOutOfRangeException et cinq ArgumentException. Chaque refus est comparé au diagnostic C++ attendu, permettant de vérifier les priorités cylindre/face communes puis format, taille et numéro. Les résultats comprennent 395 secteurs (379 valides, seize invalides), 395 structures d’en-tête et 395 structures de données.

Trois contrôles supplémentaires vérifient l’annulation des points d’entrée ; un vérifie le registre automatique explicite sur cylindre 127, face un, secteur sept et données intactes. Les 116 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **1 447 contrôles, 0 différence, 1 327 cas, 116 textes**.
- Audit : **488 fichiers C++, 29 langues**, réussi.

Le banc build/verification-datageneral et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Le catalogue complet et les politiques de reconstruction restent à porter.
