# Validation du codec Micropolis MFM — 15 novembre 2026

Le banc compile trente-quatre sources C# originales inchangées. Leurs SHA-256 correspondent à l’inventaire. Le manifeste possède l’empreinte `9e3179337a8175c2d8c8b1e799124c66cfb142480754c9fb2f2fa3d89be4ac51`.

Les 873 cas comprennent 528 calculs de checksum (seuils 255/510, séquences uniformes et déterministes), 202 créations de records, cinq parses, huit lectures directes, 48 décodages de cellules et 82 encodages. Les créations couvrent zéro à 262 octets, dépassements, limites de coordonnées et mutations de chaque région : identité, réserves, charge utile, checksum et trailer.

Les pistes vérifient les deux modes de sortie, coordonnées invalides, durées, tailles/numéros sectoriels, métadonnées ignorées, pistes multiples dans l’ordre fourni, troncatures, marque altérée, bits d’horloge modifiés et synchronisation interne. Toutes les cellules et tous les intervalles de flux uint32 little-endian sont comparés, ainsi que l’index. Le décodeur est comparé sur confiance, durée de cellule, octets, secteurs et toutes leurs métadonnées, structures/positions/longueurs/descriptions.

La référence retourne 793 résultats et 80 refus (43 ArgumentOutOfRangeException et 37 ArgumentException). Les secteurs décodés sont au nombre de 77, avec cinquante checksums valides et 27 invalides, et les structures au nombre de 87. Les erreurs C++ sont comparées comme refus, sans exiger le texte brut ni la classe .NET ; le dépassement du tampon de création devient explicitement une erreur de domaine traduisible.

Six contrôles vérifient l’annulation des points d’entrée. Deux passent chaque mode de sortie par le registre automatique explicite et vérifient taille, face logique, cylindre et octets utiles. Les 145 textes des DLL de 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **1 026 contrôles, 0 différence, 873 cas, 145 textes**.
- Audit : **468 fichiers C++, 29 langues**, réussi.

Le banc build/verification-micropolis et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Le contrat de propriété du tampon et la validation explicite de sa capacité sont documentés dans la référence ; le catalogue complet et les politiques de reconstruction restent à porter.
