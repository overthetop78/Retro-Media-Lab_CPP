# Contrôles compacts et sélection des onglets

Validation du 14 novembre 2026 sur widgets Qt réels hors écran, fenêtres normales. Référence C# consultée pour le WrapPanel des paramètres GreaseWeazle.

- Champs : hauteur de thème de référence 28 au lieu de 36, marges adaptées ; les sélecteurs invalident aussi leur cache de taille minimale. Sur le profil 800×600 à 100 %, les sélecteurs du lecteur passent de 28 à 15 pixels de haut, avec une police conservée à 10 pixels de hauteur mesurée.
- Lecteurs : la hauteur dépend du contenu après adaptation ; un lecteur tient dans son cadre sans défilement.
- Paramètres GreaseWeazle : champs de référence de 150 pixels, retour à la ligne selon l’espace disponible, hauteur du panneau mesurée après adaptation.
- Soulignement : épaisseur et retraits suivent le facteur de taille et le niveau d’onglet. Comparaison 50/100/200 %, sans modification de fenêtre par le banc.
- 21 pages parcourues pour chaque résolution : sept préférences, cinq autres catégories matérielles et neuf outils GreaseWeazle. Aucun champ de saisie/sélecteur visible ne dépasse son parent. À 100 %, aucun défilement sur les panneaux de lecteurs ou de paramètres GreaseWeazle dans ces jeux de données. La liste des journaux peut défiler pour ses dernières lignes en 1080p.
- Compilation Debug et Release réussie ; audit des constantes et catalogues réussi. La copie finale dans le dossier d’exécution Debug a réussi le 15 novembre 2026 après fermeture de la fenêtre C++. Debug et Release sont à jour. Les captures utilisent la DLL compilée correspondant aux corrections.
- Aucun accès matériel exécuté. Les fenêtres hors écran et le banc sont détruits à la fin ; fichiers du banc supprimés après validation. Captures et relevés conservés comme résultats demandés.

[Captures 800×600](captures/800x600/README.md) · [Captures 1080p](captures/1080p-15-pouces/README.md)
