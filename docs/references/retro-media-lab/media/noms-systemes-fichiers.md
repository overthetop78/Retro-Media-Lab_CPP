# Noms des systèmes de fichiers — bloc 234

Référence en lecture seule : `Definitions/FileSystemDisplayNames.cs` du C#. Les noms de secours et les clés/arguments traduisibles sont maintenant fournis par un service commun appelé directement à la sortie des lecteurs concernés, sans dépendance à App ni à la langue courante.

`Constants/MediaFileSystems/names.json` est la source unique des textes anglais. `scripts/translations/compile_fs_names.py` génère le tableau C++ de secours pendant le build ; aucune copie des textes n’est maintenue dans les lecteurs. Les noms techniques invariants restent dans Constants. Cinq phrases passent par en-US puis Argos ; leurs catalogues ont été relus et corrigés dans les 29 langues.

Les variantes couvertes sont les huit AmigaDOS, FAT12 IBM/MSX/Atari, Apple DOS 3.2/3.3, CP/M générique et Amstrad CPC/PCW, ProDOS/SOS, Lisa à catalogue tabulaire/hash/B-tree ou version hexadécimale inconnue, ainsi que les noms fixes Acorn, Macintosh, Commodore, RT-11 et UCSD. Les produits et termes techniques des phrases Atari/Lisa sont passés en arguments invariants. Un identifiant de lecteur inconnu conserve son libellé et sa recette éventuelle.

Compilation Debug du bloc réussie, build final sans avertissement. Le banc temporaire a vérifié le choix des variantes, la casse des identifiants, les noms anglais de secours, les clés/domaines, les arguments B-tree et version hexadécimale, et la conservation des libellés de modules inconnus. Aucun algorithme de système de fichiers n’a été retesté. Le banc et ses artefacts ont été supprimés.

Le suivi associe également vingt contrats/registres MFS qui existaient déjà à leurs sources natives : modèles de géométrie et de volume, résultats d’exploration, détecteurs de volumes, interfaces de représentations, documents et lecteurs. Ces entrées ont été rapprochées du code courant ; aucune seconde implémentation de ces services n’a été ajoutée. Les statuts partiels des autres fichiers ne sont pas automatiquement considérés comme terminés.
