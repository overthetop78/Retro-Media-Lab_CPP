# Référence C# des bibliothèques médias

Relevé du 15 novembre 2026. Dépôt `F:/GW GUI` consulté en lecture seule. HEAD : `476fc87797d389690634deff6eb45681e724eede`. Les empreintes SHA-256 de l’inventaire identifient le contenu local de chaque fichier ; le seul HEAD ne suffit pas si des modifications sont présentes.

| Bibliothèque | Fichiers C# hors bin/obj | Lignes |
|---|---:|---:|
| MediaEngine | 1083 | 65720 |
| MediaFileSystems | 372 | 16087 |
| MediaAnalysis | 37 | 2199 |

L’[inventaire](inventaire-csharp.csv) suit chaque fichier source et ses cibles C++. MediaAnalysis est porté et comparé au C# ; MediaEngine et MediaFileSystems sont commencés pour les sources fichier/mémoire et partitions par blocs ; les modèles sectoriels et la famille FAT12 avec analyse des entrées sont aussi portés dans le bloc 03 ; les autres familles demeurent non portées. Les empreintes des 37 sources MediaAnalysis ont été revérifiées avant validation.

## Fichiers examinés

- `MediaEngine/MediaEngineComposition.cs` : composition reconnaissance, lecture, écriture, conversion, visualisation et exploration, avec adaptation du décodage séquentiel.
- `MediaEngine/Images/Reading/MediaExplorer.cs` : transmission du document décodé à MediaFileSystems et adaptation du résultat exposé à App.
- `MediaFileSystems/Exploration/MediaExplorer.cs` : validation des lecteurs enregistrés, détecteurs de volumes et lecteurs de systèmes de fichiers.
- `MediaFileSystems/Interfaces/IMediaRandomAccessData.cs` : longueur, lectures exactes bornées et annulation.
- `MediaAnalysis/Interfaces/IMediaContentClassifier.cs` et les trois fichiers `Functions/` : classification de contenus extraits, priorités Primary/Standard/Fallback, signatures et groupes ordonnés, recherche depuis le début ou la fin, longueurs et reconnaissance textuelle.
- Les trois `.csproj` : MediaEngine référence MediaFileSystems, qui référence MediaAnalysis. MediaEngine compile également quatre sources de contrats/enum appartenant à MediaFileSystems via des liens de compilation. Ses paquets déclarés sont K4os.Compression.LZ4 et NAudio ; leurs usages devront être examinés dans les blocs concernés avant de sélectionner les équivalents C++.
- `docs/architecture/media-library-boundaries.md` et `docs/tasks/media-library-separation.md` dans le dépôt C# : responsabilités et trajet de l’exploration/migration. Les directives du portage restent prioritaires.

Les noms ci-dessus abrègent le préfixe `src/RetroMediaLab.`. Ce relevé ne constitue pas une validation des algorithmes ni de tous les formats inventoriés.

## Unicode et libellés

Le classifieur C# utilise ToLowerInvariant et des comparaisons ordinales indépendantes de la culture. Référence officielle : [Microsoft, changements de casse indépendants de la culture](https://learn.microsoft.com/en-us/dotnet/core/extensions/performing-culture-insensitive-case-changes), consultée le 15 novembre 2026. Une comparaison directe a révélé une différence entre QString::toLower et ToLowerInvariant pour U+0130 ; le portage conserve ce caractère et utilise une conversion simple par scalaire. Les identifiants natifs connus sont des étiquettes ASCII comparées sans assimilation de caractères Unicode ressemblants.

Les 25 clés de présentation ont été reprises depuis les Explorer.resx de Resources/00-Base et des 29 langues. ROM reste un invariant dans Constants. Les catalogues se trouvent sous le domaine MediaAnalysis ; sa constante publique est Localization::Domain. Les algorithmes ne chargent pas les traductions. Deux libellés ont été revus : japonais « 実行ファイル » obtenu avec Argos ; français « Document » conservé après rejet du résultat Argos « Numéro », incorrect dans ce contexte. Aucun modèle Argos n’a été installé ni modifié.

## Secteurs et FAT12

Les 27 fichiers de FileSystems/Fat12, FileSystemEntryAnalyzer, MediaFileSystemFamilyResolver et les contrats nécessaires ont été consultés directement. 43 fichiers C# concernés ont été revérifiés par SHA-256 contre l’inventaire ; aucune modification du dépôt C# n’a été faite. Le détail de la réorganisation et des différences maîtrisées est dans [secteurs-fat12.md](secteurs-fat12.md).

