# Conversion des représentations Acorn

Références C# en lecture seule : `Images/Conversion/Acorn/RawToAcornAdfRepresentationConverter.cs`, `RawToBbcDfsRepresentationConverter.cs`, `SectorContainerToAcornAdfRepresentationConverter.cs`, `Images/Formats/Floppy/BbcDfs/BbcDfsLayout.cs`.

Les trois inscriptions du C# utilisent un convertisseur C++ commun à mode immuable, avec séparation des mécanismes bruts et conteneurs. L'ordre de la composition est désormais flux vers secteurs, secteurs vers flux, conteneur vers ADF, brut vers ADF, brut vers DFS, préparation HFE et signal.

Brut : source IBM/Atari ST, blocs de 512 octets, capacité exacte et tous les blocs présents. Les erreurs d'intégrité, marques de suppression, tags, format, diagnostic, adresses et tailles enregistrées, ordre enregistré, codes de contrôle et préfixes de données empêchent la réinterprétation. Les cartes ADFS anciennes ou FileCore sont validées avant la copie complète ; ADFS 1600 est exclu de ce chemin. La reconnaissance finale ADFS utilise la bibliothèque MediaFileSystems existante. DFS impose des catalogues complets, conserve la distinction face physique/adresse enregistrée H=0 et laisse l'intégrité inconnue, comme le C#.

Conteneurs : JFD, CPCEMU et UCSD/TeleDisk, géométrie physique du profil ADF, secteurs cibles présents exactement, adresse physique/enregistrée, intégrité vraie et codes de densité/taille propres au conteneur. Une image avec des emplacements finaux vides peut être réduite après validation des cartes ADFS. JFD conserve ses tags autorisés ; CPCEMU/TeleDisk retirent seulement les métadonnées explicitement retirées en C#. Les pertes du conteneur et des emplacements vides sont déclarées séparément.

Validation réussie : compilation Debug MediaEngine et 30 catalogues, audit de 1480 fichiers C++ et 29 langues. Contrôle ciblé en mémoire : ADFS 160, DFS, préservation de tous les octets et de la source, intégrité inconnue DFS, refus des blocs manquants et de treize types de métadonnées incompatibles, carte invalide, JFD avec tags autorisés/refusés, CPCEMU et retrait de ses seuls champs concernés, TeleDisk avec diagnostic obligatoire, réduction des emplacements finaux vides et annulation.

Les données temporaires du test utilisaient initialement un code de densité JFD et un code de taille CPCEMU incorrects ; ces deux valeurs ont été corrigées dans le test uniquement. Le contrôle final sort avec le code zéro. Son répertoire et son journal sont supprimés après réussite. Aucun média externe ni changement du dépôt C#.
