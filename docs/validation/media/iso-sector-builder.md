# Validation construction sectorielle ISO — 15 novembre 2026

Seize sources C# originales sont compilées sans modifier leur dépôt, dont IsoSectorImageBuilder, normalisation, modèles de candidats, sélection d’intégrité, modèles sectoriels réels et leurs validations. Les empreintes correspondent à l’inventaire. Les projets source utilisent .NET 10, usings implicites et annotations nullables ; aucune option locale de vérification arithmétique globale n’est définie dans les projets concernés.

588 cas comprennent 216 mesures, 210 constructions, 92 extractions des meilleures données et 70 paddings. Les mesures couvrent les 81 séquences de quatre candidats parmi trois tailles (égalités finales incluses), égalités de fréquence du nombre de secteurs, numérotation négative/zéro/non contiguë et coordonnées int32 extrêmes. Les 27 combinaisons de trois intégrités vérifient priorité et stabilité. Les extractions filtrent les données absentes/vides avant sélection.

Les constructions croisent indexation à base zéro/un/par rang/constante/rejet, normalisation absente/padding/vide/slice, numéros au-delà de la géométrie autorisés/refusés et tailles variables. Elles couvrent groupes vides, données absentes/vides/courtes/longues, ordre physique à offsets égaux/négatifs, métadonnées enregistrées contradictoires, coordonnées rejetées, tailles/dimensions/capacités invalides, format vide ou espaces Unicode, doublons logiques et débordements du nombre de blocs. Les listes exactes d’appels des fonctions d’index/normalisation sont comparées, y compris avant une erreur.

Les résultats comparent mesure complète, format, géométrie, capacité, nombre logique, drapeau de taille variable et tous les blocs dans l’ordre avec adresse/longueur/SHA-256, intégrité/révolution et métadonnées. Les champs laissés vides par le C# (tag, codes format/diagnostic, CRC) sont aussi vérifiés. Les refus comparent leur catégorie ; seule l’absence de données du candidat est normalisée entre le diagnostic C++ explicite et les deux exceptions CLR observées pour ce même état. Le manifeste conserve les exceptions C# originales et cette normalisation du comparateur ne s’applique qu’au diagnostic IsoCandidateData.

Six contrôles supplémentaires vérifient la propriété des blocs après modification des candidats, la limite de lecture conservée dans une source adossée à des données, le diagnostic de fonction d’index absente, l’annulation initiale des cinq aides et l’annulation depuis les fonctions d’index/normalisation. La grande taille nominale d’une image sparse vide fait partie des cas C#, sans allocation géante. Les cinq textes des 29 langues sont chargés depuis les DLL et comparés aux catalogues relus : 145 contrôles.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **739 contrôles, aucun écart fonctionnel**, 588 cas (535 retournés, 53 refusés).
- Audit : **394 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `1b435bcc1b30eab42800029c55b4c8504d713ea2bfb2a7495b16791e6df80d5e`.

Le banc `build/verification-iso-sector-builder/`, ses références/binaires et journaux de comparaison sont supprimés après vérification. Aucun média utilisateur ou test matériel, aucun ajout au workflow GitHub. La collecte depuis le flux et les politiques ISO ne sont pas validées par ce bloc.
