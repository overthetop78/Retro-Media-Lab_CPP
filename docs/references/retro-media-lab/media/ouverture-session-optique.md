# Ouverture de session optique

Référence : `src/RetroMediaLab.MediaEngine/Images/Reading/Optical/OpticalDiscSession.cs` dans le dépôt C# conservé en lecture seule.

`OpticalDiscSession::openAsync` passe par la reconnaissance commune, désactive l’observation mécanique pendant la lecture initiale et joint les pistes audio compagnons uniquement sur demande. Le transfert au constructeur et les chemins d’exception partagent une seule responsabilité de nettoyage ; les appels directs au constructeur conservent leur nettoyage antérieur.

La compilation native du bloc a abouti. Le contrôle ciblé a ouvert une image ISO temporaire, lu un secteur, distingué ouverture simple et piste WAV compagnon, fermé deux fois sans erreur et vérifié l’annulation préalable. Les fichiers de contrôle et les médias temporaires ont été supprimés. Aucun décodeur déjà validé n’a été retesté.

Les sept clés Famicom Disk sont désormais définies dans un en-tête partagé entre lecture du conteneur et exploration. Le catalogue public des identifiants MFS comprend aussi les deux identifiants C# manquants Nintendo Wii U et GDI.

Le suivi des modèles de secteurs et de la composition a été rapproché des implémentations existantes : l’observation passe déjà par `DriveSectorImage` et `bindSectors`, et la composition construit déjà les mécaniques via `Drives/Service.cpp`. Aucun second mécanisme n’a été ajouté.
