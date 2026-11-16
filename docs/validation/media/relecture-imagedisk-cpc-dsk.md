# Relecture ImageDisk et CPC DSK

17 novembre 2026. Les 41 messages des catalogues MediaEngine/image-disk.json et cpc-dsk.json ont été lus dans les 29 langues (1 189 valeurs, anglais compris). 452 valeurs corrigées dans 56 fichiers de 28 langues : 227 CPC DSK, 225 ImageDisk.

Les corrections portent sur les phrases restées en anglais, la troncature, les pistes confondues avec des morceaux ou des traces, les secteurs devenus domaines d’activité, les champs d’un octet, les nombres de blocs/secteurs et les doublons formulés comme des commandes de copie. Les fichiers zh-Hant sont maintenant en caractères traditionnels. Les traductions Argos existantes sont conservées là où leur sens convient ; ces changements sont des corrections de relecture.

Les clés et paramètres numérotés sont identiques aux Constants. Un contrôle dédié a vérifié ImageDisk, Epson, QX-10, Amstrad, CPCEMU, DSK, EDSK, ST1/ST2 et la limite 65280 ; il a également permis de corriger deux occurrences d’Epson passées en minuscules en indonésien. Aucun nom technique n’a été ajouté aux données anglaises. Aucun codec ni writer n’a été modifié ou retesté. L’audit des littéraux existant passe pour 1 607 fichiers C++ et 29 langues.

Les batteries initiales 89/90 restent abandonnées sur instruction utilisateur et ne sont pas présentées comme réussies. Ces tâches sont clôturées pour les actions de code et de relecture réellement effectuées. Les catalogues des documents, des blocs et de l’optique restent distincts.

Les 28 DLL de langues modifiées ont été compilées ensemble en Release puis installées dans lib/Languages. Journaux : build/media-imd-cpc-catalogs-release.log et build/media-imd-cpc-catalogs-install.log. Aucune compilation des bibliothèques médias ni nouvelle batterie de formats.

