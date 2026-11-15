# UCSD p-System

Référence : les 16 sources originales de `F:/GW GUI/src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/`, consultées sans modification. Le port utilise SectorImage, FileSystemVolume, FileSystemEntry, Timestamp et les diagnostics existants. Aucun writer UCSD n’existe dans ces sources et aucun n’est inventé.

Les constantes du layout, l’ordre des octets et les types de fichiers sont conservés. Les noms Pascal sont rejetés lorsqu’ils sont vides, trop longs, hors champ ou contiennent des caractères hors ASCII imprimable (0x20 à 0x7e). Les dates compactes conservent les masques jour/mois/année et le pivot 70 : 70–127 sont ajoutés à 1900, 0–69 à 2000. Les dates nulles ou impossibles donnent une absence.

L’en-tête détecte little-endian ou big-endian à partir de la fin de répertoire 6 ou 10 ; il vérifie nom, total de blocs, compte maximal 77 et bornes par rapport à l’image. CanRead vérifie en plus le format exact `ucsd.ibm.mfm` sans casse ASCII et la taille de secteur 512. Read conserve le comportement C# qui lit un en-tête cohérent sans appliquer ce filtre de format ou de géométrie.

La lecture des blocs conserve leur position, les remplace par des zéros lorsqu’ils manquent ou n’ont pas exactement 512 octets et mémorise leur validité. Une entrée traversant la frontière de deux blocs de répertoire n’est lue que si les deux blocs sont présents. Les bornes de fichiers sont [premier, dernier), le dernier bloc étant exclusif. Les octets utiles du dernier bloc valent 512 lorsque le champ vaut zéro ; une valeur supérieure à 512 produit un fichier invalide et vide.

Les blocs système 0 à fin du répertoire sont réservés. Les chevauchements entre fichiers sont signalés, les fichiers restent exposés mais leur métadonnée de validité est fausse. L’espace libre est calculé seulement si le répertoire et les contenus sont cohérents ; sinon il vaut zéro. Les entrées vides 0/0 sont ignorées et la différence entre compte annoncé et entrées exposées produit le diagnostic du C#. Le tri est stable, ordinal sans casse sur ces noms ASCII.

Le masque de type 0x0f est conservé ; les neuf types connus et le défaut non typé disposent de clés traduisibles. Même le type SecureDirectory reste exposé comme fichier, conformément à la source. L’identifiant UcsdIbmMfm est centralisé dans FormatIds, sans alias divergent.

Les 15 sources de composants sont portées ; UcsdFileSystemReader reste partiel uniquement pour l’exposition de CatalogFormatIds et son raccordement au registre, son filtre CanRead et sa lecture étant portés. Les objets possèdent leurs buffers ; l’annulation et les erreurs libèrent les ressources par RAII.
