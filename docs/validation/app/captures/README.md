# Tests d’affichage virtuel

- [1920 × 1080 — référence physique 15 pouces, environ 147 PPP](1080p-15-pouces/README.md)
- [800 × 600 — référence physique 15 pouces, environ 67 PPP](800x600/README.md)

Chaque dossier contient six captures à la résolution indiquée, en mise à l’échelle système 100 % et réglage d’interface 100 %, avec une description des conditions de rendu. Les PPP expriment ici la correspondance physique calculée et les métadonnées des PNG ; le test exécute le rendu Qt sur un écran virtuel, sans moniteur de cette diagonale connecté.

WindowsSandbox.exe est absent du dossier système de cet environnement ; ces essais utilisent le backend offscreen de Qt. Les programmes et lecteurs de démonstration du banc sont supprimés après vérification. Les PNG demandés par l’utilisateur sont conservés comme livrables.

Les douze images ont été refaites avec les fenêtres normales, sans maximisation ni géométrie imposée par le banc. À 1080p, la fenêtre principale mesure 1360 × 820 pixels et les dialogues 1100 × 640 pixels. Les dimensions et l’état non maximisé sont consignés dans chaque dossier.
