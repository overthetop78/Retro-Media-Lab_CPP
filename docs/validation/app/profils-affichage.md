# Préparation de l’affichage Profils

La page du thème RetroMediaLab présente trois cartes de même largeur, avec titres en gras et listes vides : Lecture, Écriture, Conversion. Les doubles bordures du C# sont remplacées par les cartes du thème et des listes sans cadre supplémentaire.

Les libellés et noms accessibles suivent les changements de langue. L’onglet sélectionné est mémorisé dans l’état de présentation existant lors d’une reconstruction du thème. Aucune donnée de profil fictive, action de gestion ou logique de stockage n’est ajoutée.

Les quatre libellés sont repris des ressources C# dans les 29 langues. Leur reprise conserve les traductions historiques, y compris leurs éventuelles imperfections. Audit des constantes et traductions réussi sur 130 fichiers C++ et 29 langues. Aucun test supplémentaire n’est nécessaire pour cette préparation d’affichage.

Compilations et déploiements locaux Debug et Release réussis. Contrôle git diff --check réussi.
