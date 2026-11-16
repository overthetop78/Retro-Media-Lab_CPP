# Sessions optiques et audio associé

Référence C# consultée en lecture seule le 16 novembre 2026 : `Images/Reading/Optical/OpticalCompanionAudio.cs`, OpticalDiscSession.cs, OpticalSectorReader.cs, `Contracts/Optical/OpticalDiscTrack.cs` et `Enums/OpticalTrackMode.cs` dans `F:/GW GUI/src/RetroMediaLab.MediaEngine`. Les services MediaImageReadingService, MediaImageWritingService et OpticalImageConversionService sont aussi examinés pour identifier les dépendances encore absentes. Les quatre sources de l’inventaire modifiées par ce bloc sont vérifiées par SHA-256.

## Audio associé explicite

`Images/Optical/CompanionAudio.*` fournit attachOpticalCompanionAudio sur un document déjà reconnu. Il ne modifie pas la reconnaissance globale et n’ajoute pas d’audio sans demande d’un propriétaire de session. Il s’applique uniquement à ISO/BIN contenant une seule piste de données. Vingt conventions sont essayées dans l’ordre C# : OGG puis WAV ; préfixes vide, espace, tiret, souligné et espace-tiret-espace ; numéros sur deux chiffres puis non complétés. Dans chaque convention, le fichier 1 est essayé avant le fichier 2, et le décalage de séquence retenu sert pour les suivants. Le premier fichier manquant arrête la séquence, maximum 99 pistes au total.

Les sources Vorbis et WAVE déjà portées sont réutilisées. Vorbis exige stéréo 44100 Hz ; WAVE garde l’option C# par défaut exigeant des secteurs PCM complets. Un dernier secteur OGG partiel est complété de zéros par une source possédée. L’heuristique de pregap stocké est reprise sans la transformer en détection plus large : si le compte dépasse 150 et que les quatre octets au secteur 100 sont nuls, les 150 premiers secteurs sont considérés comme pregap stocké. Chaque piste audio commence après 150 secteurs de pregap déclaré ; le postgap initial de la piste de données est pris en compte.

Les erreurs d’I/O, fichier absent et format non pris en charge interrompent la recherche, conformément aux classes d’exception C#. Une donnée invalide ne devient pas une absence silencieuse : le type de base InvalidDataException est vérifié dans le runtime .NET local (`System.SystemException`, pas IOException). Les échecs de construction libèrent les nouvelles sources via shared_ptr ; le document original reste inchangé. Les chemins associés, compteurs et références de données sont transférés uniquement au nouveau document retourné.

## Cœur de session et contrat public

OpticalTrackMode est déplacé dans l’en-tête public du moteur ; sa valeur et son namespace restent inchangés. OpticalDiscSession.h expose un résumé de piste indépendant de MFS, des secteurs stockés, des sous-canaux, le précache et close. Son constructeur reçoit un document moteur opaque déjà ouvert ; il n’expose aucun lecteur de système de fichiers.

La session trie les pistes par numéro, conserve les informations de boucle REM:LOOP/REM:NOLOOP avec repli sur les métadonnées du document pour la première piste triée, et signale l’audio big-endian pour le format CHD exact. Les secteurs de pregap stocké sont lisibles par indices négatifs ; les pregaps seulement déclarés ne sont pas inventés comme des secteurs de données. Les tailles et limites utilisent les validations des modèles existants.

Le précache décode les secteurs et sous-canaux dans des collections locales et ne publie le cache qu’une fois tous les accès réussis. Une annulation/exception libère ces collections et laisse la lecture streaming disponible. La limite de buffer par piste reste INT32_MAX comme les tableaux C# ; le cache de sous-canaux emploie 96 octets par secteur. Les accès sont sérialisés avec close/précache. Les futures capturent l’état possédé ; après close, elles échouent explicitement plutôt que d’accéder à des sources libérées.

close est idempotent : document, pistes et caches sont libérés avant reset de la mécanique de lecteur, même si reset lève une exception. Le destructeur ferme sans propager d’exception. Un échec d’initialisation libère l’état partiel puis réinitialise la mécanique si elle existe. Les shared_ptr préservent les ressources encore empruntées ailleurs : aucune source partagée n’est détruite arbitrairement tant qu’un autre propriétaire la détient.

## Raccordement restant

L’OpenAsync par chemin du C# dépend de la composition globale des readers et du service de lecture, dont le repli RAW utilise également les readers de bandes non encore portés. La conversion optique dépend en plus du service global de writing, de ses mécaniques et de sa progression de visualisation. Aucun registre optique concurrent ni service incomplet parallèle n’est créé pour contourner ces dépendances. Ces deux actions restent ouvertes dans la tâche 158 ; OpticalDiscSession demeure partiel dans l’inventaire. Les services de conversion ne sont pas déclarés portés.

## Actualisation du raccordement

Le paragraphe « Raccordement restant » decrit le releve initial. Ces dependances sont maintenant raccordees : `Images/Optical/OpenSession.cpp` (bloc 235), `Images/Conversion/Optical.h` et `Optical.cpp`, puis `Composition/Runtime.h`. Aucun registre optique concurrent n’a ete ajoute.