## Images brutes IBM/MSX

Le bloc 04 ajoute les profils et détections IBM/MSX, les lecteurs sur sources paresseuses et les writers linéaires atomiques, avec exploration automatique FAT12/analyse. Les lecteurs restent partiels pour leur enregistrement et les documents de reconnaissance. [Référence et limites](images-brutes-ibm-msx.md), [validation](../../../validation/media/images-brutes-ibm-msx.md). Les 22 sources C# concernées ont conservé leur SHA-256 d’inventaire.

## BBC DFS

Le bloc 05 porte les six sources du système de fichiers BBC DFS et les sept sources des conteneurs associés. Les 13 SHA-256 C# ont été contrôlés contre l'inventaire sans changement de source. Les règles Atom/JFD, les profils SSD/DSD et les alias IMG/IMA/ADL sont documentés dans [bbc-dfs.md](bbc-dfs.md). L'enregistrement des lecteurs et les documents restent partiels.

## FileCore

Le bloc 06 porte les cartes d'allocation communes Acorn avant le parcours ADFS : onze sources FileCore, leur interface et le codec de noms ADFS, soit 13 empreintes C# inchangées. [Organisation et différences explicites](filecore.md), [validation](../../../validation/media/filecore.md). ADFS et ses conteneurs restent à compléter.

## ADFS

Le bloc 07 ajoute l'exploration Hugo/Nick, fichiers, horodatages RISC OS, création d'en-têtes BBC/D et adaptation paresseuse de volumes par blocs. Douze sources ADFS/date conservent leur SHA-256. [Organisation et renforcements](adfs.md), [validation](../../../validation/media/adfs.md). Les lecteurs restent partiels pour l'enregistrement et les contrats de documents ; les conteneurs Acorn ADF seront le bloc MediaEngine suivant.

## Conteneurs ADF

Le bloc 08 porte les huit profils Acorn/Amiga associés au lecteur ADF, sa reconnaissance, ses candidats partiels et les writers communs ; neuf empreintes C# inchangées. Les trois clés d'erreur existent dans les 29 langues. L'exploration Acorn rejoint les bibliothèques déjà portées. [Référence](adf.md), [validation](../../../validation/media/adf.md). Le codec MFM, les documents applicatifs et l'enregistrement restent à porter ; AmigaDOS est décrit ci-dessous.

## AmigaDOS

Le bloc 09 porte les vingt sources AmigaDos, vérifiées par empreinte : huit variantes en lecture, récupération et création OFS/FFS. MediaEngine raccorde leur exploration ADF à l’analyse. [Référence et renforcements](amigados.md), [validation](../../../validation/media/amigados.md). Les archives plates Amiga, documents, registres et codecs restent à porter.

## Archives plates Amiga

Le bloc 10 porte les huit sources AmigaFlatResource, vérifiées par empreinte, et leur raccordement ADF après AmigaDOS. Tables, plages, anomalies et textes sont comparés au C#. [Référence](amiga-flat-archive.md), [validation](../../../validation/media/amiga-flat-archive.md). Les codecs de flux, documents et registres restent à porter.

## COHERENT

Le bloc 11 porte les quinze sources du système de fichiers COHERENT : superbloc, inodes, noms/dates, données directes/indirectes et parcours. Les empreintes sont inchangées. [Référence et renforcements](coherent.md), [validation](../../../validation/media/coherent.md). Le conteneur brut zoné Commodore 900, codecs et registres restent à porter.

## Conteneur brut Commodore 900

Le bloc 12 porte les cinq sources de géométrie zonée, reconnaissance, lecture/reconstruction et écriture atomique du dump COHERENT. Empreintes inchangées. L’exploration rejoint COHERENT puis l’analyse. [Référence](commodore900-raw.md), [validation](../../../validation/media/commodore900-raw.md). Registres, documents, conversion de fichiers et codecs GCR restent à porter.

## Primitives de flux et GCR partagé

Le bloc 13 porte dix sources de modèles, estimation, reconstruction adaptative/PLL, révolution, codec GCR commun/XOR et confiance. Empreintes inchangées. [Référence](flux-core.md), [validation](../../../validation/media/flux-core.md). Le codec de piste Commodore 900, les structures décodées et les conteneurs/registres de flux restent à porter.

