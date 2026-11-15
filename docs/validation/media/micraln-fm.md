# Validation du codec Micral N FM — 15 novembre 2026

Le banc compile trente-quatre fichiers C# originaux inchangés : encodeur/décodeur/format Micral N (checksum et modèle inclus), fonctions de bits, modèles, descriptions, base d’encodage et reconstruction/génération du flux. Leurs SHA-256 correspondent à l’inventaire.

Les 478 cas comprennent la matrice exhaustive des 65 536 couples somme/donnée pour Update (empreinte comparée), séquences de checksum à valeurs limites et pseudoaléatoires déterministes, pistes à coordonnées/numéros limites, tailles et durées incorrectes, métadonnées ignorées et priorités de validation. Nombre et SHA-256 des cellules, index et intervalles uint32 little-endian sont comparés.

Les lectures directes de blocs vérifient données, adresse, checksum enregistré et validité, avec offsets, horloges modifiées et coupures autour des frontières. Les pistes comparent tous les secteurs et champs absents, suite de données, confiance, cellule et structures complètes. Des charges utiles contenant le préambule vérifient le balayage intérieur du bloc selon le C#.

La référence produit 462 résultats et 16 refus (neuf ArgumentOutOfRangeException et sept ArgumentException). Les lectures directes retournent 156 blocs valides, douze invalides et 22 absences. Les pistes comprennent 237 secteurs (219 valides et 18 invalides) et 271 structures, dont 34 tronquées. Aucun écart ni normalisation n’est nécessaire.

Quatre contrôles supplémentaires vérifient l’annulation des fonctions et helpers ; un contrôle passe une piste par le registre automatique explicite et vérifie contenu et intégrité du secteur. Les 116 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **599 contrôles, 0 différence, 478 cas, 116 textes**.
- Audit : **459 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste : `7f7f4748bad89931310aa39ecae2b188f150538922eb4f655b03ecb0f10226fc`.

Le banc build/verification-micraln et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Ces résultats concernent le codec de piste ; les politiques de reconstruction et le catalogue complet restent distincts.
