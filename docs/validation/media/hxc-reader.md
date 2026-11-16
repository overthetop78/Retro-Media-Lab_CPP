# Validation ciblée du lecteur HxC Stream

Le 16 novembre 2026, compilation Debug MediaEngine et compilation incrémentale finale réussies avec MSVC 19.44 (`build/hxc-reader-build.log`, `build/hxc-reader-final-build.log`). Les 30 DLL Constants/langues sont construites (`build/hxc-reader-catalogs.log`).

Un seul programme natif temporaire a vérifié :

- lecture du chunk produit par le codec, deux révolutions de durée 400 ticks aux intervalles [50,100,250], conservation des octets et métadonnées ;
- impulsions variables indépendamment empaquetées avec LZ4, impulsion nulle ignorée, octets surnuméraires ignorés après le nombre déclaré et révolution de repli ;
- métadonnées dupliquées : première valeur conservée, dernier taux d'échantillonnage valide utilisé pour la résolution ;
- capture sans impulsions mais avec index et protection en écriture ;
- deux fichiers physiques sous préfixe Unicode, piste hors plage et autre préfixe exclus, classement, associés, longueur totale et empreintes avec protection globale ;
- refus des CRC, signatures, tailles, types de blocs et impulsions invalides, blocs LZ4 incomplets, taille E/S impaire et durée dépassant la plage ;
- nom invalide, jeu vide, dossier absent et annulation.

Résultat : `PASS HxC reader/multitrack/index revolutions/fallback pulses/LZ4/native metadata/global protection/CRC and bounds`, dans `build/hxc-reader-smoke.log`. Aucun média utilisateur n'a été utilisé et aucune ancienne suite de codecs n'a été exécutée.

Les catalogues Constants/en-US et les 28 traductions Argos ont été relus et corrigés : taille impaire, durée de révolution, nombre déclaré, données tronquées et suffixe technique, notamment. NN.S.hxcstream est ajouté aux invariants communs. L'audit statique passe sur 1014 sources C++ et 29 langues (`build/hxc-reader-audit.log`). Il ne prouve pas l'achèvement des services globaux ni du writer multipiste.

Le programme, ses fichiers de pistes synthétiques, son projet CMake et ses binaires sont supprimés après consignation. Les journaux restent dans build, ignoré de Git.
