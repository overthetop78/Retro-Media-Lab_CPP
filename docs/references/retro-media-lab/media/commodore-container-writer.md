# Conteneurs Commodore D64/D71 — écriture

## Source et responsabilités

Ce bloc porte `MediaEngine/Images/Formats/Floppy/CommodoreDos/CommodoreDosContainerWriter.cs`, ses erreurs, son mode de carte et les codes sectoriels. Il complète le groupe des lecteurs D64/D71 et writer D81 déjà porté. Le writer commun existait dans un dossier C# distinct ; l’ancienne présentation limitée au dossier D81 ne devait pas être interprétée comme son absence du projet. [Validation](../../../validation/media/commodore-container-writer.md).

Le writer reste dans `MediaEngine/Formats/CommodoreDisks/Writer.cpp`, avec la publication atomique commune. MediaFileSystems construit les volumes et les géométries partagées ; il ne gère pas les conteneurs ni les fichiers destination. Aucune nouvelle dépendance App ou directe MediaEngine → MediaAnalysis.

## Géométries et ordre

`writeD64D71` reçoit une image sectorielle, une destination, un ErrorMapMode et un jeton d’annulation. Comme le C#, il calcule d’abord les blocs par face, ce qui peut rejeter un nombre de pistes avant le diagnostic propre au conteneur. Les formats sont comparés sans casse : 1541 exige une face et 1571 deux faces. Les deux acceptent 35 ou 40 pistes, des blocs de 256 octets et le nombre logique zoné exact. Le champ secteurs par piste ne sert pas à valider ce writer zoné.

La première passe écrit les blocs logiques dans l’ordre, sans tri physique ni remplissage des blocs absents. Chaque bloc doit être présent et posséder exactement 256 octets. La seconde passe est effectuée uniquement en mode Preserve ; elle exige le code brut de chaque bloc et l’ajoute après toutes les données. Les valeurs zéro et non nommées sont conservées, jamais remplacées selon l’intégrité. Une carte manquante n’est pas synthétisée. Les valeurs de mode différentes de Preserve se comportent comme None, conformément au C#.

Les constantes nommées reprennent les diagnostics C# : None=1, HeaderNotFound=2, SyncNotFound=3, DataNotFound=4, DataChecksumError=5, WriteVerifyError=6, WriteProtected=7, HeaderChecksumError=8, WriteError=9, DiskIdMismatch=10, DriveError=11, DriveNotReady=15. Elles ne restreignent pas les octets acceptés dans les cartes.

## Atomicité et traductions

Les validations de géométrie précèdent l’ouverture de destination. Les données et la carte sont produites vers QSaveFile avec repli direct désactivé. Une exception de source, un bloc absent/court, un diagnostic absent ou une annulation conserve la destination précédente et détruit le fichier temporaire. La seconde passe bénéficie également des contrôles d’annulation de la sortie commune. Le writer ne retient pas l’image après retour.

Les quatre diagnostics du conteneur sont dans `MediaEngine/commodore-container-writer.json`, générés avec Argos depuis en-US et relus/corrigés dans les 29 langues. Les erreurs de géométrie partagée conservent leur domaine MediaFileSystems.

Les services de conversion depuis SCP et l’orchestration de migration restent distincts et à porter ; ces contrôles ne les déclarent pas réalisés.
