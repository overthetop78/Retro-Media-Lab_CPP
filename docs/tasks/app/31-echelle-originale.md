# Échelle d’origine

- [x] Rétablir le pourcentage réel de l’interface
  - [x] Modifier `src/App/Themes/RetroMediaLab/DisplayDensity.cpp` : calculer le facteur uniquement depuis le pourcentage utilisateur, avec 100 % = 1, sans facteur compact ni réduction liée à la résolution ; conserver les dimensions de fenêtre et le DPI géré par Qt.
  - [x] Modifier `src/App/Themes/RetroMediaLab/ThemeConstants.h` : supprimer les constantes de réduction automatique devenues inutiles.
