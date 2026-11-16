# Validation ciblée des adaptateurs JFD

Le 16 novembre 2026, compilation Debug MediaEngine réussie avec MSVC 19.44, puis compilation des 30 DLL Constants/langues réussie. Journaux : `build/jfd-adapters-build.log`, `build/jfd-adapters-final-build.log`, `build/jfd-adapters-catalogs.log`.

Un unique programme natif temporaire a vérifié :

- création d'une image avec secteurs de 128 et 256 octets, classement par ordre enregistré, conservation des intégrités et table minimale de 160 pistes conforme au C# ;
- refus des marques de données supprimées et adresses enregistrées incompatibles ;
- sélection du writer par le registre, identifiants/extensions insensibles à la casse, écriture gzip par défaut et reconnaissance depuis un chemin Unicode ;
- réécriture gzip conservant la structure native, export raw demandé explicitement puis réexport raw fidèle ;
- rejet des métadonnées base64 invalides, avec conservation d'un fichier de destination préexistant ;
- deux recettes informatives pour un fichier contenant pistes brutes et positions explicites, sans textes affichés en dur ;
- appel du chargeur injectable avec le chemin principal, puis écriture conservant intégralement cette structure native spéciale ;
- rejet d'une longueur déclarée incohérente via RecognitionError du registre ;
- probe refusant une source devenue trop grande sans laisser échapper cette erreur de structure.

Résultat : `PASS JFD create/variable sectors/record order/integrity/raw+gzip registries/native metadata/messages/injected reader/atomic rejection`, journal `build/jfd-adapters-smoke.log`. Le contrôle n'a pas exécuté les anciennes suites de codecs ni utilisé de média utilisateur.

Les catalogues Constants/en-US et les 28 traductions Argos ont été relus ; les textes techniques incorrects ont été corrigés et les invariants vérifiés. L'audit statique passe sur 991 sources C++, 29 langues (`build/jfd-adapters-audit.log`). Ce résultat ne prouve pas le fonctionnement global de MediaEngine depuis App.

Le programme, le projet CMake et les binaires temporaires sont supprimés après consignation ; les journaux restent dans build, ignoré de Git.
