# Pistes DEC RX01/RX02

Bloc 50 — portage de DecRx02TrackEncoder, DecRx02Decoder et DecRx02Descriptions, complétant DecRx02Format et les primitives M²FM/marques du bloc 49. Quarante et un fichiers C# originaux sont compilés sans modification pour la comparaison. Les dumps RX02, leurs ordres sectoriels et les politiques de reconstruction ne sont pas inclus dans ce bloc.

## Organisation et encodage

Codecs/DecRx02 conserve les définitions et la transformation M²FM communes. Encoder.cpp écrit les pistes ; Decoder.cpp reconnaît et apparie les champs ; Descriptions.cpp produit des descriptions à clés et paramètres. Les CRC réutilisent Crc16 avec polynôme CCITT 1021 hexadécimal et initialisation FFFF.

Après les contrôles communs de requête, l’encodeur valide cylindre/face puis chaque numéro de secteur sur un octet. Il sélectionne ensuite la taille : 128 octets FM avec code zéro, ou 256 M²FM avec code un. Un code fourni explicitement doit correspondre à cette taille ; le numéro est validé avant la taille, et la taille avant le code. Un contrôle préalable C++ de la taille du vector évite de la convertir hors int32. Les adresses et CRC enregistrés dans les métadonnées restent ignorés comme dans cet encodeur C# ; Deleted et SizeCode sont bien pris en compte.

L’en-tête commence par le motif physique FE de 32 cellules. Le CRC porte sur FE/C/H/R/N. Les six octets après FE (adresse, taille, CRC fort/faible) utilisent DoubleFm, soit 32 cellules par octet. Un gap de 64 cellules à un suit l’en-tête. Les données normales/supprimées sélectionnent FB/F8 en FM et F9/FD en M²FM. Leur CRC inclut l’octet de marque et les données. En FM, données et CRC sont écrits en DoubleFm. En M²FM, une cellule de phase nulle suit la marque ; les octets MFM transformés par le service du bloc 49 viennent ensuite. Un gap de 64 cellules à un termine chaque champ de données. Les secteurs peuvent mélanger les deux tailles et conservent l’ordre demandé.

## Décodage

Le flux passe par la reconstruction MFM adaptative commune, y compris les parties FM comme dans la source. Les six octets d’en-tête sont lus avec le lecteur FM de 32 cellules ; les bits d’horloge ne sont pas validés séparément. Un en-tête tronqué garde une structure de 32 cellules. Un en-tête complet à CRC faux garde seulement sa structure de 224 cellules, sans secteur ni octets décodés d’identité.

Après un en-tête valide, ses quatre octets C/H/R/N sont ajoutés aux octets décodés. Le décodeur recherche la première des six marques F8 à FD depuis la fin de l’en-tête, à distance maximale 1 664 cellules, borne inclusive. L’ordre des définitions est conservé.

Si le code de la marque ne correspond pas à N, aucun secteur n’est créé. Les quatre octets d’identité restent présents ; la description d’en-tête indique le format associé à la marque et l’état faux du champ de données, sans prétendre avoir lu son CRC. La marque reste non classée pour la passe finale. Si aucune marque n’existe, un secteur incomplet est conservé avec la taille de repli 128, le code N original et une intégrité indéterminée ; la description sans marque indique la taille zéro, conformément au C#.

Une marque compatible impose la taille de lecture 128/256. En M²FM, la cellule de phase est sautée et le helper du bloc 49 inverse les motifs ; une paire 11 invalide rejette le bloc. En FM, le lecteur DoubleFm extrait les octets. Un bloc complet garde ses données même si son CRC est faux et classe la marque. Un bloc tronqué ou invalide garde un secteur incomplet et laisse la marque non classée. Seuls les blocs complets ajoutent leur charge utile aux octets décodés.

Les faces et cylindres sont ceux réellement décodés. Les données de secteurs utilisent IntegrityKind::Crc ; tags, format, CRC enregistrés, intégrités séparées et DeletedDataMark restent absents comme dans le modèle produit par le décodeur source. L’état supprimé est néanmoins conservé dans les descriptions via la valeur de marque. La longueur d’une structure de données est 32 + (taille + 2) × 32 en FM, et 32 + 1 + (taille + 2) × 16 en M²FM.

Le balayage d’en-têtes avance de 32 cellules après chaque reconnaissance, sans sauter les charges utiles. La collecte finale garde les marques de données non classées sous forme de structures de 32 cellules. Le tri est stable par position. Plusieurs en-têtes peuvent s’apparier à la même marque de données, comme dans le C#. Aucun suffixe circulaire n’est ajouté. La confiance commune utilise poids sectoriel deux et diviseur vingt.

## Ressources et traductions

Tous les résultats et tampons temporaires sont possédés par RAII et les boucles contrôlent l’annulation. Les sept textes sont générés avec Argos puis relus/corrigés dans les 29 langues ; les descriptions de CRC et de marques réutilisent les formulations ISO FM déjà corrigées. Les invariants DEC/RX02/FM/M²FM restent inchangés.
