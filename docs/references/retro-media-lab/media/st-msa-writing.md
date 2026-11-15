# Transformation et écriture ST/MSA

Sources C# en lecture seule : `Images/Conversion/Atari/AtariStGeometryTransformer.cs`, `AtariStConversionExceptions.cs`, `AtariStConversionService.cs`, `Images/Formats/Floppy/St/AtariStWriter.cs`, `AtariStExceptions.cs` et WriteAtariStAsync dans `Images/Writing/MediaWritingComposition.cs`.

La possibilité de transformation exige une cible cataloguée, des blocs de 512 octets, la même capacité et le même nombre de blocs logiques. Les dimensions physiques source peuvent différer de celles de la cible. Le transformateur parcourt les indices logiques et recalcule les adresses cylindre/face/secteur dans la géométrie cible, avec numérotation sectorielle à partir de un. Il conserve les données et les autres propriétés de chaque bloc, notamment adresse enregistrée, tags, erreurs, marque supprimée et ordre enregistré.

Un secteur absent est normalement une erreur. Le paramètre explicite completeMissingSectors autorise sa création avec 512 octets nuls, comme dans le C#. L’adapter active ce paramètre uniquement si l’extension du chemin source est SCP, insensible à la casse. Cette complétion conserve les conventions des blocs créés par le C# ; elle ne constitue pas une preuve de contenu physiquement lu.

Le writer ST résout le profil de l’image et utilise le writer linéaire commun avec validation stricte de l’identifiant et des dimensions. L’adapter commun transforme d’abord l’image vers le profil demandé, puis sélectionne le writer MSA ou ST selon l’extension cible. Le document original reste inchangé. Les opérations synchrones acceptent un stop_token, vérifié au début et pendant la transformation.

Les diagnostics géométriques conservent identifiants, dimensions et capacités source/cible. Les dimensions sont des valeurs techniques assemblées avec un séparateur nommé ; les phrases viennent des catalogues traduits.

Les chemins sont construits en UTF-8 avec une u8string et le constructeur C++20 de filesystem::path, sans l’ancienne fonction u8path dépréciée. Le service complet de conversion et sa sélection initiale SCP/MSA/ST restent en cours ; ce bloc porte la transformation, le writer ST et l’adapter d’écriture.
