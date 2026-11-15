# Conteneurs ADF — Acorn et Amiga

Référence directe : les neuf fichiers de `MediaEngine/Images/Formats/Floppy/Adf/` du dépôt C# `F:/GW GUI`. Leurs SHA-256 sont inchangés. Les constantes géométriques et identifiants sont relevés dans les sources partagées ; les algorithmes de cartes et catalogues restent dans MediaFileSystems.

## Organisation et profils

`MediaEngine/Formats/Adf/Geometry.cpp` contient les profils et comparaisons d'identifiants ; `Partial.cpp` décrit/valide les images partielles ; `Reader.cpp` conserve les décisions du lecteur C# ; `Writer.cpp` utilise l'écriture sectorielle linéaire atomique commune. Les deux writers Acorn/Amiga ont la même mécanique et partagent cette implémentation, avec sélection du profil par identifiant. Les constantes sont séparées du code.

Les six profils Acorn sont BBC S/M/L (256 octets, 40×1×16 ou 80×1×16 ou 80×2×16), ADFS 800/1600 Kio (1024 octets, 80×2×5 ou 80×2×10) et DOS Plus 800 Kio (mêmes dimensions 800 Kio, numérotation à partir de un). Les profils Amiga DD/HD sont 512 octets, 80×2×11 ou 80×2×22. Les autres numérotations commencent à zéro. Pour BBC S/M/L, la face de l'adresse enregistrée reste zéro sur les deux faces physiques.

Le marqueur d'intégrité global des secteurs complets est celui des builders C# ; il représente ici leur complétude. Aucun CRC ou drapeau d'intégrité d'en-tête/données n'est inventé. Un dernier bloc tronqué garde une intégrité inconnue.

## Reconnaissance et lecture

Les tailles exactes Amiga sont prioritaires avant toute tentative Acorn partielle, comme dans le lecteur existant. Les profils BBC suivent leur capacité exacte ; 800 Kio est désambiguïsé par la demande DOS Plus ou son en-tête FAT FD FF FF. La variante ADFS rembourrée de 1024 octets conserve une capacité utile de 800 Kio ; le rembourrage n'est pas exposé en secteurs ni réécrit. La variante 1600 Kio conserve son profil distinct.

Une ancienne carte partielle exige les deux sommes de contrôle valides et une capacité déclarée supérieure aux octets stockés, compatible avec le profil choisi. Les DiscRecords du début ou du bloc d'amorçage peuvent également déclarer la capacité. Le candidat est ensuite soumis au véritable détecteur ADFS : une taille annoncée seule ne suffit pas. Les secteurs absents ne sont pas remplis ; la capacité/logicalBlockCount déclarés peuvent différer de la géométrie régulière entière. Les positions de DiscRecord sont partagées par `FileCoreLayout.h` ; les contrôles de somme et la lecture de capacité ancienne restent dans FileCore, sans recopier le schéma dans MediaEngine.

Les règles RequestedFormatId et d'extension ont été conservées branche par branche, y compris leurs asymétries C# : `.640` est strict ; les tailles BBC ont leur filtre d'extension/demande ; certaines autres tailles exactes acceptent un identifiant reconnu sans garantir qu'il sera l'identifiant final. La demande sert aussi à désambiguïser DOS Plus et les candidats partiels. La reconnaissance ne réalise aucune conversion.

La lecture utilise une source partagée possédée par l'image et charge seulement l'en-tête nécessaire ou les secteurs demandés. Une source fichier est une interface en lecture seule, pas une copie immuable de tous ses octets. L'exploration Acorn appelle ADFS ou FAT12 pour DOS Plus puis l'analyse des entrées. Les géométries/conteneurs Amiga sont lus et écrits ; leur exploration rejoint désormais [AmigaDOS](amigados.md), sans les présenter comme des catalogues Acorn.

## Écriture, erreurs et limites

La géométrie et l'identifiant sont vérifiés avant l'écriture, puis chaque secteur doit être présent et complet. La publication reste atomique : erreur ou annulation abandonne le temporaire et préserve la destination. Il n'y a pas de remplissage implicite ni de conversion géométrique.

Les erreurs du conteneur sont codées dans MediaEngine et traduites dans les 29 langues ; les erreurs sectorielles et de source restent dans leur domaine propriétaire. Le raccordement aux documents applicatifs, à l'enregistrement général des lecteurs et aux services de conversion reste à porter. Ce bloc n'ajoute aucun lien direct d'App à MediaFileSystems ou MediaAnalysis et ne porte pas le codec MFM Amiga.
