# Validation du lecteur SCP et du cache

- Compilation Debug réussie sans avertissement signalé : build/scp-reader-cache-build.log et build/scp-reader-cache-build-final.log.
- Contrôle natif ciblé PASS : build/scp-reader-cache-smoke.log. Capture synthétique écrite dans un fichier à nom Unicode, ouverte par ReaderRegistry et lue via le contrat CaptureReader.
- Neuf métadonnées présentes ; version brute 25, texte 1.9 et checksum True vérifiés. Pistes protégées et résolution 25 ns conservées.
- Adresse de la révolution du document identique à celle de la capture mémorisée. Après destruction du lecteur, le document conserve la capture ; son dernier propriétaire disparaît quand le document est libéré.
- Cache : deux appels réutilisent un chargement ; changement d'horodatage relance le chargeur ; Remember remplace le résultat ; un chargement défaillant est réessayé.
- Synchronisation déterministe par promesses : un appelant annule son attente, le chargement se poursuit et l'appel suivant reçoit son résultat sans nouveau chargement.
- Le cache est détruit avec un travailleur attendant son jeton de durée de vie : l'arrêt et la jonction ont lieu. Les gardes du contrôle annulent/libèrent aussi les attentes si une vérification échoue.
- Une révolution partagée nulle est rejetée. Les deux nouveaux diagnostics ont été générés avec Argos dans 29 langues, relus et corrigés, notamment pour distinguer tour de disque et révolution politique. En-US correspond à la base anglaise.
- 30 DLL de catalogues compilées : build/scp-reader-cache-catalogs.log. Audit des littéraux PASS : 980 fichiers C++, 29 langues, build/scp-reader-cache-audit.log. Projets Visual C++ régénérés.
- Programme, binaires et fichier de capture temporaires supprimés. Aucun média utilisateur ni ancienne batterie de tests rejouée.

Le contrôle valide ce lecteur, son cache et leurs durées de vie ; aucune mesure de débit global ni validation complète d'App n'est revendiquée.
