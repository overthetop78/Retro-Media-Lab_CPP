# Page Thèmes — validation du 14 novembre 2026

Un seul thème est actuellement disponible : RetroMediaLab. La page présente la liste, l'image d'aperçu, les options Système/Clair/Sombre et le bouton Choisir. Le thème et l'apparence sélectionnés restent provisoires : seule l'image est rendue avec ces paramètres. Le libellé de thème actif conserve la sélection réellement appliquée. Choisir applique et enregistre ; fermer sans confirmer ne change pas l'application.

Contrôles effectués après cette correction :

- Compilations Debug et Release réussies ; audit des constantes et contrôle des clés/paramètres des 29 catalogues du thème réussis.
- Images Clair et Sombre différentes, inspectées hors écran. Le cadre de l'aperçu conserve les couleurs actives ; seul son contenu représente le choix provisoire.
- Avant Choisir : paramètres et couleurs du service actif, palette globale et masque de résolution, police, fichier de préférences et libellé de thème actif inchangés. Aucun événement global ApplicationPaletteChange ou ApplicationFontChange, aucune publication du service actif, aucune sauvegarde.
- Aperçu Système sans remise à zéro de la palette globale. Après confirmation, Système utilise réellement la palette native sans remplacement explicite (masque nul).
- Choisir confirme Sombre puis Système avec une seule notification et une seule sauvegarde par confirmation. Le doublon de notification identique après l'application système est supprimé.
- Prévisualisations répétées : nombre de fenêtres inchangé après chaque rendu, aucune modification du dossier ni de ses observateurs. Les objets temporaires de rendu sont détruits après chaque image.
- Fermeture avec une apparence provisoire différente : thème actif et préférences inchangés. Page active, sortie de console et dossier conservés.
- Le texte de confirmation a été traduit avec Argos, relu et corrigé dans les langues concernées sans régénérer les autres textes déjà relus. Le bouton Choisir réutilise sa traduction existante. Les paramètres nommés correspondent dans les 29 langues.

Les tests et images sous build/verification-preview, ainsi que la génération partielle sous build/preview-translations, ont été supprimés après utilisation. Aucun test n'est intégré au workflow GitHub. Le comportement de la palette native pendant l'aperçu est décrit dans [la référence technique](../../references/tooling/apercu-themes.md). Les tests hors écran n'ont pas modifié les préférences personnelles ou la configuration de Windows.
