# Corriger les captures sans changer les fenêtres

- [x] Refaire les rendus
  - [x] Créer `build/verification-normal-windows/` : ouvrir l’application et ses dialogues avec leurs dimensions normales, sans maximisation, déplacement ou redimensionnement forcé ; vérifier leur géométrie avant et après chaque capture.
  - [x] Modifier `docs/validation/app/captures/1080p-15-pouces/*.png` et `docs/validation/app/captures/800x600/*.png` : remplacer les douze captures par les vues normales sur un fond d’écran distinct, au même format de bureau virtuel.
  - [x] Supprimer `build/verification-normal-windows/` et ses artefacts après contrôle visuel et vérification des dimensions.
