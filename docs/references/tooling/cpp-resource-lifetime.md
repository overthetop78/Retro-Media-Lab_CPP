# Durée de vie et libération des ressources C++

Références officielles consultées le 13 novembre 2026 pour préciser la mise en œuvre de la règle IMPORTANT du projet.

- [Microsoft : RAII](https://learn.microsoft.com/en-us/cpp/cpp/object-lifetime-and-resource-management-modern-cpp?view=msvc-170). L'objet responsable d'une ressource la libère à sa destruction. Les objets de portée et leurs membres permettent un nettoyage déterministe lors d'une sortie normale et du déroulement de pile d'une exception. Ce principe concerne aussi les handles et les autres ressources non mémoire. Acquérir via des membres/gardes déjà responsables évite une fuite lors d'un constructeur partiellement exécuté.
- [Microsoft : terminaison d'un processus](https://learn.microsoft.com/en-us/windows/win32/procthread/terminating-a-process). Une terminaison forcée peut empêcher toute exécution supplémentaire et ne pas notifier les DLL. Windows ferme les handles et récupère les ressources du processus ; cela ne termine pas automatiquement ses processus enfants. Ce mécanisme ne remplace donc pas le nettoyage applicatif.

Conséquences pour RML : concevoir la libération dès chaque acquisition, couvrir initialisation partielle, exceptions et tests, détacher les abonnements, arrêter les tâches avant de détruire leurs dépendances et libérer les ressources graphiques même sans fenêtre visible. Ne pas promettre l'exécution d'un finally après une terminaison forcée. Le besoin de superviseur de test se décide selon les ressources extérieures réellement créées.
