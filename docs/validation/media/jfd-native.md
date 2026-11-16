# Validation ciblée de la structure JFD

Le 16 novembre 2026, compilation Debug de MediaEngine réussie avec MSVC 19.44 et zlib embarquée, sans avertissement dans `build/jfd-native-final-build.log`.

Un unique programme natif temporaire vérifie : application d'une table delta ; conservation intégrale du fichier à l'édition sans modification ; conservation des pistes brutes et secteurs absents ; absence de secteur synthétique pour un stub ; géométrie, ordre et marqueur natif ; modification d'un secteur et de son intégrité sans modifier les autres octets ; rejet d'une modification de numéro de secteur ; rejet d'éditions contradictoires sur données recouvrantes ; décompression gzip simple et concaténée ; rejet d'un CRC gzip erroné ; rejet d'une expansion supérieure à 64 Mio ; rejet d'une longueur native incohérente.

Résultat : `PASS JFD delta/stubs/native retention/edit conflicts/gzip CRC/concatenation/expansion`, dans `build/jfd-native-smoke.log`. Ces contrôles portent sur de petites structures synthétiques et une donnée compressible générée temporairement ; aucun média utilisateur n'est embarqué. Ils ne prouvent pas encore l'intégration de JFD dans les registres ni les services globaux.

La compilation des 30 DLL de catalogues a réussi (`build/jfd-native-catalogs.log`).

Les 19 diagnostics sont présents dans Constants et les 29 langues, générés avec Argos puis relus ; 305 corrections de traduction ont été apportées, notamment offsets, pistes, entrées, décompression, conflits et géométrie native. L'audit des littéraux passe sur 986 sources C++, voir `build/jfd-native-audit.log` ; il s'agit d'un contrôle statique, pas d'une validation fonctionnelle globale.

Le programme, ses fixtures compressées, son projet CMake et ses binaires sont supprimés après consignation. Les journaux de construction et de vérification restent dans le dossier build ignoré de Git.
