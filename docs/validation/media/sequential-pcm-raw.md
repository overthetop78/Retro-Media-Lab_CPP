# Validation séquentielle, PCM et RAW

16 novembre 2026 : compilation Debug MSVC x64 de MediaEngine réussie (build/sequential-pcm-build.log). Le contrôle temporaire natif compile et passe (build/sequential-pcm-smoke.log). Il couvre modèle possédé, collections absentes/vides, coordonnées, bornes temporelles, 32 combinaisons PCM (8/16/24/32 bits, mono/stéréo, signé/non signé, little/big endian), aller-retour octet exact, extension du signe 24 bits, interpolation, duplication des canaux, arrondi au pair, pulsations/silence, entrée vide et annulation avec destination préservée.

Les quatre erreurs sont générées avec Argos dans les 29 langues puis relues/corrigées (ordre des octets, signe numérique, média séquentiel, négation, arabe non traduit et chinois traditionnel notamment). Les 30 DLL de catalogues compilent (build/sequential-pcm-catalogs.log). Audit PASS sur 1188 fichiers C++, 29 langues (build/sequential-pcm-audit.log).

Le projet de contrôle et ses données temporaires sont supprimés après consignation. Aucun média utilisateur ni test dans le workflow GitHub. Ce contrôle porte sur le bloc nouvellement ajouté ; il ne prouve pas l’intégration des services globaux encore absents.
