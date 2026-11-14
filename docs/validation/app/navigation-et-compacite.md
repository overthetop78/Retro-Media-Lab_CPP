# Navigation et compacité

GreaseWeazle utilise une liste latérale de neuf options et une pile de pages à droite. La sélection, les brouillons, les résultats et le service de commande restent conservés hors de la navigation ; une fermeture pendant une commande suit le mécanisme d’arrêt existant.

À 100 %, le thème utilise 70 % des dimensions historiques, contre 85 % auparavant. Les onglets des préférences sont plus compacts que ceux de l’espace principal ; les onglets imbriqués le sont encore davantage. Leurs textes et dimensions utilisent la même police dans la mesure et dans le rendu, ce qui évite de couper les libellés.

Un défaut de normalisation des polices a été corrigé : un dialogue nouvellement créé pouvait hériter du facteur d’une autre fenêtre alors que sa propre police n’avait pas encore été réduite. La recherche du facteur s’arrête désormais à la frontière de la fenêtre.

Les nouveaux dialogues prennent l’écran de leur fenêtre parente pour calculer leur taille initiale. Le profil 800 × 600 a révélé cette nécessité sur un environnement à plusieurs écrans.

## Vérifications

Banc temporaire exécuté avec succès : liste de neuf options, absence d’onglets dans GreaseWeazle, navigation des neuf pages, sélection conservée au changement de langue et de thème, brouillon RPM conservé, absence de cumul lors d’un aller-retour 100/150/100 %, géométrie des fenêtres conservée pendant le réglage de taille.

Sur les profils simulés 2160p/150 % et 1080p/100 %, les hauteurs mesurées des trois niveaux d’onglets sont 30/26/23 pixels logiques et les polices 10/9/8 pixels. Les nouveaux dialogues utilisent bien la police compacte de 10 pixels à 100 %, au lieu de garder une taille supérieure.

Les profils 1920 × 1080 et 800 × 600 à 100 % ont chacun produit six captures demandées par l’utilisateur, conservées dans [captures](captures/README.md). Les dialogues tiennent dans les dimensions du profil 800 × 600 ; les paramètres longs de l’alignement restent accessibles par défilement. Les PNG ont été contrôlés visuellement et vérifiés quant aux dimensions et métadonnées de densité. Les lecteurs sont fictifs ; aucune commande matérielle n’a été exécutée pendant ces vérifications d’affichage.

Les compilations et déploiements locaux Debug et Release sont terminés. Aucun test n’est ajouté au workflow GitHub. Le banc et ses artefacts temporaires ont été supprimés. Les douze captures demandées sont conservées. Audit réussi sur 140 fichiers C++, 56 clés d’erreur et 29 langues ; git diff --check passe.

## Correction des captures

Le banc initial avait imposé une fenêtre principale couvrant le bureau virtuel. Ces images ont été remplacées par les captures en fenêtres normales, sans appel de maximisation, déplacement ou redimensionnement dans le nouveau banc. Les dimensions sont vérifiées avant et après chaque capture et enregistrées dans les manifestes geometries.json. À 1080p : application 1360 × 820, dialogues 1100 × 640. À 800 × 600 : application et dialogues 752 × 552, selon le plafonnement déjà appliqué par le logiciel. Aucun fichier source de l’application n’a été modifié pour cette correction de capture.
