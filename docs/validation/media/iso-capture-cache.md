# Validation du cache ISO — 15 novembre 2026

Le banc compile trente-cinq sources C# originales, dont IsoScpCandidateDecoder complet, avec un lecteur et un registre de contrôle pour isoler le comportement du cache. Les modèles, adaptateurs et la collecte parallèle sont les implémentations originales. Leurs empreintes correspondent à l’inventaire.

Les vingt-neuf scénarios comparent les résultats des assertions et les nombres exacts de lectures/décodages : hits, token annulé d’un demandeur ultérieur, normalisation du chemin, contenu changé avec métadonnées conservées, changements de taille/date/chemin, une seule capture par clé, indépendance/ordre/répétition des codecs, séparateur source, contournement du cache par une image, échecs du lecteur/décodeur/progression, annulation et capture vide suivis de réessais. Ils couvrent aussi empreinte d’un fichier absent puis créé, chemin Unicode, chemin vide et caractère nul.

Seize appels simultanés partagent une seule lecture et un seul décodage ; seul le premier reçoit la progression. Des barrières maintiennent une ancienne opération active pendant le chargement d’une autre capture : son succès ou son échec ne supprime pas l’entrée plus récente. L’annulation du premier appel atteint aussi le demandeur en attente, puis le réessai peut être mis en cache. Les résultats sont comparés par identité partagée, et pas seulement par égalité des données.

Sept contrôles supplémentaires utilisent le véritable lecteur fichier C++ : deux captures SCP synthétiques sont encodées en FM/MFM et écrites par le service SCP. La collecte doit restituer les données et leur intégrité, le hit ne doit pas relire/décoder, puis la date modifiée invalide le résultat. Un fichier absent conserve le diagnostic SourceLengthFailed commun et traduit. Aucun média utilisateur n’est utilisé.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **36 contrôles, aucun écart**, dont 29 scénarios C#.
- Audit : **408 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `912bb4cd2b55d49fde54f962b2474a8a842ab709b7544fdf232f4b6c02f32bd0`.

Le banc build/verification-iso-cache, les fichiers de capture/empreinte, binaires et journaux de comparaison sont supprimés après validation. Aucun test ajouté au workflow GitHub. Les décodeurs de contrôle ne valident pas le registre général ; les codecs FM/MFM réels sont couverts par les intégrations et par leurs validations antérieures.
