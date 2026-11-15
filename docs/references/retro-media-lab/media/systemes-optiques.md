# Exploration optique C# → C++

Sources locales C# consultées en lecture seule le 15 novembre 2026, empreintes conservées ci-dessous.

Les trois variantes ISO partagent les descripteurs, valeurs dans les deux endianness, parcours d'étendues et dates. Joliet remplace le décodage des noms par UCS-2/UTF-16BE ; Rock Ridge recherche ER puis concatène NM, avec repli Latin-1 uniquement si UTF-8 est invalide. Les conversions Unicode préservent un BOM initial et traitent les séquences finales comme un décodage complet.

UDF conserve les révisions 1.02, 1.50, 2.00, 2.01, 2.50 et 2.60 du C#, une partition physique directement mappée, les descripteurs courts/longs/intégrés et les mêmes refus explicites pour les variantes non prises en charge. Les tags, leur position, somme de contrôle et CRC sont vérifiés avant consommation. Les horodatages conservent le fuseau et les fractions, y compris leur report sur la seconde suivante.

XDVDFS utilise des secteurs absolus, convertis en adresses relatives pour la piste. ISO et UDF utilisent leurs adresses relatives comme dans le C#. Le parcours binaire gauche/entrée/droite est conservé.

Deux protections explicites supplémentaires évitent de bloquer sur des données corrompues : ISO rejette un secteur vide au lieu d'un parcours sans progression ; XDVDFS suit les étendues actives entre répertoires enfants et borne leur profondeur, car le C# ne contrôle que les pointeurs dans chaque table locale. Les erreurs supplémentaires ont leurs clés traduisibles.

La composition par défaut est complète pour ce service : 21 lecteurs sectoriels, adaptateurs fournis par l'appelant, puis Amstrad, console, Famicom, GameCube, XDVDFS, UDF, Joliet, Rock Ridge et ISO9660. Aucun nom de lecteur absent n'est publié et App ne dépend pas de ces lecteurs.

## Empreintes

- `src/RetroMediaLab.MediaFileSystems/FileSystems/Iso9660/Iso9660FileSystemReader.cs` : `27e8f2626e999a2296e969dd4e3f9c468c1b98c23ec96df111fd22a1fbfdebbf` → `src/MediaFileSystems/src/FileSystems/Iso9660/Reader.cpp`, `src/MediaFileSystems/src/FileSystems/Optical/Common.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Iso9660/JolietExtensionReader.cs` : `bc33422ae292dd7a67fe77fa451461104c51c4127a98c1f0e9622f8f262ec881` → `src/MediaFileSystems/src/FileSystems/Iso9660/Names.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Iso9660/RockRidgeExtensionReader.cs` : `8591bb1695a7be31804fca9ace8c7d50afe627787d744a857f880e014a91793a` → `src/MediaFileSystems/src/FileSystems/Iso9660/Names.cpp`
- `src/RetroMediaLab.MediaFileSystems/Constants/Iso9660Constants.cs` : `83e83eadd1bd0191822b09e0386ac3d7335441bc1c4df3090f576fbf96f825f9` → `src/MediaFileSystems/src/FileSystems/Iso9660/Constants.h`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Xbox/Xdvdfs/XdvdfsFileSystemReader.cs` : `32f957d2f86e03374e86b44f1e0a2fa803a80bdc3540c77e70edaffa757ad900` → `src/MediaFileSystems/src/FileSystems/Xbox/Xdvdfs/Reader.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Xbox/Xdvdfs/XdvdfsFileSystemConstants.cs` : `696c167f48beae14a38289076fe8ab4dd9e7cf290c91d940ac65336da96b088a` → `src/MediaFileSystems/src/FileSystems/Xbox/Xdvdfs/Constants.h`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Udf/UdfFileSystemReader.cs` : `ca1fe75d28eda7039e481efe9c26a907f359866de7648d51213070153a58c012` → `src/MediaFileSystems/src/FileSystems/Udf/Reader.cpp`, `src/MediaFileSystems/src/FileSystems/Udf/Entries.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Udf/UdfDescriptorValidator.cs` : `54a28948b961b8117352c34a10b91e312093724f00d27d756123d8c9db8844ed` → `src/MediaFileSystems/src/FileSystems/Udf/Descriptors.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Udf/UdfNameDecoder.cs` : `4396bbdcb50ab688828e56720295da8d4a92781e9d41ea05cb9dfa828d96fca9` → `src/MediaFileSystems/src/FileSystems/Udf/Descriptors.cpp`
- `src/RetroMediaLab.MediaFileSystems/Constants/UdfConstants.cs` : `2764c47c3e0e6c5d7708513a33c2ac055d158818a79c3bdd0505e542d8699e6e` → `src/MediaFileSystems/src/FileSystems/Udf/Constants.h`
- `src/RetroMediaLab.MediaFileSystems/Exploration/MediaExplorer.cs` : `668aba9a8cfd216bfe7e357238dbc8b6cf4f204bb43756d58086e0c0148bcb39` → `src/MediaFileSystems/src/Exploration/MediaExplorer.cpp`, `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MediaExplorer.h`
