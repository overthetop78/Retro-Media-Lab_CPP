# Portage et réorganisation des bibliothèques médias

Proposition du 15 novembre 2026, fondée sur le C# consulté en lecture seule. Le portage fonctionnel reste à réaliser par blocs.

## Responsabilités et accès

| Bibliothèque | Responsabilité |
|---|---|
| MediaEngine | Sources, conteneurs/images, flux, pistes, secteurs, blocs et contenus séquentiels ; reconnaissance, lecture/écriture, création/conversion, données de visualisation ; point d’entrée d’App pour exploration et migration. |
| MediaFileSystems | Partitions, volumes, systèmes de fichiers ; vrais dossiers, fichiers, contenus et métadonnées ; migration de fichiers et appel à MediaAnalysis. |
| MediaAnalysis | Classification de contenus déjà extraits ; signatures et métadonnées ; catégories, identifiants d’icônes et clés de traduction. |

App reste le GUI. App n’appelle que MediaEngine pour ces fonctions ; MediaEngine passe par MediaFileSystems pour l’analyse. Les échanges et retours sont bidirectionnels entre voisins autorisés ; interfaces et rappels permettent les échanges sans imposer de dépendances circulaires entre DLL. Les accès matériels hôtes restent dans Infrastructure, utilisée par les opérations de MediaEngine.

## Rangement proposé

Chaque bibliothèque possède `include/RetroMediaLab/<Bibliothèque>/` pour ses seuls contrats exposés et `src/` pour les implémentations privées. Les constantes et petits types propres à un format restent avec celui-ci. Les éléments réellement partagés sont regroupés à leur niveau commun.

| Bibliothèque | Groupes internes proposés |
|---|---|
| MediaEngine | Sources ; Models (Flux, Sectors, Blocks, Optical, Sequential) ; Formats (Floppy, HardDisk, Optical, Tape, Cartridge, Common) ; Recognition ; Decoding ; Encoding ; Creation ; Conversion ; PhysicalMedia ; Visualization ; Exploration ; Composition. |
| MediaFileSystems | Contrats d’accès ; Volumes/Partitioning ; FileSystems par famille et système ; Exploration ; Migration ; Registry ; primitives communes nécessaires. |
| MediaAnalysis | Models ; Recognition ; Classification ; Catalogs/Common et catalogues par famille ; constantes partagées nécessaires. |

Un lecteur de conteneur appartient à MediaEngine ; un lecteur de système de fichiers à MediaFileSystems ; une règle de reconnaissance d’un fichier extrait à MediaAnalysis. Conserver les familles et distinctions existantes, sans recopier les lecteurs entre composants.

Les contrats d’accès au média consommés par MediaFileSystems appartiennent à cette bibliothèque et sont implémentés par MediaEngine. Les résultats exposés à App appartiennent à l’API de MediaEngine ; leur conversion est explicite. Ne pas compiler les mêmes définitions publiques dans plusieurs DLL comme les quatre liens de sources relevés en C#. Plusieurs DLL supplémentaires sont autorisées par l’utilisateur. Leur découpage sera établi selon les responsabilités réelles ; aucun nombre maximal n’est imposé.

Distribution : `lib/MediaEngine.dll`, `lib/MediaFileSystems.dll`, `lib/MediaAnalysis.dll`. Les bibliothèques externes nécessaires seront intégrées selon les règles existantes. Les algorithmes de formats et d’analyse doivent rester indépendants des API Windows et des contrôles graphiques.

## Fiabilité par bloc

- Vérifier offsets, tailles, multiplications et limites avant accès/allocation ; distinguer format inconnu, média tronqué et contenu invalide.
- Définir propriété et durée de vie des sources, buffers et vues ; RAII sur réussite, exception, annulation et initialisation partielle.
- Préserver diagnostics et informations de protection/secteurs absents ou dégradés ; ne pas transformer silencieusement une erreur en résultat vide.
- Codes/clés d’erreurs et paramètres pour la traduction existante ; valeurs techniques dans des constantes nommées.
- Comparer les résultats C++ au C# et aux références du format pour chaque famille ; une compilation ne prouve pas la compatibilité fonctionnelle.
- Bancs regroupés par bloc, médias reçus par argument, sans fichiers volumineux embarqués ; suppression des bancs temporaires et artefacts conformément à config.toml ; aucun ajout aux workflows GitHub.

