# Définitions de disquettes

Sources C# : DiskTrackDefinition.cs, DiskDefsFormatReader.cs, GwFormatArgument.cs pour les alias de dimensions et MediaMetadataFunctions pour le contexte des conversions. Les 35 CFG de Images/Formats/Definitions ont été copiés sans changement, notices et attribution Greaseweazle v1.23 conservées. Ils sont embarqués dans MediaEngine.dll, sans fichier à rechercher à l'exécution.

Les modèles C++ distinguent paramètres, sélection de piste, définition et disposition sectorielle. La dernière sélection explicite correspondante est prioritaire ; à défaut, la première sélection globale. Les plages, faces et listes sont reprises du C#. Les tailles peuvent se répéter avec '*', se succéder ou être rembourrées dans l'image. Les numéros physiques Apple suivent la liste secs, les autres codecs suivent leur premier numéro et leur taille par défaut.

Un secteur de disposition conserve l'indice de sa définition de piste, sans pointeur pendant vers un modèle temporaire. Les conversions int32, tailles positives et calculs de tailles répétées sont contrôlés ; les valeurs invalides utilisent un diagnostic traduit. Les configurations invalides ne provoquent ni division par zéro ni débordement signé.

Le lecteur embarqué traite imports préfixés, commentaires, disques, pistes, paramètres et fins de blocs. Les identifiants/paramètres utilisent une comparaison insensible à la casse. Les valeurs de codecs restent sensibles à la casse comme dans les switch C#. Un cycle d'import est rejeté avec nettoyage RAII de la pile active et fermeture du fichier.

readFileIds conserve le comportement externe C# : énumération des noms et imports sans interpréter les corps des définitions. dimensions applique les alias atarist.1440/mac.1440 vers ibm.1440, apple3.sos vers apple2.prodos.140 et apple2.prodos.800 vers mac.800.

rawDefinitions reçoit explicitement les identifiants réservés du catalogue principal et exclut ceux-ci, puis retient les sept codecs C# autorisés. Le bloc 130 ajoute le catalogue principal complet et une surcharge par défaut qui en exclut les 98 identifiants ; aucune liste réservée réduite n'est utilisée.

Voir image-format-catalogue.md pour le raccordement du catalogue principal.