## Codec de piste Commodore 900 GCR

Le bloc 14 porte l’encodeur/décodeur GCR et leurs définitions, les modèles possédés de décodage et le socle/primitives d’encodage. Vingt-deux empreintes inchangées ; les aides communes non consommées et les contrats génériques restent marqués partiels. [Référence](commodore900-gcr.md), [validation](../../../validation/media/commodore900-gcr.md). Le raccordement des fichiers de flux, registres, documents et conversions reste à porter.

## Lecture SCP et reconstruction Commodore 900

Le bloc 15 porte parsing borné, checksum et modèles SCP, fenêtres successives, choix d’intégrité stable et reconstruction Commodore 900. Les 47 sources du banc ont des empreintes inchangées. [Référence](scp-reader.md), [validation](../../../validation/media/scp-reader.md). Les autres familles, writer SCP, adaptateurs de documents/cache et registres restent à porter ; leurs lignes d’inventaire restent partielles ou non portées.

## Écriture SCP et flux synthétiques

Le bloc 16 porte le writer SCP, ses diagnostics/constantes et la conversion des révolutions d’encodeur vers les ticks SCP. Les octets et conversions sont comparés aux sources C# originales ; 16 allers-retours GCR/SCP/secteurs couvrent les zones Commodore 900 et les deux faces. [Référence](scp-writer.md), [validation](../../../validation/media/scp-writer.md). Documents, registres, captures et autres familles restent à porter.

## Géométries Commodore et conteneurs D64/D71/D81

Le bloc 17 porte les géométries partagées, neuf dispositions, reconnaissance par longueur/format, lecture différée avec conservation des cartes d’erreurs et writer D81. Les 25 sources du banc ont des empreintes inchangées. [Référence](commodore-disks.md), [validation](../../../validation/media/commodore-disks.md). Les documents/registres, Commodore DOS et autres reconstructions SCP restent à porter.

## Lecture Commodore DOS

Le bloc 18 porte reconnaissance, répertoires, fichiers, BAM, PETSCII et exploration/analyse D64/D71/D81. Les 27 sources C# de référence sont inchangées. [Référence](commodore-dos.md), [validation](../../../validation/media/commodore-dos.md). Le booléen d’espace libre inconnu corrige explicitement le défaut C# ; l’écriture/migration et les constantes propres aux writers restent à porter.

## Construction Commodore DOS

Le bloc 19 porte politiques, géométries standard et construction sectorielle des volumes Commodore DOS, avec chaînes/REL, répertoires et BAM. Les 19 sources C# du banc ont des empreintes inchangées. [Référence](commodore-dos-writer.md), [validation](../../../validation/media/commodore-dos-writer.md). L’orchestration de migration, les registres/documents et le reste du portage demeurent à réaliser.

## Écriture des conteneurs D64/D71

Le bloc 20 complète le writer commun D64/D71 et la conservation des cartes de diagnostics, situé dans un autre dossier C# que les lecteurs. Dix sources de référence ont des empreintes inchangées. [Référence](commodore-container-writer.md), [validation](../../../validation/media/commodore-container-writer.md). Les services de conversion SCP et l’orchestration restent à porter.

## Codec GCR Commodore 1541/1571

Le bloc 21 porte l’encodeur de pistes, la collecte/appairage du décodeur et leurs descriptions localisables, en réutilisant les primitives communes. Les 36 sources C# du banc ont des empreintes inchangées. [Référence](commodore-gcr.md), [validation](../../../validation/media/commodore-gcr.md). Reconstruction SCP, registres/documents et conversions restent à porter.

## Reconstruction SCP Commodore GCR

Le bloc 22 porte la branche de reconstruction 1541/1571 : sélection stable, géométrie depuis tous les candidats, fenêtres SCP et progression. Cinquante sources C# du banc ont des empreintes inchangées. [Référence](commodore-scp-gcr.md), [validation](../../../validation/media/commodore-scp-gcr.md). La branche 1581 MFM, le routage général, les conversions et registres/documents restent à porter.

## Encodage ISO MFM et primitives CRC/taille

