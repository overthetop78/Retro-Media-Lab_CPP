# Validation du premier socle

Date : 14 novembre 2026. Plateforme : Windows x64.

## Vérifications réalisées

- Compilation et déploiement Debug puis Release avec Qt 6.12.0 et MSVC 14.44.
- Fenêtre du thème RetroMediaLab inspectée : titre, carte centrale, choix Light/Dark, langues et libellés français. Le changement de thème a aussi été confirmé par l'utilisateur.
- Tests temporaires hors écran : repli sur Constants, invariants non remplacés par une langue, marqueur de clé absente, catalogues propres à un module, rejet de langue invalide ou absente sans mutation de la langue courante, et expiration des abonnements.
- 25 reconstructions de présentation, conservation de la taille de fenêtre dans les limites de l'écran et de la langue/variante choisies, destruction des anciennes vues.
- Exception injectée pendant la préparation d'une présentation : l'ancienne vue reste vivante et utilisable, les objets partiels sont détruits.
- Vérification Windows du déchargement effectif des DLL constants/en-US/fr-FR après copie, puis du thème après destruction de QApplication.
- Lancement Release depuis le dossier temporaire, avec PATH limité à Windows : la fenêtre est ouverte et App.dll, Translations.dll, les DLL Qt et RetroMediaLab.dll proviennent du dossier lib de la distribution.
- Contrôle du classement des DLL et de l'absence de DLL à la racine.
- Lecture des dépendances Ninja de ThemeService.cpp : les en-têtes sont suivis après correction du décodage des diagnostics français MSVC.

Résultat des tests temporaires : PASS pour les traductions, les abonnements, les reconstructions, la conservation d'état couverte et le nettoyage vérifié. Leurs sources et artefacts sont supprimés après utilisation, conformément aux règles du projet ; aucun workflow GitHub ne les utilise.

## Portée

Ce socle ne contient encore ni fonctionnalités média, ni émulation, ni documents métier ou onglets de travail. Leur conservation entre thèmes devra être vérifiée lors de leur portage. Seul le thème RetroMediaLab est réalisé.

Les changements vers une interface Windows 3.x, Windows 9x ou une autre composition complète restent à tester lorsque ces thèmes seront créés. Les autres DPI, les périphériques hôtes et Linux/macOS ne sont pas validés par ces résultats. Aucune équivalence visuelle complète avec WPF n'est annoncée.
