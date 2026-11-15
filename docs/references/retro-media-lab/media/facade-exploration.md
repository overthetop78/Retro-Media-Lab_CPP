# Résultats d’exploration remis par Engine

Références C# consultées en lecture seule : `Images/Reading/MediaExplorer.cs`, `Contracts/Explorer/FileSystemVolumeMapper.cs`, `ExploredMediaImage.cs`, `ExploredMediaVolume.cs`, et les contrats d’entrées/volumes MFS liés par le projet Engine.

`ExplorationInfo.h` fournit les données destinées aux consommateurs Engine sans inclure MFS ni Analysis : entrées, horodatages, volumes, analyse des fichiers, métadonnées et diagnostics structurés avec contexte/causes. Les clés, domaines et arguments de traduction des noms, commentaires et diagnostics restent disponibles. Le document interne est un handle opaque ; les informations utiles à l’affichage sont accessibles séparément dans le résultat.

Le mapper interne convertit MFS vers ces vues. Les contenus binaires sont partagés en lecture seule par propriétaire aliasé : conserver un contenu maintient le volume ou l’entrée analysée en vie. Les chaînes et petites collections sont copiées comme données de résultat. L’absence de liste analysée est distinguée d’une liste analysée explicitement vide. La conversion inverse du mapper C# n’est pas encore portée et reste signalée dans l’inventaire.

Le service interne `MediaExplorer` délègue l’exploration à MFS. Il entoure l’opération du scope du lecteur, enrichit le document lorsque les descripteurs de volumes diffèrent et préserve son identité lorsqu’ils sont identiques. La reconstruction conserve messages, métadonnées et représentation via `withVolumes`. Les analyses proviennent de MFS, qui appelle Analysis ; Engine n’ajoute aucun accès direct à Analysis.

Les anciens diagnostics textuels et les nouvelles recettes structurées restent séparés pour ne pas figer une langue au moment de l’exploration. La conversion des messages et descripteurs, auparavant locale à `MediaDocument.cpp`, est désormais partagée avec le mapper.

Ce service interne n’est pas encore le service public complet d’ouverture de média ni l’intégration dans App. Les services de lecture, d’indexation des bandes et d’ouverture/analyse restent à terminer.
