# Lecteurs configurés dans la barre d'état

Référence C# consultée en lecture seule : `src/RetroMediaLab.App/Services/Hardware/HardwareSelectionController.cs` et `Theme/Base/Views/Controls/Shell/ApplicationStatusBar.xaml`.

Le texte fixe est remplacé par les appareils enregistrés. Un seul lecteur est présenté directement ; plusieurs lecteurs ouvrent un sélecteur, comme en C#. Le libellé des disquettes reprend DriveChoice dans les 29 langues, avec le type de contrôleur pour distinguer Greaseweazle et KryoFlux. Les autres appareils utilisent leur nom ; les adresses techniques restent dans leur configuration. Un appareil débranché reste dans la liste avec son état et un témoin grisé.

La sélection est conservée par identifiant dans WorkspaceState, indépendamment des fenêtres du thème. L'enregistrement, la suppression et la mise à jour de disponibilité rafraîchissent la barre. Supprimer le lecteur sélectionné revient au premier appareil restant. Aucun appareil enregistré conserve le message traduit d'absence de configuration.

Validation Windows du 14 novembre 2026 : cas vide/un lecteur/plusieurs lecteurs, Greaseweazle/KryoFlux/optique, changement de disponibilité et de couleur du témoin, sélection, sauvegarde/suppression, changement fr-FR/en-US et reconstruction de la présentation vérifiés. Rendu à 1100 × 700 et échelle 150 %. Tests et captures temporaires supprimés après usage.

La copie vers le dossier Debug habituel est bloquée tant que son application reste ouverte ; la validation utilise une installation distincte dans build/. L'utilisateur a demandé de ne pas fermer sa fenêtre pour le moment.
