# Validation des catalogues dynamiques

- Compilation Debug réussie sans avertissement signalé : build/runtime-format-catalogues-build.log.
- Contrôle natif ciblé PASS : build/runtime-format-catalogues-smoke.log.
- Analyse d'une aide synthétique : texte extérieur ignoré, marqueurs insensibles à la casse, identifiant dupliqué supprimé et trois suffixes normalisés. Une aide sans sections donne des capacités inconnues.
- Catalogue inconnu inchangé ; catalogue connu : alias Atari ST/Macintosh/ProDOS, refus des profils non annoncés et maintien des quatre définitions spéciales et des formats flux génériques.
- Format bitcell raw.250 : SCP puis HFE, SCP par défaut ; format inconnu avec suffixes : IMG prioritaire, famille/détail/tag conservés. Pas d'ajout de format découvert si aucun suffixe disponible.
- Fusion par identifiant insensible à la casse : présentation et drapeaux conservés, seules les extensions/compatibilités changent ; nouvelles entrées ajoutées à la fin.
- Résolution de textes testée avec deux langues simulées et avec un nom technique invariant.
- Audit des littéraux PASS : 974 fichiers C++, 29 langues, build/runtime-format-catalogues-audit.log ; projets Visual C++ régénérés. Aucun nouveau diagnostic ou traduction nécessaire.
- Programme et binaires temporaires supprimés ; aucun média utilisateur ni test de lecteur physique exécuté.

Ce contrôle porte sur les catalogues et les règles d'adaptation, sans valider les services globaux ou les capacités réelles d'un périphérique.
