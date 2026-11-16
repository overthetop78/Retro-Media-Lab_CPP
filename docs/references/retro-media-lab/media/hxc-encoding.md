# Codage HxC Stream

Références C# en lecture seule : les cinq fichiers de `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/HxcStream/`. Leurs SHA256 actuels concordent avec `inventaire-csharp.csv`.

Ce bloc porte `HxcStreamEncoding`, les trois fonctions de clés par piste de `HxcStreamMetadataKeys`, les constantes nécessaires au codage et le CRC de `HxcStreamReader`. Le lecteur de captures est terminé dans le bloc 138 (`hxc-reader.md`). Le writer de fichiers reste à porter.

## Règles du codage

La résolution exacte est recherchée dans l'ordre 40 puis 20 ns. Chaque intervalle de flux doit être un multiple exact de cette résolution ; la durée d'index doit aussi être un multiple de seize cellules. Le codage n'arrondit pas les temps. Les distances entre impulsions peuvent inclure la fin de la révolution précédente ; elles doivent être positives et représentables sur 28 bits. Les transitions ne doivent pas dépasser la durée de leur révolution.

Les signaux d'index et de protection en écriture sont placés dans des mots d'E/S échantillonnés tous les seize ticks de flux. La capture E/S est bornée à 64 Mio. Les impulsions utilisent le code variable à un, deux, trois ou quatre octets du C#.

Chaque chunk contient un bloc de métadonnées, un bloc E/S LZ4 et, si des impulsions existent, un bloc de flux LZ4 indiquant leur nombre. Les en-têtes, longueurs et alignements sur quatre octets reprennent ceux du C#. La taille finale du chunk est bornée à 64 Mio ; les calculs cumulés de temps et les tailles passées à LZ4 sont vérifiés.

La compression utilise `LZ4_compress_default` de la bibliothèque déjà embarquée ; API de référence locale : `third_party/lz4/lib/lz4.h`. Le CRC du C# utilise une graine 0xffffffff, un XOR initial et final et le polynôme réfléchi 0xedb88320. L'appel à zlib `crc32` avec cette graine produit ce calcul, vérifié sur une valeur indépendante. Référence API locale : `third_party/zlib/zlib.h`.

Les champs de métadonnées sont limités à la piste physique concernée. Le champ `sample_rate_hz` est produit par le codec et ne peut pas être remplacé. Les noms vides ou comportant un espace Unicode ou un caractère nul, et les valeurs contenant saut de ligne, retour chariot ou caractère nul, sont exclus comme dans le C#. La carte C++ fournit un ordre stable des champs.

## Empreinte native

SHA256 reçoit exactement les champs écrits par le BinaryWriter C# : cylindre et tête signés sur quatre octets little-endian, protection sur un octet, puis, pour chaque révolution, résolution, durée d'index, nombre d'intervalles et intervalles sur quatre octets. Le résultat est hexadécimal majuscule. Les clés `hxcstream.native.C.H`, `hxcstream.digest.C.H` et `hxcstream.field.C.H.` restent propres à chaque piste. L'empreinte permettra au futur writer de conserver une capture native inchangée.

## Prochain raccordement déjà étudié

Le lecteur C# énumère les fichiers dont le nom correspond au même préfixe et au suffixe NN.S.hxcstream, classe par cylindre/tête et décode des chunks CRC avec blocs métadonnées/E/S/flux. Il reconstitue les révolutions entre fronts montants d'index, ou utilise la durée cumulée des impulsions à défaut de deux index. Les métadonnées gardent les octets natifs et les empreintes calculées avec la protection globale du jeu de pistes.

Le writer C# reprend ces octets si l'empreinte correspond, sinon utilise le codec ; le nom de sortie doit désigner la première piste physique. La publication de l'ensemble utilise `AtomicMediaFileSetWriter`, encore à porter : remplacer simplement ce service par plusieurs écritures atomiques indépendantes ne reproduirait pas sa restauration du jeu de fichiers en cas d'échec.
