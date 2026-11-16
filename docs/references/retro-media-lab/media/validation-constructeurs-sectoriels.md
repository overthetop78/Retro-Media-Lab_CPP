# Constructeurs sectoriels communs

## Organisation et données

`Models/Sectors/Builders` centralise la construction linéaire, régulière, IBM et Macintosh GCR, ainsi que la densification. Les dimensions positives, le nombre de blocs et la capacité des géométries linéaires gardent la limite int32 vérifiée des constructeurs C#. Les images disque dur et autres géométries natives 64 bits ne sont pas modifiées.

Les octets fournis en mémoire sont copiés dans une source possédée ; les blocs sont matérialisés à la demande. Cela conserve les données sans créer systématiquement un objet et une allocation par secteur. Les sources IBM/MSX déjà accessibles aléatoirement restent possédées et lues à la demande : pas de chargement intégral ajouté.

- Ordre cylindre/face/secteur et numérotation zéro/un conservés.
- Le constructeur régulier valide exactement capacité + octets finaux autorisés, puis exclut ces derniers de l'image. Comme le C#, un premier numéro non nul sélectionne la numérotation à partir de un.
- IBM impose les blocs de 512 octets et la numérotation à partir de un.
- Macintosh conserve les capacités 400/800 Kio et l'adressage zoné partagé. Le nombre de blocs est déclaré explicitement, indépendamment du maximum de secteurs d'une piste.
- La densification renvoie les positions absentes. Les doublons écrasent les octets fournis sans effacer le reste du bloc déjà copié. Une donnée dépassant un emplacement peut traverser le suivant tant qu'elle reste dans le tampon total, conformément au `CopyTo` du C# ; elle ne marque pas cet emplacement suivant comme présent. Les limites sont contrôlées avant copie.
- La sélection Apple accepte un prédicat réutilisable ; l'intégrité explicitement vraie prime, les égalités conservent le premier exemplaire. Les sondes DOS/ProDOS appellent la densification commune ; elles évitent toujours une allocation dense lorsque leur ensemble de blocs est incomplet.

## Intégration et vérification

Lecteurs IBM/MSX et Apple Raw raccordés. Les pistes Apple décodées réutilisent la sélection et les tampons communs. Un banc unique vérifie les adresses CHS, dimensions/capacités, octets finaux, possession/libération des sources, copie mémoire indépendante, lecture IBM toujours différée, limites des zones Macintosh, densification partielle/dupliquée et débordements, priorité Apple et intégration des lecteurs, ainsi que l'annulation.

Compilation Debug MediaEngine sans avertissement ; Constants et 29 langues compilés. Paramètres des catalogues vérifiés. Audit global de 1 453 fichiers C++ réussi après correction d'une ancienne traduction thaïlandaise qui omettait SCP dans `ScpTrackAddress`, révélée par l'ajout des invariants du bloc précédent.

Banc et journaux temporaires supprimés après utilisation. Pas de tests ajoutés au workflow.

## Références

C# lu sans modification : `RegularSectorGeometry`, `RegularSectorImageBuilder`, `LinearSectorImageGeometry`, `LinearSectorImageBuilder`, `IbmRawSectorImageBuilder`, `MacintoshGcrSectorImageBuilder`, `SectorImageBuilderExceptions`, `AppleIISectorImageBuilder`, `DecodedAppleSectorSelection`. Correspondances et empreintes mises à jour dans l'inventaire. Ce bloc termine ces constructeurs communs ; les lecteurs SCP Apple et autres services non portés restent dans l'inventaire général.
