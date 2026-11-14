# Compilation et dossier exécutable

Outils employés le 14 novembre 2026 : Qt 6.12.0 MSVC 2022 x64, MSVC 14.44.35207, CMake 4.3.1-msvc1 et Ninja fournis avec Visual Studio Community 2026. Qt Creator 20.0.2 est disponible mais n'est pas nécessaire au script.

```powershell
./scripts/local-building/build.ps1 -Configuration Debug
./scripts/local-building/build.ps1 -Configuration Release
./scripts/local-building/build.ps1 -Configuration Release -QtRoot C:/Qt/6.12.0/msvc2022_64
```

Le script prépare l'environnement MSVC, détecte le préfixe réel des dépendances d'en-têtes du compilateur, configure CMake, compile, installe les sorties et classe les DLL collectées par windeployqt. Une erreur arrête le build. Les sondes temporaires du compilateur sont supprimées dans finally.

Les presets CMake debug/release sont utilisables directement depuis un environnement compilateur configuré, avec le préfixe Qt approprié. Le script PowerShell constitue actuellement le chemin validé pour le déploiement Windows.

## Arborescence produite

La solution [RetroMediaLab.sln](../../RetroMediaLab.sln) à la racine contient le projet Visual C++ [RetroMediaLab.vcxproj](../../RetroMediaLab.vcxproj). Elle est utilisable avec l'explorateur de solutions VS Code et Visual Studio. Debug/Release x64 appellent le même build CMake via build.ps1 ; les listes de fichiers et filtres sont actualisés automatiquement. Les commandes Clean/Rebuild utilisent également CMake. Pour actualiser uniquement l'arborescence : `python scripts/local-building/update-solution.py`.

IntelliSense lit les commandes réelles de build/debug/compile_commands.json ; les propriétés sous .vscode fournissent le compilateur MSVC et les chemins Qt de secours. Après un changement de kit, adapter ces chemins et régénérer le build. Voir [les références](../references/tooling/visual-cpp-et-intellisense.md).

```text
Retro Media Lab/
  retromedialab.exe
  Assets/app-icon.ico
  lib/
    App.dll
    Translations.dll
    Themes/
      RetroMediaLab.dll
    Languages/
      constants.dll
      en-US.dll
      fr-FR.dll
    Qt6/
      Qt6Core.dll
      Qt6Gui.dll
      Qt6Widgets.dll
      ...
      plugins/
        platforms/
        imageformats/
        ...
    DirectX/
      d3dcompiler_47.dll
      dxcompiler.dll
      dxil.dll
    OpenGL/
      opengl32sw.dll
  Modules/
```

En Debug, les noms Qt portent leur suffixe d. Les DLL système Windows et les runtimes Microsoft installés ne sont pas copiés arbitrairement depuis Windows.

Le lanceur natif configure les chemins DLL avant de charger App.dll. Il ne dépend pas de Qt à son propre démarrage. Il utilise des chemins absolus relatifs à son exécutable et libère les chargements et répertoires de recherche possédés à la sortie.

Le thème fournit le code de ses fenêtres, sa disposition et ses styles ; ses données sont embarquées via les ressources Qt. App conserve l'état et coordonne la présentation. La référence de chargement du thème est conservée jusqu'après la destruction de QApplication et de son style, y compris sur exception.

Les catalogues de textes sont compilés dans les DLL Languages. Aucun JSON externe de thème ou de texte n'est nécessaire au lancement.

## Limites actuelles

Le résultat est un dossier de développement local, pas encore un installateur ou une release publique. La gestion des prérequis Microsoft et les éléments de redistribution seront traités avec le packaging demandé ultérieurement.

Les sources communes et les contrats sont distincts des appels DWM Windows. Le lanceur Windows est une adaptation native ; le CMake prévoit une entrée directe et une adaptation sans DWM pour les autres plateformes. Les builds, chemins de plugins, intégrations natives et paquets Linux/macOS restent à vérifier sur ces systèmes.

Les modules d'émulation ne sont pas encore implémentés. Modules est le dossier prévu pour leurs paquets séparés.
