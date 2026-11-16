# Validation des images brutes définies

- Compilation Debug sans avertissement signalé : build/defined-raw-build.log.
- Contrôle natif ciblé PASS : build/defined-raw-smoke.log. Lecteur réel et adapter writer commun, tailles 128/256, stockage 512 par secteur, adresses 3/4, capacité 1024, taille nominale 256 et intégrité non déclarée.
- L'absence d'identifiant explicite refuse la reconnaissance. La casse de l'identifiant demandé est acceptée.
- Les octets de rembourrage non nuls de l'entrée ne deviennent pas des données ; le writer produit un rembourrage nul.
- Une image de test retire un secteur entre validation et écriture : erreur DefinedRawSector après début d'écriture, fichier existant 'sentinel' inchangé et aucun fichier temporaire restant. Les fichiers de test sont possédés par QTemporaryDir/QFile.
- Quatre diagnostics Argos générés, 29 langues relues/corrigées puis 30 DLL de catalogues compilées : build/defined-raw-catalogs.log.
- Audit des littéraux PASS : 965 fichiers C++, 29 langues ; solution Visual C++ régénérée.
- Programme et binaires temporaires supprimés. Aucun média utilisateur ni ancienne suite de tests rejouée.

Cette validation porte sur le nouveau backend et ses adapters ; la composition globale reste en cours.
