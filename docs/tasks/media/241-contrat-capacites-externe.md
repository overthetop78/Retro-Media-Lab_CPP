# Contrat de capacités de l'outil externe

- [x] Porter le contrat abstrait existant sans exécution dans le moteur
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/GwFormatCapabilities.h` : contrat `GwFormatCapabilityReader` avec chemin d'exécutable, résultat asynchrone et annulation, sans dépendance hôte ni GUI.
