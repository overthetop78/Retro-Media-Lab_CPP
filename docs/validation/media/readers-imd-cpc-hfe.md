# Validation des adaptateurs IMD / CPC DSK / HFE

- Compilation Debug MediaEngine réussie : `build/container-readers-build.log`, sans avertissement ni erreur signalés.
- Un seul programme natif en mémoire a vérifié le raccordement : `build/container-readers-smoke.log`, résultat PASS.
- Cas ciblés : préfixe Epson insensible à la casse et rejet d’un format demandé incompatible ; conservation d’un secteur ImageDisk supprimé avec erreur, de son contenu, du commentaire et de la structure restaurable ; signatures standard/étendue CPC DSK et contenu du secteur ; représentation flux HFE, protection en écriture et métadonnées ; rejet d’une fausse signature HFE ; future consommée après destruction du reader temporaire.
- Aucun média utilisateur, aucune comparaison C# supplémentaire et aucune ancienne batterie de codecs exécutée. Les données synthétiques vérifient l’intégration nouvelle, pas l’exhaustivité des formats.
- Audit des littéraux : 862 fichiers C++, 29 langues, PASS. Audit des frontières : 850 fichiers, aucune violation des règles vérifiées. Ces audits ne prouvent pas à eux seuls l’achèvement global du portage.
- Solution Visual C++ régénérée. Banc temporaire et binaires supprimés après exécution.
