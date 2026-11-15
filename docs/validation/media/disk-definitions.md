# Validation des définitions

- Compilation Debug réussie sans avertissement : build/disk-definitions-build.log. Le premier essai manquait l'inclusion QStringList dans Layout.cpp ; l'inclusion explicite a corrigé l'erreur de compilation.
- Contrôle natif ciblé PASS : build/disk-definitions-smoke.log. Chargement réel des ressources depuis MediaEngine.dll ; IBM 1440 : 2880 secteurs, dernier cylindre/face/numéro 79/1/18 ; alias de dimensions ; exclusion d'un identifiant réservé insensible à la casse.
- Disposition synthétique : priorité de la dernière piste explicite, première piste globale, tailles 128/256 répétées, img_bps 512, piste vide, numéros Apple 5/2 et défaut Data General 8 x 512.
- Fichiers d'import temporaires : préfixe, casse, commentaire, nom local puis cycle détecté. QTemporaryDir et QFile assurent leur nettoyage même en exception.
- Les 35 CFG ont été comparés par SHA256 aux sources C# après copie : identiques.
- Quatre nouveaux diagnostics traduits avec Argos dans 29 langues, relus et corrigés ; 30 DLL de catalogues compilées : build/disk-definitions-catalogs.log.
- Audit des littéraux PASS : 963 fichiers C++, 29 langues. Projets Visual C++ régénérés.
- Programme et binaires temporaires supprimés après consignation. Aucun média utilisateur, aucune ancienne batterie de tests rejouée.

Ce contrôle porte sur les modèles, le lecteur et les ressources ; le lecteur/writer d'images génériques et la composition complète du catalogue restent à raccorder.
