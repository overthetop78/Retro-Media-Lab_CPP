# Validation du codec E-mu FM — 15 novembre 2026

Le banc compile trente-six fichiers C# originaux inchangés : encodeur/décodeur/format/descriptions E-mu et identité, CRC IBM commun, fonctions de bits, modèles, base d’encodage et reconstruction/génération des révolutions. Leurs SHA-256 correspondent à l’inventaire.

Les 473 cas comprennent l’inversion des 256 octets, toutes les combinaisons des 128 cylindres et deux faces, motifs déterministes et uniformes, numéros de secteurs ignorés jusqu’aux limites int32, tailles et durées incorrectes, métadonnées ignorées et priorités de validation. Nombre et SHA-256 des cellules, index et intervalles uint32 little-endian sont comparés.

Les corruptions touchent marque, données/horloges de l’adresse, charge utile et CRC. Les lectures directes testent les 32 positions du premier octet, offsets et troncatures. Les limites de recherche des données, préfixes et marques brutes vérifient aussi la classification. Des données commençant par un en-tête à CRC nul sont comparées sans supprimer la reconnaissance supplémentaire source.

Les résultats comparent tous les secteurs et métadonnées absentes, données, confiance, cellule, types/positions/longueurs/descriptions et ordre stable des structures. La référence produit 456 résultats et 17 refus (dix ArgumentOutOfRangeException et sept ArgumentException). Les lectures directes retournent 29 blocs à CRC valide, quinze invalides et 22 absences. Les pistes comprennent 362 secteurs (349 valides, cinq invalides, huit indéterminés), 407 structures d’en-tête et 354 structures de données. Aucun écart ni normalisation n’est nécessaire.

Trois contrôles supplémentaires vérifient l’annulation des fonctions et helpers ; un contrôle passe une piste à cylindre 127/face 1 et numéro fourni -1 par le registre automatique explicite, puis vérifie adresse décodée, numéro logique un, contenu et intégrité. Les 145 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **622 contrôles, 0 différence, 473 cas, 145 textes**.
- Audit : **463 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste : `dc5385d9e4952ab197f17623f7c52e3d7e45a56883fb0a5752305bf498274db5`.

Le banc build/verification-emu et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Ces résultats concernent le codec de piste ; les politiques de reconstruction et le catalogue complet restent distincts.
