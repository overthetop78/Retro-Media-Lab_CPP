# Runtime des disques sectoriels et blocs

Référence consultée : `MediaRuntimeDisk.cs`, `MediaRuntimeDisk.BlockFormat.cs`, `IMediaRuntimeDisk.cs`, `MediaDiskSector.cs` dans le dépôt C# en lecture seule. Le contrat C++ public ne révèle pas les types internes de MediaFileSystems. La géométrie existante est partagée dans un en-tête public, avec son implémentation unique conservée.

Le runtime réutilise les compositions de reconnaissance et d’écriture existantes. Les lectures retournent des copies des données et des informations d’intégrité. Les changements restent en mémoire. Une sauvegarde reconstruit la représentation, écrit via l’écrivain du format puis rouvre l’image avant d’effacer les changements. Un échec conserve ceux-ci pour réessayer. Les plages d’un HDD restent référencées avec leurs offsets ; seule la partie modifiée est possédée en mémoire. La préparation de géométrie Acorn préserve les octets existants, tronque les plages si nécessaire et conserve les octets supplémentaires indisponibles tant qu’ils ne sont pas écrits.

Les tâches asynchrones possèdent l’état nécessaire jusqu’à leur fin. Les accès mécaniques sont encadrés par les scopes RAII existants. La fermeture explicite tente la sauvegarde et réinitialise une mécanique possédée même sur exception ; elle préserve une mécanique empruntée. Le destructeur assure le nettoyage sans lever d’exception : l’appelant utilise `close()` pour recevoir une erreur de sauvegarde finale. Comme en C#, les commandes d’un même disque doivent être sérialisées par son propriétaire.

## Vérification ciblée

Compilation Debug de MediaEngine réussie. Une image IBM 720 vérifie les adresses, neuf secteurs de piste, les copies indépendantes, les écritures et leur relecture après sauvegarde. Annulation d’une sauvegarde avec changements conservés ; indices, tailles et marque supprimée non prise en charge refusés ; protection écriture ; fermeture possédée et empruntée vérifiées.

Un HDD brut de quatre secteurs vérifie la superposition puis la sauvegarde. Un chemin de destination volontairement indisponible provoque un échec : les changements restent lisibles et la tentative suivante réussit. Un descripteur Acorn modifié est sauvegardé puis relu ; le Base64 invalide est refusé. Le passage de 33 à 66 secteurs conserve les données, refuse une lecture et une publication des nouvelles plages indisponibles, puis réussit après remplissage explicite. Une fermeture dont la sauvegarde échoue libère aussi la mécanique possédée.

Les cinq diagnostics sont générés avec Argos dans les 29 langues puis relus ; les erreurs manifestes, la négation coréenne et le chinois traditionnel sont corrigés. Les 30 DLL de catalogues compilent. Audit des constantes : 1 410 fichiers C++, 29 langues, aucune valeur applicative hors constantes nommées. Le banc et toutes ses petites images temporaires sont supprimés après utilisation.

## Suite du portage

Ce bloc couvre les images déjà décodées en secteurs ou blocs. Les méthodes physiques de lecture de piste, fenêtres d’écriture et formatage de piste, les projections différées et les backends runtime Flux/MFM restent à porter. Cette vérification ne certifie pas ces fonctions absentes et ne répète pas les campagnes des codecs déjà contrôlés.
