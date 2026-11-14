# Préférences des moteurs : référence C#

Référence consultée en lecture seule le 14 novembre 2026 dans `F:/GW GUI/src`.

- `RetroMediaLab.App/Theme/Base/Views/Controls/Options/OptionsEnginesSection.xaml` : quatre choix indépendants, Lecture, Écriture, Conversion, Explorateur ; moteur interne ou outils d’hôte Greaseweazle.
- `RetroMediaLab.Infrastructure/Settings/Engines/EngineSettings.cs` et `OperationEngine.cs` : outils d’hôte par défaut pour Lecture/Écriture, moteur interne pour Conversion/Explorateur.
- `RetroMediaLab.App/Options/Controllers/EngineOptionsController.cs` : sauvegarde au changement de sélection.
- `RetroMediaLab.App/Resources/<langue>/{Options,HostTools,Read,Write,Conversion,Tab}.resx` : libellés repris dans les 29 catalogues C++.

Le C# indique que le moteur choisi doit être utilisé strictement, sans remplacement silencieux. Cette page C++ prépare les préférences ; elle n’ajoute pas les moteurs d’exécution.

Directive C++ : Greaseweazle reçoit ces réglages ; KryoFlux reste entièrement vide. Les catégories HDD/CD et le chargement de DLL externes seront définis ultérieurement.
