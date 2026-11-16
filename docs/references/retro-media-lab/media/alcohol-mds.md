# Portage Alcohol MDS/MDF

Référence C# consultée en lecture seule le 16 novembre 2026 : `Images/Formats/Optical/Alcohol/AlcoholMdsReader.cs`, AlcoholMdsFormat.cs, `Constants/AlcoholMdsConstants.cs`, `Contracts/Optical/AlcoholMdsSessionBlock.cs` et AlcoholMdsTrackBlock.cs dans `F:/GW GUI/src/RetroMediaLab.MediaEngine`. Les cinq SHA-256 sont vérifiés contre l’inventaire. Pas de nouvelle recherche Internet nécessaire pour reprendre le comportement présent.

## Organisation

`Formats/Optical/Alcohol/Descriptor.*` contient les structures, les sessions possédées, l’en-tête, les blocs de pistes, les footers et la résolution des noms. `Reader.*` se limite au raccordement des sources MDF et des pistes optiques. Les constantes de disposition binaire sont séparées. Les sessions exposent des vues const sur leurs pistes ; les copies des blocs ne modifient pas les collections possédées.

Chaque lecture de structure passe par une plage validée contre RecognitionContext::length puis le mécanisme de lecture existant. Les offsets et nombres de blocs sont calculés en 64 bits ; un offset de piste UInt64 supérieur à INT64_MAX est refusé. La signature ASCII MEDIA DESCRIPTOR doit correspondre et la table des sessions doit être complète et non vide. La reconnaissance ne dépend pas de l’extension, comme dans le C#.

Le profil reste celui du code source : versions majeures <= 1, y compris 0 accepté par son prédicat, mediumType sans bit DVD 0x10. Le reader ne prétend pas décoder les structures DVD ou ajouter une écriture qui n’existe pas dans cette référence. Les tailles de payload non comprises sont refusées.

Les sessions valident numéro, comptes et bornes first/last. Un bloc sans extraOffset est ignoré ; la structure extra n’est pas interprétée par le C# et aucune interprétation supplémentaire n’est inventée. Le lead-out 0xA2 fournit une adresse MSF bornée (secondes < 60, frames < 75), convertie exactement comme le C#, sans décalage de 150 frames ajouté. Sans lead-out, la fin déclarée est utilisée ; si elle n’est pas après le début, le repli est le début maximal des pistes + 1. Une session sans piste lisible est refusée.

Les footers pointent vers des chaînes non vides terminées par zéro, avec une fenêtre maximale de 4096 octets. Latin-1 ou UTF-16LE sont décodés selon le flag. `*.extension` reprend le nom du descripteur en changeant son extension ; les autres noms sont absolus ou relatifs à son dossier. Les sources sont dédupliquées sans tenir compte de la casse selon la comparaison ordinale déjà utilisée dans le projet.

## Pistes et données

Les pistes sont triées par secteur de départ dans chaque session. Une plage est limitée par les secteurs complets restants dans le MDF, la prochaine piste ultérieure dans la session utilisant le même fichier avec un offset plus grand, puis la fin de session si celle-ci reste positive relativement à la piste. La fraction terminale d’un secteur incomplet n’est pas exposée, conformément au C#.

Le payload vaut sectorSize, ou sectorSize - 96 si subchannelMode est non nul. Un payload 2352 sans le bit data 0x04 est AUDIO ; sinon 2048 devient MODE1/2048, 2336 devient MODE2/2336 et 2352 devient MODE2/2352 pour mode 2 ou MODE1/2352 autrement. Fenêtres utilisateur : 0/2352, 0/2048, 0/2336, 16/2048, 16/2336. Le secteur stocké conserve sa taille entière et ses sous-canaux : ces derniers sont une vue de 96 octets à sourceOffset + payload, avec stride sectorSize.

Les pistes utilisent la session englobante et un INDEX 1 relatif à zéro, comme le C#. Le document conserve les chemins associés, version, mediumType hexadécimal sur quatre chiffres, sessionCount et trackCount. Les sources de fichiers partagées existantes n’ouvrent pas de handle permanent ; destruction des collections locales sur exception. La composition globale des services et le raccordement à App restent distincts du présent reader.
