# Chargement des bibliothèques

Références officielles consultées le 14 novembre 2026 :

- [AddDllDirectory](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-adddlldirectory) : ajout d'un chemin absolu de recherche ; le cookie retourné doit être retiré avec RemoveDllDirectory.
- [LoadLibraryExW](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw) : chargement explicite avec contrôle des dossiers de recherche. RML limite la recherche aux dossiers enregistrés et au système plutôt qu'au dossier de travail ou au PATH.
- [QCoreApplication](https://doc.qt.io/qt-6/qcoreapplication.html#setLibraryPaths) : configuration des chemins de plugins. RML désigne lib/Qt6/plugins avant la création de QApplication.

La bibliothèque de thème demeure chargée tant que ses objets et son style sont vivants. Les points d'entrée de création, affichage et synchronisation convertissent leurs exceptions en diagnostic copié avant le retour à l'hôte. Les exceptions d'événements sont aussi copiées en erreur de l'hôte avant de déclencher l'arrêt ordonné.

Les catalogues compilés exposent une fonction C de lecture ; le service copie et valide les données, puis appelle unload, y compris sur erreur de lecture. Aucun pointeur vers les données du catalogue n'est conservé après le déchargement.
