# Traductions

Les sources sont rangées dans src/Translations/Catalogs :

```text
Constants/App/window.json
Languages/en-US/App/window.json
Languages/fr-FR/App/window.json
```

Constants contient la référence anglaise et les invariants. Les entrées invariantes sont désignées par leurs clés dans la liste invariants du catalogue. Les noms de produit et les noms natifs des langues font partie de ces invariants.

Générer les langues avec Argos, déclaré installé et fonctionnel :

```powershell
python scripts/translations/translate.py --languages en-US fr-FR
```

Cette commande utilise Argos pour les langues autres que en-US et copie la base vers en-US en excluant les invariants isolés. Elle protège les paramètres et les invariants présents dans les phrases. Les listes annexes par langue sont dans scripts/translations/ignores. Aucun contrôle d'installation d'Argos ni appel à Google Traduction n'est effectué.

La commande régénère les fichiers ciblés : relire les résultats avant de conserver les modifications. Les premiers libellés français Light/Dark ont été corrigés après génération en Clair/Sombre. Une nouvelle génération peut nécessiter de réappliquer ces corrections de contexte.

Le build appelle compile_catalog.py pour regrouper les domaines et leurs fichiers en une DLL par langue, plus constants.dll. Les JSON restent dans le dépôt ; ils ne sont pas copiés dans le dossier exécutable. Le service compilé Translations.dll est séparé des données sous lib/Languages.

À l'exécution : invariant → constants ; texte ordinaire → langue choisie, puis constants si la traduction manque, puis marqueur [clé] si la clé n'existe pas. La langue anglaise utilise elle aussi son catalogue en-US.

Le service copie les textes dans ses propres données avant de décharger les DLL de catalogues. Les abonnements sont des objets de durée de vie explicite ; les vues les libèrent à leur destruction. Changer la langue met à jour les libellés des vues ouvertes.

Un module peut employer son propre dossier de bibliothèques de langues et son domaine avec un service distinct ; ses catalogues ne sont pas intégrés dans ceux d'App. La propagation de la langue commune via Emulation sera intégrée lors du portage de ces modules.
