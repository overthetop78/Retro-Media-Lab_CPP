# Rendu mécanique et caches

RecordedRenderer possède son catalogue et injecte le chargement PCM mono flottant et la mesure des durées. Le rendu ne dépend d’aucun périphérique audio ou contrôle App. Les loaders sont des dépendances conservées par le service ; leur état capturé doit rester valide pendant son utilisation. Les opérations concurrentes doivent être terminées avant destruction du service.

Portage de RecordedMechanicalSoundFunctions : gain constant, arrondi au pair avec la primitive PCM partagée, saturation signée 16 bits, silence après un événement fini, séquence de démarrage suivie de la boucle, offset absolu et sélection/repli sur le catalogue possédé. La sélection locale est partagée sans copie des profils à chaque rendu. Les requêtes invalides et les PCM non finis utilisent le diagnostic AudioPcmData déjà traduit.

Cache par identifiant et fréquence : un producteur charge les données, les autres appels attendent la même valeur immuable. Les exceptions sont conservées comme dans Lazy C#. Le mutex de la table n’est pas maintenu pendant le chargement ; différents sons peuvent se charger indépendamment. Les durées ont leur propre cache. prepareProfile charge les PCM du profil et de son profil de base à la fréquence demandée ; les durées restent chargées séparément. Ce service permet la préparation anticipée mais n’est pas encore branché au démarrage de l’émulation.

Compilation Debug réussie, sans le premier avertissement d’export de classe (corrigé en exportant seulement les méthodes). Contrôle ciblé réussi avec PCM déterministes : démarrage/boucle, offsets, fin/silence, gain, saturation, silence à volume nul, cache et préparation, durée, 12 appels concurrents sur le même son/fréquence, erreur de chargement conservée et paramètres invalides. Audit : 1 385 fichiers C++, 56 erreurs App, 29 langues, aucune valeur applicative hors constantes nommées. Banc temporaire supprimé après utilisation.

Ce contrôle n’a pas décodé de FLAC et n’a pas mesuré la latence en émulation. Le backend réel, le rééchantillonnage et MediaDriveMechanics restent à porter ; aucun branchement factice ne remplace ces éléments.