Le bloc 23 porte l’encodeur ISO MFM logique/physique complet, ses métadonnées enregistrées, le CRC 16 configurable et les codes de taille communs. Les 21 sources du banc ont des empreintes inchangées. [Référence](iso-mfm-encoding.md), [validation](../../../validation/media/iso-mfm-encoding.md). Le décodeur/sélecteur PLL MFM et la reconstruction 1581 restent à porter.

## Décodage ISO MFM

Le bloc 24 porte le lecteur binaire commun, la collecte/lecture des champs MFM et CRC, l’appairage circulaire, les métadonnées, descriptions et sélection stable PLL. Les 37 sources du banc ont des empreintes inchangées. [Référence](iso-mfm-decoding.md), [validation](../../../validation/media/iso-mfm-decoding.md). Reconstruction SCP 1581, registres/documents et routage restent à porter.

## Reconstruction SCP Commodore 1581

Le bloc 25 porte la branche MFM 1581, la sélection stable des candidats et leur répartition logique D81. Les 51 sources du banc ont des empreintes inchangées. [Référence](commodore-scp-mfm.md), [validation](../../../validation/media/commodore-scp-mfm.md). Le routage général, entrées fichier/orchestration, registres et documents restent à porter.

## Routage Commodore SCP

Le bloc 26 complète le dispatch 1581/900/GCR et l’entrée fichier par les sources/lecteur SCP communs. Sept sources C# de référence ont des empreintes inchangées. [Référence](commodore-scp-routing.md), [validation](../../../validation/media/commodore-scp-routing.md). Registres/documents, conversions et orchestration restent à porter.

## Encodage ISO FM

Le bloc 27 porte l’encodeur FM logique/physique, ses marques, champs et CRC enregistrés. Les 23 sources du banc ont des empreintes inchangées. [Référence](iso-fm-encoding.md), [validation](../../../validation/media/iso-fm-encoding.md). Décodeur FM, reconstruction ISO, registres/documents et conversions restent à porter.

## Décodage ISO FM

Le bloc 28 porte reconnaissance, champs/horloges/CRC, métadonnées enregistrées, appairage et décodages adaptatif/à période connue. Les 36 sources du banc ont des empreintes inchangées. [Référence](iso-fm-decoding.md), [validation](../../../validation/media/iso-fm-decoding.md). Reconstruction ISO, registres/documents et conversions restent à porter.

## Socle de construction sectorielle ISO

Le bloc 29 porte candidats/groupes ordonnés, sélection des meilleures données, mesure majoritaire, construction uniforme et normalisation. Seize sources C# de référence ont des empreintes inchangées. [Référence](iso-sector-builder.md), [validation](../../../validation/media/iso-sector-builder.md). Collecte/cache, politiques par famille, registres/documents et conversions restent à porter.

## Modèle de pistes protégées

Le bloc 30 porte les modèles de pistes/images protégées, leurs adaptateurs SCP/encodage et les fenêtres de décodage associées. Trente sources C# de référence ont des empreintes inchangées. [Référence](protected-tracks.md), [validation](../../../validation/media/protected-tracks.md). Collecte/cache ISO, politiques, représentations/documents et conversions restent à porter.

## Collecte des candidats ISO

Le bloc 31 porte score, sélection par fenêtre, collecte physique/adressée et orchestration parallèle depuis une image protégée. Trente-cinq sources de référence ont des empreintes inchangées. [Référence](iso-collection.md), [validation](../../../validation/media/iso-collection.md). IsoScpCandidateDecoder reste partiel : cache et entrée fichier à porter, puis politiques, registres/documents et conversions.

## Cache et entrée fichier ISO

Le bloc 32 complète IsoScpCandidateDecoder : empreintes de captures, partage des opérations, réessais conditionnels et entrée SCP réelle. Trente-cinq sources C# de référence ont des empreintes inchangées. [Référence](iso-capture-cache.md), [validation](../../../validation/media/iso-capture-cache.md). Registre général, politiques par famille, exploration, documents et conversions restent à porter.

## Registre de décodeurs de flux

Le bloc 33 porte le registre explicite, les scores automatique/explicite, les choix stables et le cache faible des révolutions, avec intégration à la collecte ISO. Douze sources C# de référence ont des empreintes inchangées. [Référence](flux-decoder-registry.md), [validation](../../../validation/media/flux-decoder-registry.md). Le constructeur/catalogue par défaut de 25 codecs, les codecs manquants, politiques, exploration, documents et conversions restent à porter.

