# Service global de lecture

Références C# consultées sans modification : `Images/Reading/MediaImageReadingService.cs`, `Images/Reading/Recognition/MediaRecognitionRegistry.cs`, `Images/Formats/Tape/Raw/RawTapeReader.cs`, `Services/MediaDriveService.cs`. Le contrat de mécanique natif et les portées RAII de reconnaissance sont réutilisés.

`MediaImageReadingService` conserve le registre et sa fabrique de lecteurs par valeur dans la tâche asynchrone. Le service peut disparaître avant la fin de la lecture sans invalider ses dépendances. Après reconnaissance, un document déjà lié à un lecteur est conservé. Sinon, la fabrique injectée peut créer et insérer le lecteur de la catégorie reconnue ; le nouveau document partage sa représentation et préserve volumes, diagnostics, messages et métadonnées. Une insertion ou construction échouée réinitialise le lecteur créé. Le service ne possède pas de périphérique audio ni de thread permanent.

## RAW : ordre explicite

Le commentaire C# décrit l’audio brut comme dernier recours, mais le lecteur `RawTapeReader` figure également dans le registre et peut recevoir une candidature à 0,5 par extension même lorsque sa sonde réclame un choix explicite. Le service C++ prépare une vue automatique sans ce lecteur dédié : les lecteurs de disque sont donc essayés avant le repli. Le registre public conserve ses 65 lecteurs et le choix explicite de bande RAW utilise le registre complet.

Le repli n’est permis que sans `RequestedFormatId`, sur l’extension RAW normalisée, après absence de candidat ou refus de tous les candidats par les erreurs de lecture reconnues. Il crée un nouveau contexte avec le format de bande RAW imposé et conserve les paramètres de la source. Une erreur d’accès, une annulation, une erreur de configuration, un autre suffixe ou un format imposé ne sont jamais absorbés. Une chaîne vide imposée ne déclenche pas le repli.

Le contenu RAW sans structure disque identifiable n’est pas supposé être un HDD. Le contrôle de disque réel utilise un petit fichier temporaire doté d’une signature MBR, conformément au lecteur HDD natif déjà porté ; aucune règle de reconnaissance de ce lecteur n’a été assouplie.

## Validation ciblée

Compilation Debug MediaEngine réussie sans avertissement C++. Contrôle temporaire réussi :

- repli après absence de lecteur et après refus InvalidData/Unsupported ;
- propagation des erreurs d’accès et de configuration, annulation préalable ;
- absence de repli pour un autre suffixe ou un format imposé ;
- candidat disque prioritaire sur le lecteur audio RAW, même lorsque ce dernier est enregistré avant lui ;
- choix explicite de bande RAW ;
- composition native : RAW court non reconnu vers bande, RAW avec signature MBR vers HDD ;
- lecteur automatique créé et inséré une fois, accès/fin d’accès et opérations équilibrés ;
- lecteur fourni réutilisé sans insertion ni réinitialisation ;
- liaison après reconnaissance lorsqu’aucune catégorie unique n’était annoncée ;
- nettoyage après erreur et après insertion échouée ;
- durée de vie valide lorsque le service est détruit avant la récupération du résultat.

Les lecteurs de contrôle vérifient l’orchestration et le nettoyage ; ils ne remplacent pas la mécanique concrète. Le contrôle, ses exécutables et ses fichiers temporaires ont été supprimés. Audit : 1379 fichiers C++, 56 clés App, 29 langues, conforme. Aucune nouvelle clé de traduction.

## Travail restant

`MediaRecognitionComposition::readingService()` est disponible. La création automatique des mécaniques concrètes du C# (`MediaDriveService`, `MediaDriveMechanics`, sons enregistrés et synthèse) n’est pas encore portée : la fabrique est injectable et reste vide dans la composition par défaut. Ce point demeure explicitement partiel dans l’inventaire ; aucune mécanique factice n’est installée dans le logiciel. La façade publique complète et le raccordement à App restent dans le portage global.
