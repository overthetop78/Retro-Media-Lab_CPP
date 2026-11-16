# Ouverture et exploration communes

Références C# en lecture seule : `Images/Reading/MediaImageExplorationService.cs`, `MediaOpeningAnalysisService.cs`, `Contracts/Explorer/MediaOpeningAnalysisResult.cs`, `Contracts/MediaExplorationProgress.cs` et `Enums/MediaExplorationProgressStage.cs`.

`exploreMediaAsync` lit avec repli sans indication de format uniquement lorsque cette indication n'est pas prise en charge, indexe les blocs de bande puis explore les systèmes de fichiers. Les trois notifications conservent les étapes et valeurs C# : 58, 78, 94.

`analyzeMediaOpeningAsync` lit et indexe un seul document. Sans demande de systèmes de fichiers, il retourne ce document seul. Sinon, l'exploration disque démarre sur un travail asynchrone possédé pendant que l'exploration commune utilise le même document. Les notifications d'ouverture conservent les valeurs 12, 45, 94. Pour une source de flux, l'exploration commune attend les secteurs décodés et les réutilise ; si le format reste inconnu, elle conserve la source de flux. La notification de capture fournit un instantané public possédé avec en-tête, pistes et révolutions.

La composition réutilise les services de lecture, disque et fichiers existants. La classification des formats non pris en charge est partagée avec l'explorateur disque. Les exceptions de corruption, d'entrée/sortie et d'annulation ne déclenchent pas ce repli. Aucun travail asynchrone n'est détaché ; les futurs possédés rejoignent leurs tâches même lorsqu'une autre exploration échoue.

Validation Debug : compilation du bloc et de ses dépendances, audit des littéraux (1 540 sources, 56 clés d'erreur, 29 langues), vérification ciblée de lecture unique avec lecteurs décorés par un compteur, repli d'indication, progression, ouverture sans explorations, annulation, réemploi des secteurs issus d'un décodeur injecté et capture conservée après le rappel. Les deux façades publiques passent sur une petite image FAT12 temporaire.

Le document d'exploration peut être enrichi des volumes détectés : il conserve la même source et la même représentation sans imposer l'identité du pointeur document. Le contrôle vérifie ce partage et le compteur de lecture. Le banc temporaire et ses médias ont été supprimés ; les codecs existants n'ont pas été retestés.
