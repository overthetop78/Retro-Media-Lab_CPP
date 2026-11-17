# Zoom des faces

La commande « Lier les zooms » reprend la commande Visual.LinkZoom du C#. Elle apparaît seulement pour un média graphique à plusieurs faces. En mode indépendant, chaque face conserve zoom et déplacement ; le changement de face restaure ses valeurs. En mode lié, la transformation de la face active est partagée. Le zoom reste centré sur chaque face et le dessin est découpé dans son propre cadre.

Vérification ciblée réussie sur une petite image FAT12 à deux faces : molette sur chaque face, déplacement indépendant, sélection d’un secteur après transformation, liaison, remise à zéro et conservation après reconstruction réelle du thème. La session appartient à Infrastructure ; les vues ne stockent pas les transformations. Les appels du rendu de flux limitent explicitement chaque peinture à la face demandée, pour éviter le débordement d’une autre face déplacée.

La capture independent.png montre le rendu Qt avec deux transformations distinctes. Les fichiers temporaires du programme et du média de vérification ont été supprimés.
