# Validation du codec NorthStar — 15 novembre 2026

Le banc compile trente-cinq fichiers C# originaux inchangés : checksum rotatif, encodeur/décodeur/format NorthStar et primitives/modèles communs. Leurs SHA-256 correspondent à l’inventaire. L’empreinte du manifeste est `43dbec3de8c05e4f2065d8ebe7d1a31b322cd0ae0d2a2e0c7f69a9acacd0ff8e`.

Les 1 331 cas comprennent 521 checksums, 305 adresses compactes (dont limites int32), 264 identités, huit blocs directs, 25 décodages de pistes, soixante encodages et 148 lectures par fenêtres de secteurs matériels. Les tests comparent les octets de cellules, index et intervalles uint32 little-endian, puis identité, confiance, durée de cellule, octets décodés, secteurs et toutes leurs métadonnées, structures, positions, longueurs et descriptions.

Les pistes vérifient tailles, coordonnées physiques et compactes, ordres de validation, métadonnées ignorées, ordre des secteurs, durées, troncatures et corruption de marque, horloges, adresse, données/checksum et marque interne. Les résultats de piste comprennent 53 secteurs : 44 checksums valides, quatre invalides et cinq indéterminés ; 56 structures sont reconnues. Six lectures directes de bloc réussissent et deux sont absentes.

Pour chaque branche FM/MFM, les fenêtres produisent 42 résultats sans exception et quatre-vingts secteurs, dont 34 valides et 46 invalides. Les cas couvrent un, deux et huit secteurs, adresses jusqu’à 255, nombre nul/négatif, troncatures, index et choix premier valide/dernier lisible. Les coordonnées hors octet ne sont rejetées que lorsqu’un candidat existe, conformément au C#. Les calculs 64 bits de bornes extrêmes constituent une sécurité documentée ; aucun débordement int32 volontairement non contrôlé n’est reproduit.

La référence produit 1 228 résultats et 103 refus : 64 OverflowException, 34 ArgumentOutOfRangeException, cinq ArgumentException. Les refus C++ sont comparés sans exiger la classe .NET ou un texte anglais brut. Six contrôles supplémentaires vérifient l’annulation des points d’entrée, et un vérifie le passage par le registre automatique explicite. Les 203 textes des DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **1 541 contrôles, 0 différence, 1 331 cas, 203 textes**.
- Audit : **476 fichiers C++, 29 langues**, réussi.

Le banc build/verification-northstar et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Ces vérifications concernent les primitives et codecs de piste ; catalogue complet et politiques de reconstruction restent distincts.
