# Orientations du portage et consultation ciblée

Date : 13 novembre 2026. Le dépôt C# est consulté en lecture seule.

## Directives utilisateur confirmées

- Windows en premier ; prévoir l'ajout ultérieur de Linux/macOS sans refonte de l'application. Conserver pour le moment les solutions de périphériques hôtes utilisées en C#.
- Intégrer les dépendances externes nécessaires à la distribution pour éviter à l'utilisateur de les chercher. Les prérequis système tels que VC++ et DirectX seront proposés pendant l'installation ou au démarrage de la distribution ZIP.
- Séparer les traductions dans des bibliothèques. Chaque module d'émulation peut fournir et utiliser son propre composant Translations et ses catalogues, sans les intégrer à App. Ranger les catalogues par langue/domaine/fonctionnalité. Les textes sont résolus selon la langue utilisateur et actualisés à chaud ; les traductions communes restent accessibles via Emulation.
- Commencer par App et les emplacements prévus pour les modules. Emulation et les modules Emulation.xxx seront réalisés en dernier.
- Fournir un SDK permettant de compiler un module sans les sources de l'application, puis de le charger et le tester dans celle-ci. Chips sera utilisable en liaison statique ou dynamique au choix du build du module d'émulation.
- Le GUI d'un module utilise les contrôles partagés fournis par App. Le module définit ses clés de traduction, options, dimensions, blocs et positions dans les seuls emplacements autorisés de son dialogue. Le design relève de l'application et de ses thèmes. Le module ne place pas de contrôles dans les menus ou paramètres utilisateur de l'application.
- Respecter les frontières de composants décrites par l'utilisateur, avec des échanges bidirectionnels entre les composants autorisés.

## Constats dans le code C#

Sources consultées sur main :

- [LocalizationSource.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/main/src/RetroMediaLab.App/Localization/Sources/LocalizationSource.cs) : conserve Culture et UiCulture ; Refresh incrémente Version et notifie les liaisons.
- [LocExtension.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/main/src/RetroMediaLab.App/Localization/Extensions/LocExtension.cs) : index des clés des catalogues ; résolution via ResourceManager selon UiCulture ; accès aux traductions de modules via IEmulationModuleLocalization, avec repli sur les ressources App ; paramètres formatés selon Culture ; les liaisons observent Version.
- [RetroMediaLab.App.csproj](https://github.com/overthetop78/Retro-Media-Lab/blob/main/src/RetroMediaLab.App/RetroMediaLab.App.csproj) : catalogues neutres dans Resources/00-Base et catalogues par langue ; dépendance Microsoft.GameInput et copie de GameInputRedist.msi dans Runtime. Références à Infrastructure, MediaEngine, Emulation, VideoPresentation et Updates.

Ces constats portent sur les fichiers cités, pas sur un audit complet. L'extraction future des traductions dans des bibliothèques séparées est une directive du portage, pas une description du découpage actuel.

### Résolution des langues et repli : vérification du 14 novembre 2026

LocExtension.cs (SHA `034374f2068b51ee55cfa6621292b46a2d9f658c`) appelle déjà ResourceManager.GetString avec UiCulture pour les textes et avec InvariantCulture pour GetInvariant et les clés Format.*. Le projet App (SHA `d082fd2c054bd1e2a263b34262a349295145c048`) déclare Resources/00-Base comme ressources neutres, sans culture. BuildResourceIndex utilise ces catalogues neutres pour établir les clés.

Le repli sur les ressources neutres lorsqu'une traduction manque est donc déjà assuré par ResourceManager ; il n'est pas une nouvelle fonctionnalité proposée pour le C++. Si la clé n'est trouvée dans aucun catalogue applicable, LocExtension affiche `[clé]`. Les exceptions de chargement de ressources ne sont pas transformées en ce marqueur.

Référence officielle : [ResourceManager.GetString](https://learn.microsoft.com/en-us/dotnet/api/system.resources.resourcemanager.getstring?view=net-10.0), consultée à cette date : recherche selon la culture demandée avec repli vers les ressources par défaut, puis null si la clé demeure introuvable. Cette vérification porte sur la résolution à l'exécution, pas sur le script de génération des langues.

## Proposition pour le compilateur du SDK

Proposition à valider : publier pour chaque version du SDK une chaîne de compilation Windows de référence identique à celle de l'application, avec architecture et options de compatibilité définies. Ne pas imposer le même éditeur aux développeurs. Aucune version précise ni aucun compilateur n'a encore été choisi.

La documentation [Microsoft sur la compatibilité binaire C++](https://learn.microsoft.com/en-us/cpp/porting/binary-compat-2015-2017?view=msvc-170), consultée le 13 novembre 2026, précise que la compatibilité entre versions MSVC reste soumise à des contraintes : éditeur de liens suffisamment récent, runtime redistribuable suffisamment récent et, avec /GL et /LTCG, même version exacte des outils pour compilation et liaison. Ce dernier point concerne particulièrement une future distribution de Chips statique optimisée.

Cette proposition simplifie la première version du SDK ; elle ne constitue pas une décision de l'utilisateur et ne remplace pas le versionnement des contrats entre application et modules.
