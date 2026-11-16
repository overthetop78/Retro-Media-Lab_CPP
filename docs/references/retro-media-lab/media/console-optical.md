# Portage des conteneurs optiques Wii U et Xbox

Référence en lecture seule consultée le 16 novembre 2026 : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Optical/WiiU/` et `Xdvdfs/` : formats, métadonnées, readers, writers et source WUX. Les neuf SHA-256 courants sont vérifiés contre l’inventaire. Les identifiants sont relevés dans les constantes C# du moteur et du système de fichiers. Pas de recherche Internet nécessaire pour reprendre ces fonctions existantes.

## Wii U : WUD/WUX

`WiiU/Container.*` sépare le profil/en-tête/index WUX et la source logique des readers/writers. WUD accepte une longueur positive d’au moins 2048, multiple de 2048, sans en-tête WUX valide ; cette reconnaissance n’affirme pas identifier le contenu chiffré du disque. WUX exige ses deux magics little-endian, un en-tête de 28 octets, une taille de bloc comprise entre 0x100 inclus et 0x10000000 exclu, une longueur logique positive multiple de 2048 et au plus INT64_MAX, un nombre d’index au plus INT32_MAX et un tableau de données stockées non vide/aligné sur la taille des blocs.

La taille des blocs n’est pas obligatoirement une puissance de deux : le C# l’autorise, le portage aussi. L’offset du tableau est l’alignement du header + indexCount*4 à cette taille. Chaque index doit désigner un bloc stocké existant. La lecture de l’index est faite par tranches bornées, au lieu d’une unique longueur int susceptible de déborder. La source possède le tableau, valide ses dimensions, conserve la source de données et permet des lectures arbitraires traversant les blocs ; aucune compression/décompression supplémentaire n’est inventée.

Le reader expose les blocs logiques 2048, un volume direct Nintendo Wii U et les mêmes clés de métadonnées que le C#. Les constantes SystemId et MediaRole servent également de clés dans le code source : ce comportement est conservé. Les données restent opaques, sans interprétation d’un système de fichiers hôte.

Le writer refuse les couvertures incomplètes/Unavailable. WUD recopie les données logiques. WUX emploie des blocs de 32768, complète le dernier par des zéros et les déduplique par SHA256 dans l’ordre de première apparition, comme le C#. Il réserve explicitement header/index/padding, écrit les blocs uniques puis complète l’index et l’en-tête dans le fichier temporaire atomique seekable existant. Cela supprime le second fichier temporaire de blocs du C# et son nettoyage séparé. Les réserves et le padding restent zéro ; aucune sortie n’est publiée avant réussite de commit. L’annulation ou une exception détruit ce temporaire via QSaveFile.

## Xbox XDVDFS/XISO

`Xdvdfs/Reader.*` reconnaît un ISO aligné sur 2048 avec au moins 33 secteurs, double magic MICROSOFT*XBOX*MEDIA au secteur 32 et à son offset 0x7ec, et une racine vide 0/0 ou un intervalle racine non vide, aligné et borné dans l’image. Le document fournit une piste MODE1/2048, un volume session 1/piste 1 et les propriétés de racine.

Le volume utilise `MediaFileSystems::FileSystemIds::Xdvdfs` (`microsoft-xdvdfs`) : correction d’un identifiant initial saisi incorrectement pendant le portage. L’exploration des répertoires et fichiers reste dans MFS ; ses constantes de structure et son alignement d’entrées ne sont pas recopiés inutilement dans MediaEngine. Le reader traite le conteneur et le writer copie atomiquement les secteurs de la piste unique autorisée par le C# ; il ne reconstruit pas un système de fichiers et ne sérialise pas d’autres structures optiques.

Les sources existantes sont réutilisées ; les readers peuvent conserver une source possédée fournie au RecognitionContext au lieu de rouvrir obligatoirement le chemin. Les écritures utilisent uniquement les primitives atomiques déjà communes au moteur. Aucun code matériel ou graphique n’est ajouté à App. La composition par défaut et le raccordement global restent à réaliser séparément.
