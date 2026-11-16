# Progression des écritures

Référence C# en lecture seule : `Images/Writing/MediaImageWritingService.cs`, particulièrement `ReportInputUnitsAsync` et `ReadElement`.

La surcharge d'écriture avec callback prépare les unités avant le writer, après l'ouverture des activités mécaniques de source/destination. Elle conserve les valeurs 60–95 et le signal final pour un fournisseur absent ou un descripteur vide. Les couples piste/face sont fournis par les descripteurs de visualisation. Les sorties du writer et les nettoyages existants restent contrôlés.

Le détail de progression peut contenir une recette `MessageInfo`. Les messages de groupes de pistes et de piste courante réutilisent les clés `RepresentationConvertingTracks` et `RepresentationConvertingTrack`, déjà traduites. Le libellé technique `T<cylindre>.<face>` reste disponible ; aucun texte de présentation brut n'a été ajouté.

Pour les secteurs, les adresses physiques sont indexées une fois puis les blocs de chaque piste sont observés. Cela évite le parcours C# de tous les blocs pour chaque piste, tout en conservant leur ordre au sein d'une piste. Pour le flux, la recherche exploite l'ordre physique validé des pistes. Comme en C#, les autres représentations produisent leur progression sans lecture de contenu supplémentaire. En l'absence de callback, aucun descripteur ni index n'est construit.

La progression publique est isolée dans `MediaExplorationProgress.h`, sans dépendance au résultat disque. L'ancienne surcharge d'écriture reste disponible. La nouvelle surcharge prend explicitement callback et jeton pour éviter une ambiguïté avec les appels existants dont le quatrième argument est `{}`.

Validation : compilation Debug, audit des littéraux (1 553 sources), contrôle ciblé avec writer injecté : notifications avant le writer, valeurs et recettes traduisibles, un seul parcours des adresses et une lecture par bloc disponible, absence de parcours sans callback, descripteur vide, flux, annulation pendant le parcours et exception du callback avant écriture. Tous ces contrôles passent. Le writer de vérification n'a créé aucun fichier média ; le banc temporaire a été supprimé.