## Analyse du flux brut et classes de codecs

Le bloc 34 porte RawFluxDecoder et ses anomalies temporelles, et relie au registre les classes des quatre codecs déjà portés. Quinze sources C# de référence ont des empreintes inchangées. [Référence](raw-flux-analysis.md), [validation](../../../validation/media/raw-flux-analysis.md). Amiga et les autres codecs, le catalogue complet, les politiques, l’exploration, les documents et conversions restent à porter.

## Codec Amiga MFM

Le bloc 35 porte les primitives, définitions, encodeur et décodeur Amiga MFM, avec validation de 35 sources originales et intégration au registre/collecte SCP. [Référence](amiga-mfm-codec.md), [validation](../../../validation/media/amiga-mfm-codec.md). Les plages de parité C# en débordement sont refusées explicitement avant accès en C++. Les autres codecs, le catalogue complet, les politiques de reconstruction, l’exploration, les documents et conversions restent à porter.

## Primitives Apple II GCR

Le bloc 36 porte AppleIIGcrCodec intégralement et AppleBitLatch, avec six sources originales vérifiées. AppleIIGcrFormat reste partiel en attendant les paramètres et codecs de piste. [Référence](appleii-gcr-primitives.md), [validation](../../../validation/media/appleii-gcr-primitives.md). Les codecs manquants, catalogue complet, politiques de reconstruction, exploration, documents et conversions restent à porter.

## Codec de piste Apple II GCR

