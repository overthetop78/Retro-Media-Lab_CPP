# Navigation latérale et compacité

- [x] Adapter le dialogue GreaseWeazle
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/GreaseweazleDialog.h/.cpp` et `ThemeConstants.h` : liste latérale de neuf options, pages à droite, textes longs repliés, sélection conservée et attente du nettoyage à la fermeture.
- [x] Rendre la hiérarchie graphique plus compacte
  - [x] Modifier `src/App/Themes/RetroMediaLab/ThemeConstants.h`, `DisplayDensity.h/.cpp` et `RetroMediaLabStyle.cpp` : réduction globale plus marquée, sous-onglets plus petits que les onglets principaux, niveaux imbriqués encore plus compacts, police explicite dans le rendu des titres et conservation du pourcentage utilisateur.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.cpp` et `GreaseweazleDialog.cpp` : dimensionner les nouveaux dialogues selon l’écran de leur fenêtre parente, y compris le profil 800 × 600.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-compact-navigation/` : contrôler la navigation, les tailles comparées des trois niveaux, l’absence de cumul, les brouillons conservés et les rendus 1080p/100 % et 2160p/150 %.
  - [x] Créer `docs/validation/app/captures/1080p-15-pouces/*.png` : conserver les captures demandées à 1920 × 1080, mise à l’échelle système 100 %, avec leurs conditions de rendu.
  - [x] Créer `docs/validation/app/captures/800x600/*.png` : conserver les mêmes vues sur un écran virtuel 800 × 600 à 100 %.
  - [x] Supprimer `build/verification-compact-navigation/` et ses artefacts après validation Debug et Release.
