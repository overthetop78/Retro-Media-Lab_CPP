# Disponibilité de tous les périphériques au démarrage

- [x] Vérifier les configurations enregistrées depuis Infrastructure
  - [x] Créer `src/Infrastructure/Hardware/HardwareAvailability.h/.cpp` et modifier `src/Infrastructure/CMakeLists.txt` : inventaire par catégorie et backend configurés, confirmation Greaseweazle par info, état réel audio/optique/disques/cartouches/bandes, erreurs isolées et nettoyage sur fermeture.
  - [x] Modifier `src/App/Bootstrap/Application.cpp` : lancer la vérification asynchrone au démarrage, appliquer les résultats aux configurations actuelles et notifier les vues via WorkspaceState, sans faire de gestion matérielle dans App.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-hardware-startup/` : couverture de toutes les catégories, conservation des configurations, états des périphériques présents et absents, ports actualisés et vérification du service asynchrone.
  - [x] Supprimer `build/verification-hardware-startup/` et ses artefacts après validation.
