# Validation de l’encodage des images sectorielles — 15 novembre 2026

Le banc compile 64 sources C# originales en lecture seule : orchestrateur, registres, politiques, modèles sectoriels réels, classe de base des encodeurs, primitives et géométries. Les identifiants nécessaires sont extraits des constantes originales. FluxEncoderCatalog est remplacé uniquement pour la liaison du constructeur par défaut non invoqué : le registre C# est injecté explicitement avec des encodeurs d’enregistrement dérivés de TrackEncoderBase.

Les 85 sources compilées ou consultées correspondent à l’inventaire. Empreinte du manifeste : `6edb4e1f023760f4185dfa6e550d28310cadb4d300e615b1315bfe0ceb554a3e`.

1 333 cas comparent les résultats et les requêtes effectivement transmises aux encodeurs : cylindres/faces, ordre des secteurs, charges utiles, attributs, adresses et tailles enregistrées, intégrités et CRC, cadences et unités. Les encodeurs d’enregistrement C#/C++ produisent le même résultat minimal pour isoler l’orchestration ; ils ne remplacent pas les algorithmes des politiques ni les validations de la classe de base.

Les cas couvrent les familles du catalogue, images logiques, politiques absentes, encodeurs absents, pistes ciblées vides, blocs courts/longs, géométries Apple autour de 80 pistes, assemblage 1581, séparation RX02, ordre physique avec égalités, densités JFD, durées nulles et limites de conversion nanoseconde. 545 cas supplémentaires désactivent l’autorisation de tailles variables afin de vérifier l’équivalence d’AvailableBlocks. Les annulations sont exercées avant la première piste et après une ou plusieurs requêtes d’encodage ; les appels déjà réalisés sont également comparés.

Pour les gardes UTF-8, les attentes C# reprennent ThrowIfNullOrWhiteSpace et le prédicat blanc/caractère de contrôle de la récupération Amiga. Le C++ appelle les vrais services analyzeEntries et FAT12::write ; le helper privé Amiga est testé en compilant son fichier Codec.cpp original dans le banc. Cela valide ces gardes ciblées, sans prétendre revalider tout FAT12 ou toute la récupération Amiga.

Les contrôles d’intégration supplémentaires utilisent les composants réels :

- encodage physique d’image avec le registre complet puis décodage ISO MFM et ISO FM, avec comparaison des données ;
- encodage GCR Commodore 900 avec un secteur valide et un numéro dépassant la géométrie, puis reconstruction par readFlux : seul le secteur valide subsiste et ses octets sont conservés ;
- annulation de l’extension C++ de piste ciblée.

Les quatre textes des 29 DLL de langues sont comparés aux catalogues, soit 116 textes. Les traductions Argos ont été relues et corrigées, notamment l’unité ticks, les négations et le vocabulaire de disque/piste.

- Construction complète et déploiement Debug/Release réussis.
- Chaque configuration : **1 456 contrôles, 0 différence, 1 333 cas, 116 textes**.
- Audits : **563 fichiers C++**, **551 fichiers pour les frontières**, **0 violation** ; git diff --check réussi.
- Aucun avertissement ou erreur C++ dans les journaux complets finaux de ce bloc.
- Défauts corrigés : exception d’annulation typée de l’énumération, contrôle de taille nominale indûment ajouté au parcours AvailableBlocks, validations UTF-8 perdant U+FEFF ; comparaison signé/non signé Commodore 900 corrigée.
- Une mauvaise normalisation de la clé d’annulation dans le banc a aussi été corrigée : la clé existante est Media.Cancelled, pas MediaEngine.Cancelled.
- Banc et journaux temporaires retirés après validation. Aucun test ajouté au workflow GitHub.

La validation porte sur l’encodage d’images sectorielles et ses raccordements ; le portage complet de toutes les bibliothèques médias reste en cours.
