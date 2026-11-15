# FileCore — cartes Acorn

Référence directe : les onze fichiers `FileSystems/Acorn/FileCore/`, `IFileCoreAddressResolver.cs` et `AcornAdfsNameCodec.cs` dans `F:/GW GUI/src/RetroMediaLab.MediaFileSystems`. Les 13 SHA-256 correspondent à l'inventaire ; aucune source C# n'a été modifiée.

Le composant reste dans MediaFileSystems : les cartes d'allocation décrivent le placement des objets du système de fichiers. Il ne dépend ni du GUI ni de MediaEngine. Les API et résolveurs sont regroupés dans `FileCore.h`, avec trois fichiers internes : Codec, OldMap et NewMap. Les constantes de disposition sont isolées. Les zones privées possèdent leurs octets ; les résolveurs mémorisent les dimensions nécessaires et ne gardent aucun emprunt à l'image fournie au constructeur. Leur espace libre est calculé à la construction.

## Anciennes cartes

Les noms de volume sont recomposés à partir des deux demi-champs entrelacés, puis décodés sur sept bits selon ADFS. Les 82 longueurs libres de 24 bits sont additionnées et bornées à la capacité. L'adressage emploie les unités FileCore de 256 octets ; les options BBC ADFS L convertissent l'ordre par face en pistes entrelacées dans le conteneur. Une adresse ou un offset hors capacité ne produit aucun accès.

## Nouvelles cartes

Le bloc d'amorçage vérifié est essayé en premier, puis le DiscRecord de la carte au bloc zéro. Le DiscRecord de la carte trouvée prévaut sur celui d'amorçage. Taille sectorielle, nombre de zones, identifiants, bits de réserve, racine et taille disque sont validés. Les deux copies sont essayées ; une première copie corrompue permet de retenir la seconde.

Chaque zone est contrôlée par son terminateur FreeLink, sa somme de contrôle et ses limites utiles ; le contrôle XOR couvre l'ensemble des zones. Les zones décrivent les fragments, leurs liens libres et les offsets de partage. La recherche parcourt les zones depuis celle correspondant à l'identifiant ; les fragments libres sont exclus de la résolution. Les fragments successifs portant le même identifiant consomment l'offset demandé. La dernière zone est bornée à la capacité décrite.

## Renforcements explicites

Les décalages et conversions d'adresses vérifient les dépassements numériques ; le remappage d'une ancienne carte vérifie ses multiplications avant de les exécuter. Les boucles de construction et résolution honorent l'annulation. Les ressources possédées utilisent RAII, y compris lors d'une exception de validation.

Le lecteur de bits C# emploie une fenêtre fixe de quatre octets qui échoue lorsqu'un champ de 32 bits non aligné nécessite cinq octets. Le lecteur C++ utilise une fenêtre de 64 bits chargée uniquement sur la plage requise : ces champs bornés sont lisibles, sans lecture hors limites. Les comparaisons C# excluent cette défaillance de référence, et sept contrôles autonomes la couvrent côté C++.

Les trois diagnostics FileCore sont des codes du domaine MediaFileSystems, avec arguments séparés et catalogues dans les 29 langues. Les refus de détection de carte invalide restent des résultats négatifs ; ils ne sont pas convertis en exceptions anglaises.

Ce bloc fournit l'allocation FileCore. Le parcours des répertoires et fichiers, ses formatters et ses conteneurs sont décrits dans [ADFS](adfs.md) et [ADF](adf.md) ; le composant FileCore seul ne constitue pas un lecteur ADFS complet.

## Schéma réutilisé par ADF

Le bloc ADF expose les positions communes de DiscRecord dans `FileCoreLayout.h` et réutilise la somme de contrôle FileCore ainsi que la capacité déclarée de l'ancienne carte. Le schéma n'est pas recopié dans MediaEngine. Les cartes E/F et leur amorçage sont de nouveau exercés par le [banc ADF](../../../validation/media/adf.md).
