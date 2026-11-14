# Traductions

Les sources sont rangées dans src/Translations/Catalogs :

```text
Constants/App/{window,menus,navigation,console,status,preferences}.json
Languages/en-US/App/{window,menus,navigation,console,status,preferences}.json
Languages/fr-FR/App/{window,menus,navigation,console,status,preferences}.json
```

Constants contient la référence anglaise et les invariants. Les entrées invariantes sont désignées par leurs clés dans la liste invariants du catalogue. Les noms de produit et les noms natifs des langues font partie de ces invariants.

Languages.json définit les 29 langues du C#, leurs noms natifs, cultures et codes Argos. Cette liste est embarquée dans Translations.dll et sert aussi aux préférences et au choix initial de langue. Comme dans le script C#, pt-PT/pt-BR utilisent le modèle pt et zh-Hans/zh-Hant le modèle zh ; les textes des variantes restent dans des catalogues séparés et sont relus.

Générer les langues avec Argos, déclaré installé et fonctionnel :

```powershell
python scripts/translations/translate.py --languages en-US fr-FR
```

Cette commande génère d'abord en-US depuis Constants en excluant les invariants isolés, puis utilise effectivement ces fichiers en-US comme source Argos des autres langues. en-US est aussi préparé lorsque seule une langue cible est demandée. Elle protège les paramètres et les invariants présents dans les phrases. Le fichier unique scripts/translations/ignores/common.txt s'applique à toutes les langues, sans exception ni fichier par langue. Aucun contrôle d'installation d'Argos ni appel à Google Traduction n'est effectué.

Pour générer toutes les langues : utiliser --all à la place de --languages. Si un modèle altère un jeton de protection, le script traduit les fragments entre invariants et recopie les invariants sans les soumettre au modèle. Une génération ne remplace pas la relecture : les libellés courts et les variantes peuvent nécessiter des corrections de contexte.

Pour limiter une génération aux nouveaux fichiers et préserver les autres traductions relues :

```powershell
python scripts/translations/translate.py --languages fr-FR --files App/preferences.json
```

Les chemins de --files sont relatifs à Constants et sont validés avant toute écriture.

La commande régénère les fichiers ciblés : relire les résultats avant de conserver les modifications. Les premiers libellés français Light/Dark ont été corrigés après génération en Clair/Sombre. Une nouvelle génération peut nécessiter de réappliquer ces corrections de contexte.

Le build appelle compile_catalog.py pour regrouper les domaines et leurs fichiers en une DLL par langue, plus constants.dll. Les JSON restent dans le dépôt ; ils ne sont pas copiés dans le dossier exécutable. Le service compilé Translations.dll est séparé des données sous lib/Languages.

À l'exécution : invariant → constants ; texte ordinaire → langue choisie, puis constants si la traduction manque, puis marqueur [clé] si la clé n'existe pas. La langue anglaise utilise elle aussi son catalogue en-US.

Le service copie les textes dans ses propres données avant de décharger les DLL de catalogues. Les abonnements sont des objets de durée de vie explicite ; les vues les libèrent à leur destruction. Changer la langue met à jour les libellés des vues ouvertes.

Un module peut employer son propre dossier de bibliothèques de langues et son domaine avec un service distinct ; ses catalogues ne sont pas intégrés dans ceux d'App. La propagation de la langue commune via Emulation sera intégrée lors du portage de ces modules.

Au premier démarrage, la langue d'interface du système est résolue dans les langues disponibles et enregistrée immédiatement dans les préférences. Si elle n'est pas prise en charge, en-US est enregistré. Les lancements suivants conservent la langue sauvegardée, même si le système change de langue ; le choix des préférences reste prioritaire.

Les erreurs sont dans App/errors.json selon le même mécanisme Constants/Languages. compile_errors.py génère au build leur secours embarqué pour les pannes empêchant de charger les DLL ou le GUI ; les textes proviennent des mêmes catalogues. Ne pas maintenir d'anglais en parallèle dans les sources C++. Le contrôle `python scripts/quality/audit_literals.py` vérifie les littéraux, clés et paramètres. Les messages des scripts de développement ne sont pas concernés par la traduction, selon la consigne utilisateur ; ceux du futur installateur le seront.
