# Validation du lecteur TeleDisk

- Compilation Debug réussie sans avertissement signalé : `build/teledisk-reader-build.log`.
- Contrôle natif ciblé : `build/teledisk-reader-smoke.log`, PASS. Conteneur synthétique avec commentaire binaire, métadonnées d’en-tête, données indisponibles, secteurs de tailles mixtes, taille dominante, tri logique, ordre et adresses enregistrés, drapeaux CRC physique et données supprimées.
- Le même contrôle vérifie le classement IBM scan avec saut DOS, le repli UCSD, les filtres du profil demandé, la lecture par le contrat commun, les rejets de CRC global/commentaire et la terminaison tronquée. Un petit flux avancé synthétique vérifie le raccordement décompression → signature/CRC normalisés → lecture sectorielle.
- Aucun média utilisateur, aucune répétition des suites des anciens codecs. Données en mémoire ; programme et binaires temporaires supprimés après consignation.
- Quinze diagnostics générés avec Argos dans 29 langues, relus puis corrigés : troncature, en-têtes de pistes/secteurs, unités, adresses et paramètres, CRC, données encodées et chinois traditionnel. Deux messages arabes aux accolades altérées ont été corrigés avant compilation.
- Compilation des 30 catalogues : `build/teledisk-reader-catalogs.log`, succès. Audit des littéraux : PASS, 927 fichiers C++, 29 langues. Audit des frontières : PASS, 915 fichiers, aucune violation. Projets Visual C++ régénérés.

Ces contrôles couvrent le lecteur et son adapter, pas encore le writer TeleDisk ni la composition globale du service de lecture.
