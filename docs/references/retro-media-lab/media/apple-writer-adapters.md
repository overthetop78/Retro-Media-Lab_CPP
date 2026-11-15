# Writers Apple : validation et raccordement

Sources C# consultées en lecture seule : les services AppleSector, AppleNibble, Macintosh et Lisa sous `Images/Conversion/Apple/`, `AppleSectorConversionValidationFunctions.cs`, `Images/Formats/Floppy/Apple/AppleDiskImageWriter.cs`, ses exceptions et les quatre entrées correspondantes de `Images/Writing/MediaWritingComposition.cs`. Les capacités des adapters reprennent les cibles du BuiltInImageFormatCatalog défini dans `Images/Formats/ImageFormatCatalog.cs`.

La validation sectorielle reprend les cinq associations autorisées : DOS 3.2 → Apple DOS 113 ; DOS 3.3 ou Apple DOS 140 → Apple DOS 140 ; ProDOS générique ou ProDOS 140 → ProDOS 140 ; ProDOS générique ou ProDOS 800 → ProDOS 800 ; SOS → SOS. Elle ne convertit aucun système de fichiers. Le backend vérifie ensuite la géométrie et les charges utiles. Les extensions autorisées dépendent du profil cible, comme dans le service C#.

Le writer sectoriel sélectionne 2IMG pour 2MG et le writer brut pour D13/DO/DSK/PO. La façade DiskImageWriter sélectionne l’encodage RWTS18 ou Apple II standard selon le profil de l’image, puis NIB ou WOZ1 selon l’extension. Les limites de bits proviennent du conteneur choisi. L’encodeur est injectable ; un callback synchrone recevant les octets du conteneur peut remplacer l’écriture, en reprenant le rôle du callback C#. La vue des octets est valable pendant l’appel ; le callback doit les copier s’il veut les conserver.

L’adapter nibble conserve le profil réel de la source lors de l’encodage ; le format sélectionné ne réétiquette pas silencieusement les données. Ses trois cibles visibles sont celles du catalogue C# ; les API natives peuvent également encoder les autres identifiants techniques pris en charge par le service de pistes.

Les cibles Macintosh du catalogue sont mac.400, mac.800 et mac.1440 ; les cibles Lisa sont Office et MacWorks. Pour Macintosh, IMG utilise le writer brut et IMAGE/DC42 le writer DiskCopy. Lisa expose IMAGE/DC42. Le prédicat Macintosh brut et celui du layout DiskCopy sont conservés indépendamment. Les métadonnées DiskCopy sont restaurées depuis le document avec son image détenue ; en leur absence, les métadonnées par défaut sont créées à partir du nom du fichier, comme dans le backend C#.

Les nouveaux diagnostics résident dans `apple-writer-adapters.json`. Une collision de nom avait écrasé temporairement `apple-writing.json` ; la clé préexistante MediaEngine.AppleWriteAddress a été rétablie exactement, dans Constants et les 29 langues, depuis les sources C++ de catalogues générées avant ce bloc. L’audit confirme que toutes les clés sont présentes. Apple, NIB et WOZ sont protégés par le fichier commun d’invariants.

Les services complets de conversion, leur choix des lecteurs et la composition globale restent en cours. Ce bloc expose les fonctions de validation, la façade NIB/WOZ et les quatre adapters d’écriture.
