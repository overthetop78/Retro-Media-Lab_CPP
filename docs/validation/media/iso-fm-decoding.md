# Validation décodage ISO FM — 15 novembre 2026

36 sources C# originales sont compilées sans modification de leur dépôt ; leurs empreintes correspondent à l’inventaire. Le banc utilise les modèles, encodeur, primitives de flux, CRC et décodeur originaux. La méthode privée DecodeStream est appelée par réflexion pour comparer directement les cellules et limites de période ; les deux méthodes publiques sont aussi exercées.

670 cas comprennent 363 décodages de cellules, 146 décodages à durée connue, 119 adaptatifs, 17 lectures de données, 16 d’en-tête, huit reconnaissances aux offsets limites et une matrice exhaustive des 65 536 motifs de seize bits. Cette matrice compare le nombre de marques reconnues et le SHA-256 de tous les résultats dans l’ordre.

Les flux couvrent les 256 codes de taille, les huit tailles ISO, secteurs supprimés, combinaisons d’intégrité, en-têtes/données isolés ou multiples, CHRN extrêmes, troncatures aux frontières, rotations, altérations séparées d’horloge/donnée/CRC, pistes physiques/logiques à un/trois/dix secteurs et jitter. Les périodes explicites couvrent zéro, valeurs négatives et frontières de marques/champs. Le mode fixe vérifie arrondis au pair, index/intervalles incohérents, intervalles nuls/absents, durées non finies/non positives ou inférieures au minimum, débordements int32 sans grande allocation.

Les comparaisons portent sur identité, confiance et temporalité, longueur/SHA-256 des octets et données ; tous les secteurs, adresses, tailles, offsets, intégrités nulles/vraies/fausses et marque supprimée ; les octets exacts de CRC enregistrés ; toutes les structures avec offsets, longueurs et descriptions anglaises. Les aides de lecture comparent aussi les champs bruts et CRC. Les refus comparent type d’exception et paramètre.

L’annulation initiale est vérifiée sur les décodages de cellules, adaptatif, fixe, et les lecteurs d’en-tête/données. Les six textes des 29 langues sont chargés depuis les DLL et comparés aux catalogues relus : 174 contrôles. Le banc Ninja utilise le préfixe d’inclusion MSVC français du projet.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **845 contrôles, aucune différence**, 670 cas dont 653 valides et 17 refusés.
- Audit : **387 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `783e406546cfc44acc70fd2ee1808f683322a96eecea29a854ae9528f2e9d1a4`.

Le banc `build/verification-iso-fm-decoding/`, ses références/binaires et journaux de comparaison sont supprimés après validation. Aucun média utilisateur ou test matériel, aucun ajout au workflow GitHub. Les reconstructeurs ISO ne sont pas validés par ce bloc.
