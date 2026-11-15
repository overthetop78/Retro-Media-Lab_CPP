# Validation du reader Apple commun

- Compilation Debug réussie : `build/apple-container-final-build.log`. L’erreur initiale d’identifiant Mac GCR absent a été corrigée avec la constante source `applemac.gcr` ; aucun avertissement dans la compilation finale.
- Programme mémoire ciblé : `build/apple-container-smoke.log`, PASS. Capacités exactes 19/10/3, préfixe Apple, refus d’un format IBM demandé, image brute, priorité de signatures WOZ et 2IMG sur l’extension NIB, extension NIB insensible à la casse, restauration du nom DiskCopy depuis les métadonnées du document, rejet des données invalides et propagation de l’annulation.
- Aucun média utilisateur, aucune ancienne batterie de codecs et aucune nouvelle comparaison C# exécutés.
- Constants et 29 DLL de langues compilées : `build/apple-container-catalogs.log`. Relecture du nouveau message, corrections des contresens et ponctuations observés ; aucune prétention à une certification linguistique professionnelle.
- Audit des littéraux : 865 fichiers, 29 langues, PASS. Frontières : 853 fichiers, zéro violation des règles vérifiées. Solution Visual C++ régénérée. Ces résultats portent sur ce raccordement, pas sur l’achèvement global de l’objectif.
- Banc temporaire retiré après validation.
