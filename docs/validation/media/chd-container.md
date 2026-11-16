# Validation ciblée du conteneur CHD

Le 16 novembre 2026, MediaEngine compile en Debug (`build/chd-container-final-build.log`). L’avertissement de comparaison signé/non signé du champ de taille d’en-tête est corrigé. L’audit des littéraux passe sur 1121 fichiers C++ et 29 langues (`build/chd-container-audit.log`).

Le contrôle natif temporaire réussit : `PASS CHD v1-v5 maps-RLE-self/metadata/legacy-mini/self-reference/DEFLATE/CD-ECC/CRC/bounds/cancellation` (`build/chd-container-smoke.log`). Les cinq versions d’en-tête sont couvertes ; cartes historiques brutes/mini/self et cookie, carte v5 brute avec hunk zéro, carte compressée avec données brutes ou DEFLATE, répétitions courtes/longues et références self successives. Les CRC des cartes et hunks sont calculés par une fonction bit à bit indépendante pour construire les fixtures.

Les métadonnées valides, l’absence de tag, le tag invalide et le cycle sont contrôlés ; les parents, signatures invalides, références hors borne/cycliques, tampon court, CRC invalides et annulation sont refusés. Un secteur CD synthétique vérifie la décompression séparée données/sous-code, le rétablissement de la synchronisation, la régénération ECC et l’entrelacement 2352/96. Les codecs de base avaient été contrôlés dans la tâche 149 ; ces tests ne relancent pas les anciennes suites.

Les 13 erreurs ont été générées avec Argos dans les 29 langues, relues et corrigées, notamment hunk, marqueur terminal, ordre des octets, troncature et CRC. Les 30 DLL Constants/langues compilent (`build/chd-container-catalogs.log`). Programme, données et binaires temporaires sont supprimés après consignation. Aucun média utilisateur n’a été lu ou modifié.

Ces contrôles ne prouvent pas encore la lecture d’un CHD depuis les lecteurs HDD/optiques ou App ; leur raccordement reste à faire.
