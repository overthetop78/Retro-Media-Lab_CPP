# Epson, UCSD, COHERENT et helper de sonde

Références C# consultées en lecture seule : `Raw/EpsonQx10RawImageReader.cs`, `CoherentRawImageReader.cs`, `UcsdRawImageReader.cs`, `UcsdIbmMfmGeometry.cs`, le helper générique de `MediaRecognitionRegistry` et `Reconstruction/EpsonQx10/EpsonQx10Exceptions.cs`.

Le reader Epson annonce les six profils existants et demande un identifiant explicite. La sonde compare uniquement la longueur attendue par ce profil. La lecture conserve l’ordre cylindre/tête/secteur, les pistes vides du profil Logo, les bases de numérotation particulières et les tailles variables. Le bloc dominant est choisi d’après le nombre de secteurs de chaque taille, avec priorité d’insertion sur égalité comme en C#. Un identifiant inconnu reste une erreur d’argument et ne fait pas partie des rejets de format.

Le reader UCSD reprend la géométrie IBM MFM de 160 Kio : 40 cylindres, une tête, huit secteurs de 512 octets, numérotation à partir de un. COHERENT réutilise le backend Commodore 900, avec validation de contenu et géométrie zonée déjà portées. Ces readers retournent les documents sectoriels communs et ne réalisent pas eux-mêmes l’exploration MFS.

`ReaderValidation.h` porte le helper générique C# : filtrer le format demandé, exécuter la validation, puis retourner vrai lorsqu’elle termine sans exception. La valeur du résultat du validateur n’est pas interprétée comme un booléen de sonde ; ce point reproduit le C#. Seuls les rejets de format reconnus par la politique commune retournent faux. E/S, annulation et arguments continuent à remonter. Les futures possèdent contexte, formats et validateur.

Les diagnostics de lecture Epson/COHERENT sont raccordés à la politique commune. Les nouveaux textes Epson sont traduits dans Constants et les 29 langues. La composition globale des readers reste à terminer.
