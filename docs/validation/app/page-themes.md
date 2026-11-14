# Page Thèmes — 14 novembre 2026

La page suit la composition fournie : liste à gauche, aperçu de la fenêtre courante à droite, nom et description, paramètres du thème, thème actif, indication de persistance et bouton Choisir. Un seul thème est présent : RetroMediaLab ; Choisir reste désactivé puisqu'il est déjà actif. L'aperçu reprend les vues effectivement réalisées dans le portage.

Pour RetroMediaLab, Système utilise les couleurs de la plateforme, et Clair/Sombre les palettes embarquées. Les paramètres des autres thèmes pourront être différents : contrôleur et page sont propres au thème, sauvegarde séparée par identifiant.

Vérifications effectuées :

- Compilations et déploiements Debug et Release avec Qt 6.12/MSVC 14.44 x64 réussis.
- Contrôles Qt hors écran : liste avec une seule entrée, position gauche/droite, aperçu dimensionné et rendu, bouton Choisir désactivé, activation des trois apparences et sauvegarde/relecture.
- Système retire les couleurs explicites de QApplication (masque de résolution nul). Les rôles du thème correspondent à sa palette ; une palette de test modifiée notifie et actualise Window, Button et Accent. Les modes explicites restent indépendants de ces notifications.
- Un objet de paramètres arbitraires appartenant à un autre thème survit à la sauvegarde sans modification. Migration du fichier version 1 vers les paramètres RetroMediaLab vérifiée.
- Reconstruction des vues : section Thèmes, page active, dossier et sortie de console conservés. Un paramètre d'apparence invalide est refusé sans perdre la sélection courante.
- Les contrôles se retraduisent dans les 29 langues. Clés et paramètres nommés vérifiés ; en-US identique à la base traduisible. Argos a généré le seul nouveau catalogue themes.json ; les libellés équivalents du C# ont été repris et les erreurs manifestes relues/corrigées, notamment japonais, coréen, suédois, turc, vietnamien et chinois traditionnel. Cela ne constitue pas une certification linguistique native des 29 langues.
- Rendu sombre inspecté en image hors écran avec Segoe UI chargée explicitement pour le backend de test. Aucun changement des préférences Windows de l'utilisateur n'a été effectué. Le basculement réel des réglages Windows et Linux/macOS n'est pas validé par cette simulation.

Les tests temporaires sont regroupés sous build/verification-themes, puis supprimés avec leurs exécutables, journaux et images. Aucun test n'est ajouté au workflow GitHub. L'ensemble des fonctions métier et de l'émulation reste hors de ce bloc.
