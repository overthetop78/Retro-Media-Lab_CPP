# Installation Qt Windows

Demandée par l'utilisateur le 14 novembre 2026 : kit Qt pour MSVC x64 et Qt Creator, avec les outils Qt nécessaires. MSVC est déjà installé ; ne pas le réinstaller.

- [x] Environnement Qt
  - [x] Préparer l'installateur
    - [x] Créer `%TEMP%/RetroMediaLab-QtSetup/qt-online-installer-windows-x64-online.exe` depuis le serveur officiel Qt ; vérifier la signature de l'exécutable avant lancement.
  - [x] Installer les composants
    - [x] Créer l'installation sous `C:/Qt/` avec l'installateur officiel : kit Qt de bureau MSVC x64, Qt Creator et outils associés ; relever les composants réellement proposés avant de fixer leur liste.
  - [x] Nettoyer les téléchargements temporaires
    - [x] Supprimer les fichiers de téléchargement et journaux temporaires créés sous `%TEMP%/RetroMediaLab-QtSetup/` après installation et vérification ; conserver les outils installés et leur outil de maintenance.

Vérifier les versions de Qt Creator, de Qt et l'accès à Qt6Config.cmake, puis reprendre le socle graphique. La documentation conserve les versions, chemins et sources utiles. Si l'installateur demande une connexion Qt, laisser l'utilisateur s'authentifier dans son interface, sans demander de mot de passe dans la conversation.

État final : installation utilisateur terminée, puis ajout réussi du composant qt.qt6.6120.win64_msvc2022_64 via MaintenanceTool. Qt 6.12.0, spécification win32-msvc, kit C:/Qt/6.12.0/msvc2022_64 et Qt Creator 20.0.2 vérifiés. L'installation comprend aussi un kit MinGW et d'autres composants choisis par l'utilisateur, laissés en place. Le dossier temporaire de téléchargement a été supprimé.