Le bloc 37 complète AppleIIGcrTrackEncoder, AppleIIGcrDecoder et AppleIIGcrFormat. Trente-six sources originales ont des empreintes inchangées ; les 1 276 cas comparés passent en Debug/Release. [Référence](appleii-gcr-track.md), [validation](../../../validation/media/appleii-gcr-track.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Charge utile Apple RWTS18

Le bloc 38 porte AppleRwts18Codec intégralement, avec neuf sources originales vérifiées et 5 492 cas comparés en Debug/Release. AppleRwts18Format reste partiel. [Référence](rwts18-payload.md), [validation](../../../validation/media/rwts18-payload.md). Les codecs de piste RWTS18, autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec de piste RWTS18

Le bloc 39 complète AppleRwts18TrackEncoder, AppleRwts18Decoder et AppleRwts18Format. Trente-sept sources originales ont des empreintes inchangées ; 1 206 cas comparés passent en Debug/Release. [Référence](rwts18-track.md), [validation](../../../validation/media/rwts18-track.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Primitives Apple IWM GCR

Le bloc 40 porte AppleIwmGcrCodec intégralement, avec huit sources originales vérifiées et 5 917 cas comparés en Debug/Release. AppleIwmGcrFormat reste partiel. [Référence](iwm-gcr-primitives.md), [validation](../../../validation/media/iwm-gcr-primitives.md). Les codecs de piste Macintosh/Lisa, autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Pistes IWM Macintosh/Lisa

Le bloc 41 complète AppleIwmGcrDecoder, les deux décodeurs/encodeurs Macintosh et Lisa, AppleIwmGcrFormat et la constante par défaut Lisa. Quarante sources originales ont des empreintes inchangées ; 886 cas comparés passent en Debug/Release. [Référence](iwm-gcr-track.md), [validation](../../../validation/media/iwm-gcr-track.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec Victor 9000 GCR

Le bloc 42 porte Victor9kGcrTrackEncoder, Victor9kGcrDecoder et Victor9kGcrFormat, avec ses sommes et modèles. Trente-cinq sources originales ont des empreintes inchangées ; 454 cas comparés passent en Debug/Release. [Référence](victor9000-gcr.md), [validation](../../../validation/media/victor9000-gcr.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec Micral N FM

Le bloc 43 porte MicralNFmTrackEncoder, MicralNFmDecoder et MicralNFmFormat, avec checksum et modèle. Trente-quatre sources originales ont des empreintes inchangées ; 478 cas comparés passent en Debug/Release. [Référence](micraln-fm.md), [validation](../../../validation/media/micraln-fm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec E-mu FM

Le bloc 44 porte EmuFmTrackEncoder, EmuFmDecoder, EmuFmFormat et EmuFmDescriptions (identité incluse). Trente-six sources originales ont des empreintes inchangées ; 473 cas comparés passent en Debug/Release. [Référence](emu-fm.md), [validation](../../../validation/media/emu-fm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec Micropolis MFM

Le bloc 45 porte MicropolisMfmTrackEncoder, MicropolisMfmDecoder et MicropolisMfmFormat, avec records et checksum. Trente-quatre empreintes sources sont inchangées ; 873 cas comparés passent en Debug/Release. [Référence](micropolis-mfm.md), [validation](../../../validation/media/micropolis-mfm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec NorthStar MFM et secteurs matériels

Le bloc 46 porte NorthstarMfmTrackEncoder, NorthstarMfmDecoder, NorthstarMfmFormat et RotatingChecksumCalculator, avec les deux branches de lecture par fenêtres FM/MFM. Trente-cinq empreintes originales sont inchangées ; 1 331 cas comparés passent en Debug/Release. [Référence](northstar-mfm.md), [validation](../../../validation/media/northstar-mfm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec Heathkit FM

Le bloc 47 porte les cinq fichiers Heathkit (format, codec, descriptions, encodeur et décodeur) et partage l’inversion d’octet avec E-mu. Trente-sept empreintes originales sont inchangées ; 1 297 cas comparés passent en Debug/Release. [Référence](heathkit-fm.md), [validation](../../../validation/media/heathkit-fm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Codec Data General 2F

Le bloc 48 porte les cinq fichiers Data General (format, checksum, descriptions, encodeur et décodeur). Trente-six empreintes originales sont inchangées ; 1 327 cas comparés passent en Debug/Release. [Référence](datageneral-fm.md), [validation](../../../validation/media/datageneral-fm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Primitives RX02 M²FM

Le bloc 49 porte DecRx02M2FmCodec et FmAddressMarkPatterns, ainsi que les définitions/sélections de marques et tailles de DecRx02Format. Quinze empreintes originales sont inchangées ; 5 434 cas passent en Debug/Release avec neuf différences explicites de validation de bornes et aucun écart inattendu. [Référence](rx02-m2fm.md), [validation](../../../validation/media/rx02-m2fm.md). Les pistes RX02, descriptions, autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Pistes DEC RX01/RX02

Le bloc 50 porte DecRx02TrackEncoder, DecRx02Decoder et DecRx02Descriptions et complète DecRx02Format du bloc 49. Quarante et une empreintes originales sont inchangées ; 929 cas comparés passent en Debug/Release sans écart. [Référence](rx02-tracks.md), [validation](../../../validation/media/rx02-tracks.md). Les dumps/ordres sectoriels RX02, autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Pistes HP MMFM

Le bloc 51 porte les cinq fichiers HP MMFM (format, codec, descriptions, encodeur et décodeur). Trente-sept empreintes originales sont inchangées ; 1 086 cas comparés passent en Debug/Release sans écart. [Référence](hp-mmfm.md), [validation](../../../validation/media/hp-mmfm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Pistes Centurion MFM

Le bloc 52 porte les quatre fichiers Centurion (format, descriptions, encodeur, décodeur). Trente-six empreintes originales sont inchangées ; 612 cas comparés passent en Debug/Release sans écart. [Référence](centurion-mfm.md), [validation](../../../validation/media/centurion-mfm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Pistes AED 6200P MFM

Le bloc 53 porte les trois fichiers AED 6200P (format, encodeur et décodeur). Trente-six empreintes originales sont inchangées ; 1 204 cas comparés passent en Debug/Release sans écart. [Référence](aed6200p-mfm.md), [validation](../../../validation/media/aed6200p-mfm.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Blocs Arburg système/données

Le bloc 54 porte les cinq fichiers Arburg (format, checksum, transformation système, encodeur et décodeur). Trente-six empreintes originales sont inchangées ; 1 209 cas comparés passent en Debug/Release, avec les gardes d’arguments invalides documentées et aucun écart inattendu. [Référence](arburg.md), [validation](../../../validation/media/arburg.md). Les autres codecs, catalogue complet, politiques, exploration, documents et conversions restent à porter.

## Pistes Membrain MFM

Le bloc 55 porte les trois fichiers Membrain. Trente-cinq empreintes originales inchangées ; 934 cas et deux lots exhaustifs d’adresses passent en Debug/Release sans écart. [Référence](membrain-mfm.md), [validation](../../../validation/media/membrain-mfm.md). Les autres codecs et services restent à porter.

## Pistes QD MO5 MFM

Le bloc 57 porte les trois fichiers QD MO5. Trente-quatre empreintes originales inchangées ; 1 064 cas passent en Debug/Release sans écart. [Référence](qd-mo5-mfm.md), [validation](../../../validation/media/qd-mo5-mfm.md). Tycom, catalogue complet, politiques et autres services restent à porter.

## Pistes TYCOM FM

Le bloc 58 porte les trois fichiers TYCOM. Trente-six empreintes originales inchangées ; 1 456 cas passent en Debug/Release sans écart. [Référence](tycom-fm.md), [validation](../../../validation/media/tycom-fm.md). Le catalogue complet, les politiques et les autres services restent à porter.

## Composition des codecs de flux

Le bloc 59 porte les deux catalogues, complète les identités/noms et le constructeur de registre par défaut. Cinq empreintes source inchangées ; 370 contrôles passent en Debug/Release. La participation automatique Lisa est corrigée. [Référence](composition-codecs-flux.md), [validation](../../../validation/media/composition-codecs-flux.md). FluxEncoderRegistry, politiques et autres services restent à porter.

## Registre d’encodage des pistes

Le bloc 60 porte FluxEncoderRegistry et ses diagnostics, et corrige la validation UTF-8 commune aux deux registres pour préserver U+FEFF. Huit empreintes originales inchangées ; 281 cas et 453 contrôles passent dans chaque configuration Debug/Release. [Référence](registre-encodage-flux.md), [validation](../../../validation/media/registre-encodage-flux.md). Les politiques, la reconstruction des images et les autres services restent à porter.

## Cadences standard et profils rares

Le bloc 61 porte les règles de cadence sectorielle et les quatorze profils rares, sans dupliquer les codecs. Vingt et une empreintes originales inchangées ; 1 559 cas et 1 601 contrôles passent dans chaque configuration Debug/Release. La comparaison des identifiants ASCII est corrigée pour ne pas assimiler des caractères Unicode. [Référence](cadences-profils-sectoriels.md), [validation](../../../validation/media/cadences-profils-sectoriels.md). Les politiques de transformation des secteurs et la reconstruction complète restent à porter.

## Politiques de reconstruction sectorielle

Le bloc 62 porte les politiques, leur contrat et le registre ordonné, avec conservation des métadonnées et des cadences zonées. Cinquante-neuf empreintes originales inchangées ; 2 688 cas et 2 985 contrôles passent dans chaque configuration Debug/Release. [Référence](politiques-sectorielles.md), [validation](../../../validation/media/politiques-sectorielles.md). L’orchestrateur SectorImageTrackEncoder et les autres services restent à porter.

## Encodage des images sectorielles

Le bloc 63 raccorde SectorImageTrackEncoder aux registres et politiques, avec ordre physique stable, unités distinctes et annulation. Quatre-vingt-cinq empreintes originales inchangées ; 1 333 cas et 1 456 contrôles passent dans chaque configuration Debug/Release. Les parcours AvailableBlocks et les gardes UTF-8 sont corrigés. [Référence](encodage-images-sectorielles.md), [validation](../../../validation/media/encodage-images-sectorielles.md). Les services de visualisation/conversion et les autres familles restent à porter.

## Visualisation et conversion sectorielles vers SCP

Le bloc 64 porte SectorImageFluxVisualizer, SectorImageScpConversionService et le catalogue de types SCP, en réutilisant les encodeurs et le writer. Cent neuf empreintes originales inchangées ; 1 097 cas et 1 110 contrôles passent dans chaque configuration Debug/Release. [Référence](images-scp-sectorielles.md), [validation](../../../validation/media/images-scp-sectorielles.md). Les autres conteneurs, conversions et services restent à porter.

## Géométries et dispositions Apple

Le bloc 65 regroupe les géométries Apple II, Macintosh GCR/MFM, Lisa FileWare et taguées ainsi que le catalogue de capacités ; la reconstruction réutilise leurs constantes. Dix empreintes originales inchangées ; 4 276 opérations comparées passent dans chaque configuration Debug/Release. [Référence](geometries-apple.md), [validation](../../../validation/media/geometries-apple.md). Les lecteurs/writers, sondes et conversions Apple restent à porter.

## Reconnaissance et lecture Apple brute

Le bloc 66 porte les sondes, le lecteur brut Apple II/Macintosh et les conversions DOS/ProDOS, avec validations de structures dans MFS. Quarante et une empreintes C# inchangées ; 195 comparaisons ciblées sans différence et 29 contrôles natifs passent en Debug. Les géométries déjà validées ne sont pas retestées. [Référence](lecture-apple-brute.md), [validation](../../../validation/media/lecture-apple-brute.md). Writers, conteneurs et parseurs complets restent à porter.

## Écriture Apple brute

Le bloc 67 porte les writers Apple II/Macintosh et les charges utiles destinées à 2IMG. Vingt-trois empreintes C# inchangées ; 157 comparaisons de writers sans différence et quatre contrôles ciblés par chemin/annulation passent en Debug. [Référence](ecriture-apple-brute.md), [validation](../../../validation/media/ecriture-apple-brute.md). Les conteneurs sont traités en parallèle dans les blocs suivants.

## Portages Apple parallèles

Les blocs 68 à 70 ajoutent 2IMG/NIB, DiskCopy et le système de fichiers Apple DOS. Les fonctions portées et les helpers encore partiels sont distingués dans l’inventaire. 2IMG/NIB : 62 comparaisons et 3 contrôles par chemin ; DiskCopy : 222 cas initiaux identiques puis 89 contrôles ciblés du correctif Base64 et 4 contrôles natifs ; AppleDOS : 165 cas distincts et 3 annulations, écarts ASCII UTF-16/zéro négatif/NUL corrigés. Voir [2IMG](twoimg.md), [DiskCopy](diskcopy.md), [DOS](apple-dos.md), [intégration](../../../validation/media/integration-apple-parallele.md). Les registres globaux et autres formats restent à porter.

## Conteneurs et systèmes de fichiers Apple intégrés

Les blocs 72 à 75 ajoutent WOZ, ProDOS/SOS, Macintosh MFS et les writers NIB/encodeurs Apple partagés. WOZ : 194 comparaisons et 3 contrôles natifs ; ProDOS : 150 comparaisons et 3 annulations ; MFS : 63 comparaisons, 14 diagnostics et 2 annulations ; encodage Apple : 414 cas distincts et 4 contrôles natifs. Les corrections ciblées concernent les limites de débordement du packer et la localisation des erreurs de forks. Les inventaires de ces blocs ont été intégrés après vérification des empreintes C# et des fichiers C++. Les Reader CatalogFormatIds et les registres globaux restent explicitement partiels. Voir [WOZ](woz.md), [ProDOS](prodos.md), [MFS](macintosh-mfs.md), [encodage](encodage-apple.md).

## Lecteurs et conteneurs intégrés en parallèle — blocs 76 à 82

HFS : 89 comparaisons C# et 2 annulations ; HFE/flux : 300 cas distincts et 3 contrôles natifs ; Lisa : 72 comparaisons et 3 annulations ; UCSD : 163 comparaisons et 2 annulations ; CP/M/Amstrad : 77 comparaisons et 5 annulations ; MSA/géométrie Atari ST : 276 comparaisons et 5 contrôles natifs ; RT-11 : 120 comparaisons et 2 annulations. Les écarts HFE sur l'index et les intervalles nuls ont été corrigés puis seuls les cas concernés et leurs limites ont été rejoués.

Les inventaires des sources ont été fusionnés après contrôle des empreintes C# et des chemins C++. Les parseurs gardent leurs responsabilités : conteneurs et flux dans Engine, systèmes de fichiers dans MFS. Les constantes publiques Amstrad sont exposées pour les futurs consommateurs sans inclure les fichiers privés. Les métadonnées de découverte encore partielles et le registre global demeurent explicitement suivis ; [référence du registre](registre-systemes-sectoriels.md).

Les 29 langues des nouveaux blocs ont été relues et corrigées ; les 30 DLL de catalogues Constants/langues ont été reconstruites avec succès en Debug. Le contrôle commun quiescent a passé 692 fichiers C++ pour les littéraux/traductions et 680 fichiers pour les frontières, sans violation. Il s'agit des vérifications ciblées de ces blocs ; le port global et sa validation finale restent en cours. Les blocs suivants prennent Atari DOS, SpartaDOS et ATR/ATX en parallèle.
