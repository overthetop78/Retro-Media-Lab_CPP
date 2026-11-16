# Mécaniques des lecteurs et composition

RecordedDrive implémente le contrat DriveMechanics existant. Mechanics.cpp porte les transitions de MediaDriveMechanics.cs, Audio.cpp le mixage de MediaDriveMechanics.Audio.cs. Le lecteur possède son état et ses files, sans thread ni sortie audio. Le cache de la banque est partagé ; aucune dépendance à App ou Infrastructure n’est introduite.

État verrouillé : insertion, éjection, rotation, activité, déplacement et avance continue. Les opérations et accès imbriqués partagent la rotation ; le dernier EndOperation attend la fin des accès avant d’arrêter le moteur. Les sauts logiques déclenchent un déplacement tandis que les incréments séquentiels et changements de face seuls n’en créent pas. Les compteurs et offsets sont protégés contre le dépassement des entiers C++.

Les notifications sont émises hors du verrou et peuvent consulter l’état. Les 32 voix de déplacement maximales jouent simultanément, les plus anciennes étant retirées au dépassement. Les transitoires, le moteur et les voix sont mélangés ; le gain final atténue uniquement un pic dépassant le PCM 16 bits. Les bandes conservent leurs commandes et offsets propres. La désactivation vide l’audio ; la réactivation avec moteur en marche repart au début du son moteur. Reset efface état et événements.

MediaDriveService crée les lecteurs disquette/HDD/optique, relaie les notifications à un propriétaire sans conserver de collection de lecteurs, et peut survivre par son état partagé dans une fabrique asynchrone. La composition de reconnaissance par défaut fournit cette fabrique au service de lecture : les images reconnues reçoivent maintenant une mécanique concrète, sans recopier leur contenu. Les médias sans mécanisme disque ne reçoivent pas de lecteur automatiquement. Une mécanique fournie par l’appelant reste conservée.

## Vérifications

Compilation Debug MediaEngine réussie sans nouvel avertissement. Contrôle ciblé réussi : insertion idempotente, état, opérations et accès imbriqués, arrêt différé, retrait du son désactivé puis réactivation, limite de 32 voix, limitation des pics, position séquentielle et changement de face, bandes, remise à zéro, paramètres invalides, remise en état après exception dans des scopes RAII, 8 threads d’accès, consultation de l’état depuis les callbacks et relais du service. Lecture via la composition réelle d’une petite image RAW HDD avec signature MBR : mécanique attachée et insertion vérifiées. Banc et image temporaires supprimés après utilisation.

Erreur MechanicalRequest générée avec Argos dans 29 langues, relue et corrigée notamment pour le sens sonore de volume et les caractères traditionnels chinois. Compilation des 30 catalogues Constants/langues réussie. Audit : 1 394 fichiers C++, 56 clés App, 29 langues ; aucune valeur applicative hors constantes nommées.

Ce bloc ne raccorde pas la sortie audio de l’application, ne valide pas une émulation complète et ne porte pas encore l’ancien catalogue de profils synthétiques. Le rendu réel repose sur la banque FLAC déjà portée.
