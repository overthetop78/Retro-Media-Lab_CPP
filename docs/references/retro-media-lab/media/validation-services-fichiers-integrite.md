# Services publics de fichiers et d'intégrité

Sources C# en lecture seule : `Services/MediaDataIntegrityService.cs`, `MediaFileSystemProcessingService.cs`, interfaces `IMediaDataIntegrity`/`IMediaFileSystemProcessing`, et `MediaFileSystems/Exploration/MediaExplorer.cs`, méthode générique `EnumerateFiles`.

`computeMediaCrc32` délègue au calcul WOZ existant et emprunte les octets sans copie. `readMediaMetadataIntOrDefault` délègue au parseur invariant MFS existant. Ces deux opérations n'initialisent pas la composition globale, les lecteurs ou les convertisseurs.

`enumerateMediaFiles` fournit un range C++20 d'entrée. Il conserve les callbacks et emprunte la racine ainsi que les collections contiguës d'enfants (`span` ou vecteur par référence). Les objets et collections doivent rester vivants et stables pendant le parcours. Les types d'entrée du consommateur sont conservés ; aucun en-tête public MFS n'est requis.

Le curseur itératif appartient à MediaFileSystems. MediaEngine le relaie par un pont opaque. La recherche du premier fichier dans MFS réutilise le même curseur. Le parcours est profondeur d'abord dans l'ordre source ; une racine-fichier est renvoyée avant consultation de ses enfants, qui restent parcourus ensuite si l'itération continue. Aucun descendant n'est matérialisé, aucune entrée n'est copiée ; la mémoire du curseur est proportionnelle à la profondeur et ne consomme pas la pile d'appels. Une exception de callback termine le curseur et reste propagée.

Validation : compilation Debug des deux DLL, audit de 1 575 sources ; programme de vérification compilé avec le seul répertoire d'en-têtes publics MediaEngine et sa bibliothèque d'import. Contrôles ciblés de délégation CRC/entiers, racine-fichier, différé, ordre, redémarrage d'un parcours, arrêt anticipé, identité des entrées empruntées, vecteurs et spans, exceptions de callbacks et chaîne plate de 50 000 nœuds parcourue sans récursion. Tous ces contrôles passent. Les valeurs de nœuds ne possèdent pas récursivement cette chaîne de vérification. Le banc temporaire a été supprimé.
