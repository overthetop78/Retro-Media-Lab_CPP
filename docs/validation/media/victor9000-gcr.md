# Validation du codec Victor 9000 GCR — 15 novembre 2026

Le banc compile trente-cinq fichiers C# originaux inchangés : encodeur/décodeur/format Victor (sommes et modèles inclus), GCR Commodore, constantes, modèles, descriptions, base d’encodage et reconstruction/génération des révolutions. Leurs SHA-256 correspondent à l’inventaire.

Les 454 cas comprennent une matrice des 65 536 paires cylindre/secteur pour la somme d’adresse (empreinte comparée), des sommes 16 bits avec débordements et sérialisation faible/fort, des marqueurs courts/limites, blocs avec cible préexistante, en-têtes invalides, préfixes de données variés et checksums faux. Les recherches croisent bornes, position initiale négative, offsets de champs et filtrage des positions appariées.

Les pistes couvrent motifs déterministes, faces, coordonnées/numéros limites, tailles incorrectes, durées nulles/extrêmes, métadonnées ignorées et priorités de validation. Nombre et SHA-256 des cellules, index et intervalles uint32 little-endian sont comparés. Le décodage compare tous les secteurs, données, intégrités et métadonnées absentes, ainsi que confiance, cellule et structures complètes dans leur ordre source. Les mutations de cellules, coupures, préfixes et insertions testent la reconnaissance et les limites de recherche des données.

La référence produit 423 résultats et 31 refus : 22 ArgumentException et neuf ArgumentOutOfRangeException. Les lectures directes produisent 156 en-têtes et 34 blocs de données. Les pistes reconnues comprennent 333 secteurs (296 valides, neuf invalides, 28 indéterminés), 333 structures d’en-tête et 352 structures de données. Aucun écart ni normalisation n’est nécessaire.

Huit contrôles supplémentaires vérifient l’annulation des fonctions et helpers ; un contrôle passe une piste par le registre automatique explicite et vérifie contenu et intégrité du secteur. Les 203 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **666 contrôles, 0 différence, 454 cas, 203 textes**.
- Audit : **454 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste : `91e9e8d51bf219eb167f166f7a383e665b8f495eeedb4b167ab2c0f25f687a56`.

Le banc build/verification-victor9000 et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Ces résultats concernent les champs et codecs de piste ; les politiques de reconstruction Victor et les services de documents restent distincts.
