# Composition d’écriture

MediaWritingComposition assemble 55 écrivains dans l’ordre de MediaWritingComposition.CreateDefault du C# : 25 adaptateurs de disquettes, 4 cartouches, 9 HDD, 5 optiques et 12 bandes. Elle ne contient aucun algorithme de format. Les adaptateurs et écrivains déjà portés fournissent leurs capacités au WriterRegistry existant. Le service de lecteurs peut être partagé explicitement avec la composition de reconnaissance.

MediaImageWritingService possède une copie du registre et du service de lecteurs pendant sa tâche, conserve le document et l’écrivain, sélectionne l’écrivain avant d’ouvrir les activités, puis encadre les mécaniques d’entrée et de sortie avec les scopes existants. Le document de sortie reprend volumes, messages, métadonnées et représentation, avec sa mécanique en mode écriture. Une liste vide ou un chemin vide/blanc retourné par l’écrivain est refusé. Les exceptions des écrivains sont conservées et les activités sont terminées même sur erreur.

Les notifications publiques du MediaDriveService transmettent maintenant shared_ptr<DriveMechanics>, équivalent utile à la référence conservable de l’événement C#. Le propriétaire audio peut garder le lecteur temporaire après la fin de l’écriture, jouer sa queue sonore puis le relâcher. Le relais de la mécanique ne possède qu’une référence faible vers elle-même : il ne crée pas de cycle service/lecteur. Un propriétaire de callback doit respecter son propre cycle de vie et retirer son callback lorsqu’il se termine.

ConsoleWriter complète le lecteur console : 21 formats et 40 extensions, couples format/extension exacts, longueur storedLength strictement positive et bornée par la capacité. Les données sont lues à travers le contrat de représentation bloc et écrites atomiquement jusqu’à cette longueur, sans écrire le remplissage de la dernière banque. Aucun réordonnancement implicite des octets des formats console n’est ajouté.

## Vérifications

Compilation Debug réussie. Registre de 55 écrivains construit, identifiants uniques et capacités non vides. Une cartouche de 41 octets est reconnue puis réécrite à l’identique, sans remplissage ; la mauvaise extension cible est refusée. Une disquette IBM 720 est écrite via le service par défaut et relue via la composition de reconnaissance : taille de 737 280 octets et catégorie disquette vérifiées.

Annulation préalable sans sortie créée ; écrivain simulé renvoyant une liste vide, un chemin vide ou une erreur : diagnostic attendu conservé et activités d’entrée/sortie terminées. La tâche reste valide après destruction du service lanceur. Le destinataire d’une notification conserve la mécanique de sortie après terminaison et constate qu’elle n’est plus utilisée, puis la réinitialise et la libère.

Trois diagnostics supplémentaires générés avec Argos dans 29 langues, relus et corrigés pour les négations, les paramètres et les formulations manifestement incorrectes. Les 30 catalogues de constantes/langues compilent. Audit des constantes : 1 403 fichiers C++, 56 clés App, 29 langues ; aucune valeur applicative hors constantes nommées. Le banc, ses images, les sorties et exécutables temporaires sont supprimés après utilisation.

## Travail restant

La progression détaillée ReportInputUnitsAsync du C# dépend des fournisseurs de visualisation et reste à porter. Le service présenté ici n’invente pas de progression de remplacement. Les disques runtime modifiables et la façade générale restent dans l’objectif ; le registre complet ne certifie pas les algorithmes des 55 formats et n’a pas déclenché une nouvelle campagne de tests de chaque codec.
