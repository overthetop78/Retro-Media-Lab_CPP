# Validation des pistes RX01/RX02 — 15 novembre 2026

Le banc compile quarante et un fichiers C# originaux inchangés. Leurs SHA-256 correspondent à l’inventaire. L’empreinte du manifeste est `5c40ae8e443d80c1eb18142f9e3eb8e77f3aaf7c57a96a04211007532d956ced`.

Les 929 cas comprennent 604 encodages de piste, 322 flux construits avec les primitives originales et trois décodages de cellules brutes. Les pistes couvrent tailles 128/256, les 256 valeurs d’octet en charges utiles uniformes, secteurs normaux/supprimés, coordonnées extrêmes, mélange des formats dans l’ordre fourni, codes de taille explicites, métadonnées ignorées et durées. Les générateurs de flux adverses utilisent les helpers de marques, CRC, MFM et transformation M²FM des sources originales, sans les remplacer par des implémentations de test.

Les flux construits vérifient les six marques F8–FD (dont alternatives FA/FC), CRC d’en-tête et données vrais/faux, incompatibilité de code de taille, absence de données et troncatures des champs. Les distances de recherche 1 663, 1 664 et 1 665 cellules testent la borne inclusive. Les altérations touchent marque, horloges, adresse, CRC, cellule de phase et charge utile ; les données peuvent être forcées dans l’autre encodage. Des en-têtes répétés, marques insérées et préfixes de un, sept, seize et 31 cellules contrôlent aussi appairage et progression du balayage.

Toutes les cellules, l’index et tous les intervalles uint32 little-endian sont comparés, puis durée de cellule, confiance, octets décodés, secteurs et l’ensemble de leurs métadonnées, structures/type/position/longueur/description et ordre stable. Les différences de validation des primitives M²FM aux coordonnées invalides du bloc 49 ne sont pas sollicitées par ces pistes ; aucun écart n’est normalisé ici.

La référence retourne 884 résultats et 45 refus : 27 ArgumentOutOfRangeException et dix-huit ArgumentException. Les diagnostics attendus sont comparés, y compris numéro avant taille puis code de taille. Les résultats comprennent 733 secteurs : 681 à CRC valide, 26 à CRC faux et 26 indéterminés ; 396 de taille 128 et 337 de taille 256. Les 1 593 structures se répartissent entre huit cents en-têtes et 793 données.

Deux contrôles vérifient l’annulation des entrées de piste. Un contrôle passe quatre secteurs mêlant FM/M²FM et normal/supprimé par le registre automatique explicite, puis vérifie adresses, contenu et intégrité. Les 203 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **1 135 contrôles, 0 différence, 929 cas, 203 textes**.
- Audit : **499 fichiers C++, 29 langues**, réussi.

Le banc build/verification-rx02-tracks et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Les dumps RX02, ordres sectoriels et politiques restent à porter séparément.
