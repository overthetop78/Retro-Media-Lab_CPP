# Taille du contenu de l’application

## Réalisation

Préférences → Général contient un cadre Taille de l’interface avec les six choix demandés : 50 %, 75 %, 100 %, 125 %, 150 % et 200 %. Le choix s’applique immédiatement et se sauvegarde dans `interfaceScale`. Les anciens fichiers sans cette propriété utilisent 100 %. Une valeur non autorisée, non numérique ou fractionnaire est rejetée par une erreur existante traduite.

Le thème RetroMediaLab adapte polices, contraintes des contrôles, icônes, marges, espacements et métriques du style à partir de références non cumulatives. Les fenêtres gardent leur géométrie. Les pages des préférences défilent si l’agrandissement dépasse leur espace disponible. La densité est recalculée après un changement de pourcentage, d’écran ou de géométrie/DPI de cet écran. Les nouveaux contrôles sont pris en compte lors de leur affichage.

Le contrôleur de densité appartient à la fenêtre : abonnement faible, connexions et rappels Qt liés à son contexte, suppression du filtre d’événements à sa destruction. Les références aux widgets et layouts sont des QPointer ; les objets détruits sont retirés des références conservées.

## Validation du 14 novembre 2026

Banc temporaire unique, sans média ni matériel externe :

- Six choix présents ; sélection reliée à l’état et sauvegarde/relecture des six valeurs ; rejet d’un pourcentage invalide.
- Retour à 100 % après 50/75/100/125/150/200 % : même police et même hauteur de contrôle, sans effet cumulatif.
- Géométrie principale et préférences conservée pendant ces changements.
- Écrans simulés Qt : 1920 × 1080 à 100 %, 3840 × 2160 à 150 % (2560 × 1440 logiques), espace réduit 1280 × 720. Passage entre écrans vérifié avec les vrais signaux Qt du backend hors écran.
- À 100 %, police de saisie de 12 pixels logiques et hauteur conseillée de 31 ; à 200 %, 24 et 61. À 50 %, certains minima intrinsèques du style demeurent, pour ne pas invalider la géométrie des contrôles.
- Toutes les pages des préférences visitées à 200 % ; défilement vérifié pour Thèmes, matériel et Moteurs lorsqu’ils débordent.
- Changement FR/EN et reconstruction clair/sombre : sélection conservée. Captures hors écran contrôlées visuellement en sombre 100 %, clair 75 % et Thèmes 200 %.
- Traduction Argos des deux libellés dans les 29 langues, relecture et correction des traductions dégradées du mot interface et du symbole pourcentage.

Ces essais simulent les écrans ; ils ne constituent pas un essai physique sur le portable 14 pouces de l’utilisateur. Aucun test n’est ajouté au workflow GitHub. Les compilations et déploiements locaux Debug/Release ont réussi. Le banc et ses artefacts ont été supprimés. Audit réussi : 133 fichiers C++, 56 clés d’erreur, 29 langues. git diff --check passe.