## Ordre proposé

1. Fixer les contrats, leurs propriétaires et les cibles DLL des trois composants.
2. Porter MediaAnalysis avec tous ses catalogues et règles ; vérifier priorités, familles, signatures, recherches inverses, contenus vides et limites.
3. Porter un trajet complet : source bornée → format d’image → représentation sectorielle → volume/système de fichiers → classification → résultat MediaEngine. Le premier format sera retenu après examen de ses dépendances dans le bloc correspondant.
4. Étendre par familles aux autres formats, systèmes de fichiers, conversions/migrations, flux, séquentiel, optique et opérations physiques, selon les dépendances réelles. Écrire les actions détaillées de chaque bloc avant de modifier le code.

[Inventaire et constats C#](../references/retro-media-lab/media/etat-source.md).

## Avancement concret

MediaAnalysis est porté et comparé au C#. MediaFileSystems contient maintenant le contrat de lecture par blocs et les détecteurs MBR/EBR/GPT/volume direct ; MediaEngine contient les sources fichier/mémoire privées. Le bloc secteurs/FAT12 ajoute les modèles sectoriels, FAT12 (lecture et construction de volumes) et le raccordement interne de leur exploration à MediaAnalysis. Les contrats publics applicatifs, les autres lecteurs/encodeurs de conteneurs, les autres systèmes de fichiers, les flux, la migration complète, les conversions, la visualisation et les opérations physiques restent à porter. La présence des trois DLL n’implique pas leur achèvement.

### Secteurs et FAT12

Les primitives FAT communes (BPB, dates, noms) sont séparées de la famille FAT12. Cette famille regroupe disposition, tables, parcours des répertoires, lecture et construction des secteurs. Les modèles et contrats consommés par les lecteurs résident dans MediaFileSystems ; leurs sources sectorielles concrètes et le pont d’exploration restent internes à MediaEngine. App ne reçoit aucune dépendance directe nouvelle.

Les données sont possédées par leurs valeurs ou des interfaces de sources partagées en lecture seule. Les fichiers restent lus à la demande, sans charger implicitement une image complète. L’énumération visite les secteurs disponibles sans allouer une liste proportionnelle à la capacité. Les diagnostics sont des codes/paramètres résolus par le domaine de traduction MediaFileSystems.

[Validation FAT12](../validation/media/secteurs-fat12.md).

### Images brutes IBM/MSX

Les profils, détections et opérations de conteneur brut sont regroupés dans MediaEngine/Formats/Raw. La publication atomique des fichiers (Sources) et l’écriture sectorielle linéaire (Models/Sectors) sont partagées, sans rattacher les futures familles au catalogue IBM. Le trajet fichier brut → FAT12 → analyse est fonctionnel ; la reconnaissance/enregistrement des lecteurs et les documents/façade publics restent à construire. [Validation](../validation/media/images-brutes-ibm-msx.md).

### BBC DFS et conteneurs SSD/DSD

Les catalogues et volumes DFS résident dans MediaFileSystems/FileSystems/Acorn/BbcDfs ; MediaEngine/Formats/BbcDfs réutilise leur décodeur pour les profils et la reconnaissance SSD/DSD/IMG/IMA/ADL. Les vues de faces empruntent les secteurs physiques, sans copie de données. Le writer atomique reste partagé avec les images brutes. [Référence](../references/retro-media-lab/media/bbc-dfs.md), [validation](../validation/media/bbc-dfs.md). Les registres/documents et la façade publique restent en attente.

### Cartes FileCore

MediaFileSystems/FileSystems/Acorn/FileCore regroupe les anciennes et nouvelles cartes d'allocation, leur décodage et leurs résolveurs possédant leurs données. Les contrôles de cartes, copies secondaires et calculs d'adresses sont comparés au C#. Le parcours ADFS s’appuie désormais sur ces résolveurs. [Référence](../references/retro-media-lab/media/filecore.md), [validation](../validation/media/filecore.md).

### Exploration ADFS

MediaFileSystems/FileSystems/Acorn/Adfs sépare répertoires, fichiers, dates, initialisation et vue de volume par blocs. Les anciennes/nouvelles cartes restent dans FileCore. La vue bornée lit des secteurs à la demande ; les diagnostics sont traduisibles. Les conteneurs Acorn ADF sont portés dans MediaEngine ; leur raccordement aux documents reste à construire. [Référence](../references/retro-media-lab/media/adfs.md), [validation](../validation/media/adfs.md).

### Conteneurs ADF

MediaEngine/Formats/Adf sépare profils, reconnaissance des images partielles, lecture et écriture. Les cartes/capacités et DiscRecords restent définis dans FileCore ; le moteur réutilise ces contrats et fait valider les candidats par ADFS. Les writers Acorn et Amiga partagent le writer linéaire atomique. L'exploration Acorn rejoint ADFS/FAT12 puis l'analyse. Les documents et enregistrements généraux restent à construire. [Référence](../references/retro-media-lab/media/adf.md), [validation](../validation/media/adf.md).

### AmigaDOS

MediaFileSystems/FileSystems/Amiga/Dos sépare les codecs/noms/dates, racines/bitmaps, répertoires, fichiers/extensions, récupération et création OFS/FFS. Les huit variantes de lecture conservent le comportement C#. MediaEngine sélectionne ce système pour les ADF Amiga puis appelle l’analyse des entrées via MediaFileSystems. Les limites et renforcements sont documentés ; aucun lien direct nouveau d’App ni accès matériel. [Référence](../references/retro-media-lab/media/amigados.md), [validation](../validation/media/amigados.md).

### Archives plates de ressources Amiga

MediaFileSystems/FileSystems/Amiga/FlatArchive sépare les tables, plages et projections de volumes. MediaEngine/Formats/Adf conserve la sélection AmigaDOS puis archive plate et laisse l’analyse à MediaFileSystems. Les anomalies physiques restent des diagnostics paramétrés ; le commentaire technique est référencé par une clé traduisible. [Référence](../references/retro-media-lab/media/amiga-flat-archive.md), [validation](../validation/media/amiga-flat-archive.md).

### COHERENT

MediaFileSystems/FileSystems/Commodore/Coherent sépare schéma, codecs, blocs présents, inodes, contenus et répertoires. Les blocs utiles sont retenus à la demande. Diagnostics et commentaires techniques exposent des paramètres et causes traduisibles. Le conteneur brut zoné et les codecs physiques restent dans le périmètre MediaEngine à porter. [Référence](../references/retro-media-lab/media/coherent.md), [validation](../validation/media/coherent.md).

### Conteneur brut Commodore 900

MediaEngine/Formats/Commodore900 regroupe géométrie zonée, reconnaissance/lecture et écriture atomique du dump COHERENT. Les images sur source possèdent une fonction facultative d’adressage physique, sans modifier les formats réguliers. Le moteur appelle COHERENT dans MediaFileSystems puis son analyse. Les codecs GCR, conversions et registres/documents restent à construire. [Référence](../references/retro-media-lab/media/commodore900-raw.md), [validation](../validation/media/commodore900-raw.md).

### Primitives de flux et GCR

MediaEngine/Models/Flux possède bits/intervalles et reconstruction indépendante des conteneurs. Codecs/Commodore porte la table GCR et le XOR partagés ; Decoding/Confidence porte la notation commune. Les codecs de piste s’appuieront sur ces primitives, sans déplacer leur responsabilité dans MediaFileSystems. [Référence](../references/retro-media-lab/media/flux-core.md), [validation](../validation/media/flux-core.md).

### Codec de piste Commodore 900 GCR

MediaEngine/Codecs/Commodore sépare encodage et décodage de piste ; Encoding possède leurs requêtes et primitives communes, Decoding leurs résultats et descriptions traduisibles à la demande. Le codec reste distinct du conteneur brut zoné et des systèmes de fichiers. Aucun nouveau lien d’App vers MediaFileSystems/MediaAnalysis. Registres et raccordement des fichiers de flux restent à construire. [Référence](../references/retro-media-lab/media/commodore900-gcr.md), [validation](../validation/media/commodore900-gcr.md).

### Lecture SCP et reconstruction de secteurs

MediaEngine/Formats/Scp sépare modèles possédés, lecture bornée/checksum et fenêtres de révolutions. Formats/Commodore900 raccorde ces fenêtres au GCR et sélectionne les secteurs selon la priorité d’intégrité commune, en conservant leur provenance. Les sources ne sont empruntées que pendant la lecture ; les résultats possèdent leurs données. Le writer SCP, les registres et documents restent à construire. [Référence](../references/retro-media-lab/media/scp-reader.md), [validation](../validation/media/scp-reader.md).

### Écriture SCP

Formats/Scp sépare le writer à plan de taille/checksum de la conversion temporelle des pistes synthétiques. Le writer émet des blocs bornés vers un rappel synchrone ou la publication atomique commune, sans relecture/repositionnement de fichier. Le tick d’encodeur reste défini dans Encoding. Les documents, registres et acquisitions matérielles restent à porter. [Référence](../references/retro-media-lab/media/scp-writer.md), [validation](../validation/media/scp-writer.md).

### Conteneurs D64/D71/D81 et géométries Commodore

MediaFileSystems/Formats/Commodore possède les géométries partagées 1541/1571/1581. MediaEngine/Formats/CommodoreDisks conserve dispositions, reconnaissance, vue sectorielle sur source et écriture D81 atomique. Les codes de cartes d’erreurs restent attachés aux secteurs sans inventer de détails physiques. Le système de fichiers Commodore DOS et les registres/documents restent à porter. [Référence](../references/retro-media-lab/media/commodore-disks.md), [validation](../validation/media/commodore-disks.md).

### Exploration Commodore DOS

MediaFileSystems/FileSystems/Commodore/Dos sépare reconnaissance/lecture sectorielle, répertoire, contenu de fichier et BAM. Le codec PETSCII est commun aux futurs lecteurs/writers. MediaEngine/Formats/CommodoreDisks appelle cette exploration puis le point d’analyse de MediaFileSystems. Les commentaires affichés restent traduisibles, la classification reçoit leur type invariant. [Référence](../references/retro-media-lab/media/commodore-dos.md), [validation](../validation/media/commodore-dos.md).

### Construction Commodore DOS

MediaFileSystems/FileSystems/Commodore/Dos répartit la construction des plans entre politique, géométrie, allocation, fichiers/REL et secteurs système. Le résultat possède tous les secteurs et ne publie aucun fichier. MediaEngine garde la responsabilité des conteneurs. [Référence](../references/retro-media-lab/media/commodore-dos-writer.md), [validation](../validation/media/commodore-dos-writer.md).

### Écriture D64/D71 avec diagnostics

MediaEngine/Formats/CommodoreDisks publie données zonées et carte optionnelle de diagnostics en deux passes atomiques. Les codes bruts sont conservés, sans inférence depuis l’intégrité. Les géométries restent dans MediaFileSystems ; les conversions et la reconstruction SCP sont des responsabilités distinctes. [Référence](../references/retro-media-lab/media/commodore-container-writer.md), [validation](../validation/media/commodore-container-writer.md).

### Codec GCR Commodore 1541/1571

Codecs/Commodore distingue les codecs 1541/1571 et Commodore 900 tout en partageant la primitive GCR. L’encodeur consomme les contrats communs de piste ; le décodeur produit des structures/secteurs possédés et des recettes de descriptions traduisibles. Le champ cylindre décodé conserve la piste disque enregistrée ; la conversion physique appartiendra au constructeur sectoriel. [Référence](../references/retro-media-lab/media/commodore-gcr.md), [validation](../validation/media/commodore-gcr.md).

### Reconstruction SCP Commodore 1541/1571

Formats/CommodoreDisks sépare le parcours SCP/progression et l’accumulateur de reconstruction GCR. Celui-ci garde le meilleur payload et l’ordre des clés, sans perdre les indices géométriques fournis par les candidats éliminés. Le résultat sectoriel possède ses données et conserve les manques. [Référence](../references/retro-media-lab/media/commodore-scp-gcr.md), [validation](../validation/media/commodore-scp-gcr.md).

### Encodage ISO MFM

Codecs/Iso sépare la construction des champs MFM de la disposition logique/physique. Les CRC configurables appartiennent à Codecs/Crc16 et les codes de taille communs aux modèles sectoriels. Les champs enregistrés et CRC fournis restent prioritaires conformément à la source. [Référence](../references/retro-media-lab/media/iso-mfm-encoding.md), [validation](../validation/media/iso-mfm-encoding.md).

### Décodage ISO MFM

Le lecteur binaire commun borne chaque accès ; les champs/CRC, l’appairage circulaire et la sélection PLL sont séparés dans Codecs/Iso. Les résultats possèdent leurs secteurs et conservent les intégrités optionnelles et recettes traduisibles. [Référence](../references/retro-media-lab/media/iso-mfm-decoding.md), [validation](../validation/media/iso-mfm-decoding.md).

### Reconstruction SCP Commodore 1581

Formats/CommodoreDisks sépare le parcours/décodage MFM de l’accumulation physique et du découpage logique D81. Les coordonnées physiques SCP déterminent l’adresse, le meilleur candidat détermine les données/intégrités ; les faces restent inversées conformément à la source. [Référence](../references/retro-media-lab/media/commodore-scp-mfm.md), [validation](../validation/media/commodore-scp-mfm.md).

### Routage des reconstructions Commodore SCP

Formats/CommodoreDisks/Router sélectionne les reconstructeurs existants et relie le point d’entrée fichier au lecteur SCP commun. La demande reste exacte ; aucune autre famille n’est essayée silencieusement après un rejet. [Référence](../references/retro-media-lab/media/commodore-scp-routing.md), [validation](../validation/media/commodore-scp-routing.md).

### Encodage ISO FM

Codecs/Iso sépare les champs FM spéciaux et CRC de la disposition logique/physique, en réutilisant le socle de piste et les primitives communes. Les métadonnées enregistrées gardent leur priorité et les contraintes de temporalité sont explicites. [Référence](../references/retro-media-lab/media/iso-fm-encoding.md), [validation](../validation/media/iso-fm-encoding.md).

### Décodage ISO FM

Codecs/Iso sépare lecture des champs/horloges, appairage et reconstruction temporelle. Le mode à durée connue préserve la période source et les métadonnées de CRC enregistrés, en réutilisant les primitives communes. [Référence](../references/retro-media-lab/media/iso-fm-decoding.md), [validation](../validation/media/iso-fm-decoding.md).

### Socle de construction sectorielle ISO

Reconstruction/Iso distingue groupes de candidats ordonnés, mesure majoritaire, sélection stable, construction uniforme et normalisation. Les blocs possèdent leurs données et conservent uniquement les métadonnées produites par le constructeur source. La géométrie sparse déclarée est distinguée des limites de lecture des sources. [Référence](../references/retro-media-lab/media/iso-sector-builder.md), [validation](../validation/media/iso-sector-builder.md).

### Pistes protégées

Models/Flux possède bits, plages, caractéristiques et révolutions avec résolution. Les adaptateurs SCP et encodage partagent le même modèle et les mêmes fenêtres sans rééchantillonnage implicite. [Référence](../references/retro-media-lab/media/protected-tracks.md), [validation](../validation/media/protected-tracks.md).

### Collecte sectorielle ISO

Reconstruction/Iso sépare score, traitement des fenêtres et orchestration parallèle. Le service de décodage est fourni explicitement ; les résultats possédés sont fusionnés dans un ordre déterministe et la progression reste structurée. [Référence](../references/retro-media-lab/media/iso-collection.md), [validation](../validation/media/iso-collection.md).

### Cache des captures ISO

CandidateDecoder possède le cache par liste ordonnée de codecs et empreinte de fichier. Les opérations concurrentes partagent un résultat immuable ; l’entrée fichier réutilise lecture SCP, adaptation et collecte. La surcharge image contourne le cache. [Référence](../references/retro-media-lab/media/iso-capture-cache.md), [validation](../validation/media/iso-capture-cache.md).

### Registre de décodeurs de flux

Decoding sépare interface, scoring et registre explicite. Le cache faible partage résultats et exceptions par identité de révolution/codec ; la collecte transmet une identité possédée commune à chaque fenêtre. Le catalogue complet reste distinct. [Référence](../references/retro-media-lab/media/flux-decoder-registry.md), [validation](../validation/media/flux-decoder-registry.md).

### Analyse brute du flux

Codecs/Raw réutilise l’estimation temporelle commune et produit uniquement des anomalies localisées. Cinq classes concrètes de décodeurs peuvent désormais être enregistrées explicitement ; le catalogue complet reste à assembler. [Référence](../references/retro-media-lab/media/raw-flux-analysis.md), [validation](../validation/media/raw-flux-analysis.md).

### Codec Amiga MFM

Codecs/Amiga sépare primitives odd/even, parités, construction des secteurs et décodage. Les contrats communs permettent l’encodage, le registre explicite et la collecte de captures SCP ; les politiques de reconstruction restent distinctes. [Référence](../references/retro-media-lab/media/amiga-mfm-codec.md), [validation](../validation/media/amiga-mfm-codec.md).

### Primitives Apple II GCR

Codecs/Apple fournit la lecture synchronisée commune ; Codecs/AppleII sépare adresses, 6-and-2 et 5-and-3. Les codecs de piste restent l’étape suivante. [Référence](../references/retro-media-lab/media/appleii-gcr-primitives.md), [validation](../validation/media/appleii-gcr-primitives.md).

### Pistes Apple II GCR

Codecs/AppleII comprend désormais encodeur et décodeur 13/16 secteurs : sélection explicite des pistes partielles, association circulaire des champs et diagnostics localisés. Les politiques Apple restent distinctes. [Référence](../references/retro-media-lab/media/appleii-gcr-track.md), [validation](../validation/media/appleii-gcr-track.md).

### Charge utile RWTS18

Codecs/AppleRwts18 réutilise la table Apple II pour transformer trois pages indépendantes en symboles et calculer leur checksum. Les codecs de piste restent distincts. [Référence](../references/retro-media-lab/media/rwts18-payload.md), [validation](../validation/media/rwts18-payload.md).

### Pistes RWTS18

Codecs/AppleRwts18 comprend désormais encodeur et décodeur de piste : six secteurs ordonnés physiquement, adresses validées, recherche synchronisée et descriptions localisées. Les politiques Apple de reconstruction restent distinctes. [Référence](../references/retro-media-lab/media/rwts18-track.md), [validation](../validation/media/rwts18-track.md).

### Primitives Apple IWM GCR

Codecs/AppleIwm isole transformations de blocs tagués, regroupement 6-and-2 et accumulateurs communs Macintosh/Lisa. La table Apple II est réutilisée ; les codecs de piste restent distincts. [Référence](../references/retro-media-lab/media/iwm-gcr-primitives.md), [validation](../validation/media/iwm-gcr-primitives.md).

### Pistes IWM Macintosh et Lisa

Codecs/AppleIwm partage encodage/décodage avec deux identités explicites : tags, format, adresses, checksum et NRZI à durée automatique ou imposée. Les structures suivent le tri stable source. [Référence](../references/retro-media-lab/media/iwm-gcr-track.md), [validation](../validation/media/iwm-gcr-track.md).

### Pistes Victor 9000 GCR

Codecs/Victor9000 sépare champs, sommes et pistes ; les cellules GCR entrelacées réutilisent le codec Commodore. L’ordre des structures et les limites de recherche restent fidèles au C#. [Référence](../references/retro-media-lab/media/victor9000-gcr.md), [validation](../validation/media/victor9000-gcr.md).

### Pistes Micral N FM

Codecs/MicralN isole checksum avec retenue, encodage FM et lecture de bloc. Le balayage et les contrôles d’intégrité suivent le C#, avec les primitives communes de flux. [Référence](../references/retro-media-lab/media/micraln-fm.md), [validation](../validation/media/micraln-fm.md).

### Pistes E-mu FM

Codecs/Emu partage CRC IBM et FM 32 cellules ; il isole adresse inversée, lecture et classification des marques communes aux en-têtes/données. Le tri stable et les règles de balayage restent fidèles au C#. [Référence](../references/retro-media-lab/media/emu-fm.md), [validation](../validation/media/emu-fm.md).

### Pistes Micropolis MFM

Codecs/Micropolis isole checksum à réduction différée et record possédé. Encodage et décodage partagent les primitives MFM ; les deux modes données/record et la face logique source sont conservés. [Référence](../references/retro-media-lab/media/micropolis-mfm.md), [validation](../validation/media/micropolis-mfm.md).

### Pistes NorthStar et checksum rotatif

Codecs/RotatingChecksum fournit le XOR/rotation commun ; Codecs/Northstar sépare adresse compacte et pistes. Le décodeur porte aussi les fenêtres de secteurs matériels FM/MFM, avec sélection et intégrité fidèles au C#. [Référence](../references/retro-media-lab/media/northstar-mfm.md), [validation](../validation/media/northstar-mfm.md).

### Pistes Heathkit FM

Codecs/Heathkit réutilise le checksum rotatif commun ; Primitives/Bits partage l’inversion d’octet avec E-mu. L’appairage, les états de checksum et le tri stable suivent le C#. [Référence](../references/retro-media-lab/media/heathkit-fm.md), [validation](../validation/media/heathkit-fm.md).

### Pistes Data General 2F

Codecs/DataGeneral isole checksum avec zéro terminal, identité composée et décodage par synchronisations adjacentes. Les distances admissibles et la progression après reconnaissance suivent le C#. [Référence](../references/retro-media-lab/media/datageneral-fm.md), [validation](../validation/media/datageneral-fm.md).

### Primitives RX02 M²FM et marques FM

Codecs/FmAddressMarks partage les motifs physiques F8–FE. Codecs/DecRx02 isole sélection des encodages RX01/RX02 et conversion des motifs MFM/M²FM, avant raccordement des pistes. [Référence](../references/retro-media-lab/media/rx02-m2fm.md), [validation](../validation/media/rx02-m2fm.md).

### Pistes RX01/RX02

Codecs/DecRx02 réutilise les marques FM et la transformation M²FM du bloc 49. Les pistes mixtes, les contrôles CRC et le classement des marques suivent le C# ; les descriptions restent séparées de la reconnaissance. [Référence](../references/retro-media-lab/media/rx02-tracks.md), [validation](../validation/media/rx02-tracks.md).

### Pistes HP MMFM

Codecs/HpMmfm isole permutation/inversion partagée, identité et champs CRC. La consommation unique des synchronisations et l’ordre de production des structures suivent le C#. [Référence](../references/retro-media-lab/media/hp-mmfm.md), [validation](../validation/media/hp-mmfm.md).

### Pistes Centurion MFM

Codecs/Centurion sépare allocation/préfixes/CRC, émission et reconnaissance des pistes. La recherche bornée par le prochain en-tête, les tailles déclarées incomplètes et l’ordre des structures suivent le C#. [Référence](../references/retro-media-lab/media/centurion-mfm.md), [validation](../validation/media/centurion-mfm.md).

### Pistes AED 6200P MFM

Codecs/Aed6200p sépare identité, émission et reconnaissance. Les quatre marques, la fenêtre de recherche, les secteurs incomplets et les descriptions conservent le comportement C#. [Référence](../references/retro-media-lab/media/aed6200p-mfm.md), [validation](../validation/media/aed6200p-mfm.md).

### Blocs Arburg système/données

Codecs/Arburg sépare checksum/construction, codage système variable et pistes. Les deux balayages, le regroupement secteurs/octets et le tri stable des structures suivent le C#. [Référence](../references/retro-media-lab/media/arburg.md), [validation](../validation/media/arburg.md).

## Contrôle des frontières du 15 novembre 2026

Lors du bloc 56, `python scripts/quality/audit_media_architecture.py` a examiné les inclusions et cibles CMake de 514 fichiers. Aucun accès direct App → MediaFileSystems/MediaAnalysis, MediaEngine → MediaAnalysis, ni inclusion privée entre composants n’avait été détecté. Les liens MediaEngine → MediaFileSystems → MediaAnalysis sont PRIVATE. Les listes de compilation avaient été regroupées explicitement par responsabilité (133/54/2 sources à cette étape), avec régénération CMake Debug réussie. Les validations des blocs suivants donnent les effectifs et résultats actualisés. Ce contrôle établit les frontières statiques examinées ; la couverture fonctionnelle et les API applicatives restent suivies dans les blocs de portage.

### Pistes Membrain MFM

Codecs/Membrain sépare adresse, champs, émission et reconnaissance ; CRC IBM et marques F8–FB sont contrôlés. [Référence](../references/retro-media-lab/media/membrain-mfm.md), [validation](../validation/media/membrain-mfm.md).

### Pistes QD MO5 MFM

Codecs/QdMo5 sépare champs, émission et reconnaissance ; les secteurs 16 bits, préfixes et checksums suivent le C#. [Référence](../references/retro-media-lab/media/qd-mo5-mfm.md), [validation](../validation/media/qd-mo5-mfm.md).

### Pistes TYCOM FM

Codecs/Tycom partage les marques FM, le CRC et la lecture FM bornée de BitReader. Le rejet des en-têtes invalides et l’appairage suivent le C#. [Référence](../references/retro-media-lab/media/tycom-fm.md), [validation](../validation/media/tycom-fm.md).

### Composition des codecs de flux

Composition/FluxCodecs construit les 25 décodeurs et 24 encodeurs dans l’ordre C#. Decoding/Registry peut désormais utiliser ce catalogue par défaut ; le contrat du décodeur exclut Lisa de l’automatique, sans dépendance du registre au type concret. [Référence](../references/retro-media-lab/media/composition-codecs-flux.md), [validation](../validation/media/composition-codecs-flux.md).

### Reconstruction des images sectorielles

Reconstruction/Sector porte les décisions de format, métadonnées, adressage et cadences ; Encoding/SectorImageEncoder orchestre les pistes. Visualization/SectorFluxVisualizer et Formats/Scp/SectorConverter assemblent leurs sorties distinctes sans dupliquer les encodeurs ou le writer. Ces composants restent dans MediaEngine, sans accès direct à MediaAnalysis, au GUI ou au matériel. [Politiques](../references/retro-media-lab/media/politiques-sectorielles.md), [orchestration](../references/retro-media-lab/media/encodage-images-sectorielles.md), [SCP](../references/retro-media-lab/media/images-scp-sectorielles.md).

### Reconnaissance Apple

MediaEngine/Formats/Apple utilise le contrat AppleRecognition de MediaFileSystems pour valider les structures DOS, ProDOS, Lisa et Macintosh. Les sondes de formats et les transformations DOS/ProDOS restent dans Engine ; les validations de fichiers système restent dans MFS et seront réutilisées par leurs parseurs. [Référence](../references/retro-media-lab/media/lecture-apple-brute.md).

### Conteneurs Apple et systèmes de fichiers Macintosh

Les conteneurs 2IMG, DiskCopy, NIB et WOZ restent dans MediaEngine/Formats. Apple/DecodedTracks partage la sélection standard/RWTS18 entre NIB et WOZ ; Apple/TrackEncoding partage les encodeurs Apple et Encoding/BitPacker centralise le rangement MSB sans l'imposer aux formats LSB comme HFE. Les writers conservent le chemin atomique commun.

DOS, ProDOS, MFS et HFS restent dans MediaFileSystems/FileSystems/Apple. Macintosh/Primitives centralise MacRoman, noms, dates, types Finder et tri ; les parseurs MFS et HFS gardent leurs propres allocations et catalogues. Les diagnostics de forks portent des contextes typés et des clés traduisibles, sans phrases françaises opaques passées comme arguments. Le contrôle HFS a corrigé le traitement partagé des types Finder uniquement blancs ; 89 comparaisons et 2 annulations passent après cette correction. Les contrats applicatifs et le catalogue global de lecteurs restent à terminer ; les fonctions déjà portées ne constituent pas encore l'intégration complète de l'explorateur.
