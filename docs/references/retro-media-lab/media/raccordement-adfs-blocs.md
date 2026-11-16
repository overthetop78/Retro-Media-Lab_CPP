# Raccordement des volumes ADFS

Le C# `AcornAdfsFileSystemReader.Blocks.cs` distingue une image sectorielle entière et un volume par blocs adressé en octets. La vue C++ `Adfs/BlockView.cpp` existait, mais `Composition/Catalog.cpp` enregistrait ADFS dans l’adaptateur sectoriel générique. Celui-ci refusait les représentations par blocs : le raccordement manquait réellement.

`AdfsDocumentReader` est maintenant enregistré au même emplacement et avec les mêmes identifiants. Il conserve la branche sectorielle commune et délègue la branche par blocs aux fonctions ADFS existantes. La source est empruntée uniquement pendant la lecture synchrone, les entrées retournées possèdent leur contenu. Les lectures restent unitaires ; le HDD entier n’est pas chargé.

Compilation native réussie. Le contrôle ciblé a obtenu le lecteur depuis le registre de l’explorateur, reconnu puis lu un volume ADFS commençant à l’octet 512, vérifié des lectures de 256 octets et le refus des volumes mal alignés ou hors capacité, puis l’annulation. Il n’a pas retesté le décodage ADFS. Contrôle temporaire supprimé.

Les onze autres mentions anciennes de registre/documents restants ont été rapprochées de `Catalog.cpp`, `CatalogConstants.h`, `Document.cpp` et `MediaExplorer.cpp`. Les lecteurs, identifiants de formats et adaptateurs sectoriels y étaient déjà intégrés. Leurs algorithmes n’ont pas été réécrits.

`FileSystemWarningMessages` est remplacé par les diagnostics typés et leur contexte dans les trois consommateurs actifs ADFS/Amiga. Le helper C# `MissingDataBlocks` n’a aucun consommateur ; aucun helper inutilisé n’a été ajouté pour reproduire un nom de fichier.
