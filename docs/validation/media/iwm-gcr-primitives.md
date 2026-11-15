# Validation des primitives Apple IWM GCR — 15 novembre 2026

Le banc compile huit fichiers C# originaux inchangés : AppleIwmGcrCodec, AppleIwmGcrFormat, AppleIIGcrFormat, identifiants/noms des codecs, BitPrimitives, SectorSizeCode et BitConstants. Leurs SHA-256 correspondent à l’inventaire.

Les 5 917 cas comprennent 1 032 encodages valides, neuf tailles d’encodage refusées, 4 868 décodages valides en longueur et huit longueurs de décodage insuffisantes. Les motifs couvrent les 256 remplissages constants, tags et données remplis indépendamment, bits isolés aux frontières de tags/données et du dernier groupe, 128 blocs pseudoaléatoires déterministes, les 256 valeurs brutes à dix-sept positions de symbole et 256 jeux numériques supplémentaires. Les valeurs non limitées à six bits et les symboles excédentaires sont comparés au comportement source.

Le banc compare les 703 octets GCR encodés, les 524 octets reconstruits et les quatre valeurs de checksum, dans leur ordre exact. Les 1 032 encodages source redonnent aussi leur bloc d’entrée après conversion inverse de la table et décodage. Les tailles incorrectes sont comparées par catégorie d’exception ; IwmSymbolLength correspond explicitement au refus IndexOutOfRangeException source, avec contrôle préalable de longueur en C++.

Deux contrôles supplémentaires vérifient l’annulation des points d’entrée avant validation de longueur. Les 58 textes chargés depuis les DLL des 29 langues sont comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **5 977 contrôles, 0 différence fonctionnelle, 5 917 cas, 58 textes**.
- Audit : **446 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste : `31910a9f7f2c83ad8ad895c5572c33f67e274e59e854529520da74c08034ddb4`.

Le banc build/verification-iwm-primitives et ses journaux de référence/comparaison sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Cette validation concerne les primitives communes ; elle ne valide pas encore les codecs de piste Macintosh et Lisa.
