# Frontière d’exploration et conversion des volumes

Références C# consultées sans modification : `Contracts/Explorer/FileSystemVolumeMapper.cs`, `Images/Reading/SequentialContentDecoderAdapter.cs`, `MediaEngineComposition.cs` et les décodeurs dans `Images/Reading/Decoding/Sequential/`. La composition C# enregistre explicitement l’adaptateur séquentiel ; son absence dans la fabrique C++ par défaut a été corrigée.

## Diagnostics entre bibliothèques

Le diagnostic MFS peut conserver une recette de traduction provenant d’un autre composant : clé, arguments et domaine. Il garde aussi son contexte et ses causes. MFS ne référence jamais MediaEngine. `diagnosticMessage` retourne la recette d’origine si présente, sinon la recette native MFS. `MediaError::what()` conserve la clé d’origine et sa durée de vie. Le convertisseur public Engine utilise cette recette sans remplacer le domaine par MediaFileSystems.

À la frontière de l’adaptateur de bandes, les erreurs correspondant aux `InvalidDataException` explicitement identifiées dans le C# sont transformées en erreurs MFS de type InvalidData : volume séquentiel incompatible, incompatibilités des sept décodeurs, document TZX absent, nombre de bits TZX invalide et bloc TZX tronqué. Leur recette reste celle de MediaEngine. L’explorateur MFS peut alors les rapporter et poursuivre son parcours. Les erreurs inconnues, annulations, erreurs d’accès, de configuration et d’opération non prise en charge continuent à se propager. Ce changement ne prétend pas reclasser toutes les erreurs de tous les codecs.

## Volumes dans les deux sens

Le convertisseur inverse reprend les volumes, entrées récursives, contenus, dates, attributs, types natifs, informations de disque, commentaires et recettes, métadonnées, analyses et diagnostics avec leurs causes. Le contenu est copié lors du retour vers les modèles MFS, comme en C#. Le sens MFS vers Engine garde son partage de contenu possédé, y compris après libération du propriétaire initial côté appelant. Un diagnostic public est reconstruit avec son message complet ; son code interne MFS est UnknownDiagnostic, car le contrat public décrit la recette et ne transporte pas ce code interne.

Le service de migration réutilise ce convertisseur général ; sa projection privée des entrées a été retirée. L’explorateur Engine par défaut enregistre l’adaptateur utilisant la composition commune des décodeurs séquentiels. Aucun accès direct d’App à MFS ou MediaAnalysis n’est ajouté.

## Validation

Compilation Debug de MediaFileSystems et MediaEngine réussie sans avertissement C++. Contrôle temporaire ciblé réussi :

- aller-retour d’un volume et d’entrées imbriquées renseignés, dates et données facultatives, analyses, commentaires traduisibles, messages, contextes et causes ;
- copie indépendante vers MFS et conservation du contenu partagé Engine après libération du propriétaire initial ;
- conservation de la clé externe par `what()` et domaine MFS des erreurs natives ;
- erreur de décodage injectée par un décodeur de contrôle : rapportée avec clé Engine, argument, domaine et contexte du lecteur ;
- propagation distincte des erreurs d’accès MFS, annulation Engine, configuration et opération non prise en charge ;
- succès d’un décodeur de contrôle avec extraction d’un vrai nom fourni ;
- exploration d’un document Atari CAS vide avec la composition native par défaut : le lecteur séquentiel est effectivement choisi ; ce cas ne constitue pas une nouvelle validation des protocoles de bande ;
- création d’une migration FAT12 via le convertisseur commun.

Audit : 1377 fichiers C++, 56 clés App, 29 langues, conforme. Aucune nouvelle clé de traduction : les recettes existantes sont conservées. Le contrôle temporaire et ses sorties ont été supprimés. Le raccordement général à App et l’audit final du portage restent ouverts.
