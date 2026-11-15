# Validation du décompresseur TeleDisk avancé

- Compilation Debug réussie dans `build/teledisk-advanced-build.log`, puis recompilation après remplacement de valeurs d’arité par leur constante nommée dans `build/teledisk-advanced-final-build.log`.
- Contrôle ciblé natif : `build/teledisk-advanced-smoke.log`, PASS. Les flux synthétiques ont été encodés par un générateur temporaire indépendant de la DLL, à partir des chemins de l’arbre adaptatif. Vérification exacte des octets : littéraux, référence LZSS avec recouvrement, puis 40 000 littéraux faisant reconstruire une fois les fréquences.
- Un flux de références répétées atteignant 3 000 000 octets vérifie le diagnostic de limite. L’entrée vide et l’annulation préalable sont également vérifiées. Aucun média utilisateur ni suite des anciens codecs.
- Fixtures, générateur Python, programme de contrôle et binaires temporaires supprimés après consignation.
- Diagnostic de limite généré avec Argos dans 29 langues puis relu ; corrections notamment de la confusion compressé/décompressé en finnois, indonésien, coréen, thaï et hébreu, du contresens « ruisseau » en vietnamien et du chinois traditionnel.
- Catalogues compilés : `build/teledisk-advanced-catalogs.log`, succès. Audit des littéraux : PASS, 922 fichiers C++, 29 langues. Solution Visual C++ régénérée.

Le contrôle de ce décompresseur ne valide pas encore un parcours de lecture TeleDisk complet, qui nécessite le reader et sa classification sectorielle.
