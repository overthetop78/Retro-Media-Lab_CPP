# Validation D64/D71 writer — 15 novembre 2026

Le banc compile dix sources C# originales inchangées : writer, diagnostics, modes/codes, géométries partagées et constantes. Les modèles de secteur sont des adaptateurs de données ; la sortie atomique est injectée avec un flux mémoire dans le banc C#, sans remplacer l’algorithme d’écriture.

219 cas comparent les longueurs et SHA-256 des données/cartes, ainsi que les catégories de rejet et numéros de blocs concernés. Ils couvrent D64/D71, 35/40 pistes, formats en majuscules, modes None/Preserve et valeurs de mode non nommées, diagnostics cycliques couvrant les 256 octets, blocs absents/courts/longs au début/milieu/fin, diagnostic absent, géométrie/format/compteur invalides et priorité des erreurs de données avant les erreurs de carte.

Pour les 64 sorties valides, tous les secteurs sont relus par le lecteur de conteneur : octets et diagnostics comparés individuellement. Chaque échec conserve une destination sentinelle et ne laisse aucun fichier temporaire. Quatre injections supplémentaires couvrent erreur de source durant le payload, erreur de source durant la carte, bloc disparu à la seconde passe et annulation durant la carte. L’annulation initiale ne lit aucun bloc. Deux parcours supplémentaires construisent un volume DOS, écrivent le conteneur, puis l’explorent jusqu’à l’analyse et vérifient son contenu.

Les quatre clés des 29 langues sont chargées depuis les DLL et comparées aux catalogues relus, soit 116 textes.

- Compilation/déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **140 086 contrôles, aucune différence**, 64 cas valides, 155 refusés, 64 allers-retours et deux parcours DOS.
- Audit des littéraux : **351 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `6c349da18f3432e3eb03a739321edf737a7ce9ea446b5bde1feda3bbdc8efe76`.

Le banc `build/verification-commodore-container-writer/`, ses sorties temporaires et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. La reconstruction SCP Commodore 1541/1571, les conversions et l’orchestration de migration ne sont pas validées par ce bloc.
