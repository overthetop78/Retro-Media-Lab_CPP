# Catalogue effectif des formats

Références C# en lecture seule : `Images/Formats/ImageFormatWorkspace.cs`, `Constants/ImageFormatFallbackExtensions.cs` et les fonctions de `DiskDefsFormatReader` déjà portées.

`ImageFormatWorkspace` publie un instantané possédé comprenant les capacités fournies, le catalogue effectif et son détecteur. Le catalogue est celui déjà possédé par le détecteur, sans deuxième copie. Un appelant peut conserver l'ancien instantané et continuer à l'utiliser après une reconstruction.

La construction reprend les identifiants intégrés des définitions, ceux des capacités fournies et les arguments GW des formats physiques du catalogue de base. Les extensions effectives incluent toujours IMG/SCP/HFE. Ces ajouts ne sont pas enregistrés comme capacités fournies. Les doublons sont éliminés sans tenir compte de la casse.

L'ajout d'un fichier de définitions reprend les identifiants et imports du lecteur existant ; il ne remplace pas les paramètres des définitions embarquées, comme en C#. Les extensions de repli SCP/IMG/IMA/HFE ne sont utilisées que si aucune extension n'était fournie. Les textes restent des clés résolues à l'affichage : reconstruire le catalogue ne fige pas une langue.

La nouvelle construction est terminée avant remplacement de l'état. Un import défaillant conserve l'instantané précédent, y compris ses capacités et son détecteur. Cette garantie concerne une utilisation à mutations non concurrentes ; aucun accès simultané aux mutateurs n'est revendiqué.

Validation : compilation Debug, audit des littéraux, contrôle ciblé du catalogue initial, partage du stockage avec le détecteur, distinction capacités fournies/effectives, normalisation, remplacement et reconstruction, imports utilisateur et extensions de repli, conservation de l'ancien instantané et absence de publication partielle sur erreur. Tous les contrôles passent. Le banc et ses fichiers de définitions temporaires ont été supprimés.
