# Définitions intégrées et arguments

Références C# : `Images/Formats/BuiltInDiskDefinitions.cs`, `Contracts/Options/EnabledOption.cs` et `RetroMediaLab.App/Assets/DiskDefinitions/built-in.cfg`.

Le contrat public C++ prépare les arguments pour atarist.810, amstrad.cpc, amstrad.pcw et ucsd.ibm.mfm. Il respecte une option `--diskdefs` explicite, sans distinction de casse. Le répertoire de l'application est fourni par l'appelant ; le moteur ne cherche pas le processus hôte et ne lance aucun outil matériel.

La sélection des formats est commune aux arguments, au catalogue des capacités et à la visualisation. Le CFG est distribué sous `Assets/DiskDefinitions/built-in.cfg` au build et à l'installation : un outil externe a besoin d'un chemin de fichier. Les quatre définitions ont été reprises sans modification du C# ; empreinte SHA256 source, fichier du projet et fichier livré identique : `7627a7f3efe22887a73dfccfb06dfa4d1979b0e039073ee961b76ead34f0c51b`.

Compilation native réussie. Le contrôle ciblé a vérifié les quatre formats, la casse, le format inconnu, la priorité de l'option explicite et les quatre identifiants lus dans le CFG livré. Le contrôle initial comparait les séparateurs de chemin comme du texte ; il a été corrigé pour comparer les chemins. Aucun défaut du code de production n'a été trouvé par ce contrôle. Fichiers de contrôle supprimés.
