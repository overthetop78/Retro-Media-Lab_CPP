# Préférences Conversion

Validation Windows du 14 novembre 2026, Qt 6.12.0 et MSVC x64. Référence : [bloc C#](../../references/retro-media-lab/conversion-csharp.md).

Un quatrième onglet Conversion regroupe les options de nommage auparavant présentes dans le bloc Général C#. Général reste limité au dossier et à la langue. Le nouvel onglet contient deux cartes : activation, modèles et historique à gauche ; variables avec insertion au curseur à droite. Une carte inférieure affiche l'exemple et permet de parcourir les quatre exemples C#.

L'activation et la sélection d'un modèle prédéfini sont sauvegardées immédiatement. La saisie personnalise l'aperçu sans réécrire le fichier à chaque caractère ; le modèle est sauvegardé en fin de saisie, à la fermeture ou lors de la synchronisation avant reconstruction de l'interface. Les cinq derniers modèles personnalisés sont mémorisés sans doublons de casse. Un double-clic ou une activation au clavier restaure un modèle récent. Les espaces du modèle actif sont conservés ; les récents sont normalisés par Trim comme en C#.

Les réglages vivent dans WorkspaceState et PreferencesStore, indépendamment de la vue. L'onglet actif et les réglages sont conservés lors de la reconstruction du thème. Les anciens fichiers de préférences dépourvus du champ conversion utilisent les valeurs par défaut.

Les textes existants proviennent des ressources C# des 29 langues, relues avec corrections ciblées des contresens. Aucun nouveau texte n'a nécessité de génération Argos. Les jetons, formats et exemples fixes utilisent des constantes nommées. Les erreurs de structure des préférences réutilisent le diagnostic traduit existant.

Vérifications réalisées :

- Builds Debug et Release réussis sans avertissement C++ relevé dans les journaux finaux.
- Sauvegarde/rechargement, valeurs par défaut, cinq modèles prédéfinis, limite des cinq récents et dédoublonnage insensible à la casse.
- Aperçus C# : nom, famille, format, extension, date, heure et compatibilité TAG ; rotation des exemples.
- Activation persistée, sélection de modèle, modèle personnalisé, réutilisation d'un récent et insertion d'une variable.
- Rafraîchissement des textes dans les 29 langues sans perdre la saisie ; conservation des préférences et de la page métier active après reconstruction du thème.
- Rendu complet des préférences inspecté en mode Qt offscreen en clair et sombre. L'interaction physique avec la souris dans une session Windows n'a pas été testée par ce banc.
- Audit de 88 fichiers C++, 55 diagnostics traduits et 29 langues ; git diff --check réussi.

Le banc temporaire, ses captures et ses artefacts ont été supprimés après utilisation. Aucun ajout au workflow GitHub. Le moteur de conversion métier n'est pas encore porté par ce bloc.

## Ajustement de taille

Marges réduites à 12, espacements à 6 et lignes de variables espacées de 4. La hauteur des boutons de variables suit celle du texte avec 6 de marge verticale ; l'historique est limité à 100 au lieu d'occuper tout l'espace disponible. Les autres contrôles conservent leur taille normale.

Contrôle Qt offscreen à 1100 × 640 avec la police réelle du thème et une échelle de 150 % : maximum de défilement vertical nul, dix variables visibles et aperçu entièrement contenu dans la zone visible. Capture inspectée. Compilation Debug et Release et audit des constantes réussis ; banc et capture temporaires supprimés.
