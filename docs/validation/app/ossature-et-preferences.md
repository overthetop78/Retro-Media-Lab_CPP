# Validation de l'ossature et des préférences

14 novembre 2026, Windows x64, Qt 6.12.0 MSVC et MSVC 14.44. Le dépôt C# est resté en lecture seule.

## Périmètre livré

Les sept pages vides reprennent l'ordre Lecture, Écriture, Conversion, Visualisation, Explorateur, Outils et Émulation. Les menus métier sans implémentation restent désactivés. Options > Préférences ouvre les deux sections disponibles : Général pour la langue, Thèmes pour RetroMediaLab Clair/Sombre. Les modifications sont immédiates ; langue et variante sont sauvegardées atomiquement dans le répertoire utilisateur Qt.

La console est initialement repliée. Son bouton est dans la barre d'état ; le séparateur conserve sa hauteur. Commande et sortie sont en lecture seule, la copie est iconographique. La session porte les identifiants des pages, leur sélection, la géométrie des fenêtres et les données/sélections/défilements de console. Les vues du thème restent remplaçables.

## Vérifications exécutées

- Builds et déploiements Debug et Release réussis.
- Ordre des sept pages, actions disponibles/désactivées, absence des sélecteurs dans la barre principale, ouverture et fermeture des préférences.
- Changement de langue par son contrôle : traduction immédiate des préférences et des pages principales. Changement de variante par ses boutons.
- Sauvegarde/relecture des préférences ; valeurs par défaut sans fichier ; rejet d'une langue invalide sans altérer le fichier valide. Les fichiers de test sont isolés dans un répertoire temporaire à destruction automatique.
- Copie de la sortie, lecture seule, conservation des contenus, sélections de commande et de sortie, défilement de sortie, visibilité et hauteur de console.
- 25 reconstructions avec préférences ouvertes : page active et section des préférences conservées, anciennes fenêtres détruites.
- Exception après création d'une présentation partielle : destruction de la candidate, maintien de la présentation courante, reconstruction suivante réussie.
- Destruction des fenêtres principales et de préférences ; déchargement effectif des catalogues puis de la DLL de thème après QApplication.
- Fenêtres Windows réelles : validité des handles et retours réussis de l'adaptation qui définit les couleurs DWM du titre et de son texte.
- Script de traduction : génération fr-FR depuis les fichiers en-US ; contrôle de tous les en-US contre Constants sans invariants isolés ; fichier ciblé inexistant rejeté sans aucune écriture.

Les vérifications utilisent un seul programme temporaire sous build/verification-shell, sans workflow GitHub. Ses sources, sorties et préférences temporaires sont supprimées après usage.

## Contrôle visuel et limites

Le menu, les deux sections des préférences et le changement Clair vers Sombre ont été inspectés à l'écran avant la dernière correction native de titre. Le contrôle visuel du dernier lancement a été interrompu par Échap ; son rendu final de titre n'est donc pas annoncé comme vérifié à l'écran. Le test natif valide l'acceptation des attributs par Windows, pas une comparaison de pixels.

Les fonds principaux ont été relevés dans les captures fournies : sombre Window #282A2F, Card #31343C, Control #363B44, Status #2F3238 ; clair Window #DDDEDF, Card/Control/Status #E1E1E1 ; console #1D2025 ; titres #303030 et #DEDEDE. La source et les PNG restent documentés dans les références.

Seul RetroMediaLab existe actuellement. Les préférences complètes du C#, les pages métier, les modules d'émulation, le mode système, les autres DPI et les adaptations Linux/macOS restent hors de ce bloc. Les icônes sont des dessins portables ; aucune identité exacte avec les glyphes WPF n'est revendiquée.

## Modification ciblée de Général

Destination et Langue sont disposés côte à côte. Le bloc inférieur de tags de conversion reste absent, conformément à la demande. Le dossier est sauvegardé et restauré, y compris avec accents et espaces ; la lecture des anciens fichiers sans cette clé a été vérifiée.

Builds Debug et Release réussis. Vérification temporaire : position relative des cartes à la taille normale et à la largeur minimale, restauration et conservation de saisie lors d'une reconstruction, changement de langue sans altération du chemin, ouverture du sélecteur de répertoire et annulation sans mutation. Le programme sous build/verification-general et ses artefacts sont supprimés après exécution. Ces résultats établissent la disposition et les comportements testés ; aucune inspection du bureau n'a été relancée.

## Langues et correction du sélecteur

Les 29 langues du catalogue C# disposent chacune des six catalogues App en JSON, compilés dans leur DLL. Les noms natifs du sélecteur proviennent de Languages.json ; les préférences acceptent les 29 codes et le démarrage résout la culture système, dont zh-TW et pt-BR.

Génération avec les modèles Argos déjà présents, relecture des libellés et comparaison avec les ressources C#. Corrections contextuelles de contresens de « console », « général », « aide », « à propos » et de termes matériels. Contrôle des clés, des valeurs non vides et de la conservation des invariants communs dans toutes les langues.

Builds Debug et Release réussis. Test temporaire : chargement effectif de chaque catalogue, sauvegarde/relecture des 29 codes, synchronisation du choix dans les préférences et destination inchangée. Pour le sélecteur : mode liste ordinaire, 12 lignes visibles au maximum, hauteur de ligne normale et ouverture sous le champ avec sélection du premier puis du dernier élément. Le test a utilisé Qt hors écran ; il ne constitue pas une inspection visuelle du bureau.
