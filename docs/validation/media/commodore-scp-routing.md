# Validation routage Commodore SCP — 15 novembre 2026

Le banc compile sept sources C# originales inchangées, empreintes vérifiées. Le lecteur reçoit un registre de trace qui interrompt le premier appel de codec : il mesure le dispatch du code original, sans remplacer celui-ci par une comparaison réécrite dans le banc. Les modèles/contrats et branches non atteintes sont adaptés ; ce banc de routage ne revalide pas les algorithmes internes de ces branches.

17 demandes couvrent null, vide, identifiants exacts 1541/1571/1581/900, casse différente, espaces avant/après, caractère nul final et identifiants inconnus. Le choix C++ est observé par le diagnostic propre au codec sur une capture sans secteur décodable et comparé au codec demandé par le C#.

Neuf captures synthétiques C++ utilisent les trois encodeurs GCR 1541, MFM 1581 et GCR 900, deux pistes et un/trois/dix secteurs. Pour chaque demande, la reconstruction directe de branche est comparée au routeur en mémoire et au routeur de fichier après écriture SCP. La comparaison porte sur format, géométrie, capacité, nombre logique, tous les blocs/adresses/SHA-256/intégrités/révolutions et toutes les notifications de progression. Les références internes de chaque codec ont été validées dans leurs blocs dédiés ; ces 306 comparaisons vérifient ici leur intégration par le routeur et le chemin fichier.

Les contrôles supplémentaires couvrent l’annulation initiale sur capture/fichier, l’annulation et l’exception depuis la progression, la suppression immédiate du fichier après succès/échec, une capture invalide et son fichier libéré, et l’annulation avant ouverture d’un chemin inexistant.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **371 contrôles, aucune différence**, 17 routes C# et neuf captures synthétiques.
- Audit : **378 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `3bc8708bd96be0fb29657773040d3dff634a6ae9a76e6120e8839c7660d09a75`.

Le banc `build/verification-commodore-scp-routing/`, les captures, exécutables, références et journaux de comparaison sont supprimés après vérification. Aucun média utilisateur ou test matériel, aucun ajout au workflow GitHub. La gestion asynchrone GUI et les registres de documents ne sont pas couverts.
