# Validation des pistes IWM Macintosh/Lisa — 15 novembre 2026

Le banc compile quarante fichiers C# originaux inchangés : primitives/format et décodeur IWM commun, spécialisations Macintosh/Lisa, encodeurs et format Lisa, définitions Apple, modèles, descriptions, base d’encodage et reconstruction/génération de flux. Leurs SHA-256 correspondent à l’inventaire.

Les 886 cas couvrent les deux identités avec contenu/tag distinct, formats et coordonnées limites, douze clés de tags et clé supplémentaire ignorée, dispositions/taille erronées, durées nulles/extrêmes, métadonnées ignorées et priorités de validations. Les encodages comparent nombre et SHA-256 des cellules binaires, index et intervalles uint32 little-endian.

Les décodages comparent identifiant/nom, confiance, cellule, octets, structures complètes triées, secteurs avec tags, format, contenu, coordonnées, intégrité et champs absents. Les cas comprennent reconstruction NRZI automatique et durées imposées, symboles invalides d’en-tête et de données, checksum/adresse/numéro de données modifiés, champs tronqués, marques non appariées, rotations, préfixes et limites exactes de recherche. Les entrées brutes sans secteur encodable vérifient aussi les résultats incomplets.

La référence produit 826 résultats et 60 refus (40 ArgumentOutOfRangeException et 20 ArgumentException). Les résultats contiennent 1 524 structures d’adresse et 1 500 structures de données, avec 1 524 secteurs : 1 058 valides, 146 invalides et 320 indéterminés. Aucune normalisation d’écart n’est appliquée.

Quatre contrôles supplémentaires vérifient l’annulation initiale des points d’entrée ; deux contrôles passent les identités Macintosh et Lisa par le registre explicite avec une piste à cylindre 192, face 1 et secteur 63 : contenu, tags, format et intégrité sont vérifiés. Les 174 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **1 066 contrôles, 0 différence, 886 cas, 174 textes**.
- Audit : **449 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `ebfe0d3c22446ff3301cd7f310c409601039916c1897877a5438f6af28cd17bd`.

Le banc build/verification-iwm-track et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Ces résultats concernent les codecs de piste ; les politiques de reconnaissance/reconstruction Apple, géométries et services de documents restent distincts.
