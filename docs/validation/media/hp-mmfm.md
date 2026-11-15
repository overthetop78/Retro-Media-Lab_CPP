# Validation du codec HP MMFM — 15 novembre 2026

Le banc compile trente-sept fichiers C# originaux inchangés. Leurs SHA-256 correspondent à l’inventaire. L’empreinte du manifeste est `b11ce91253f6d2d781da591c262f7337dff173c4d1dc4bdf0b7d74d33a02aeaf`.

Les 1 086 cas comprennent 325 encodages de piste, 270 transformations encodePayload et 270 decodePayload, 121 flux construits avec les primitives originales, soixante lectures directes d’octets, treize recherches de synchronisation, douze lectures d’en-tête, douze lectures de données et trois décodages de cellules. Les transformations couvrent les 256 valeurs d’octet à plusieurs positions et des longueurs paires/impaires de zéro à 1 024. Les deux méthodes source sont comparées séparément à la transformation C++ partagée.

Les pistes testent les limites de coordonnées, numéros, tailles et durées, les 256 charges utiles uniformes, plusieurs secteurs dans l’ordre fourni et les métadonnées ignorées. Les générateurs adverses emploient les helpers C# originaux pour l’identité, la permutation, le CRC et les cellules MFM. Les CRC d’en-tête et de données vrais/faux sont croisés avec les données absentes ou tronquées. Les positions de début de synchronisation 159, 160, 161, 927, 928 et 929 vérifient les deux bornes réelles de recherche. Les en-têtes répétés partagent une synchronisation de données ; sa consommation unique est comparée, y compris lorsque le bloc est tronqué. Des marques insérées, cellules altérées et préfixes contrôlent la progression et l’absence de tri des structures.

Toutes les cellules, l’index et tous les intervalles uint32 little-endian sont comparés, puis durée de cellule, confiance, octets, secteurs et toutes leurs métadonnées, structures/types/positions/longueurs/descriptions et ordre de production. Les helpers directs vérifient les troncatures, départs négatifs et lectures vides, ainsi que le refus des comptes négatifs.

La référence produit 1 028 résultats et 58 refus : 24 ArgumentException, 22 ArgumentOutOfRangeException et douze OverflowException. Les diagnostics sont comparés, dont les douze erreurs de permutation impaire et douze comptes négatifs traduits en diagnostic de compte d’octets. Les pistes produisent 401 secteurs : 311 à intégrité vraie, 62 fausse et 28 indéterminée ; les 741 structures comprennent 401 en-têtes et 340 données. Aucun écart ni normalisation n’est nécessaire.

Huit contrôles vérifient l’annulation des points d’entrée. Un contrôle passe une piste de cylindre 255/face un/secteur 127 par le registre automatique explicite et vérifie adresse, contenu et intégrité. Les 145 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **1 240 contrôles, 0 différence, 1 086 cas, 145 textes**.
- Audit : **504 fichiers C++, 29 langues**, réussi.

Le banc build/verification-hp-mmfm et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Les politiques de reconstruction et le catalogue complet restent à porter séparément.
