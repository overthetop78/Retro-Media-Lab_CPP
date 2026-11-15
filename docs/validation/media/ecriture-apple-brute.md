# Validation de l’écriture Apple brute — 15 novembre 2026

Le banc compile 23 fichiers C# originaux : writers Apple/Macintosh, géométries nécessaires, modèles sectoriels, convertisseur DOS/ProDOS et writers atomiques réels. Les sources sont inchangées par rapport à l’inventaire ; empreinte du manifeste : `5742675a8b78e7689da7a6ccad3f452d48b5dd42c202564f43ebcdf3973a8678`.

**157 cas ciblés C#/C++, zéro différence**. Pour chaque sortie acceptée : longueur et SHA-256 de tous les octets. Pour chaque entrée : prédicat Macintosh de géométrie et prédicat ProDOS 140 Kio. Refus normalisés par catégorie : cible, données ou nombre de faces. Sont couverts : cibles DOS/ProDOS/SOS, casse, extensions valides/inconnues, sorties 2IMG, géométries GCR/MFM, capacité erronée, blocs absents au début/à la fin, contenu trop court, compte ou taille nominale erronés, adresses incorrectes, et dimensions que le C# accepte sans contrôle supplémentaire.

**Quatre contrôles additionnels par chemin** : octets Apple identiques à sa charge utile ; exception d’annulation attendue ; destination précédente intacte après annulation ; octets Macintosh identiques à sa charge utile. QTemporaryDir et QFile garantissent la fermeture/suppression des fichiers du banc même sur exception. Les comparaisons précédentes des lecteurs et géométries ne sont pas relancées.

La cible MediaEngine compile en Debug sans erreur ou avertissement C++. Les traductions ont été générées par Argos et corrigées après lecture ; les invariants Apple partagés ont été ajoutés à common.txt pour les portages suivants. Les catalogues et audits ont été validés lors de l’[intégration groupée](integration-apple-parallele.md).

Le banc et ses journaux sont supprimés après ces vérifications ; aucun test ajouté au workflow GitHub. Cette validation ne certifie pas les autres conteneurs ni le portage global.
