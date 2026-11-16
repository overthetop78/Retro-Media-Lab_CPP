# Relecture des traductions cartouches, Famicom et GameCube

Le 17 novembre 2026, lecture des 34 messages des catalogues MediaFileSystems/cartridge-readers.json, famicom-disk.json et gamecube-fst.json dans les 29 langues (986 valeurs, anglais compris). Les traductions Argos existantes sont conservées quand leur sens convient. 336 valeurs ont été corrigées dans 66 fichiers de 28 langues.

Corrections : banques mémoire confondues avec des banques financières, cartouches devenues cartouches d’encre ou munitions, volume devenu volume sonore, offset confondu avec compensation/délai, perte des notions de nombre/taille/étendue, négation perdue dans FdsNoFiles ukrainien, phrases restées en anglais, nom GameCube déformé et accents français corrompus. Les trois catalogues zh-Hant sont corrigés en caractères traditionnels.

Les identifiants et paramètres numérotés restent identiques aux Constants. GameCube est ajouté à la liste commune des invariants : la protection antérieure de Nintendo GameCube FST ne couvrait pas les formulations abrégées réellement présentes. Les catalogues restent issus du pipeline Argos ; les présentes modifications sont des corrections de relecture. Le script actuel réécrit les fichiers explicitement régénérés : ne pas relancer ces catalogues sans modification des sources, conformément aux règles du projet.

Cette relecture ne clôture pas les autres catalogues encore ouverts (optique, blocs, documents et CPC DSK). Aucun décodeur ou système de fichiers n’a été modifié ni retesté.

Contrôles finaux réussis : 66 fichiers modifiés valides, mêmes clés et paramètres que Constants, noms invariants conservés ; audit des littéraux PASS (1 607 fichiers C++, 29 langues). Les 28 DLL de langues modifiées ont été compilées ensemble en Release et installées dans lib/Languages de la distribution. Aucun module média n’a été recompilé.
