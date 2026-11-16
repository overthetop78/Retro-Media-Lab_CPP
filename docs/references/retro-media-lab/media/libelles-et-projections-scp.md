# Noms des candidats et projections SCP

La référence `ScpSectorImageCandidate.cs` fournit des noms invariants de produits ou de formats. `ScpAutomaticImageExplorer.cs` utilise ces noms lors du début et de la fin de chaque candidat. Le C++ conservait les identifiants techniques à ces deux endroits.

`Candidate::displayName` reprend les huit correspondances exactes, puis les six motifs dans leur ordre, puis le nom des cinq familles. Les correspondances exactes restent sensibles à la casse comme le switch C# ; les motifs sont insensibles à la casse. Les noms de produits sont des constantes invariantes. Le rappel de progression utilise ces noms sans changer l'identifiant du candidat ni son classement.

Compilation native réussie. Contrôle ciblé des noms, de leur priorité, de la casse et des cinq replis réussi, sans décodage de capture. Contrôle temporaire supprimé.

Rapprochement des anciennes mentions restantes :

- Les projections publiques de pistes, formats, volume physique et propriétés de conteneur sont déjà dans `Exploration/Disk/Projection*.cpp`. Flags, largeur de cellule et sélecteur de faces sont conservés et exposés ; leur présentation par le GUI relève de l'application.
- Les six géométries Epson QX-10 et leur détecteur sont déjà dans `Formats/Epson/Geometry.cpp`. L'ordre 320, 400, Booter, 399, 396, Logo, les seuils 1/4/6 et les dispositions des pistes correspondent au C#.
- Le flux fichier à accès borné est remplacé par `Sources/RandomAccessSources.cpp` : chaque lecture exacte ouvre un flux local RAII, le ferme également sur exception et ne laisse pas de descripteur persistant entre les lectures du décodeur.
- `ScpReconstructionExceptions.MissingTrackSectors` ne possède aucun appel qualifié dans le C# courant. Il n'est pas nécessaire d'ajouter un chemin d'erreur inactif pour reproduire le nom de cette fabrique ; les diagnostics utilisés restent suivis séparément.
