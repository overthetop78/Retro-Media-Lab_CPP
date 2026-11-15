# Validation XFD / Atom / IMG

- Compilation Debug : `build/xfd-atom-img-build.log`, réussie. Un avertissement de masquage dans ImgReader a ensuite été relevé et corrigé lors du bloc 114 ; voir `apridisk-rx02.md`.
- Un programme natif ciblé en mémoire : `build/xfd-atom-img-smoke.log`, PASS. Vérifie les quatre profils XFD avec padding nul, capacité logique sans padding, secteurs de boot courts et taille du dernier secteur, filtre de profil demandé et rejet de padding non nul après présélection.
- Le même programme vérifie le catalogue Atom, sa base zéro, le refus d’un compte d’entrées invalide et d’une extension incompatible ; pour IMG il vérifie la conservation du profil IBM avec BPB et le rejet d’un format demandé incompatible.
- Les sondes CPC/PCW existantes sont réutilisées ; leurs anciens contrôles n’ont pas été répétés dans ce raccordement. Aucun média utilisateur ni ancienne batterie de codecs utilisés.
- Trois messages générés avec Argos dans les 29 langues, relus et corrigés pour les contresens observés, notamment le padding et les noms invariants. Constants et les 29 DLL compilées : `build/xfd-atom-img-catalogs.log`.
- Audit des littéraux : 902 fichiers C++, 29 langues, PASS. Frontières : 890 fichiers, aucune violation des règles vérifiées. Solution Visual C++ régénérée ; banc temporaire supprimé après consignation. La validation globale du portage reste à faire.
