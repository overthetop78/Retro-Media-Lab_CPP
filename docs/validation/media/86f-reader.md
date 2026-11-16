# Validation ciblée du lecteur 86F

Le 16 novembre 2026, compilation Debug MediaEngine réussie avec MSVC 19.44, sans avertissement dans `build/86f-build.log`. Les 30 DLL Constants/langues sont construites dans `build/86f-catalogs.log`.

Un unique programme natif temporaire vérifie une table à deux faces : piste logique 3, piste 7 sans transition et piste 257 sur la seconde face. Les nombres explicites de 9 et 13 cellules, l'ordre inversé des octets des mots, les drapeaux inconnus conservés et l'omission de la piste vide sont contrôlés. Les coordonnées protégées sont C3/H0 et C1/H1, les cellules durent 1000 ns, les révolutions sont en résolution 25 ns et leurs intervalles attendus sont respectivement [40,320] et [160,360]. Aucune structure ou caractéristique n'est inventée.

Le contrôle couvre aussi un en-tête standard, l'inversion d'octets sans champ explicite, un masque de vitesse non nul empêchant l'utilisation du nombre explicite, la sélection par le registre, l'identifiant `raw.86f` et la métadonnée décimale des drapeaux 22664.

Les rejets ciblés concernent signature, table tronquée, offset supérieur à la plage signée, ordre des offsets incompatible, nombre explicite négatif, piste tronquée, conversion d'une image vide et annulation.

Résultat : `PASS 86F two sides/explicit cells/word byte order/zero tracks/flux timing/metadata/registry/range rejections`, dans `build/86f-smoke.log`. Les anciennes suites de codecs n'ont pas été exécutées et aucun média utilisateur n'a été utilisé.

Les catalogues Constants/en-US et les 28 traductions Argos ont été relus et corrigés ; paramètres et invariant 86F sont vérifiés. L'audit statique passe sur 1001 sources C++ et 29 langues (`build/86f-audit.log`). Ce contrôle ciblé ne prouve pas l'intégration globale des services média depuis App.

Le programme de vérification, son projet CMake et ses binaires temporaires sont supprimés après consignation. Les journaux restent dans build, ignoré de Git.
