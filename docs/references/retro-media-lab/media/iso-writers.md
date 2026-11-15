# ISO et registre des writers

Sources C# consultées en lecture seule. Le format optical-iso admet les extensions .iso et .gcm, exige une longueur positive multiple de 2048 et décrit une piste continue Mode1Data2048. La sonde ne vérifie pas ISO 9660 : le système de fichiers est identifié ensuite par MFS. Le lecteur vérifie que la longueur réelle correspond à la reconnaissance et conserve la casse de l’extension source dans les métadonnées.

Le writer utilise la sortie atomique partagée et le profil C# : une piste, une session, aucun nombre explicite de couches/faces, aucun sous-canal, gap, flag, catalogue ou ISRC ; index éventuels tous égaux à 1 à l’adresse relative zéro. Les images non compatibles sont refusées avant écriture.

Correction volontaire par rapport au C# : le prédicat refuse également StoredPregapSectors > 0. Dans le C#, la validation du descripteur autorise un prégap stocké sans PregapSectors déclaré, mais IsoWriter ne teste que PregapSectors et commence l’écriture au secteur relatif zéro ; les octets du prégap stocké seraient donc perdus. Seul le C++ est corrigé. Le banc construit ce cas et vérifie son refus.

Le registre conserve ordre et propriété des writers, refuse les références nulles et doublons d’identifiant, sépare SupportsTarget de CanWrite et filtre la représentation avant le premier writer compatible. La normalisation met l’extension en minuscules et ajoute le point sans supprimer les espaces, comme NormalizeCore C#. Les comparaisons ordinales utilisent la primitive partagée déjà présente.

| Source | SHA-256 vérifié |
| --- | --- |
| `src/RetroMediaLab.MediaEngine/Functions/MediaFileExtensionFunctions.cs` | `17d5ebe7b1af8234e9f1ae564361e65b82bb7276867ed38ebca27277bb96865b` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Optical/Iso/IsoFormat.cs` | `c7295e4d11398ed9607224e70b705e5632d0352dbbb66cd8d773f21ede553119` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Optical/Iso/IsoReader.cs` | `aaf64861ed81b8b8489d30501e058ea3704628169172779019297f4e7daa3cc1` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Optical/Iso/IsoWriter.cs` | `3aa8d9686aef3de3d549af20910dfee0a3838f17977c3ed079c8e56a47eeba70` |
| `src/RetroMediaLab.MediaEngine/Images/Writing/MediaImageWriterRegistry.cs` | `12167af1390fa46c4de8cbc4e2017e7620301c043c83f52fc7bebacdaedf5610` |
