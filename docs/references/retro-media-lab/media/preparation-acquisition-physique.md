# Préparation des acquisitions et écritures physiques

Référence : contrats `MediaPhysicalDataUnit`, `MediaAcquisitionResult`, `MediaAcquisitionProgress`, `MediaWritePlan`, `MediaPhysicalWriteProgress`, `MediaPhysicalWriteResult`, interfaces `IMediaAcquisitionProvider` et `IMediaPhysicalWriter`, fonctions `MediaTapePhysicalDataFunctions`, services `TapeMediaAcquisitionService`, `TapeMediaWritePlanningService`, `FloppyFluxAcquisitionService` et `FloppyMediaWritePlanningService` du dépôt C# conservé en lecture seule.

## Frontière publique

`PhysicalMediaTypes.h` fournit des valeurs possédées et des interfaces abstraites. Les fournisseurs matériels implémentent ces interfaces hors du GUI. Aucun accès Windows, port série, périphérique audio ou commande de contrôleur n'est exécuté par cette préparation.

Les données d'une unité sont immuables et partagées entre copies ; les métadonnées sont possédées. Le résultat d'acquisition trie les unités par position de manière stable. Le plan conserve l'ordre fourni et accepte un sous-ensemble d'index uniques, comme le C# ; il n'impose pas d'écrire toutes les unités.

## Bandes

Le format neutre est du PCM mono signé 16 bits little endian, sans rééchantillonnage ni modification d'amplitude. La position d'une unité est l'index du premier échantillon. Le réassemblage exige des fragments contigus depuis zéro et une fréquence identique. La préparation depuis un fichier délègue à `readTapeAsync`.

## Disquettes

Les unités de flux utilisent la sérialisation neutre existante en nanosecondes. L'acquisition construit du SCP à 25 ns. La conversion arrondit le temps cumulé, avec la règle de demi à pair déjà utilisée dans le moteur, pour éviter de cumuler les erreurs d'arrondi de chaque intervalle. Un intervalle qui disparaîtrait à cette résolution est refusé.

Les pistes doivent être uniques et posséder le même nombre de révolutions, entre 1 et 64. Le type SCP est obligatoire ; les noms C# sans distinction de casse et les valeurs numériques sur un octet sont acceptés. La capture publique possède indirectement les buffers via le SCP partagé. Le document reste opaque pour App.

L'écriture prépare une unité par piste triée par cylindre et tête. Le flux choisi est converti en nanosecondes avec contrôle de dépassement. Pour une image de secteurs, les encodeurs physiques existants sont utilisés. Pour un document de flux, les révolutions capturées sont réutilisées directement. Aucun test de lecteur ni écriture matérielle n'est déclenché.

Les erreurs utilisent des clés du catalogue `MediaEngine/physical.json`, avec paramètres techniques séparés des textes traduits. Les diagnostics retournés par le matériel restent des données fournies par le fournisseur.

## Validation du bloc 229

Compilation Debug réussie, sans avertissement du compilateur. Le contrôle public temporaire a vérifié les bornes des contrats, le partage immuable des octets, les plans partiels et index dupliqués, le PCM signé little endian fragmenté, les erreurs d’ordre et de fréquence, les types SCP nommés et numériques, l’arrondi cumulé, les pistes dupliquées et révolutions incohérentes, la conservation des buffers capturés, les plans triés, les dépassements numériques et l’annulation. Il n’a utilisé ni périphérique physique ni fichier média. Son dossier et ses artefacts ont été supprimés.

Audit des littéraux réussi sur 1 583 fichiers C++ ; `git diff --check` sans erreur. Les 29 catalogues ont été générés avec Argos puis relus et corrigés, notamment pour le sens mécanique de « révolution », les messages partiellement non traduits et le chinois traditionnel. Le banc contrôle les nouveaux contrats et raccordements ; il ne revalide pas les codecs et n’atteste pas les capacités d’un appareil réel.
