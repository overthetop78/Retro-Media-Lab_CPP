# Référence du portage Émulation

Source C# utilisée en lecture seule. [Retro Media Lab, branche osd-support](https://github.com/overthetop78/Retro-Media-Lab/tree/4e1f3866e20c8df817287b80fe8b39cbc6d4016c)

- `RetroMediaLab.Emulation` définit les contrats communs. Le port natif fournit `Emulation.dll` et charge les binaires C++ via `Modules/<id>/module.json` et le point d'entrée `rml_emulation_factory_v2`. Les interfaces de ce premier SDK passent des types C++ et Qt. Un module natif doit donc utiliser une chaîne de compilation et une version de Qt compatibles avec celles de RML.
- `EmulationStateFileFunctions` définit les sauvegardes communes. `StateFile` utilise le marqueur `RetroMediaLabSTATE1`, une longueur little-endian, les clés JSON du C# et la vérification SHA-256 de l'état. La restauration des octets reste sous la responsabilité du module et son cœur.
- `VideoPresentationProfileStore` et `EmulationVideoPresentationProfile` définissent les dossiers par module et par identifiant de configuration et les clés JSON camelCase. Le rendu Qt actuel couvre la sortie raster, les proportions, les trames entrelacées et deux modes d'échantillonnage.
- L'onglet App expose les machines et réglages déclarés par les modules chargés. Le thème présente leurs commandes et leur sortie. Le chargement des modules et l'accès aux médias passent par Emulation.
- La compilation et la copie Release ont été vérifiées sous Windows. Un module temporaire a servi à exercer le chargement, la durée de vie des sessions, les trames RGB565, les profils et le format des sauvegardes. Il a été retiré après vérification.

Le [suivi du portage](../../tasks/emulation/01-portage-socle.md) liste les fonctionnalités communes qui restent à reprendre.

Le bandeau des périphériques expose les médias montés, l'insertion et l'éjection. Les commandes de cassette reprennent l'ordre et les règles d'état de `TapeTransportPresentationFunctions.cs`. Les libellés proviennent de `EmulationTape.resx` pour toutes les langues du projet. L'enregistrement sélectionne le format parmi les extensions prises en charge par le lecteur et MediaEngine. Le profil RAW reprend les réglages PCM du lecteur. Les formats WAV, UEF, CSW et RAW ont été vérifiés via la création de cassettes vierges. Le montage et le démarrage du transport restent à vérifier avec un module réel. L'ABI native est passée à 2.1 pour les champs ajoutés au contrat des médias.

Le moteur logiciel reprend le compositeur de champs du C# avec détection du mouvement par blocs, interpolation Bob, tissage et mélange vertical. Il applique les ajustements du profil en lumière linéaire à l'image composée. Les libellés du dialogue proviennent de `EmulationVideo.resx`. Les appellations Bob et Weave utilisent le libellé anglais si le catalogue source de la langue ne les définit pas. Le test Release vérifie le tissage, la distinction entre mouvement et stabilité, la désaturation et la sauvegarde du profil. Les moteurs matériels et traitements d'affichage restent à porter.

Le chemin OpenGL présente l'image issue du compositeur commun via un contexte OpenGL Qt et conserve la surimpression de l'OSD. Le sélecteur vidéo enregistre le champ `renderer` du profil. La présentation logicielle Qt sert de repli si le contexte OpenGL ne peut pas être créé. Le test Release a lu les pixels du tampon OpenGL et vérifié le retour au moteur Qt et la persistance du profil. Les shaders OpenGL de simulation d'affichage du C# restent à porter.

Direct3D 11 est le moteur initial sous Windows pour les profils sans choix enregistré. Sa chaîne DXGI présente l'image issue du traitement commun, avec la mise à l'échelle sur le GPU et une texture distincte pour les surimpressions. En cas d'échec du moteur, Qt présente l'image. Le test Release vérifie l'appel de présentation, le changement de taille, le déplacement vers une autre fenêtre et la sauvegarde du moteur. Les shaders CRT et le chemin Vulkan du C# ne font pas encore partie de ce rendu.

L’onglet Signal reprend du C# les cinq connexions analogiques, les trois normes de couleur et leurs intensités. Le mode automatique utilise le seuil de 18,2 ms du moteur logiciel C# entre PAL et NTSC. Les filtres travaillent en lumière linéaire avant les ajustements d’image, puis le moteur choisi présente l’image obtenue. Le test Release vérifie les cinq connexions, les trois normes, le choix automatique et l’écriture du profil. La simulation du moniteur CRT, ses shaders et les autres technologies d’affichage restent à porter.

Le moteur logiciel CRT reprend `CrtSignal`, `FilterCrt` et `CrtPhosphorMask` pour la synchronisation, le décodage couleur, le faisceau, la courbure et les masques à la résolution de sortie. Le profil enregistre les réglages CRT dans les champs du C#. Le test Release vérifie la courbure, le décodage monochrome, les masques et la persistance. Le portage de FilterCrtConnection ajoute la dégradation des connexions analogiques et les palettes MDA, CGA et EGA. La validation Release couvre la Péritel avec et sans dégradation, les palettes MDA et EGA et la persistance du profil. Les shaders GPU et les autres technologies d'affichage restent à porter.

Le rendu logiciel des écrans à pixels fixes reprend les filtres LCD, LCD rétroéclairé par LED et OLED de osd-support. Le pipeline applique les sous-pixels, la grille et l’éclairage à la taille de présentation, puis le temps de réponse et la persistance à chaque trame. Les réglages utilisent fixedPixel, lcdProfile, ledBacklitLcdProfile et oledProfile. Les libellés proviennent de EmulationVideo.resx. Le contrôle Release hors écran vérifie les niveaux de noir des trois technologies, le temps de réponse OLED, la teinte monochrome, la taille du rendu et le rechargement du profil.

Les effets VHS, aberration chromatique, halo lumineux, sépia et grain reprennent les filtres logiciels du C#. Ils s’appliquent après la simulation de l’écran, avant sa réponse temporelle ou le masque CRT. Le halo utilise une somme glissante pour conserver le voisinage 9 × 9 du C# en évitant de relire chaque fenêtre. Les profils enregistrent les valeurs dans processing.stylistic. Le contrôle Release vérifie les cinq effets sur mire, dont le calcul du halo et le grain déterministe selon la séquence, ainsi que le rechargement des réglages.

La restauration vidéo reprend EmulationImageRestorationFunctions. Elle atténue sept phases de motifs de tramage sur une fenêtre de cinq pixels par cinq, lisse les bandes de faible contraste et récupère les détails locaux perdus lors d’une mise à l’échelle avec interpolation. Les réglages sont conservés sous processing.restoration avec le désentrelacement. Le contrôle Release a couvert les motifs simples et appariés, la préservation d’un contour marqué, la réduction d’une bande, la récupération d’un détail et le rechargement du profil.

Le rendu logiciel plasma reprend l’ordre des huit filtres de osd-support, de la profondeur des noirs à la rémanence après les effets stylistiques. Les paramètres utilisent processing.plasma et la technologie accepte la valeur numérique ou textuelle du profil C#. La validation Release couvre le limiteur, les noirs et le gamma, les phosphores, la structure des cellules, le changement de phase du tramage, la diffusion, l’extinction progressive et la remise à zéro de la rémanence. Le profil conserve les paramètres lors du retour au rendu normal.

Le rendu logiciel vectoriel reprend la détection des contours, la largeur et la focalisation du faisceau, le rayon et l’intensité du halo, la teinte du phosphore et la rémanence du C#. Le calcul du halo et de la focalisation conserve les bords étendus de FilterSampling et utilise deux sommes glissantes. La rémanence se calcule par trame reçue, même si des numéros de séquence ont été sautés. Le contrôle Release vérifie les contours, le faisceau, le halo ambre, la décroissance et la remise à zéro de la rémanence ainsi que le rechargement du profil.

Le rendu logiciel VFD reprend le seuil d’émission, les structures graphique et matricielle, la teinte et l’intensité du phosphore, le verre fumé, le halo et la rémanence calculée selon le temps écoulé. Le profil persiste sous processing.vfd. Le contrôle Release vérifie la couleur, la forme des cellules, le halo, le verre, la décroissance à 20 et 60 ms et le rechargement du profil.

La matrice LED reprend la moyenne des pixels par cellule, la géométrie ronde ou carrée, les teintes, le halo, la luminosité et le niveau du noir de osd-support. Le compositeur stocke les moyennes par cellule et calcule les masques lors de l’écriture de l’image. La sélection de technologie est commune au dialogue, au rendu logiciel et au stockage de profil. Le contrôle Release vérifie les formes, la moyenne, le halo teinté, le niveau du panneau, le rendu hors écran et la conservation des réglages après un changement de technologie.

Le rendu matriciel à points reprend les palettes LCD réfléchissantes et émissives, les points ronds, carrés ou rectangulaires, et les deux lois de rémanence de osd-support. Le calcul des moyennes et des bords de cellule est commun avec la matrice LED. Le contrôle Release vérifie les deux familles de palettes, la géométrie, la réponse temporelle, le rendu hors écran, le rechargement du profil et un cas de rendu LED après mutualisation.

L’afficheur à segments reprend les géométries à 7, 14 et 16 segments, le point décimal, les deux-points, les terminaisons et l’activation isolée de chaque segment à partir de cinq échantillons. Le profil stocke aussi la lumière et les durées d’allumage et d’extinction. Les essais Release vérifient les trois géométries, les points, la réponse temporelle, le rendu hors écran et la sauvegarde du profil.

Le papier électronique et la projection reprennent les filtres logiciels `FilterEPaper` et `FilterProjection`. Le papier électronique comprend les modes monochrome, 16 niveaux de gris et 4096 couleurs, avec tramage ordonné, teinte du papier, temps de rafraîchissement et image résiduelle. La projection reprend le flou optique, la diffusion, la convergence, la luminosité, la lumière ambiante, la texture et le vignettage. Leurs réglages sont enregistrés avec le profil vidéo. Les 30 catalogues reprennent les clés de `EmulationVideo.resx` avec le texte source anglais comme repli. La compilation Release, la persistance des profils et les effets de couleur, de rafraîchissement et de lumière ambiante ont été vérifiés. Les DLL copiées vers la distribution Release correspondent aux binaires compilés.

L'OSD plein écran reprend la superposition à gauche, les sous-menus, la navigation par flèches, Entrée et Échap et la fermeture après 15 secondes sans activité de `FullscreenOsdWindow`. F10 met en pause une session en cours et la reprend à la fermeture. Maj+F10 la laisse tourner. Les commandes déjà disponibles sont accessibles depuis la superposition. Les sauvegardes utilisent encore les dialogues de fichiers et les réglages vidéo ouvrent leur dialogue Qt. Les éditeurs intégrés à l'OSD, les raccourcis manette et les commandes spécifiques aux modules restent à porter. Le test Qt hors écran vérifie le focus, l'entrée et la sortie d'un sous-menu, une commande désactivée, un état de case à cocher, le redimensionnement et la fermeture. La compilation Windows Release et la copie des DLL ont été vérifiées.

Les commandes rapides reprennent `MachineQuickStates`, `MachineController.Open.cs` et `EmulationShortcutDefaults`. Le fichier `<module>-<configuration au format N>.gwas` est créé à la première sauvegarde dans `Documents/Retro Media Lab/Emulation/States`, le dossier par défaut du C#. La session expose un dossier injectable via Host. L'OSD, la barre de session et Ctrl+Maj+F6/F7 utilisent les mêmes commandes. La disponibilité dépend de l'état de la machine, de sa capacité à sauvegarder et, pour le chargement, de la présence du fichier. La sonde temporaire a vérifié le chemin, l'annulation, l'isolation de deux configurations et la restauration après réouverture. La compilation Windows Release et la copie des DLL ont été vérifiées.

L'éditeur de paramètres reprend la navigation de `FullscreenOsdWindow.Menu.cs` et les lignes vidéo de `MachineController.OsdVideo.cs`. La technologie sélectionnée détermine les lignes affichées. Gauche et droite modifient la valeur, haut et bas passent au paramètre suivant, le curseur permet de régler les valeurs numériques. Le profil vidéo est prévisualisé puis enregistré immédiatement. En cas d'échec, la dernière valeur enregistrée et son rendu sont restaurés. Cette étape couvre la technologie, le moteur, l'échantillonnage, les ajustements d'image et de signal, les paramètres de la plupart des technologies, la restauration et les effets. Les réglages propres aux modules, ceux de mise en œuvre GPU et les options du C# encore absentes du profil natif restent à porter. La vérification Qt hors écran a exercé la navigation, les valeurs, les lignes conditionnelles et le refus de sauvegarde. La distribution Release a été mise à jour et les DLL ont été vérifiées par empreinte.

Les groupes de réglages compatibles avec les contrats natifs reprennent `MachineController.OsdModuleSettings.cs`. Le menu regroupe les sélections et interrupteurs par section et actualise leur visibilité après sauvegarde. Le sous-menu audio reprend l'état muet et les réglages du module. Les interfaces facultatives `IModuleRuntimeOptions` et `IMachineRuntimeOptions` appliquent uniquement les options disponibles à chaud et dont la valeur a changé. La session utilise la configuration enregistrée au prochain démarrage. La sonde temporaire a vérifié le chargement des interfaces à travers la DLL, la préservation des champs non modifiés, l'application à chaud et celle au redémarrage. Les éditeurs numériques et informatifs des modules, les règles avancées et le transfert automatique des médias lors de la recréation restent à aligner avec le C#. La compilation et la copie Windows Release ont été vérifiées.

Les médias montés sont conservés dans la session, conformément à `MachineSession.cs`. Une machine recréée reçoit les médias compatibles avant son démarrage, avec le chemin, le mode lecture seule et les paramètres du média. `IModuleMountedMedia` permet au module de les passer directement à son constructeur et de les enregistrer dans sa configuration, suivant `PersistMountedMediaAsync`. Sans cette interface, le montage avant démarrage passe par les commandes natives d'insertion et d'éjection. Un refus détruit la machine de remplacement et permet de corriger le média puis de réessayer. La sonde temporaire a vérifié ce chemin, le montage avant démarrage, les paramètres d'une cassette, la recréation après une modification de configuration et l'éjection à l'arrêt. Les DLL de la distribution Windows Release ont été vérifiées par empreinte.

Les champs `Information` et `Slider` du C# sont présentés dans l'OSD et dans le dialogue de configuration. L'interface facultative `IModuleNumericSettings` fournit les bornes, le pas, la valeur automatique et les unités sans modifier le format des structures précédentes du SDK. Les valeurs numériques sont persistées avec le séparateur invariant et affichées selon la langue. La sonde Qt a vérifié le dernier pas partiel, les unités localisées, le mode automatique, le refus d'un pas invalide et l'actualisation du statut informatif. Les règles conditionnelles générales, le pas fin et la saisie numérique directe restent à porter. Les DLL de la distribution Windows Release ont été vérifiées.

Les règles de `EmulationSettingsRuleFunctions.cs` et la dépendance de visibilité des blocs sont prises en charge par le dialogue et l'OSD. L'interface facultative `IModuleSettingsDependencies` expose les exclusions mutuelles, les champs ou groupes dépendants et les onglets masqués. Le champ venant d'être réglé l'emporte sur son option incompatible. L'OSD calcule la modification conjointement avec le module sur la dernière configuration sauvegardée. Le dialogue applique la même règle à son brouillon. La sonde reprend les scénarios du C# dans les deux sens et vérifie les groupes conditionnels, les sources absentes et la conservation des paramètres indépendants. La compilation Windows Release passe et les DLL déployées correspondent aux DLL compilées.

L'interface facultative `IModuleEditableNumericSettings` expose la saisie numérique et le pas fin au dialogue de configuration, d'après `CreateEditableSlider` et `NormalizeEditableSlider`. Le curseur utilise le pas fin, les boutons et les flèches utilisent le pas principal, et la valeur saisie s'affiche avec le séparateur décimal de la langue. Le mode automatique désactive les contrôles manuels et permet de retrouver la dernière valeur manuelle dans le dialogue. Les valeurs sont sérialisées avec deux décimales invariantes, comme dans ce contrôle du C#. L'OSD conserve le pas principal du C#. La sonde reprend la plage et les pas du réglage de pointeur Sega à partir du contrat, sans porter le module. La compilation Windows Release et les empreintes des DLL copiées ont été vérifiées.

La navigation à la manette reprend les six directions et actions de `MachineController.cs`. À la fermeture de l'OSD, les entrées encore maintenues sont filtrées jusqu'au relâchement, suivant `OsdControllerReleaseFilter.cs`. Le socle Windows utilise l'interface GameInput v0 du SDK Windows. La croix directionnelle et les boutons A/B pilotent l'OSD déjà ouvert. Le C# laisse les raccourcis manette non assignés par défaut. Le choix natif est provisoire tant que leur configuration n'est pas portée. Les boutons et axes bruts du C# et la navigation à la manette sous Linux et macOS restent à porter. La compilation Windows Release et la copie des DLL sont vérifiées. L'essai sur une manette physique reste à faire.

- [Microsoft GameInputCreate](https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/input/gameinput/functions/gameinputcreate)
- [Versions de GameInput](https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/input/overviews/input-versioning?view=gdk-2604)

Les raccourcis clavier du socle reprennent les combinaisons de `EmulationShortcutDefaults.cs` de la branche `osd-support`. Les raccourcis de session déclenchent les mêmes actions que les boutons et sont actifs sur la surface vidéo et son OSD. Au pavé numérique, Ctrl+Maj+/ ouvre l’OSD avec pause, Ctrl+Maj+* l’ouvre en laissant tourner la machine et Ctrl+Maj+- bascule les statistiques en plein écran. Ctrl+Maj+F5 ouvre le dialogue vidéo actuel. Le dialogue complet de réglages de la session, la capture et la libération relatives de la souris et les affectations personnalisées restent à raccorder. La compilation Windows Release et l’empreinte de la DLL du thème déployée ont été vérifiées.

La capture relative reprend `MachineInputController.cs` et `RelativeMouseCapture.cs` de `osd-support`. Une machine active peut opter pour la capture au clic via `IMachinePointerCapture`. La surface masque le curseur, le recentre après mouvement et transmet les deltas et les deux axes de molette. Ctrl+Maj+F12 et la perte de focus libèrent la souris. L’OSD suspend la capture et la restaure à sa fermeture si la session est toujours active. Les boutons maintenus pendant la fermeture sont ignorés jusqu’au relâchement. La compilation Windows Release et les empreintes des DLL déployées ont été vérifiées. Le comportement avec une machine en cours d’exécution et la prise en charge du recentrage selon la plateforme restent à vérifier.

Ctrl+Maj+F5, la commande du menu Émulation et le bouton des réglages de session ouvrent l’éditeur de la configuration de la machine active. Comme `MachineController.Open.cs` sur `osd-support`, l’éditeur récupère la dernière version enregistrée en recherchant l’identifiant de la session. Le dialogue conserve les valeurs en brouillon jusqu’à l’enregistrement. La session persiste les champs modifiés et applique à chaud les options compatibles. Les réglages vidéo gardent leur propre accès. La compilation Windows Release et l’empreinte de la DLL déployée ont été vérifiées. L’essai en session reste à faire.

Comme sur `osd-support`, les captures sont enregistrées directement en PNG, dans le dossier configuré ou par défaut dans `Documents/Retro Media Lab/Emulation/Captures`. Le nom suit `capture-yyyyMMdd-HHmmss.png` et reçoit un suffixe en cas de collision. La règle vient de `MachineCaptureFunctions.cs`. Le dernier dossier média est mémorisé par module, modèle de machine et catégorie, d’après `EmulationMediaFolderFunctions.cs`, en utilisant les chemins déjà persistés dans `preferences.json`. Le dialogue d’état manuel mémorise aussi son dernier dossier. Les statistiques plein écran ont une préférence persistante, distincte de la case de la session en mode fenêtré. Le build Windows Release passe et les empreintes de l’exécutable et des DLL déployées correspondent aux sorties de compilation. Les essais en session restent à faire.

Les raccourcis clavier de session et d’OSD lisent `emulationShortcuts` dans `preferences.json`, avec les identifiants et alternatives de `EmulationShortcutMap.cs`. Une clé absente utilise le réglage par défaut. Une chaîne vide désactive l’affectation. Les noms du pavé numérique du C# sont convertis en touches Qt. Les affectations clavier utilisent la syntaxe portable Qt et sont lues à l’ouverture de la session. L’écran de configuration, les combinaisons physiques à plusieurs sources et les contrôles non-clavier configurables restent à porter. La sonde temporaire vérifie les trois équivalences du pavé numérique, les remplacements, les alternatives, la désactivation et l’absence de déclenchement clavier pour une affectation composite non prise en charge. Windows Release compile et les empreintes des fichiers déployés correspondent à la compilation.

Le menu Émulation ouvre un dialogue de préférences avec l’onglet Raccourcis. L’éditeur reprend les commandes et alternatives de `EmulationShortcutMap.cs`, la recherche et la restauration des valeurs par défaut de `EmulationPreferencesSection`. Il signale les doublons. L’enregistrement explicite met à jour les sessions ouvertes, alors que le C# sauvegarde à chaque modification valide. Vingt-huit libellés ont été repris des trois fichiers RESX d’origine dans chacun des 30 catalogues. Les autres onglets, les associations souris/manette et la validation des raccourcis système réservés restent à porter. L’éditeur et tous les catalogues compilent en Release. Les empreintes des fichiers déployés ont été vérifiées.

L’éditeur des raccourcis signale les combinaisons Windows de `ReservedKeyboardChordCatalog.cs`, y compris leurs variantes avec des modificateurs supplémentaires. Il signale aussi les séquences Qt qui commencent par un raccourci déjà affecté. Les raccourcis réservés trouvés dans un fichier de préférences sont ignorés par les sessions sous Windows. Le libellé est repris de `EmulationInput.resx` pour tous les catalogues. La compilation Release et les empreintes des DLL déployées ont été vérifiées.

Les raccourcis de session lisent les sources `Mouse:` et `Controller:` de `EmulationInputMappingFunctions.cs`. Windows utilise l’identifiant GameInput hexadécimal décrit par `GameInputDeviceInspector.cs`. Le traitement reconnaît les boutons standard, les directions des sticks, les gâchettes, les boutons de souris et les deux axes de molette. Les associations souris et manette peuvent être combinées dans un même raccourci. Les commandes se déclenchent à l’activation et les raccourcis manette prédéfinis de l’OSD ont été retirés comme dans la configuration par défaut du C#. Les contrôles bruts, les combinaisons incluant le clavier, l’affectation graphique et le test avec le matériel réel restent à faire. La sonde a vérifié la sélection du périphérique, les directions analogiques, les combinaisons et la molette. La version Windows Release compile et les empreintes des DLL déployées correspondent aux DLL produites.

Le bouton Affecter ouvre une capture temporaire, limitée à 15 secondes comme `InputBindingCapture.cs`. L’utilisateur peut affecter une touche, un bouton de souris, un axe de molette ou un contrôle standard de manette GameInput. La manette doit franchir le seuil depuis l’état au repos et relâcher le contrôle avant validation. La capture et l’exécution utilisent la même nomenclature. Les affectations physiques apparaissent dans la recherche et peuvent être effacées ou remplacées. Les libellés Affecter et l’invite sont repris de `EmulationInput.resx` dans les 30 catalogues. La sonde Qt hors écran vérifie la validation souris et clavier et le front analogique avec identifiant. La compilation Release et les empreintes des fichiers déployés ont été vérifiées. La capture de combinaisons mixtes, les contrôles bruts et la validation sur une manette connectée restent ouverts.

L’éditeur capture les raccourcis jusqu’au relâchement des entrées, suivant `InputBindingCapture.cs`. Le traitement accepte plusieurs touches simultanées et les combinaisons avec la souris, la molette ou les contrôles standard des manettes. Les affectations physiques sont comparées après normalisation de l’ordre et de la casse. Les combinaisons réservées par Windows restent bloquées. La sonde Qt vérifie Ctrl avec un bouton de souris, le relâchement, une combinaison avec manette simulée et le signalement des doublons et des raccourcis réservés. La compilation Windows Release et l’empreinte de la DLL déployée ont été vérifiées. Les contrôles bruts, les modificateurs gauche/droite, le MIDI et la validation en session avec du matériel réel restent à faire.

Le socle Windows transmet les axes, boutons et commutateurs bruts GameInput aux sessions via l’interface facultative du SDK. La capture reprend les noms et l’état initial de la manette du C# et l’OSD filtre les contrôles maintenus jusqu’à leur relâchement. La sonde a vérifié les fronts, la direction et l’identifiant du périphérique. Le build Windows Release passe. La vérification sur du matériel réel et le repli HID du C# restent ouverts.

Le repli Windows utilise RawGameController comme sur osd-support. Il reprend l’identifiant non itinérant, les axes à partir de zéro, les boutons à partir de un et les directions des commutateurs. Il évite les doublons avec GameInput par identifiants constructeur et produit, puis par nom normalisé. Le build Windows Release et les empreintes des DLL déployées ont été vérifiés. Le mappage des contrôles par modèle, le décodage des rapports HID propres à GameInput et la vérification avec une manette qui requiert le repli restent ouverts.

Le repli RawGameController embarque SDL_GameControllerDB et sa licence, repris à l’identique de la branche C# osd-support. Le profil Windows est choisi à partir des identifiants constructeur et produit. Il expose les sticks et gâchettes normalisés, les boutons standard, les directions et le bouton Guide quand il est mappé. Les contrôles bruts restent accessibles. La sonde utilise le profil Xbox 360 de la ressource embarquée et vérifie un bouton, une diagonale, les signes des sticks, une gâchette et un modèle inconnu. Le build Windows Release passe, les empreintes des DLL déployées correspondent et la sonde temporaire a été retirée. Le décodage des rapports HID GameInput et l’essai sur matériel restent ouverts.

Le C# utilise les capacités HID du chemin PnP fourni par GameInput récent. Avec l’interface GameInput v0 du SDK Windows, le port C++ utilise les éléments de rapport fournis par le périphérique et GetItemValue pour éviter de déduire les offsets des octets. Le repli traite les axes absolus, les boutons individuels et les chapeaux. Il filtre les périphériques selon les mêmes usages de manette que le C#. Les rapports sans description exploitable restent du ressort du repli RawGameController. La sonde a vérifié un axe signé, un bouton, un chapeau et l’isolation entre identifiants de rapports. Le build Windows Release passe et les empreintes des DLL mises à jour correspondent. Les tableaux HID, le chemin PnP GameInput v4 et la validation sur du matériel réel restent ouverts. [Description des éléments GameInput v0](https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/input/gameinput/deprecated/structs/gameinputrawdevicereportiteminfo?view=gdk-2604).

La session active peut publier des sorties de vibration par l’interface facultative IMachineControllerFeedback. Le thème conserve l’intensité maximale par moteur et périphérique, comme MachineControllerFeedbackController.cs sur osd-support. GameInput applique les moteurs annoncés par la manette et évite les envois redondants. Les moteurs sont arrêtés quand la sortie disparaît, en cas d’erreur, de perte du focus ou de fermeture de la session. Le build Windows Release et les empreintes des DLL mises à jour ont été vérifiés. Le test matériel, l’accumulateur des commandes fortes et faibles, ainsi que leur configuration par port restent ouverts.

Le SDK exporte l’accumulateur des effets forts et faibles et la résolution des sorties par port. Les comportements de référence sont ceux de EmulationControllerFeedbackAccumulator.cs et EmulationControllerFeedbackService.cs sur osd-support. Les ports configurés de 1 à 10 correspondent aux indices de cœur de 0 à 9. Un effet peut être remis à zéro seul, par port ou pour tous les ports. La sonde liée à Emulation.dll a vérifié les limites, le rejet des affectations invalides, deux effets coexistants et leur remise à zéro. Windows Release compile et les empreintes des DLL mises à jour correspondent. L’éditeur des affectations, le transport des effets depuis les cœurs hébergés et l’essai sur matériel restent ouverts.

Le socle fournit les identifiants, noms et moteurs annoncés par les manettes GameInput pour le futur sélecteur. Comme ControllerOutputChoices.cs sur osd-support, choisir un périphérique affecte l’effet fort au moteur basse fréquence et l’effet faible au moteur haute fréquence. Choisir Aucun retire ces affectations. Le build Windows Release et les empreintes des DLL mises à jour ont été vérifiés. Le raccordement de ces choix au dialogue de configuration de chaque port et la vérification sur matériel restent ouverts.

L’éditeur de configuration affiche les ports déclarés par IModuleControllerFeedbackSettings. Il reprend le choix d’une seule manette pour les vibrations fortes et faibles et le choix d’un moteur par commande pour les autres sorties, conformément à EmulationControllerOutputEditor.cs. Il préserve une affectation à un périphérique débranché et actualise les choix à l’ouverture du menu. Les ports modifiés sont enregistrés avec les autres réglages. Une machine qui expose IMachineControllerFeedbackSettings reçoit les affectations au chargement, à la recréation et après enregistrement. Dix libellés de Controllers.resx, EmulationController.resx et EmulationTab.resx ont été repris dans les 30 catalogues, avec %1 pour le numéro de port. Le build Windows Release et les empreintes des DLL copiées ont été vérifiés. Le dialogue et les sorties restent à vérifier avec un module exposant ces ports et une manette réelle.

Emulation.dll exporte les codecs des en-têtes, chaînes UTF-8, blocs binaires, effets de vibration, trames vidéo dans le flux et échantillons PCM. Leur disposition reprend EmulationHostProtocolFunctions.cs et EmulationHostProtocolFunctions.Host.cs sur osd-support. Ils opèrent sur un message déjà disponible et laissent le flux ouvert. Le port borne aussi les chaînes à la limite des blocs binaires et vérifie le format pixel et la conversion des horodatages. La sonde liée à la DLL a vérifié les octets de référence des en-têtes, effets et longueurs UTF-8, la lecture successive vidéo et audio et le rejet de données tronquées ou invalides. Windows Release et les empreintes déployées ont été vérifiées. Les entrées, les autres blocs de statut, la vidéo partagée et l’intégration au transport de processus restent à porter.

Le codec des entrées reprend les identifiants de touches du protocole C#, les caractères UTF-16, les déplacements et cinq boutons de la souris, les six axes signés et les contrôles nommés des manettes. Les statuts de tampon audio, région, activité des lecteurs et disque sélectionné ont également été portés. Les types de transport gardent les identifiants du C# et les conversions depuis les événements Qt seront réalisées au raccordement du client. La sonde liée à Emulation.dll a comparé les octets des entrées au format C# et vérifié la lecture successive des statuts ainsi que le rejet d’un décompte hors limites. Windows Release et les empreintes déployées ont été vérifiées.

Le transport vidéo partagé utilise deux emplacements de 128 Mio et publie leur indice avec les métadonnées de EmulationHostProtocolFunctions.cs. L’indice d’écriture suit la parité du numéro de séquence. Sous Windows, le nom natif du segment est conservé pour que le processus C# puisse l’ouvrir. La sonde a ouvert le même segment via Win32, vérifié les pixels des deux emplacements, les octets de l’en-tête, le rejet d’un emplacement hors limites et la disparition du segment après fermeture des références. Windows Release et les empreintes déployées ont été vérifiées. La sérialisation des échanges et le lancement des processus restent à porter. [Interopérabilité de la mémoire partagée Qt](https://doc.qt.io/qt-6/native-ipc-keys.html#interoperability-with-non-qt-applications).

## Canal local des cœurs hébergés

Le transport reprend les trois arguments du C# `osd-support`. Les requêtes incluent leur propre en-tête. Les réponses commencent par une longueur signée de 32 bits en little endian. L’hôte limite cette longueur à 512 Mio et ferme la connexion si l’échange est interrompu ou si la longueur est invalide. Le canal local de Qt utilise un tube nommé sur Windows. [Documentation officielle de QLocalSocket](https://doc.qt.io/qt-6/qlocalsocket.html#details)

Validation Release sous Windows avec un processus enfant utilisant les API natives du tube nommé. Le processus ouvre le segment vidéo et renvoie deux réponses, dont une de 200 000 octets. Une longueur négative entraîne la fermeture de la connexion. La sonde temporaire a été supprimée après exécution.

## Commandes du client des cœurs hébergés

Les codes des commandes sont repris du protocole commun et de l’hôte Example sur `osd-support`. Le client encode les arguments communs et vérifie la version de chaque réponse. Une erreur du cœur est décodée depuis son objet JSON. Un en-tête invalide ferme la connexion. La version et la disposition des données propres au cœur sont fournies par le module.

La sonde Release Windows a contrôlé les octets d’éjection, de réglage d’une option, de vérification des modifications de média et de réinitialisation logicielle. Elle a vérifié qu’une erreur du cœur permet la commande suivante et qu’une version de réponse incorrecte ferme la connexion. La sonde a été supprimée.

## Vidéo redimensionnable

Sur `osd-support`, Example utilise une variante où le cœur crée le segment et en transmet le nom et la capacité avant les métadonnées de la trame. La capacité d’un emplacement est arrondie à une puissance de deux et les deux emplacements alternent selon la séquence de la trame. Le protocole fixe reste disponible pour les hôtes qui l’utilisent. Le mode redimensionnable est choisi au démarrage et n’alloue pas le grand segment fixe.

Validation Release Windows avec des emplacements successifs de 64, 256 et 64 Kio. La sonde a contrôlé les métadonnées binaires, les pixels et le rejet d’un segment d’une autre session. La sonde temporaire a été supprimée.

## Cycle du client hébergé

Le client commun `HostedCoreSession` suit l’ordre des champs de `ProcessCore.Host.cs` et de `WriteNativeCoreFrameResponse` sur `osd-support`. Il conserve l’image précédente et les diagnostics quand la trame n’en apporte pas de nouveaux. Il livre les blocs audio reçus à l’appelant et renvoie les erreurs distantes structurées. À la fermeture il demande la destruction du cœur, puis libère le transport et efface les sorties. Le SDK laisse au futur module le choix de la version, du format des arguments et des champs facultatifs.

La sonde Release Windows a ouvert un processus hôte, décodé l’identité et le statut, transmis les entrées, reçu la vidéo en mémoire partagée et l’audio, conservé les données en l’absence de mise à jour et vérifié la réception de Dispose. La sonde a été supprimée.

## Présentation Vulkan

La fabrique C# `osd-support` utilise le backend Vulkan de Veldrid. Dans le port Qt, le moteur Vulkan de QRhiWidget affiche la trame composée, avec les mêmes proportions, le même filtrage et le même OSD que la surface commune. Les traitements vidéo restent sur le processeur. Le sélecteur conserve Vulkan lors du chargement d’un profil, et une erreur d’initialisation affiche la surface logicielle. [Documentation de QRhiWidget](https://doc.qt.io/qt-6/qrhiwidget.html)

La sonde Release Windows a vérifié la présentation effective dans Vulkan, la couleur au centre du tampon et le retour au moteur logiciel. La sonde a été supprimée.

## Services de firmware du SDK

`Firmware.h` reprend les candidats et observations de `osd-support`. Le parcours plat ou récursif trie les chemins avant de les transmettre à l’inspecteur. Le parcours récursif exclut les points de réanalyse sur Windows et les liens symboliques sur les autres systèmes. L’empreinte CRC32 réutilise la primitive de MediaEngine. L’hôte délègue la reconnaissance, la compatibilité et l’affectation aux modules qui implémentent `IModuleFirmwareManager`.

La sonde Windows Release a vérifié le CRC32, le tri des deux parcours, l’exclusion d’une jonction, l’annulation après le premier fichier et les sélections par identité et par rôle. La sonde et ses fichiers temporaires ont été supprimés.

## Sélecteur de firmware

Les champs `FirmwareFile` ouvrent le sélecteur quand le module fournit un dossier de firmware. Le parcours s’exécute hors du fil graphique. La liste suit le filtrage du contrôleur C# `osd-support`, avec priorité au rôle demandé, un seul résultat par chemin et exclusion des candidats incompatibles. Une sélection reconnue passe par le gestionnaire du module lors de l’enregistrement. Une sélection inconnue ou vide passe par la validation habituelle du champ. Le clic sur Annuler écarte les choix de la boîte. Les libellés viennent des ressources C# dans les catalogues disponibles.

La compilation Release de la DLL du thème, du SDK et de tous les catalogues de langues a réussi sous Windows. La validation de l’interface avec un module de machine reste à faire.

## Gestion des moteurs d’émulation

Le gestionnaire facultatif reproduit le contrat `IEmulationEmulatorManager` de `osd-support`. Chaque module fournit les moteurs disponibles, le moteur de la configuration, les versions et l’installation. L’hôte conserve le choix dans le brouillon du module et vérifie que l’enregistrement ou une modification ultérieure respecte le verrou du moteur de la machine. La recherche et l’installation utilisent la configuration sélectionnée.

La sonde Release Windows a chargé un module temporaire à deux moteurs. Elle a vérifié un choix unique dans le brouillon, le rejet d’un identifiant inconnu, le verrou après sauvegarde, le rejet d’un ancien brouillon concurrent et le relais de la recherche et de la progression d’installation. La sonde et sa DLL ont été supprimées.

## Panneau de gestion des moteurs

L’onglet reprend le contrôleur et les libellés de `EmulationCore.resx` sur `osd-support`. Un changement de moteur met à jour le brouillon puis reconstruit les champs qui dépendent du moteur. Le moteur des machines sauvegardées et des sessions ouvertes est affiché et verrouillé. La recherche privilégie une version requise, puis la version installée. La recherche et l’installation s’exécutent hors du fil graphique. La fermeture annule l’opération en cours et attend la libération du module.

La DLL du thème et les catalogues de langues ont compilé en Release sous Windows. L’essai du panneau avec un fournisseur de versions réel reste à faire.

## Contrats et règles du stockage

Le contrat `IModuleStorageSettingsManager` suit la description et l’application du C# `osd-support`. Les périphériques disponibles viennent du module. Le SDK distingue les lecteurs configurés, leurs réglages et les médias associés, dont ceux conservés après éjection. Les contrôles d’extension sont insensibles à la casse et la restriction par catégorie conserve les autres médias et réglages. Le panneau de stockage et les règles de création propres aux formats de disque dur restent à raccorder.

La sonde Windows Release a vérifié les répétitions de lecteurs et de montages, les lecteurs inconnus et ambigus, la reconnaissance des extensions, la lecture seule obligatoire pour les cartouches et la distinction entre disquette et disque dur lors de l’éjection. Elle a été supprimée.

## Cycle de vie du stockage

Les réglages de lecteurs sont validés contre la liste fournie par le module et enregistrés avec les autres réglages du profil. Pour un module qui fournit ce contrat, le prochain démarrage repose sur la configuration sauvegardée et ne monte que les médias déclarés insérés. Une éjection garde le média associé et ne remplace pas un autre fichier configuré pour le démarrage suivant. Les modules utilisant le contrat historique de montage gardent leur parcours.

La sonde Release Windows a ouvert une machine de test temporaire, modifié son profil pendant l’exécution, éjecté son ancien média, contrôlé le montage suivant puis vérifié la conservation du support éjecté et l’insertion d’un autre. Les fichiers de la sonde ont été supprimés.

Le panneau de stockage du thème RetroMediaLab reprend l’inventaire des lecteurs permanents et ajoutables, les réglages fournis par le module et l’association d’images existantes. Le build Release du thème et des 30 catalogues est déployé. La création d’images vierges et le rendu avec une machine réelle restent à valider.


La création de disquettes vierges reprend les formats proposés par le modèle, le choix du dossier et les actions de création seule ou avec insertion. Le socle publie l’image seulement une fois l’écriture terminée et refuse d’écraser un fichier existant. Ce bloc est compilé et déployé en Release. L’interface reste à vérifier avec une machine réelle.

Les cartes mémoire sur fichier reprennent la taille fixe et l’initialisation facultative déclarées par le lecteur. La création et l’association utilisent le dossier mémorisé. Le contrôle Release confirme le contenu initial, la taille, le rejet des extensions non prises en charge, le refus d’écrasement et la suppression des fichiers partiels. Le dialogue attend une validation avec un lecteur fourni par un module.

Le panneau propose la création de disques durs à partir des profils du lecteur. Le socle vérifie les capacités, tailles de secteur et géométries, crée les images brutes et délègue les conteneurs Acorn et MFM aux écrivains de MediaEngine. Les profils peuvent fournir leur propre écrivain. Le contrôle Release couvre l’image brute initialisée, la géométrie et la publication Acorn .dat/.dsc sans écrasement. Le parcours graphique avec une machine et le reste des formats et préparations de disque dur du C# restent à valider ou à porter.

Le service de création utilise les écrivains existants pour VHD, VHDX, VDI, VMDK, QCOW2 et CHD. VHD, VHDX, VDI et VMDK exposent également leur variante fixe. La publication conserve les fichiers produits par chaque écrivain et expose le descripteur VMDK après son fichier de données. Le contrôle Release a rouvert les dix variantes et confirmé le rejet sans écrasement sur collision du fichier de données VMDK. Ces six conteneurs créent actuellement des images vierges. Les préparations de système de fichiers, les autres conteneurs C# et la validation graphique restent à traiter.

L’insertion d’images noires suit BlackFrameCadence.cs et NativeDisplayRefreshRateFunctions.cs. Le rafraîchissement Windows vient de la synchronisation verticale de l’écran. Une seule image noire peut suivre une trame, et uniquement si l’écran est plus rapide que l’émulation. Les surimpressions et les captures conservent la trame traitée. L’horloge s’arrête à la mise en pause, à la fermeture ou lorsque la surface est masquée. Le dialogue vidéo et l’OSD partagent le réglage, y compris la lecture de processing.temporal pour les anciens profils. Le libellé est repris des catalogues C# et corrigé en vietnamien, où la ressource disait Comment. Le contrôle vérifie la cadence et la persistance. Windows Release et les DLL déployées sont vérifiés. La validation sur écran à fréquence élevée et les horloges natives Linux et macOS restent ouvertes.

Le magasin de profils reprend la structure C# Emulation/VideoPresentation/<module UTF-8 en hexadécimal>/<GUID N>.json sous le répertoire de configuration de l’application C++. Les profils précédents restent lisibles dans le dossier VideoPresentation de premier niveau. Un enregistrement atomique écrit au nouvel emplacement. Les opérations de copie et de suppression sont disponibles dans la bibliothèque. La suppression retire aussi le profil hérité pour éviter sa réapparition. Le contrôle Release couvre le repli, la priorité du nouvel emplacement, les propriétés JSON inconnues, la copie et la suppression. Le raccordement à la gestion des configurations enregistrées dans les préférences reste ouvert.

L’onglet Configurations reprend les filtres par marque et machine, le tri par nom de machine et l’édition dans le dialogue existant. Il affiche le processeur, la mémoire déclarée et les lecteurs configurés. La suppression confirmée retire la configuration puis son profil vidéo. Elle est indisponible tant que la configuration est ouverte dans une session. La colonne Périphériques attend le contrat de configuration des entrées. Le build Release du thème et des 30 catalogues est vérifié. Le parcours avec une configuration de machine réelle reste à vérifier.

Le SDK reprend IEmulationInputSettingsManager et les contrats de clavier, souris, ports, associations, représentations et commandes de sortie de la branche osd-support. Le Host expose la description et les opérations de mise à jour indépendantes, avec contrôle de l’identité lors de l’application. La colonne Périphériques affiche les entrées effectivement configurées et omet les ports sans périphérique sélectionné. Clavier et Souris sont repris des catalogues C#. Windows Release et les DLL déployées sont vérifiés. La modification des associations dans les réglages de machine et la validation avec un module sont encore ouvertes.

Les réglages de machine présentent les associations clavier et souris fournies par le module. Les contrôles reprennent la recherche, la capture et le rétablissement des valeurs par défaut du dialogue C#. Le brouillon est enregistré par le gestionnaire du module, y compris depuis une session active. Compilation Windows Release et copie des bibliothèques vérifiées. Les éditeurs par port et l’essai fonctionnel avec un module restent à faire.

L’onglet Manettes reprend le choix des types et modèles et les associations propres à chaque port. Une catégorie est proposée lorsque toutes les options du port en déclarent une. Les modèles proviennent des compatibilités publiées par le module. Les associations restent dans le brouillon lors d’un changement de modèle. Les périphériques USB configurés sous Clavier ou Souris renvoient vers ces réglages. Les libellés proviennent des ressources C# et Joystick est fourni par Constants, comme dans la branche source. Build Windows Release et DLL de langues déployés. Les représentations interactives, les sorties dédiées à chaque modèle et la vérification avec un module restent ouvertes.

Les sorties du modèle actif utilisent les mêmes sélecteurs physiques que les anciens réglages de vibration. Les changements de modèle conservent les associations pendant l’édition. Le brouillon enregistré et les liaisons envoyées à la session retiennent les commandes déclarées par le modèle actif. Les anciens réglages restent utilisables pour les ports sans définition par modèle. Vérification du build et du déploiement Windows Release. L’essai avec une manette et une machine reste ouvert.

Les 201 profils de périphériques et leurs 2 262 zones proviennent de CreateProfiles dans osd-support (159e93a0577773bd456aed70a5044c8eed05fb53). Les 200 PNG référencés sont repris à l’identique et embarqués dans le thème RetroMediaLab. Les numéros de commandes visuelles ont été comparés à l’énumération C++. Les zones cliquables ouvrent la capture de l’action associée et suivent le placement et les secteurs directionnels du C#. Build et déploiement Windows Release vérifiés. La visualisation des commandes physiques en temps réel reste à raccorder.

Les représentations utilisent les associations courantes toutes les 33 ms, comme EmulationBindingVisualizationController dans osd-support. La pression analogique déplace le marqueur du joystick et les entrées clavier, souris et manette allument les zones associées. Le suivi s’arrête lorsque la représentation est masquée et relâche les touches mémorisées à la perte d’activation. La compilation Release et le déploiement de la bibliothèque sont vérifiés. L’essai visuel avec des périphériques physiques reste à faire.

Emulation.dll expose InputAccumulator et InputFrames pour les futurs modules. Ils reprennent la mise à jour des états absolus et la consommation unique des quatre mouvements relatifs de EmulationInputAccumulator et EmulationInputFrameStore. Les instantanés Qt utilisent le partage implicite et le stockage prend un verrou pendant les mises à jour, les lectures et la publication d’une frame. Un programme temporaire lié à la DLL Release a vérifié les deux bornes de saturation, la conservation des commandes et la stabilité entre deux Poll. Il a été supprimé après l’essai.

Le SDK reprend les identifiants clavier persistés, la table clavier native et les requêtes natives de EmulationInputProcessingService pour clavier, souris, manette, axes, pression et périphériques de pointage. Il expose aussi la résolution des associations par défaut, des zones mortes et des manettes par identifiant. Le test Release lié à Emulation.dll confirme ces chemins et le respect des périphériques déconnectés. Les fichiers temporaires ont été supprimés. La projection des événements Qt vers les identifiants clavier de la branche source reste ouverte.

Le clavier de la session publie les identifiants EmulationKey de osd-support. Les raccourcis Qt gardent leurs propres codes clavier. Le relâchement se rattache au code de la touche physique, et la perte de focus remet les entrées à zéro. Sous Windows, le caractère de chaque touche maintenue est recalculé avec la disposition active et ses modificateurs, comme dans EmulationKeyMapper.HostCharacters. Sur les autres systèmes, le texte est fourni par les événements Qt et les modificateurs latéraux dépendent du code remonté par Qt. Build Windows Release et DLL déployées. L’essai avec un cœur et plusieurs dispositions clavier reste ouvert.

Les PNG des manettes sont chargés depuis Assets/Controllers. La base SDL et sa licence sont chargées depuis Assets/Input. Les copies distribuées avec Debug et Release ont été comparées aux sources. Le SDK expose la consultation d’un port par indice technique et de ses commandes, selon GetControllerPort et GetControllerBindingDefinitions dans EmulationInputProcessingService. Les DLL Debug et Release ont été compilées et déployées.

Les fonctions de clavier natif du SDK reprennent ExternalCoreKeyboardFunctions.cs et ses constantes. Les touches ordinaires sont relâchées avant les modificateurs. Lors d’une pression, les modificateurs sont émis avant les autres touches. Le test Release couvre la casse, les caractères décalés, le masque de modificateurs et les limites des axes. Le raccordement de ces fonctions aux futurs adaptateurs reste à vérifier.

Le sélecteur du thème RetroMediaLab reprend EmulationSectionLayoutFunctions.cs, EmulationConfigurationSelector.cs et EmulationConfigurationSelection.cs. Le cadre repliable affiche les marques et machines qui ont une configuration enregistrée et une image fournie. Le bouton Ouvrir apparaît pour la sélection, et le double-clic sur une machine l’ouvre. Le libellé du cadre est repris des RESX. Debug et Release sont compilés et installés. L’onglet Release a été contrôlé sans module par l’API d’accessibilité Windows. Le parcours avec une machine fournie par un module reste à vérifier.

Le menu Émulation liste les modules installés après Préférences, par nom localisé, comme MainMenu.SetEmulationModules sur osd-support. Le dialogue de module reprend la liste des machines à gauche et ouvre l’éditeur existant à droite. Une machine sans configuration utilise le brouillon produit par le module et le bouton Créer. Le build et le déploiement sont vérifiés en Debug et Release. La conservation des brouillons lors d’un changement de machine et l’essai avec un module restent à vérifier.

Le portage du brouillon reprend le stockage en mémoire par identifiant de module et de machine de EmulationConfigurationDraftStore. À la fermeture ou au changement de machine, les valeurs valides et les associations d’une nouvelle machine sont conservées. Créer enregistre la configuration et retire le brouillon. Les changements valides d’une configuration existante sont enregistrés à la fermeture ou au changement de machine. Le contrôle Release charge un module temporaire par Host, vérifie qu’une valeur modifiée et le GUID restent dans le brouillon, puis confirme l’enregistrement explicite et le retrait du brouillon. Le module et la sonde ont été supprimés. L’enregistrement immédiat dans le dialogue et le test du parcours graphique avec un module restent ouverts.

Les options de module enregistrent les modifications valides d’une configuration existante pendant l’édition. Les sélecteurs et cases à cocher déclenchent l’écriture, les champs de texte sur une ligne attendent la validation, les champs multilignes attendent une courte pause et un curseur standard attend la fin du glissement. Les configurations nouvelles restent des brouillons jusqu’au bouton Créer. Une sonde Qt hors écran a ouvert le menu Émulation avec un module temporaire, changé une case et lu la nouvelle valeur dans Host avant la fermeture du dialogue. Les fichiers de la sonde et du module ont été supprimés.

Le SDK reprend GetPeripherals et DescribeInput de EmulationInputProcessingService. Les ports USB sont triés par numéro avant l’accès par indice sur le bus. Les ports classiques utilisent le numéro public moins un. L’adaptateur fournit la conversion des identifiants dans sa propre énumération pour remplacer la réflexion .NET. Le contrôle Release vérifie les deux bus, les ports absents, la déduplication, le filtrage des choix non pris en charge et la restitution des définitions. La DLL déployée a été comparée à celle du build.

La préparation des volumes AmigaDOS reprend AmigaDosVolumeFormatter.cs sur osd-support. Le moteur écrit les secteurs DOS, la racine et les bitmaps, avec extensions et cache de répertoire selon la variante. Il réutilise les écrivains de conteneurs et publie les fichiers sans écrasement. La sonde Release a rouvert six variantes en RAW, VHD et QCOW2, vérifié le comptage sur des volumes de 128 Mio et confirmé le refus d’écrasement. Les images restent des volumes entiers. La création de partitions RDB relève du portage de RdbPartitionWriter.

Les préparations amiga-rdb-ofs et amiga-rdb-ffs reprennent RdbPartitionWriter.cs et AmigaRdbPartitionWriter.cs sur osd-support. DH0 débute au cylindre 1, à 16 Kio du début de l’image. MediaFileSystems écrit et valide le plan de partitionnement RDB. MediaEngine prépare le volume AmigaDOS et publie le conteneur. La sonde Release a rouvert des images HDF et QCOW2, vérifié les sommes de contrôle RDSK et PART, les noms, le type DOS, les bornes et le bloc racine. La sonde a vérifié aussi l’ordre du chaînage avec deux partitions et l’absence de fichier après une géométrie refusée. Les DLL déployées ont les mêmes empreintes que les DLL compilées. La détection des partitions RDB dans l’Explorateur reste à traiter.

L’Explorateur détecte le RDB dans les conteneurs et reconnaît automatiquement les HDF qui contiennent un en-tête RDSK valide. Les entrées PART sont contrôlées avant de transmettre la plage de secteurs au lecteur AmigaDOS. Le contrôle Release a ouvert un HDF OFS avec un en-tête déplacé au secteur 2 et un QCOW2 FFS. Les deux volumes DH0 ont la capacité et l’espace libre attendus. Un pointeur PART cyclique a été signalé et la partition valide a été conservée. Les fichiers temporaires ont été supprimés et les deux DLL déployées ont été vérifiées. [Lecteur de partitions Amiga du noyau Linux](https://github.com/torvalds/linux/blob/master/block/partitions/amiga.c)

Les préparations atari-ahdi-fat16 et atari-ide-fat16 reprennent AtariAhdiPartitionWriter, AtariIdePartitionWriter et AtariFat16VolumeFormatter sur osd-support. La partition commence au secteur physique 1. Le préfixe initialise la table GEM ou BGM, le secteur d’amorçage, les deux FAT et le répertoire racine. Le mode IDE inverse les octets de chaque mot de 16 bits. La sonde Release a vérifié des images de 16 et 64 Mio, les secteurs BPB de 512 et 1024 octets, les deux FAT, la variante IDE et les limites de capacité. Les DLL déployées correspondent aux produits de la compilation et la sonde a été supprimée. La détection de ces partitions et la lecture de leur catalogue restent à raccorder à l’Explorateur.

Les paquets Windows Debug et Release contiennent le thème avec le sélecteur à vignettes et les 495 PNG présents sous src/App/Assets, aux mêmes chemins relatifs dans Assets. Les DLL du thème correspondent aux produits des deux compilations. Les vignettes proviennent des modules qui disposent d’une configuration enregistrée, selon EmulationConfigurationSelector.cs et EmulationSectionConfigurationFunctions.cs.

L’Explorateur reconnaît les tables AHDI et IDE à mots inversés et traite les entrées logiques XGM. Pour GEM et BGM, il réutilise le lecteur de répertoire FAT avec des entrées de chaîne de 16 bits. Le contrôle Release a ouvert trois images RAW sans format imposé, vérifié les partitions GEM et BGM, la capacité libre et le contenu d’un fichier réparti sur deux clusters au-delà de la plage FAT12. Les DLL livrées ont les mêmes empreintes que celles du build. Les images temporaires de contrôle ont été supprimées. La lecture FAT conserve le plafond de mémoire du lecteur partagé. [Structure de partition Atari dans le noyau Linux](https://github.com/torvalds/linux/blob/master/block/partitions/atari.c)

Le SDK Emulation reprend les contrats et la comparaison de EmulationNetworkService.cs. Le chargement, l’enregistrement et la suppression des configurations publient les interfaces déclarées. Une interface active remplace sa description hors ligne jusqu’à la destruction de son enregistrement. Le registre demande l’accord des deux règles natives après comparaison du réseau, de l’interface, du format et du transport. La sonde Release a vérifié les transitions de présence, les incompatibilités, le refus d’une inscription dupliquée, un port invalide, la notification et la libération en fin de durée de vie. Les libellés réseau sont repris de EmulationWelcome.resx, avec 72 invariants communs et 49 entrées par langue. Les quatre diagnostics du portage ont été générés avec Argos et relus dans les 29 langues. Emulation.dll et les catalogues ont été déployés en Release. L’affichage Réseau des préférences reste à intégrer.

L’onglet Réseau des préférences d’Émulation reprend le tableau en lecture seule de EmulationNetworkSection.cs et les valeurs de EmulationNetworkRow.cs. L’état de la machine est présenté par une case inactive. Les colonnes séparent protocoles, format, transport, coordonnées hôtes, adresses natives et relais. Le tableau s’abonne au registre pendant son affichage et redemande un instantané à son retour. La sonde Qt hors écran a vérifié les douze colonnes, le rendu, le passage du français à l’anglais, les notifications depuis un autre thread, le masquage et le retour à l’état enregistré. Elle réutilise l’objet compilé du thème et les bibliothèques Release. La DLL du thème a été déployée et la sonde supprimée.

L’onglet Général d’Émulation reprend les trois chemins de EmulationPreferencesSectionLayoutFunctions.cs. Les dossiers de captures et d’états suivent le stockage commun quand ils sont laissés vides. Le dossier commun par défaut vient de SettingsMigrator.cs, sous Documents/Retro Media Lab/Emulation. Les nouvelles sessions reçoivent les chemins Sessions, States et Converted sous le répertoire de leur module, selon EmulationMachineRuntimeService.cs. Le SDK est passé à 2.2 pour exposer ces chemins et celui de l’exécutable. Une session ouverte conserve son chemin de sauvegarde rapide jusqu’à sa fermeture. La sonde Release a relu une ancienne préférence de capture, vérifié l’écriture de preferences.json, sauvegardé un PNG et exercé les chemins d’une machine chargée dans une DLL temporaire. La DLL temporaire, les images et les états ont été supprimés. Les quatre libellés viennent des RESX et la compilation Windows Release a été déployée.
Après le signalement de l’ancien écran à listes déroulantes, la DLL Debug datée du 24 novembre a été recompilée et installée. Les empreintes des DLL compilée et installée sont identiques. Les 495 PNG de src/App/Assets sont présents dans chacun des paquets Debug et Release aux mêmes chemins relatifs.

### Enregistrement des raccourcis d’Émulation

EmulationPreferencesSection et InputBindingRows sur osd-support enregistrent les associations valides pendant l’édition. Le dialogue Qt applique cette règle et reprend les actions Restaurer et Effacer les conflits à côté de la recherche. Effacer les conflits vide toutes les lignes en conflit ou réservées. Les trente catalogues reprennent le libellé des RESX C#. La vérification Qt Release a saisi un raccourci au clavier, constaté sa publication avant fermeture, vérifié le rejet d’une saisie en double, puis confirmé l’effacement et la publication des deux lignes. La sonde temporaire a été supprimée.

### Ordre des onglets de machines

Le thème reprend EmulationSectionTabDragFunctions.cs de osd-support. Le glissement affiche un aperçu et un repère d’insertion dans la barre. Seuls les onglets de machines peuvent servir de source et de destination. Au dépôt, l’onglet déplacé est sélectionné et Accueil reste en première position. La sonde Qt Release a vérifié les deux sens de déplacement, la sélection et le refus de déplacer Accueil ou de déposer une machine sur Accueil. La DLL du thème a été déployée en Release et la sonde supprimée.

### Cycle de vie dans la barre de session

Les quatre commandes reprennent MachineCommandBar.cs et MachineView.cs de osd-support. L’alimentation et la pause changent de rôle avec l’état de la machine. Les raccourcis utilisent les mêmes boutons. Les cinq glyphes ont été comparés aux RESX et les couleurs aux valeurs C#. Une icône de style Qt sert de repli si la police Segoe MDL2 Assets n’est pas disponible. La DLL du thème a été compilée, déployée et vérifiée par empreinte sous Windows Release. L’essai des boutons avec une machine chargée reste à faire, ainsi que le portage des autres groupes de la barre.

### Sauvegarde rapide et affichage dans la barre de session

MachineCommandBar.cs de osd-support place la sauvegarde et le chargement rapides dans un groupe, puis la capture et le plein écran dans le groupe suivant. Le thème C++ reprend cet ordre et place l’engrenage des paramètres à droite. La capture devient disponible quand la machine est alimentée. Reprendre utilise le vert du C#. Les commandes de sauvegarde et de chargement sur fichier et les réglages vidéo existants restent accessibles. Les cinq glyphes correspondent aux RESX. Le thème Windows Release est compilé et installé. L’essai avec une machine chargée reste ouvert.

### Audio et indicateurs d’entrée en session

Le groupe de droite reprend MachineCommandBar.cs de osd-support. Le bouton audio ne s’active que quand la machine est alimentée. Il partage l’état muet avec l’OSD et le raccourci clavier. La manette passe au vert quand la lecture du périphérique fournit un état. La souris passe au vert quand elle est capturée. Leurs états accessibles reprennent EmulationValue.resx. Les quatre glyphes correspondent aux RESX et le repli Qt conserve les couleurs. Le thème et les trente catalogues Windows Release sont compilés et installés avec des empreintes identiques aux produits du build. La bascule optionnelle manette/souris et l’essai avec une machine et des périphériques restent ouverts.

### Bascule manette et souris

La commande de MachineCommandBar.cs et le contrat IEmulationInput.cs de osd-support sont repris par la capacité facultative IMachineControllerPointerSwitch. Les interfaces existantes gardent leur disposition binaire. La session vérifie l’alimentation et l’annulation avant d’appeler la machine. Une valeur false indique le mode manette. L’App présente la commande uniquement si la capacité est déclarée, et récupère l’état de la machine pour choisir le glyphe. Les trente catalogues reprennent EmulationController.resx. Une DLL temporaire a vérifié la capacité facultative, les deux modes, le refus hors tension et l’annulation dans Emulation.dll. La sonde et sa DLL ont été supprimées. Le thème et les DLL Windows Release sont compilés et installés. La vérification des clics dans la vue et avec des périphériques physiques reste ouverte.

### Vérification de la barre avec une machine chargée

La sonde Qt a chargé la DLL du thème installée et deux sessions d’un module temporaire. Elle a cliqué sur Alimentation, Pause, Reprendre, Audio, Sauvegarde rapide, Chargement rapide et la bascule manette/souris. Le premier essai a montré que la sauvegarde rapide attendait le prochain rafraîchissement périodique après l’allumage. Les actions de session actualisent désormais cette disponibilité immédiatement. Le nouvel essai a confirmé les états de la barre, la création et le chargement d’un état, le changement de l’icône audio et de l’icône de mode, et le masquage de la bascule sur la seconde session qui ne déclare pas la capacité. Le thème Release a été recompilé et installé. La sonde et ses fichiers temporaires ont été supprimés. La vérification avec des périphériques physiques reste ouverte.

### Raccourcis dans les infobulles de session

La barre reprend la correspondance de EmulationShortcutMap.cs. Une commande associée affiche le libellé localisé et le raccourci effectif sur une seconde ligne. Une association explicitement vide affiche le libellé seul. Les sessions ouvertes mettent à jour leurs infobulles lors des changements de raccourcis et de langue. Une sonde Qt sur le thème Windows Release installé a vérifié le raccourci par défaut, son remplacement au clavier et à la manette, son effacement et le retour au défaut. La sonde a également revérifié les clics de la barre avec le module temporaire. Tous les fichiers de la sonde ont été supprimés.

### Cadence et vitesse d’émulation

Le calcul reprend EmulationExecutionRateFunctions.cs et la fenêtre d’une seconde de MachineVideoPresenter.cs. Le C++ compte les nouvelles images reçues par la vue au rythme de son interrogation de la session. Les doublons sont ignorés. Une rupture du temps émulé ou de la séquence, une pause ou le remplacement de la machine redémarrent la mesure. Les statistiques affichent la résolution, la fréquence nominale et les FPS mesurés. Si le module fournit son horloge de référence via IMachineProcessorClock, elles ajoutent les MHz effectifs et le pourcentage de vitesse. Le format de Status.resx est invariant et reste dans Constants. La sonde Release a vérifié le calcul de 16 MHz pour une horloge de 8 MHz à vitesse double, le cas sans horloge, les doublons et les remises à zéro. Le contrôle a été supprimé et les DLL compilées ont été comparées aux DLL installées. Le rendu des statistiques en cours d’émulation reste à vérifier.

### Cadence et redémarrage logiciel communs

MachineExecution porte le minuteur et le redémarrage logiciel de EmulationFrameTimer.cs et EmulationMachineExecutionService.cs. Le module possède son minuteur. Windows utilise le minuteur haute résolution avec repli sur le minuteur standard. L’annulation réveille l’attente et les ressources natives sont libérées par leur propriétaire. Le chemin portable utilise une attente C++ annulable. La cadence reste absolue jusqu’à un retard d’une frame entière, puis repart de l’heure courante. Le redémarrage logiciel appelle la commande native ou maintient les touches configurées pendant une frame et les relâche, y compris après exception. La sonde Windows Release a vérifié les bornes, l’échéance, l’annulation, la réutilisation, le recalage et le relâchement. La DLL est installée et la sonde supprimée. Les boucles de commandes du C# et la validation des autres systèmes restent ouvertes.

### File et boucles d’exécution des machines

MachineExecution porte EmulationMachineCommand.cs et les boucles de EmulationFrameTimer.cs. La file partage le verrou de la machine et exécute les commandes dans l’ordre sur le fil de la machine. Une commande réveille la boucle pendant une pause, et l’annulation de l’attente d’un appelant conserve la commande déjà déposée. La boucle libre rend l’erreur à l’appelant et continue. La boucle cadencée propage aussi l’erreur au module. Le module termine les commandes en attente dans sa sortie de session, et la destruction de la file complète celles qui restent. La sonde Release a vérifié le fil propriétaire, l’ordre, la pause, les deux politiques d’erreur, la validation sous verrou et l’annulation. La DLL est déployée et la sonde supprimée.

La publication des sorties reprend EmulationFrameOutputFunctions.cs de osd-support dans MachineExecution.h. Le code de la machine conserve la dernière séquence publiée, reçoit les images et les blocs audio par référence et traite la vidéo avant l’audio. La sonde MSVC a confirmé l’ordre des blocs, la déduplication et la propagation d’une exception vidéo. Elle a été supprimée après exécution. Le service est un modèle C++ dans l’en-tête du SDK.

MachineExecution reprend les règles de ExternalCoreLifecycleFunctions.cs de osd-support pour l’exécution et la libération des cœurs externes. Le nettoyage reçoit l’état de l’initialisation. La libération enchaîne le contenu chargé, le cœur initialisé, les callbacks et la bibliothèque, même après une exception. Le contrat C++ est synchrone et l’appel de fermeture attend le thread avant de libérer les ressources. La sonde MSVC a vérifié l’ordre, la propagation de la dernière erreur de nettoyage et le signalement des échecs. Emulation.dll a été déployée en Windows Release et la sonde a été supprimée.

La sauvegarde et la restauration de MachineExecution reprennent le transfert natif borné de ExternalCoreLifecycleFunctions.cs sur osd-support. La sauvegarde retourne les octets dans un QByteArray propriétaire. La restauration passe une copie temporaire au cœur. Un état vide, hors limite ou non pris en charge est rejeté avant le transfert. Une taille supérieure à la limite de représentation du tableau C# utilise le diagnostic de sauvegarde indisponible. La sonde MSVC a confirmé l’intégrité des octets, les bornes et les erreurs remontées par le cœur. Emulation.dll a été déployée en Windows Release et les fichiers de la sonde ont été supprimés.

Les états des cœurs hébergés reprennent EmulationMediaRules.cs de osd-support. Les voyants 0 à 2 désignent les premiers emplacements disquette, cassette et cartouche. Le voyant 3 désigne le premier lecteur optique pour les cœurs qui le déclarent. Un voyant absent vaut éteint. Les identifiants d’emplacement restent définis par le module. La sonde MSVC Release a vérifié ces cas et a été supprimée après déploiement de Emulation.dll.

Le débandeur et la récupération des détails du mode Normal reprennent VideoImageRestorationShader.cs de osd-support. OpenGL, Vulkan et Direct3D 11 partagent les fonctions et transmettent les mêmes réglages à la présentation et aux captures. Le débandeur et la récupération opèrent avant les ajustements de couleur. La récupération des détails exige un redimensionnement avec un filtre autre que le plus proche voisin. Le traitement logiciel couvre les combinaisons qui exigent d’autres effets, dont le détramé. La sonde Windows Release à 150 % a confirmé des changements de pixels indépendants pour les deux réglages dans chacun des trois moteurs actifs et dans le moteur logiciel. Vulkan a été validé dans une fenêtre neuve. Lors du changement OpenGL vers Vulkan dans la même fenêtre, le premier essai a utilisé le repli logiciel. La DLL Release a été déployée, son empreinte vérifiée et la sonde supprimée.

La fabrique EmulationVideoSurfaceFactory.cs de osd-support crée une surface indépendante par moteur. Qt fixe le moteur RHI pour la durée de vie de la fenêtre de composition. La surface Vulkan dispose donc de sa propre chaîne de composition dans une fenêtre native enfant de la vue vidéo. Elle est masquée puis réutilisée pendant les changements de moteur et libérée à la fermeture de la vue. La sonde Windows Release à 150 % a confirmé OpenGL vers Vulkan, la coexistence des deux moteurs sous la même fenêtre principale, Vulkan vers OpenGL après redimensionnement et un second passage à Vulkan. Les captures ont conservé leurs couleurs et leur orientation. La DLL Release a été déployée et la sonde supprimée. [Documentation Qt](https://doc.qt.io/qt-6/qrhiwidget.html#api-prop)

Le menu plein écran reprend la rubrique Contrôles de MachineController.OsdControls.cs. Les commandes des manettes sont groupées par port. Les commandes clavier et souris affichent la valeur enregistrée, la valeur par défaut ou le libellé Non assigné. Les onglets masqués par le module sont écartés. Les rubriques de réglages de ces trois onglets sont reprises sous Contrôles. Le nom des actions et le type de manette proviennent des ressources du module ou de sa valeur invariante. La présentation détaillée des composants de raccourci de InputBindingRow.cs et un essai dans une session chargée restent ouverts. Les deux clés sont reprises des RESX dans les 30 catalogues. Le thème et les catalogues ont compilé et leurs empreintes de déploiement Release ont été vérifiées.

Le dialogue des affectations et le menu plein écran partagent la présentation de InputBindingRow.cs de osd-support. Les touches de modification, boutons de souris, molettes, directions des manettes et raccourcis combinés reprennent les libellés C#. Les identifiants inconnus restent lisibles et l’ordre des composants saisis est conservé. Les trois invariants clavier sont dans Constants. Les traductions localisées et les replis pour Shift suivent les RESX. La sonde a exercé les modificateurs latéralisés, les boutons et directions XInput et GameInput, les raccourcis composés et les valeurs inconnues. App.dll, le thème et les catalogues ont été déployés en Windows Release, avec vérification des empreintes. La sonde a été supprimée.

Le menu plein écran reprend les rubriques de CreateOsdEntries dans MachineController.cs de osd-support. Les commandes de montage et de transport restent dans la barre des supports. Le sous-titre utilise le nom du média inséré sans extension pendant que la machine est alimentée, avec priorité à la cartouche ou au disque optique. Les réglages masqués par le module restent masqués dans le menu. La navigation haut et bas parcourt les commandes activées et ignore les séparateurs. La navigation entre les curseurs réglables reste réservée au mode édition. La sonde Qt hors écran a validé la navigation à trois niveaux et la mise à jour du sous-titre. La sélection du média dans une session chargée reste à vérifier. VideoPresentation.dll et le thème ont été compilés et installés en Windows Release.

Le menu utilise une fenêtre transparente possédée par la fenêtre plein écran, comme FullscreenOsdWindow.cs de osd-support. Elle suit les déplacements et dimensions du propriétaire. Le fond à alpha minimal permet de cliquer hors des panneaux pour fermer le menu. Les raccourcis sont transférés à la surimpression pendant son affichage puis restitués à la vue vidéo. La sonde Windows Release, avec Vulkan actif, a vérifié le propriétaire natif, la réception des clics dans les panneaux et la zone transparente, le déplacement, le redimensionnement et l’activation du même raccourci avant et après fermeture. Le thème et VideoPresentation.dll ont été déployés et la sonde supprimée. La capture physique de la souris au retour du menu reste à valider sur une machine chargée.

Le pied du menu reprend CreateOsdFooterHints et CreateFooter de la branche osd-support. Il groupe les raccourcis de déplacement puis ceux de validation et de retour. Les raccourcis Qt actifs et les liaisons physiques valides utilisent les mêmes données que les commandes de session. Les variantes visibles sont dédupliquées et l’affichage est actualisé lorsque les préférences changent. Les touches sont présentées avec un fond contrasté dans un libellé Qt repliable. La sonde Qt a vérifié le rendu des groupes, l’échappement des caractères de raccourci, le remplacement et le masquage des groupes vides. Le thème et VideoPresentation.dll ont été installés en Release et la sonde supprimée.

## Précompilation des filtres Direct3D 11

Le build Windows produit le bytecode DXBC des filtres d’échantillonnage et conserve le SPIR-V dans les mêmes ressources. Direct3D 11 utilise le DXBC et ses liaisons en priorité. Les builds configurés sans fxc conservent le chemin HLSL. L’indicateur de préparation visible dans MachineController.cs sur osd-support reste à porter pour les opérations qui préparent encore des shaders à l’exécution.

La compilation Windows Release et l’empreinte de la DLL installée ont été vérifiées. Une sonde a affiché une image dans Direct3D 11, exercé le bicubique et les deux filtres de restauration, puis contrôlé que la capture différait du rendu logiciel. La sonde temporaire a été supprimée.

L’indicateur utilise la clé `Emulation.Video.Shader.Loading` reprise de `EmulationVideo.resx` pour Constants et les 29 langues. Dans le C#, MachineVideoPresenter publie l’état lors de la mise en file GPU et protège la remise à zéro par la génération de présentation. MachineController le relaie à la vue en fenêtre, au menu plein écran, au plein écran lorsque le menu est fermé et aux réglages de la configuration active.

Le moteur C++ publie les transitions à partir des initialisations OpenGL, Vulkan et Direct3D 11, puis annule l’état à la fermeture, lors du changement de moteur ou à l’issue de l’initialisation. La sonde Windows Release a relevé les transitions inactif, actif, inactif pour chacun des trois moteurs. Le rendu logiciel est resté inactif. La sonde a été supprimée.

Le thème présente l’indicateur centré au-dessus de la vidéo, avec la variante bleu foncé en plein écran. Lorsque le menu plein écran est ouvert, il devient propriétaire de l’indicateur pour conserver l’ordre des fenêtres natives. Les réglages vidéo et les réglages du module relaient l’état de la session active en mode fenêtré. Les DLL Release du thème, de VideoPresentation et des 30 catalogues sont compilées et déployées. La géométrie et la visibilité de l’indicateur restent à vérifier avec une session dans l’application installée.

Une sonde a chargé le thème Release avec un module temporaire isolé. Lors des changements de moteur vers Direct3D 11, le panneau a reçu les événements d’affichage et de masquage et sa géométrie était centrée en fenêtre, en plein écran et par-dessus le menu plein écran. La rangée de préparation des réglages vidéo a également reçu son événement d’affichage. La préparation Direct3D 11 précompilée s’est terminée sans événement de peinture du panneau dans cet essai. Le rendu du panneau pendant une préparation qui bloque le fil graphique reste à établir. La tentative de repeindre immédiatement le panneau a été retirée après un résultat identique. La sonde, son module et les fichiers associés ont été supprimés.

Direct3D 11 prépare les deux programmes de présentation et d’échantillonnage sur un fil de travail possédé par l’application. Les vues partagent le résultat de la préparation et ne bloquent pas en attendant sa fin. La création du périphérique et la première présentation restent sur le fil graphique. Changer de moteur détache la vue de la préparation en cours. La sonde Windows Release a contrôlé les notifications, le traitement d’un événement Qt pendant la préparation, le passage au moteur logiciel suivi d’un retour à Direct3D 11 et une capture bicubique de 432 × 216. La DLL installée a été vérifiée par empreinte. La sonde a été supprimée.

OpenGL lie son programme dans un contexte partagé quand le pilote autorise OpenGL hors du fil graphique. Qt crée et détruit la surface hors écran sur le fil graphique. Le travailleur restitue le programme lié et la vue crée ses propres objets de sommets dans son contexte actif. Si la plateforme refuse le contexte partagé, OpenGL conserve la préparation sur le fil graphique. Une génération écarte les résultats d’un contexte devenu obsolète. La sonde Windows Release a vérifié le changement de moteur pendant la préparation, la reprise du rendu et la capture de chacun des deux filtres de restauration. Qt a traité ses événements pendant la préparation de 9,3 secondes observée. La DLL installée a été vérifiée par empreinte.

- Surface hors écran et format du contexte partagé. [QOffscreenSurface](https://doc.qt.io/qt-6/qoffscreensurface.html)
- Prise en charge des fils par le pilote et partage entre contextes. [QOpenGLContext](https://doc.qt.io/qt-6/qopenglcontext.html)
- Cache des binaires et liaison des sources OpenGL. [QOpenGLShaderProgram](https://doc.qt.io/qt-6/qopenglshaderprogram.html)
- Les ressources QRhi restent liées au fil de leur instance. Le cache sérialisé est l’interface dont dispose une autre instance QRhi pour réutiliser une préparation Vulkan. [QRhi](https://doc.qt.io/qt-6/qrhi.html)

La sonde du thème Release a vérifié le centrage, les rafraîchissements et la disparition du panneau pendant la préparation OpenGL en fenêtre. Elle a relevé 518 événements de peinture, puis la reprise du rendu natif. Les fichiers temporaires ont été supprimés.

Vulkan conserve un pipeline pour les nouvelles cibles compatibles du même périphérique QRhi. Le descripteur de passe reste disponible quand Qt remplace la cible. Le test Windows Release a vérifié trois changements de taille avec des captures Vulkan de tailles différentes, sept soumissions d’images et une seule préparation lors du démarrage. Le pilote a mis environ 2,46 secondes à préparer le premier pipeline dans cet essai. Le premier chargement reste synchrone et ouvert dans les tâches. La DLL installée et la suppression de la sonde ont été vérifiées.

- Compatibilité des passes et durée de vie du descripteur. [QRhiRenderPassDescriptor](https://doc.qt.io/qt-6/qrhirenderpassdescriptor.html)

Une sonde Windows a compilé le pipeline d’échantillonnage réel sur une instance Vulkan hors écran. La création du pipeline et l’extraction du cache Qt ont pris 3 237 ms à froid. Une seconde instance utilisant le cache de 161 462 octets a réalisé cette même séquence en 14 ms. La mesure utilise un format de cible et des liaisons identiques dans les deux instances. Elle valide le passage du cache généré entre instances, sans modifier le chemin de rendu de l’application. Les en-têtes et les fichiers de la sonde ont été supprimés.

- Version des en-têtes publics utilisée dans la sonde. [Khronos Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers/tree/vulkan-sdk-1.4.321.0)
- Le cache persistant peut passer entre instances compatibles. Les instances QRhi et leurs ressources restent chacune sur leur propre fil. [QRhi](https://doc.qt.io/qt-6/qrhi.html)

Sur une vue d’émulation ouverte, le passage du moteur logiciel à Vulkan bloquait dans l’initialisation de la fenêtre de composition, avec une demande de surface Vulkan rejetée par Qt Windows. Le port crée d’abord la fenêtre hôte native. La fenêtre de composition Vulkan dispose de sa propre hiérarchie QWidget et rejoint la vue au niveau QWindow. La vue en conserve la propriété et la détruit avec la session. La sonde Windows Release a validé le passage depuis le moteur logiciel et une capture produite par le moteur Vulkan. Le premier chargement a pris environ 3,3 secondes et le panneau n’a pas été peint pendant cet essai synchrone. La préparation asynchrone reste à vérifier. La DLL Release a été déployée et les sondes temporaires ont été supprimées.

Le cache de préparation est maintenant produit sur un périphérique Vulkan hors écran avec la version d’API de la fenêtre de composition. Un cache produit avec la version d’API par défaut de QVulkanInstance était accepté par Qt mais la création du pipeline de présentation prenait encore 2,5 à 3 secondes sur le fil graphique. La version de la fenêtre est obtenue par QRhi et réutilisée dans l’instance de travail. [Création de l’instance Vulkan Qt](https://github.com/qt/qtbase/blob/v6.11.0/src/gui/vulkan/qvulkandefaultinstance.cpp) [Chargement du cache QRhi dans Qt](https://github.com/qt/qtbase/blob/v6.11.0/src/gui/rhi/qrhivulkan.cpp)

Dans la session Release de vérification, le basculement a pris 3 116 ms au total. Le minuteur graphique a effectué 154 passages pendant la préparation, avec un intervalle maximal de 402 ms. Le panneau a reçu 152 événements de peinture et la capture matérielle a réussi. Les DLL et licences ont été déployées en Debug et Release. La sonde a été supprimée.

## Diagnostics de démarrage des cœurs

CoreDiagnostics reprend EmulationCoreDiagnosticMonitor.cs et WithNativeDiagnostics d’EmulationErrorService.cs de osd-support. Le callback compare les signatures avec respect de la casse et conserve le premier message correspondant, même lorsque plusieurs callbacks publient en parallèle. L’appel explicite après retour du cœur construit l’exception fournie par le module. L’enveloppe joint les diagnostics dans leur ordre et garde le pointeur d’exception d’origine. Elle retourne la cause initiale quand le bloc figure déjà dans le message. Le contrôle Windows Release a couvert les publications concurrentes, le déclenchement différé, l’UTF-8, le dédoublonnage et l’identité de la cause. Emulation.dll a été déployée et la sonde supprimée.

## Chaînes et contenu prêtés aux cœurs

NativeCoreContent reprend ExternalCoreUtf8String.cs, ExternalCoreLoadedContent.cs et la disposition de GameInfo dans ExternalCoreApi.cs sur osd-support. Les chaînes UTF-8 terminées par zéro ont une capacité minimale initialisée à zéro. Le choix de l’encodage local Windows reste disponible pour les cœurs qui le demandent. L’ajout par clé conserve une adresse stable et laisse l’entrée existante intacte en cas de doublon. Le descripteur et les octets restent possédés par la session. Le mode par chemin ne touche pas au fichier. La sonde MSVC Release a vérifié les octets binaires et les chemins accentués, la stabilité des données après fermeture du fichier, l’insertion par clé et les erreurs. DLL déployée et sonde supprimée.

## Chargement des bibliothèques de cœur

NativeCoreLibrary reprend ExternalCoreLibrary.cs, ExternalCoreExportNames.cs et les signatures C de ExternalCoreApi.cs. L’inspection vérifie le chemin et les exports, contrôle la version du protocole et le nom attendu, puis copie les chaînes et les extensions avant de décharger le cœur. Les codes d’échec restent accessibles aux adaptateurs pour la contextualisation. La durée de vie des exports empruntés est celle de leur bibliothèque QLibrary. [Documentation Qt](https://doc.qt.io/qt-6/qlibrary.html)

La sonde Windows Release a chargé une DLL temporaire. Elle a vérifié les métadonnées copiées, les appels avec les descripteurs du SDK, les erreurs de chemin, de version, d’identité et d’export, puis le renommage de la DLL déchargée. Emulation.dll a été déployée et la sonde a été supprimée.

## Réception audio et vidéo des cœurs

NativeCoreFrames reprend les copies et les limites de ExternalCoreAudioCallbackHost.cs, ExternalCoreVideoCallbackHost.cs et EmulationAudioFunctions.cs. La file stéréo garde jusqu’à un cinquième de seconde de données selon la fréquence déclarée. Elle conserve les échantillons les plus récents et compte les anciens blocs retirés. Les pixels sont copiés avec leur pitch et leur format. Un pointeur vidéo nul répète la trame précédente avec un nouvel horodatage. La première erreur reste consultable après le retour du callback. Les propriétaires sont utilisables par un adaptateur, qui doit relier les fonctions reçues par le cœur à sa session.

La sonde Windows Release a vérifié la file PCM et ses bornes, la conservation des copies après modification du tampon source, les formats RGB565 et XRGB8888, la répétition et le rejet des pitches invalides ou hors capacité. Emulation.dll a été déployée et la sonde supprimée.

## Commandes d’environnement des cœurs

NativeCoreEnvironment reprend de ExternalCoreEnvironmentCallbackHost.cs l’installation ordonnée des six callbacks, les chemins système, contenu et sauvegardes à adresse stable, les annonces AV, le clavier, la demande d’arrêt et le démarrage possible sans média. Le calcul du remplissage audio, du risque de sous-alimentation et le bornage de la latence suivent ExternalCoreEnvironmentFunctions.cs. Les pointeurs de journal et de retour de force sont fournis par l’adaptateur et empruntés jusqu’à l’arrêt du cœur. Les diagnostics UTF-8 préservent les espaces, et la première exception détectée reste consultable après la sortie du callback. Les options et le système de fichiers virtuel du cœur restent à porter.

Une sonde Windows Release a passé les appels par la fonction native installée. Elle a vérifié les six enregistrements, la durée de vie du chemin, les annonces vidéo et audio, l’arrondi de fréquence à la valeur paire, les transitions clavier, le remplissage, la latence, la désinscription et les erreurs. Emulation.dll a été déployée et la sonde supprimée.

### Variables des cœurs natifs

NativeCoreLegacyOptions reprend EmulationLegacyOptionHost.cs et raccorde SetVariables, GetVariable et GetVariableUpdate. La première valeur déclarée sert de valeur par défaut. Le cœur emprunte les chaînes UTF-8 et un changement invalide uniquement la chaîne correspondante. Le signal est consommé à la lecture. La sonde Windows Release a contrôlé les définitions, la valeur configurée, la stabilité d'une autre chaîne après modification et la consommation du signal. Emulation.dll a été déployée et la sonde supprimée.

### Options natives version 1 et 2

Les commandes d'ExternalCoreEnvironmentFunctions.cs raccordent les définitions de versions 1 et 2 et leurs variantes internationales. EmulationCoreOptionFunctions.Native.cs fournit les bornes, l'ordre du catalogue anglais, la fusion par clé et la sélection du défaut. La visibilité déclarée avant le catalogue est conservée. Le cœur peut modifier une valeur autorisée et reçoit son rappel de mise à jour. La sonde Windows Release a vérifié les commandes via NativeCoreEnvironment, notamment le catalogue localisé de version 2, la valeur configurée, la visibilité, le rejet d'une valeur invalide et la consommation du signal. La DLL Release a été déployée et la sonde supprimée.

### Interface de fichiers des cœurs natifs

NativeCoreFileSystem reprend ExternalCoreVirtualFileSystem.cs, ExternalCoreVirtualFileSystem.Directories.cs et la table ABI d'ExternalCoreApi.cs. Le fournisseur négocie la version 3, résout les alias, applique la lecture seule et conserve les handles et chaînes empruntées pendant leur durée de vie. La table native ne reçoit aucun contexte pour les appels par chemin. Le C++ associe ces appels à des fonctions distinctes pour chacun des 64 fournisseurs simultanés disponibles. Les fichiers et dossiers restants se ferment à la destruction du fournisseur. La sonde Release a contrôlé la négociation via NativeCoreEnvironment, deux fournisseurs isolés, les chemins UTF-8, la lecture et son observateur, l'écriture, la troncature, les répertoires, la protection en lecture seule et le renommage. Emulation.dll a été déployée et la sonde supprimée.

### Changement de disque des cœurs natifs

NativeCoreDiskControl reprend ExternalCoreDiskControl.cs, ExternalRichDiskControl.cs et SelectDisk dans ExternalCoreEnvironmentFunctions.cs. Le service accepte les tables standard et étendue, lit les chemins et libellés en UTF-8, et publie la liste dans le DiskStatus du SDK. La sélection rétablit l'index précédent et referme le tiroir si nécessaire après un refus. L'insertion referme aussi le tiroir si la préparation du nouveau média échoue après son ouverture. La sonde Windows Release a exercé les commandes par NativeCoreEnvironment, la sélection acceptée et refusée, l'insertion dans un emplacement nouvellement créé, le chemin accentué, l'échec de la fabrique de contenu et la remise à zéro. Emulation.dll a été déployée et la sonde supprimée.

### Cycle de vie des cœurs natifs

NativeCoreSession réunit la bibliothèque, le contenu, les callbacks de l'adaptateur et les fonctions de MachineExecution. L'ordre de démarrage suit ExternalCore.cs de la branche osd-support. Le déchargement du jeu précède la désinitialisation et la libération de la DLL, comme dans ExternalCoreLifecycleFunctions.cs. L'adaptateur possède l'environnement et conserve ses callbacks jusqu'au retour de close. La sonde Windows Release a exécuté des trames audio et vidéo, restauré un état, traité la demande d'arrêt du cœur et vérifié l'ordre de fermeture, y compris après un échec de préparation du média. Emulation.dll a été déployée et le cœur temporaire supprimé.

### Raccordement des callbacks natifs

NativeCoreSession peut installer les six callbacks à partir de son environnement et des deux fonctions de lecture des entrées fournies par le module. Comme ExternalHostCallbacks.cs de osd-support, chaque table dirige les appels vers les propriétaires de la session. Le C++ réserve 64 ensembles de fonctions pour les signatures natives dépourvues de contexte. La session conserve ses callbacks jusqu'au retour de la désinitialisation. La sonde Windows Release a chargé deux copies d'un cœur temporaire, vérifié l'isolation audio, vidéo et entrées, l'exception de lecture d'entrée et la fermeture dans l'ordre. Emulation.dll a été déployée et la sonde supprimée.

### Mémoire persistante des cœurs natifs

Le service reprend le contrôle de taille et de pointeur des sauvegardes Bandai et NEC sur osd-support et le remplacement atomique du fichier de CoreSaveRamFunctions.cs. QSaveFile possède le fichier temporaire. L'adaptateur choisit la région native, le chemin, la limite et le moment d'appel. La sonde Windows Release a vérifié les octets restaurés, un chemin accentué, la région absente, le pointeur nul et le rejet des tailles invalides sans modifier la mémoire ni écraser le fichier existant. Emulation.dll a été déployée et la sonde supprimée.

### Chargement spécial des médias natifs

NativeCoreSession utilise retro_load_game_special pour les sous-systèmes, comme ExternalCore.Content.cs dans le module Sony et LoadedSubsystem.cs dans le module Coleco de osd-support. La table contiguë reproduit l'ordre des entrées, avec un descripteur nul pour chaque emplacement vide. La session possède la table et les chemins et tampons associés jusqu'au retour du déchargement. La sonde Windows Release a vérifié trois emplacements mêlant des octets, un emplacement vide et un chemin seul, la disponibilité du contenu pendant le déchargement et la désinitialisation après refus du cœur. Emulation.dll a été déployée et le cœur de test supprimé.

### Isolation de la bibliothèque d’un cœur natif

La copie locale reprend le mécanisme d'ExternalCore.cs dans le module Sega de osd-support, également utilisé par Amstrad, Nintendo, NEC et Sony. Le chemin d'isolation fourni par l'adaptateur héberge un dossier propre à la session. Ce dossier conserve le nom original du binaire et est retiré après le déchargement de QLibrary. Le chargement direct reste disponible pour les adaptateurs qui le demandent. La sonde Windows Release a ouvert deux sessions depuis le même binaire, contrôlé l'isolation de leurs callbacks, vérifié la suppression indépendante des copies et contrôlé le nettoyage sur une version ABI refusée. Emulation.dll a été déployée et la sonde supprimée.

### Descriptions natives des entrées et de la mémoire

NativeCoreEnvironment reprend les dispositions d'ExternalCoreApi.cs et les copies bornées d'ExternalCoreEnvironmentFunctions.cs. Les bornes et la politique de noms reprennent l'adaptateur Acorn de osd-support. Les descriptions UTF-8 sont possédées par l'hôte. Les adresses et fonctions empruntées au cœur sont retirées par NativeCoreSession après sa désinitialisation. La sonde Windows Release a vérifié les identifiants et l'ordre, la copie des chaînes, les limites de la table des contrôleurs, la carte mémoire, la déclaration de succès et l'effacement sur un pointeur nul. Emulation.dll a été déployée et la sonde supprimée.

### Annonces et capacités de l’environnement natif

NativeCoreEnvironment reprend ExternalCoreEnvironmentFunctions.cs et ExternalHostCallbacks.EnvironmentDetails.cs de osd-support. Il conserve la géométrie, l'annonce de rotation et le niveau de performances. Les capacités d'entrée, les LED et les capteurs reflètent ce que l'adaptateur fournit. Les messages simples et étendus conservent la durée, les champs natifs et le texte UTF-8. La sonde Windows Release a vérifié les bornes de rotation, les pointeurs nuls, les capacités sur 64 bits, les callbacks optionnels et les messages étendus. Emulation.dll a été déployée et le dossier de la sonde du build a été supprimé.

### Commandes du client hébergé

HostedCoreSession reprend les requêtes communes de ProcessCore.cs du module Sega de osd-support en utilisant le transport et les codecs existants. Les états restent des blocs bornés. Les insertions et statuts de disque emploient les formes JSON utilisées par le client C#. Le rapport du client conserve séparément l'index, le chemin et le libellé, y compris un chemin nul. Les sorties sont publiées après validation de la réponse. La sonde Windows Release a exercé les dix méthodes avec un processus enfant et vérifié les octets transmis ainsi que la fermeture sur un état tronqué. Emulation.dll a été déployée et la sonde supprimée.

### Initialisation des cœurs hébergés avec sélection de disque

Dans osd-support, EmulationHostProtocolFunctions.Host.cs définit deux requêtes d’initialisation. ProcessCore.cs de Sega utilise celle sans identifiant de moteur. CoreHost.cs de Sega place ensuite le nombre de disques et l’index courant après les extensions et avant les voyants. Le client C++ expose les deux signatures et active explicitement ce champ pour les hôtes qui l’envoient. Le dernier état de disque reçu est accessible depuis HostedCoreStatus.

Une sonde Windows Release entre deux processus a vérifié les octets de la requête, la position de l’identifiant dans la seconde variante et la réception simultanée de l’index et des voyants. Elle a aussi vérifié le rejet de cette réponse avec le profil sans sélection de disque. Emulation.dll a été régénérée et sa copie Release a été vérifiée.

Le client Sega actualise aussi CurrentDiskIndex dès la confirmation d’une insertion, d’une éjection ou d’une sélection. Le JSON d’insertion porte l’index dans slot.index. Le client commun C++ suit ces transitions lorsqu’un rapport de sélection est disponible. Une sonde entre deux processus a vérifié les commandes, la mise à jour immédiate et la conservation de l’index pour une insertion hors plage ou une requête invalide. La DLL Release a été régénérée.

### Cycle de vie des cœurs hébergés

Dans osd-support, Acorn et Apple envoient HardReset au processus. Amstrad, NEC, Nintendo, Sega et Sony créent un nouveau processus et relancent son initialisation. Amstrad libère aussi son processus après Stop. Le client commun permet aux adaptateurs de demander le redémarrage avec leur configuration courante et de sélectionner la politique de fermeture. Pour l’insertion, Sega utilise slot.index, Amstrad et Nintendo utilisent mountOrder, et NEC et Sony utilisent slotIndex avec repli sur mountOrder.

La sonde Release a vérifié la continuité du processus pour les commandes simples, le changement d’identifiant après redémarrage, la fermeture configurable après arrêt et les trois variantes d’index. Emulation.dll et sa copie dans le paquet Release ont la même empreinte.

### Disposition des réponses de trame

Dans osd-support, Acorn conserve la capacité de sauvegarde de l’initialisation. Le module Example la transmet également à chaque trame. Sega transmet à cet endroit le nombre et l’index du disque. La capacité est désormais lue par trame sur demande explicite. Pour l’insertion Commodore, slotIndex est optionnel et son repli est le premier disque.

Une sonde C++ entre deux processus a rejoué les trois dispositions de réponse sous Windows Release. Elle a vérifié le transfert vidéo par mémoire partagée fixe ou redimensionnable, les données audio, la conservation de l’image et des diagnostics en l’absence de remplacement et la fermeture sur réponse tronquée. Emulation.dll a été régénérée et vérifiée dans le paquet Release.

### Aide du panneau de réglage OSD

FullscreenOsdWindow.Menu.cs sépare les touches de modification et de navigation, affiche la navigation verticale lorsque plusieurs réglages sont accessibles et propose un retour cliquable vers le menu. Le port C++ reprend ces indications avec les affectations actuelles de la session et le réglage à la molette. Une sonde Qt a vérifié l’affichage conditionnel, la modification par molette, le passage au réglage suivant et le retour au menu. Le paquet Windows Release a été généré et installé.

### Enregistrement sur cassette

MachineController.ExecuteTapeCommandAsync sur osd-support remonte la cassette si le chemin ou le profil PCM change. Session::recordTape applique cette règle et laisse au thème les dialogues. Le service commun conserve la sauvegarde des modifications de la cassette en cours avant la création du nouveau fichier.

La sonde Windows Release a ouvert une session par le SDK et vérifié un premier montage, le maintien de ce montage pour le même fichier et le même profil, le remontage après changement de fréquence et le rejet d’une extension non acceptée. Le paquet Release a été régénéré.

### Retour du tampon audio des machines

Le C# transmet le statut de son tampon de sortie au cœur avant chaque trame. La session C++ possède désormais la sortie Qt et publie le statut par IMachineAudioBufferFeedback. Le type du SDK est partagé par les codecs de transport et les points d’entrée des cœurs natifs. À la pause, à l’arrêt ou au changement de machine, la session libère la sortie et retire son statut.

Le paquet Windows Release a été généré. Un module temporaire chargé par le SDK a vérifié la réception de l’état absent lorsque la sortie est muette, inactive ou arrêtée. La mesure avec un périphérique audio actif reste à vérifier.

### Latence minimale et vérification Windows

EmulationAudioOutputController.cs et WasapiAudioOutput.cs utilisent la latence minimale demandée par le cœur pour dimensionner la sortie. Le contrat C++ expose désormais cette demande et règle le tampon Qt avant le démarrage ou après un changement. Une valeur hors de l’intervalle de 0 à 500 ms conserve la demande précédente. Zéro utilise la taille par défaut de Qt.

Sur la sortie Windows par défaut, un module de sonde a transmis des échantillons silencieux à 48 kHz. La capacité remontée était de 5 760 trames à 120 ms puis 14 400 trames à 300 ms, avec 1 920 trames en attente dans les deux cas. Le statut a été retiré après l’arrêt. Le paquet Windows Release a été régénéré.

### Choix du périphérique de sortie audio

EmulationSettingsChoiceFunctions ajoute les périphériques de sortie aux choix déclarés par le module. En C++, IModuleAudioOutputSettings fournit le champ et le choix courant, et Host::describe ajoute les sorties fournies par Qt aux choix déclarés. Leur identifiant opaque est encodé en hexadécimal. L’identifiant vide suit la sortie par défaut et un identifiant indisponible laisse la sortie inactive.

La sonde SDK Windows Release a vérifié la présence du périphérique et de son libellé dans les réglages, la sortie par défaut, l’absence de repli lorsque le périphérique choisi manque, puis la reprise avec ce périphérique choisi explicitement. Le paquet Windows Release a été régénéré.

### Réglages audio de la machine

WasapiAudioOutput.cs sur osd-support utilise le maximum de la latence configurée et de celle demandée par le cœur. EmulationAudioOutputController.cs normalise le gain entre 0 et 1. Le C++ transmet le gain à Qt et recalcule le tampon uniquement si la latence effective, le format ou le périphérique change.

La sonde Release Windows à 48 kHz a mesuré 10 560 trames à 220 ms configurés, 5 760 trames lorsque la machine descend à 50 ms et que le cœur demande 120 ms, puis 14 400 trames lorsque le cœur demande 300 ms. Une modification du volume à zéro a conservé le tampon actif à 220 ms. La sonde utilisait du PCM silencieux, donc elle ne mesure pas le niveau sonore.

La vue Émulation du thème RetroMediaLab utilise le cadre repliable de la branche osd-support avec deux rangées de vignettes et l’onglet Accueil. Les sélecteurs déroulants de la version précédente ont été retirés. Les binaires Debug et Release déployés contiennent la DLL du thème compilée et les 595 PNG du dossier source Assets, aux mêmes chemins relatifs.

Les réglages d’image de l’écran hôte restent sous `processing.adjustments`. Les douze autres écrans utilisent `processing.screenAdjustments`, avec les noms de la branche source. Un profil ancien sans réglages par écran utilise les valeurs neutres pour ces écrans. Le dialogue, l’OSD et le traitement vidéo utilisent les réglages de l’écran actif. Ils appliquent les restrictions par écran et par palette de `EmulationVideoProcessingConfigurationFunctions.cs`. La sonde Qt a vérifié le rechargement des treize profils, les restrictions LED et matrice de points, le papier électronique et le rendu logiciel LCD. Les DLL Windows Release déployées correspondent aux fichiers compilés.

Le choix Technologie de l’OSD expose directement les treize écrans de EmulationVideoProcessingCatalog.Mappings.cs. Les trois variantes LCD, LCD rétroéclairé et OLED gardent chacune leur profil. Le dialogue et l’OSD suivent les conditions de EmulationVideoProcessingConfigurationFunctions.cs pour le désentrelacement, la réduction du tramage et les réglages réservés à l’écran hôte. Le rendu force l’échantillonnage au plus proche pour les écrans simulés et limite la restauration selon l’écran. L’exception de réduction du tramage liée à l’entrée analogique simulée reste à raccorder avec les réglages de signal propres à chaque écran. La sonde vérifie les treize sélections, les trois profils, la disponibilité des traitements et le rendu logiciel vectoriel. Les neuf libellés proviennent des RESX du C# avec le catalogue anglais comme repli.

Les entrées LCD, LCD rétroéclairé, OLED, plasma et projection mémorisent séparément le type de connexion, le décodeur couleur, la fréquence verticale, la palette du moniteur et la dégradation, dans les objets `lcd`, `led`, `oled`, `plasma` et `projection` de la branche C#. Le rendu réutilise le décodeur CRT et la synchronisation verticale. Comme dans FilterDisplayConnection.cs et EmulationVideoProcessingConfigurationFunctions.cs, les écrans dotés d’une entrée dédiée ignorent le signal global et la réduction du tramage est désactivée en composite et RF. La définition de dalle, les zones de rétroéclairage LED et la séparation des couleurs DLP restent à porter. La sonde hors écran vérifie les cinq profils, les sorties composite et RF, l’isolation du signal global et la désynchronisation à 50 Hz. Windows Release inclut les DLL vérifiées.

Les définitions natives et VGA à 4K reprennent EmulationVideoProcessingCatalog.Lcd.cs. Le logiciel projette chaque pixel du viewport vers la dalle simulée, puis vers la source, avec le calcul entier de ResampleForLcd. Le profil conserve une définition par écran. Dans le dialogue et l’OSD, le réglage est exposé pour LCD, LCD rétroéclairé, OLED et projection, comme dans la branche C#. Le plasma peut lire une définition enregistrée. La sonde vérifie les dix tailles, la persistance des cinq profils et les 4 096 pixels du rendu logiciel VGA. Les neuf libellés de résolution fixes sont des invariants. Le libellé du champ et la définition native viennent des RESX.

Le LCD rétroéclairé reprend les quatre modes de FilterLedBacklitLcdDisplay.cs. Full Array utilise jusqu’à 8 × 5 zones et Mini LED jusqu’à 32 × 18, mesurées sur les pixels décodés avant la mise à l’échelle. L’éclairage, le niveau de noir TN, IPS ou VA et l’intensité sont appliqués dans le traitement logiciel C++. Le dialogue et l’OSD enregistrent ces choix sous processing.led. Si le profil ne contient pas encore cette intensité, l’ancienne valeur explicite de la dalle LED est lue en repli. Les neufs libellés viennent des catalogues du C#. La sonde vérifie les quatre modes, les niveaux des zones claires et sombres, l’extinction et le rechargement du profil. Le paquet Windows Release contient les DLL vérifiées.

Les cinq technologies de projection de la branche source sont disponibles dans le dialogue et l’OSD et enregistrées sous processing.projection.technology. Le filtre logiciel FilterProjectionColorBreakup.cs est porté pour le DLP mono-puce. Il sépare les composantes rouge et bleue des fronts en mouvement avant la mise à l’échelle, conserve le résultat lors d’un rafraîchissement de la même trame et recommence son historique si les trames sont trop éloignées. La sonde vérifie le rechargement des cinq technologies, le pixel coloré attendu ainsi que le traitement des trames immobiles, répétées ou discontinues. Les autres différences optiques des technologies et les surfaces de projection restent à porter.

Le portage de FilterProjection.cs et ProjectionVideoParameters.cs reprend le niveau de noir, l’ouverture, la diffusion et la convergence fixes des cinq technologies, ainsi que les surfaces blanche, grise et mur blanc. La lumière ambiante est bornée de 0 à 20 comme en C#. Les anciens curseurs d’optique génériques ont été retirés du dialogue et de l’OSD. Le choix de surface est stocké dans processing.projection.surface. La sonde compare les pixels des trois surfaces, la diffusion et la convergence CRT, la grille LCD et l’effet DLP après ce changement. Les quatre libellés de surface sont repris des catalogues C#. Le paquet Windows Release contient les DLL vérifiées.

Le rendu OLED reprend FilterOledDisplay.cs et FilterOledPixelStructure.cs. Le limiteur mesure le signal décodé et applique la courbe RGB OLED, WOLED ou QD-OLED. Les sept structures de sous-pixels utilisent la définition simulée de la dalle. Le dialogue et l’OSD enregistrent panelType et pixelStructure sous processing.oled. L’émission et le niveau de noir génériques ont été retirés des commandes OLED. La sonde Windows Release compare les trois niveaux de blanc, le noir et les sept structures après rechargement. Les choix de connexion propres à l’OLED et le portage des structures LCD et LED restent à traiter.

Le LCD et le LCD rétroéclairé partagent la grille de FilterLcdPixelStructure.cs, aussi utilisée par l’OLED pour ses structures RGB, BGR et monochrome. Les deux profils conservent indépendamment leur structure, orientation, dalle TN/IPS/VA et intensité. Le LCD applique la fuite lumineuse de FilterLcdDisplay.cs. Le LCD et le LED suivent les temps de réponse par dalle du C#, dont le ralentissement des transitions sombres en VA. La lecture des anciens profils reprend l’intensité et la structure compatibles quand le nouveau profil est absent. Les anciens curseurs génériques sont remplacés par les choix du C#. La sonde vérifie la persistance, la reprise d’un ancien fichier, les orientations, le rendu LCD/LED et les trois temps de réponse. Les DLL Windows Release déployées correspondent aux fichiers compilés.

### Entrées OLED et projection

Sur osd-support, les catalogues OLED et Projection et EmulationVideoProcessingConfigurationFunctions définissent les entrées autorisées et le retour à Numérique pour les valeurs incompatibles. VideoPresentation partage cet ordre avec le dialogue de réglage et l’OSD et applique la même règle aux profils chargés, enregistrés ou transmis au rendu. La sonde Windows Release a vérifié l’ordre des trois listes, la normalisation d’anciens profils et la conservation de valeurs compatibles. Les DLL déployées correspondent aux binaires compilés.

### Réglages et rendu du plasma

Les quatre structures de cellules, les six définitions et les six connexions reprennent EmulationVideoProcessingCatalog.Plasma.cs. Les profils anciens qui contenaient une intensité de grille numérique reviennent à Bandes (RGB). La chaîne du C# applique l’intensité des phosphores, le limiteur, le tramage, le plancher des noirs et la structure de cellules dans cet ordre. La persistance s’applique après les effets stylistiques. Les trois nouveaux libellés ont été générés avec Argos puis relus dans les catalogues. La sonde Release a vérifié les quatre rendus distincts, la teinte ambre, la persistance et la lecture des profils.

### Réglages de l’écran vectoriel

Le dialogue du thème RetroMediaLab reprend les groupes Détection et tracé et Lueur et rémanence de EmulationVectorSettingsBlock.cs. Les curseurs couvrent la plage du C# et l’OSD suit MachineController.OsdDisplayTechnologies.cs pour l’ordre des champs. Les deux titres viennent des RESX des 29 langues. Un phosphore inconnu est normalisé en couleur comme dans EmulationVideoProcessingConfigurationFunctions.cs. La sonde a vérifié le rechargement du profil, le rendu ambre et le retour au rendu couleur pour un phosphore invalide. Le paquet Windows Release contient les DLL vérifiées.

### Réglages VFD et afficheurs matriciels

Les groupes du dialogue reprennent EmulationVfdSettingsBlock.cs, EmulationLedMatrixSettingsBlock.cs et EmulationDotMatrixSettingsBlock.cs. Les trois éditeurs utilisent des curseurs avec l’indication de la valeur. Les durées s’ajustent par pas de 5 ms. Les réglages des cellules VFD restent visibles mais désactivés en mode Graphique, dans le dialogue et dans l’OSD. Les réglages de l’OSD suivent l’ordre du C# sous des titres de section. Les titres VFD et LED viennent des RESX de la branche. Build Windows Release réussi. Le thème et les 29 DLL de langue du paquet correspondent aux produits de compilation.

### Ordre et visibilité des commandes vidéo de l’OSD

EditableVideoRows de MachineController.OsdVideo.cs aplatit les groupes vidéo du C# en sections. Le C++ reprend leurs conditions et l’ordre des commandes intégrées, avec les réglages CRT du faisceau, du phosphore et de la dégradation de connexion, et les bascules décimale et deux-points des afficheurs à segments. En mode VFD graphique, l’OSD masque la taille et l’écart des cellules. Le dialogue conserve ces contrôles désactivés selon EmulationVfdSettingsBlock.cs. Le titre LED reprend l’invariant du C#. La sonde a exécuté le constructeur des lignes et vérifié les transitions de visibilité et l’écriture du profil sur une bascule. La répartition des réglages vidéo fournis par les modules entre Display, Image et Effects reste ouverte.

### Répartition des réglages vidéo des modules dans l’OSD

Sur osd-support, VideoSection est une propriété du champ et vaut Display par défaut. MachineController.OsdVideo.cs place les réglages Display après la technologie, Image après la restauration et Effects après les effets intégrés. Le SDK C++ expose ces déclarations via IModuleVideoSettingsSections, une interface facultative. Le module associe un identifiant de champ à une section dans sa configuration. Les modules qui utilisent le format actuel de SettingsField peuvent continuer à fournir leurs champs, qui vont dans Affichage par défaut. Comme EditableVideoRows, l’OSD affiche les champs éditables et applique la visibilité du bloc et l’activation du champ. Le constructeur des lignes a été exécuté avec un bloc mêlant les trois sections, un champ d’information et le réglage de moteur filtré. La sonde vérifie également le masquage de l’en-tête quand le dernier contrôle devient indisponible. Build Windows Release et DLL installées vérifiés.

### Aperçu et enregistrement des réglages de l’OSD

FullscreenOsdWindow.Menu.cs prévisualise les valeurs numériques, regroupe les changements en attente et attend l’écriture en quittant la commande. Le contrôle Qt émet l’aperçu avant l’écriture, conserve le dernier changement en attente dans la boucle d’événements et traite l’enregistrement avant de quitter l’éditeur ou de fermer l’OSD. En cas de refus de l’écriture, la valeur et le rendu enregistrés sont rétablis. Les choix et les champs des modules se passent d’aperçu, selon osd-support. La sonde Qt hors écran a vérifié le regroupement de deux changements, la restauration après échec et l’écriture lors du passage à un autre champ, du retour et de la fermeture. Les DLL Release du paquet sont à jour.

### Paquets Debug et Release du sélecteur

EmulationConfigurationSelector.cs et EmulationSection.cs dans osd-support présentent deux rangées de vignettes, Marque et Machine, dans un cadre repliable au-dessus de l’onglet Accueil. Le paquet Debug pouvait encore charger l’ancienne interface lorsque MSVC interrompait la compilation d’EmulationPage.cpp avec C1128. CMake applique désormais /bigobj à ce fichier. Les deux paquets ont été régénérés. Les DLL installées correspondent aux DLL compilées et chaque dossier Assets contient les 595 PNG des sources, dont 39 dans Icons.

### Commandes et ordre du menu plein écran

MachineController.cs et EmulationMachineTabConstants.cs dans osd-support définissent les commandes du menu et l’ordre des catégories. Le sous-menu Vidéo regroupe les réglages vidéo. Les raccourcis de capture et de statistiques restent actifs. Audio affiche la coche ou le tiret selon l’état muet. Les réglages intégrés suivent l’ordre C# et les onglets supplémentaires déclarés par le SDK apparaissent à la suite. Compilation et installation Release vérifiées.

### Libellé du groupe de sortie audio

MachineController.OsdModuleSettings.cs utilise Emulation.Audio.Output pour le groupe audio qui contient le choix de périphérique. Le thème C++ obtient l’identifiant de ce champ par Host, à partir du contrat audio existant. Les 30 catalogues reprennent le libellé de EmulationAudio.resx. Une sonde Qt hors écran a vérifié l’affichage du groupe et l’actualisation immédiate du statut muet pendant la navigation. La compilation et les empreintes des DLL Release ont été vérifiées. L’essai du menu complet avec une machine reste ouvert.

### Disposition des panneaux du menu plein écran

FullscreenOsdWindow.Menu.cs affiche le menu racine de 240 pixels et le sous-menu courant de 310 pixels. Les sous-menus courts commencent à la ligne du menu racine. Les groupes déclarés pleine hauteur occupent l’espace disponible et l’éditeur centré en bas masque les deux panneaux. Le portage Qt reprend cette disposition et conserve le niveau actif au retour. Une sonde hors écran a vérifié l’ancrage et la hauteur des deux types de sous-menu, le remplacement du panneau en profondeur, la saisie clavier dans l’éditeur et la restauration des menus. La sonde a été supprimée. La version Release et les DLL installées ont été vérifiées.

### Visibilité et sélection après un réglage

RefreshMenu dans FullscreenOsdWindow.Menu.cs réévalue la visibilité le long du chemin ouvert. Si un réglage disparaît, le portage C++ ferme son éditeur après sauvegarde et sélectionne une ligne accessible. Si le parent du sous-menu disparaît, le menu revient au dernier niveau visible. La sonde Qt hors écran a vérifié le masquage de la ligne et de son séparateur, le repli du sous-menu, la sélection de la commande de secours et les deux écritures. La sonde a été supprimée. La compilation Release et les empreintes des DLL installées ont été vérifiées.

### Chargement du shader dans le menu plein écran

MachineController.VideoShaderLoadingChanged dans osd-support dirige le même état vers l’OSD ou le badge autonome. VideoPresentation partage le composant Qt entre ces deux modes et le thème relaie l’état produit par VideoSurface. Le badge centré laisse passer les clics et conserve la navigation du menu. Une sonde hors écran a vérifié le placement, la mise à jour du texte, le redimensionnement, la visibilité et la géométrie du badge autonome. La sonde a été retirée et les DLL Release ont été vérifiées par empreinte.

### Cache disque Direct3D 11

VideoShaderDiskCache.cs et VeldridShaderBytecode.cs sur osd-support vérifient SHA-256 avant la lecture du binaire et remplacent chaque entrée atomiquement. Le backend natif utilise le cache utilisateur Qt sous ShaderCache/Direct3D11. La clé inclut la source, le profil, le point d’entrée, la version du format et les options du compilateur. Le cache sert aux shaders HLSL compilés à l’exécution. Les packs Qt qui contiennent déjà du DXBC restent directement chargeables. Une sonde a compilé les véritables shaders de présentation, vérifié une seconde lecture sans réécriture, régénéré une entrée corrompue et utilisé un chemin de cache indisponible. La sonde a été retirée et la DLL Release a été vérifiée par empreinte.

### AltGr dans le clavier de la machine

KeyboardModifierState.cs sur osd-support utilise une fenêtre de 50 ms entre le Ctrl gauche généré par Windows et AltGr. EmulationHostKeyboard utilise la même fenêtre sur les événements natifs et écarte le Ctrl gauche synthétique des touches transmises à la machine. Le relâchement de Ctrl et la remise à zéro de l’entrée effacent l’état. La sonde liée à App.dll a vérifié le seuil, les deux Ctrl, les relâchements et le débordement du compteur Windows. La sonde a été retirée et la DLL Release vérifiée. La normalisation des raccourcis hôte et leur capture restent à traiter.

### AltGr dans la capture et les raccourcis hôte

InputBindingCapture et MachineInputController partagent KeyboardModifierState en C#. Le port C++ partage la détection de Ctrl synthétique entre le clavier de la machine, la capture et le traitement des raccourcis. La capture distingue les deux Ctrl et attend leurs relâchements. La sonde Qt a vérifié quatre captures et la normalisation des modificateurs avec un Ctrl droit réel. Le build et les empreintes des DLL Release ont été vérifiés. Le déclenchement sur un clavier physique reste à vérifier.

### Raccourcis MIDI du menu plein écran

MidiShortcutInput et OsdMidiBindingCapture de osd-support définissent les identifiants et les transitions de notes. Le port Windows ouvre les entrées WinMM quand le premier client en a besoin, les ferme au départ du dernier et suit Note On, Note Off, la vélocité nulle et les commandes de relâchement par canal. La capture des commandes OSD ignore les notes déjà maintenues. La session plein écran lit les notes lorsqu’un raccourci MIDI est configuré et les transmet au système de raccourcis. La sonde a vérifié le codage des identifiants, la capture des nouvelles notes, les relâchements et une combinaison clavier et MIDI. Windows Release a compilé et la DLL du thème a été vérifiée. L’essai avec un périphérique MIDI physique reste ouvert. [Ouverture des entrées WinMM](https://learn.microsoft.com/windows/win32/api/mmeapi/nf-mmeapi-midiinopen) · [Format des messages de notes](https://learn.microsoft.com/en-us/windows/win32/multimedia/mim-data) · [Contraintes du rappel WinMM](https://learn.microsoft.com/en-us/previous-versions/dd798460(v=vs.85)).

Vérification du paquet après le signalement des anciens sélecteurs. Le source C++ utilise les deux rangées de vignettes du thème Base dans EmulationConfigurationSelector.cs et le cadre repliable dans EmulationSectionLayoutFunctions.cs. Debug a été recompilé et réinstallé. Dans Debug et Release, la DLL installée concorde avec celle compilée et les 595 PNG du dossier Assets, dont 39 dans Icons, concordent octet pour octet avec les sources. Sans module de machine configuré, les rangées ne contiennent aucune vignette.

MachineInputController.cs limite la répétition aux quatre directions pendant l’édition OSD. La sonde Qt Release a vérifié la flèche droite par défaut, le premier rappel à 300 ms, les rappels suivants à 35 ms et la réinitialisation au relâchement, au changement de direction et à la sortie de l’éditeur. La sonde temporaire a été supprimée.

La sonde Qt hors écran a chargé la DLL du thème et les données externes du paquet Release. En ouvrant Émulation sans module, elle a vérifié les libellés Marque et Machine, l’onglet Accueil, l’absence de listes déroulantes et le repli puis le dépliage du cadre. La sonde et la copie temporaire des données du thème ont été supprimées.

Le tampon commun du SDK reprend EmulationAudioBuffer.cs et EmulationAudioFunctions.cs sur osd-support. Sa capacité est d’un cinquième de seconde et il conserve les échantillons les plus récents. Le compteur de dépassements comptabilise séparément les blocs tronqués et évincés. La fin du vidage d’un rappel natif ne compte pas comme un manque. Le tampon déduit le nombre de canaux de la disposition du bloc pour préserver les trames multicanales. Le contrôle lié à Emulation.dll a vérifié la rétention stéréo et multicanale, les deux compteurs, la séquence, l’horodatage et le vidage de la file et du dernier bloc. La DLL compilée correspond à celle déployée dans Windows Release. La sonde temporaire a été supprimée.

Les sorties natives sont vidées à la remise à zéro, après restauration acceptée et à la fermeture. Les transitions de remise à zéro et de restauration suivent ExternalCore.State.cs du module exemple. La fermeture suit ExternalHostCallbacks.cs, qui vide la sortie après la désinitialisation du cœur. Le bloc audio précédent, la file PCM et la dernière trame vidéo sont retirés. Les numéros de séquence continuent d’augmenter. Le contrôle lié à la DLL Windows Release a vérifié ces transitions avec un cœur temporaire, puis supprimé le cœur et son image. La DLL installée a la même empreinte que la DLL compilée.

FilterProjectionColorBreakup.cs invalide l’historique lorsque la configuration de projection change. FilterDisplayConnection.cs l’invalide à la sortie du DLP monopuce. Le traitement logiciel C++ prend en compte la configuration de projection et son entrée avant de réutiliser une trame. Le mode hôte efface l’historique même si aucun filtre ne s’exécute. Le contrôle a exercé la séparation des couleurs sur un contour mobile, la répétition de trame, le changement de profil et d’entrée, et le passage au mode hôte. La DLL Windows Release a été vérifiée par empreinte et la sonde a été supprimée.

MachineControllerFeedbackController.cs isole les erreurs des sorties et arrête les moteurs. Le C++ applique cette règle dans Shared/Emulation. Le thème transmet la source uniquement quand la machine tourne et que les entrées de session sont actives hors menu plein écran. Le contrôleur commun agrège les intensités valides par moteur et journalise une fois par type d’exception et par code système disponible. La sonde a vérifié les quatre moteurs, l’intensité maximale, les commandes rejetées, l’arrêt à la désactivation et à l’erreur, l’unicité du diagnostic et la reprise. Elle utilise un faux périphérique. Windows Release a été compilé et les DLL du thème et d’App vérifiées par empreinte.

MachineController masque la vidéo lorsque la machine est hors tension. MachineVideoPresenter remet son historique à zéro quand la machine change. Le port C++ invalide l’image et les statistiques à l’arrêt ou au changement de révision du cœur. La pause peut conserver la dernière image. Une source qui retire sa trame pendant l’exécution invalide aussi les travaux logiciels différés. La sonde a vérifié sur VideoSurface la pause, la disparition de la trame, l’annulation d’une image différée et la reprise avec une nouvelle image. Le remplacement avec un module réel reste à vérifier. Les trois DLL du paquet Windows Release ont été vérifiées par empreinte et la sonde retirée.

Les titres Affichage, Image et Effets utilisent déjà la déclaration par champ du module. Le menu plein écran applique la règle de EmulationSettingsValuePresentationFunctions.cs aux choix numériques dont le descripteur fournit une unité. Comme MachineController.OsdModuleSettings.cs, les groupes de réglages gardent les sélecteurs et curseurs sans éditeur en lecture seule. La liste vidéo aplatie conserve seulement les champs éditables. La sonde a vérifié le libellé invariant, le libellé traduit, le nombre avec unité, le choix non numérique, la valeur sans liste et la valeur vide. Le thème Windows Release est compilé et vérifié par empreinte. Le rendu avec une machine réelle reste à vérifier.

### Actualisation des commandes après enregistrement

Le SDK permet au cœur de mettre à jour les entrées d’une session active avec la configuration enregistrée, comme `IEmulationInput.UpdateBindings` dans `osd-support`. La sonde Windows Release a vérifié l’ordre entre sauvegarde, commandes et options à chaud, ainsi que le cas d’une machine en pause ou arrêtée. Les fichiers temporaires de la sonde ont été supprimés.

### Sélection d’une configuration ouverte

Le clic sur une machine reprend l’identifiant de la configuration de l’onglet ouvert, même lorsque celui-ci est déjà actif. Le double-clic et le bouton Ouvrir suivent l’identifiant de la configuration choisie, comme dans EmulationSectionConfigurationFunctions.cs. Le bouton reprend la largeur minimale de 130 pixels logiques déclarée dans EmulationSection.cs. Le paquet Windows Release a été reconstruit et la DLL du thème vérifiée par empreinte. Le parcours avec plusieurs configurations de la même machine reste à vérifier à l’exécution.

### Validation des raccourcis Émulation

La validation reprend les formes de raccourcis de InputBindingValidationFunctions.cs et KeyboardChordFunctions.cs. Les sources souris et manette passent par le validateur du SDK. Les entrées illisibles sont signalées avant sauvegarde. Le champ Qt enregistre une combinaison par saisie. La capture reconnaît les combinaisons composées uniquement de modificateurs au relâchement. La sonde Qt a vérifié Ctrl seul, la différence avec Ctrl+A, le mélange de la molette et d’un modificateur, le rejet de deux entrées physiques invalides et le statut réservé de Alt+F4. Elle a été supprimée. Le build Windows Release et les empreintes des DLL ont été vérifiés.

### Répertoire du processus hôte et extensions de contenu

ProcessCore.Host.cs du module Example lance le cœur hébergé dans le dossier de son exécutable et traite ses extensions sans tenir compte de la casse. Le transport C++ utilise ce dossier lorsque l’adaptateur laisse le réglage vide. Une valeur explicite reste prioritaire. Le client conserve la première graphie de chaque extension et ignore les entrées vides. La sonde Windows Release a lancé un vrai processus temporaire dans les deux configurations, avec un chemin relatif pour le cas par défaut, et contrôlé les extensions renvoyées par le protocole. La sonde a été supprimée et Emulation.dll a été vérifiée dans le paquet Release.

### Reprise après une demande d’arrêt native

ExternalCore.State.cs de osd-support efface la demande d’arrêt avec les sorties après réinitialisation ou restauration réussie. Le socle natif C++ reprend ce comportement et efface également la demande lors de la libération du cœur. Un état refusé par le cœur conserve le signal et les dernières sorties. Le contrôle Release avec un cœur temporaire a provoqué un arrêt, vérifié la reprise par réinitialisation et par restauration, puis contrôlé la conservation après refus et le nettoyage à la fermeture. La sonde a été supprimée et la DLL Windows Release vérifiée par empreinte.

### Densité d’écran de la vidéo émulée

WpfVideoSurface.cs porte à la résolution physique les traitements des écrans à pixels fixes. Le port Qt recalcule le viewport au changement de densité et reconstruit les surimpressions de statistiques et d’état en pixels physiques, avec leur taille logique d’origine. La sonde Windows Release a simulé le passage de 100 % à 150 % avec la vidéo en pause. L’image traitée est passée de 160 × 120 à 240 × 180 pixels à taille de fenêtre constante. La sonde a également vérifié le cas d’une image effacée, puis a été supprimée. La DLL installée a été vérifiée par empreinte. Le déplacement entre deux écrans physiques et la surimpression des moteurs matériels restent à vérifier sur écran. [Événement de changement de densité Qt](https://doc.qt.io/qt-6/qevent.html#Type-enum) · [Densité de l’image dans Qt](https://doc.qt.io/qt-6/qimage.html#setDevicePixelRatio)

### Historique vidéo en pause

MachineVideoPresenter.cs réaffiche la dernière trame après changement des réglages et réinitialise notamment l’historique des dalles LCD. Le traitement de cette trame dans SoftwareEmulationVideoProcessingPipeline.Temporal.cs initialise l’état de la dalle. Le port réamorce ainsi le traitement temporel après une remise à zéro des réglages et au retour d’une surface masquée qui utilise le traitement différé. La sonde Qt a changé la dalle LCD en pause, soumis une trame blanche une milliseconde après la trame noire et vérifié que le temps de réponse restait visible. La même configuration réappliquée a conservé le rendu. Le build Windows Release a réussi, la DLL a été vérifiée et la sonde retirée.

### Proportions de l’écran et du masque CRT

EmulationVideoLayoutFunctions.FitDisplay sur osd-support ajuste d’abord l’écran au cadre disponible, puis centre l’image dans cet écran à ses proportions d’origine. CrtSignal.From dimensionne le masque à partir des proportions de l’écran. Le C++ reprend ces calculs selon les rapports autorisés pour chaque technologie. En absence d’un rapport annoncé valide, une trame entrelacée utilise sa hauteur d’image complète. La sonde Qt hors écran a vérifié le cadrage 4:3 dans un écran 16:9 et le repli pour une trame entrelacée. Le build Windows Release passe, les empreintes des DLL concordent et la sonde a été supprimée.

Le dialogue et VideoPresentation partagent les choix de proportions de EmulationVideoProcessingConfigurationFunctions.AspectRatioChoices. Le sélecteur suit la technologie choisie et affiche le rapport d’origine si le rapport enregistré ne s’y applique pas. Un aller-retour entre technologies retrouve le rapport enregistré tant que le sélecteur n’a pas été modifié. Le build Windows Release a réussi et les empreintes de lib/Themes/RetroMediaLab.dll et lib/VideoPresentation.dll concordent. La manipulation dans le dialogue reste à vérifier.

Le rendu logiciel emploie maintenant le même CrtSignal pour le tube et le masque phosphore, comme CrtSignal et CrtPhosphorMask sur osd-support. La courbure et la largeur de faisceau s’appliquent au masque et les pixels hors du tube restent noirs. La sonde hors écran a vérifié les coins avec et sans courbure ainsi que l’effet du masque au centre. Elle a été supprimée. Le build Windows Release passe et VideoPresentation.dll déployée correspond à la DLL compilée. Le shader CRT matériel reste à raccorder.

### Passe CRT Direct3D 11

Direct3D 11 applique la géométrie, la synchronisation du signal, le décodeur couleur, le faisceau et le masque issus de CrtSignal.Shader et CrtPhosphorMask.Shader sur osd-support. Les uniformes reprennent les paramètres calculés pour la trame logicielle. Le conditionnement de la connexion reste exécuté à la résolution d’entrée. La restauration d’image, la netteté et les effets stylistiques utilisent encore le rendu CRT logiciel, de même que Vulkan et OpenGL. Le shader CRT est compilé dans un programme distinct pour éviter de le mêler aux filtres de mise à l’échelle. La sonde Windows Release a vérifié le choix de Direct3D, les coins noirs, la teinte du phosphore, le contraste de la capture et le retour au moteur logiciel. Le test temporaire a été supprimé et les empreintes de VideoPresentation.dll concordent.

### Passe CRT Vulkan et OpenGL

Vulkan utilise le même shader de tube et de phosphore que Direct3D 11. Le cache de pipelines construit aussi la passe CRT. Les surimpressions conservent leur pipeline normal et la capture omet les surimpressions. OpenGL réutilise les fonctions CRT avec des uniformes adaptés au profil OpenGL de la fenêtre. Les sondes Windows Release ont vérifié séparément les deux moteurs, les coins noirs, la teinte du phosphore, le contraste de la capture et le passage au moteur logiciel. Les sondes ont été supprimées et les empreintes des DLL Release déployées concordent. Le CRT utilise encore le moteur logiciel pour les combinaisons avec restauration, netteté ou effets stylistiques. Les pilotes et profils OpenGL de Linux et macOS restent à vérifier.

### Détramage CRT et réglages de restauration

Le chemin matériel reprend les règles de EmulationVideoProcessingConfigurationFunctions.ForRendering sur osd-support. Les réglages de débandeur et de récupération de détails restent mémorisés pour le mode Normal. Le rendu CRT applique le détramage à la résolution d’entrée avant le traitement de connexion, sauf en composite et RF. La sonde Direct3D 11 sous Windows Release a comparé les pixels avec les réglages Normal mémorisés, vérifié la réduction du contraste d’un damier et confirmé le résultat identique avec ou sans détramage en composite. La sonde a été supprimée et la DLL déployée a la même empreinte que celle compilée. La netteté et les effets stylistiques utilisent encore le rendu CRT logiciel.

### Netteté CRT matérielle

Le CRT matériel applique la netteté sur les couleurs ajustées avant le décodage du signal, le faisceau et le masque, dans l’ordre de SoftwareEmulationVideoProcessingPipeline.cs sur osd-support. Le shader partage les poids du filtre de netteté avec le mode Normal. La sonde Windows Release a vérifié que le contraste du bord augmente en accentuation et diminue en adoucissement dans Direct3D 11 et OpenGL. Vulkan utilise le même shader CRT que Direct3D 11. La sonde a été supprimée et les empreintes de VideoPresentation.dll concordent. Les effets stylistiques déclenchent encore le rendu CRT logiciel.

### État des processus de cœurs hébergés

Le type HostProcessState du SDK reprend les états de osd-support. HostedCoreSession fournit l’état courant et conserve le dernier PID démarré même après une fermeture, une réponse invalide ou une absence de connexion du processus lancé. La sortie est détectée sur le fil du propriétaire, y compris sans boucle Qt active. Une sonde Windows Release entre deux processus a vérifié le démarrage, la fermeture, la sortie spontanée après Stop, le défaut de protocole et le défaut de connexion. Elle a aussi vérifié la reprise après défaut. La sonde a été supprimée et les empreintes de Emulation.dll concordent.

### Informations d’exécution communes du cœur

Le contrat facultatif du SDK reprend IEmulationCoreRuntimeStatus.cs de osd-support. La session hébergée publie la région, la disponibilité des sauvegardes, les compteurs audio et l’identité du processus. La session native publie la région et les mesures audio locales. La vue utilise l’horloge nominale fournie par la machine et conserve le repli sur son contrat d’horloge existant.

La sonde Windows Release entre deux processus a vérifié la région PAL, la capacité de sauvegarde, les trois valeurs audio et la conservation du PID après fermeture. La sonde a été supprimée et les DLL Emulation et RetroMediaLab déployées concordent avec la compilation. Le chemin natif et la vue ont été compilés en Release.

### Retour de force des cœurs natifs

La session native reprend le rappel de ExternalCoreEnvironmentCallbackHost et le stockage de EmulationControllerFeedbackAccumulator sur osd-support. Le rappel est lié à la session et utilise les deux effets déjà définis par le SDK. Un rappel fourni par le module reste prioritaire. La réaffectation efface le port concerné et la fermeture libère les états.

Le test Windows Release a chargé une DLL temporaire et transmis deux effets. Il a vérifié le rejet d’un effet invalide, la réaffectation d’un port, la fermeture et les informations natives NTSC et sauvegarde. La DLL temporaire a été supprimée et Emulation.dll déployée a été vérifiée par empreinte. L’essai sur une manette physique reste ouvert.

### Retour de force des cœurs hébergés

HostedCoreSession expose aux adaptateurs les effets reçus du processus actif par le protocole des cœurs de osd-support. Une réaffectation acceptée retire immédiatement les anciens effets du port correspondant. Un refus distant conserve le dernier état valide. La fermeture ou la sortie du processus rend les effets indisponibles. Les trames suivantes continuent à fournir les états du cœur.

La sonde Windows Release entre deux processus a vérifié les deux ports en vibration, la réaffectation, le refus distant, la fermeture et la sortie spontanée. Elle a été supprimée et la DLL Emulation déployée a été vérifiée par empreinte.

### Réinitialisation du retour de force

ExternalCore.State.cs efface les sorties, dont le retour de force, après une réinitialisation ou une restauration réussie. Les sessions natives et hébergées exposent un état de vibration vide après ces opérations. Un refus distant conserve le dernier instantané valide.

La compilation Windows Release et les sondes avec une DLL native temporaire et un processus hébergé ont vérifié les deux réinitialisations distantes, la restauration native et distante et le refus distant. Les deux sondes ont été supprimées. La DLL déployée correspond à la DLL compilée.

### Sorties du cœur hébergé après une commande de cycle de vie

Le cœur de référence de osd-support efface les sorties lors de la restauration, de la réinitialisation et de l’arrêt. La session C++ efface l’image, le nombre de trames audio en attente et les vibrations dès la confirmation de la commande. Les compteurs audio cumulés sont conservés. Une réponse de trame sans nouvelle image garde la dernière image, comme ProcessCore.Host.cs.

Une sonde Windows Release entre deux processus, avec une image transmise en mémoire partagée, a contrôlé les quatre opérations, la réponse sans nouvelle image et le retour de la vidéo à la trame suivante. La sonde a été supprimée et la DLL déployée vérifiée par empreinte.

### Table et événements du clavier natif

InputProcessing reprend les 44 touches explicites et les quatre plages de ExternalCoreKeyboardFunctions.cs sur osd-support. NativeCoreSession expose la publication des pressions et relâchements dans le rappel enregistré par le cœur. La fermeture efface la mémoire des touches détenue par le contexte natif.

Une sonde Windows Release avec une DLL temporaire a contrôlé Maj+A, l’ordre des pressions et des relâchements, la casse, l’absence de répétition sur un instantané identique et la première pression après réouverture. La sonde a été supprimée et la DLL déployée vérifiée par empreinte.

### Instantané des entrées pour les cœurs natifs

NativeCoreSession transmet les événements clavier et conserve les entrées interrogées par le cœur à partir du même instantané. InputProcessing partage les correspondances natives entre interrogation et événements, comme EmulationInputProcessingService.cs sur osd-support. Un rappel fourni par un adaptateur conserve la priorité.

Une sonde Windows Release a contrôlé les valeurs clavier, souris et manette lues par une DLL native temporaire, leur remise à zéro et la conservation de l’instantané après modification de l’objet fourni par l’appelant. Elle a été supprimée et la DLL déployée vérifiée par empreinte.

### Options à chaud des cœurs natifs

NativeCoreSession expose les options et catégories de la session active. La validation et le signal de mise à jour restent dans le gestionnaire des options, comme dans ExternalCoreEnvironmentCallbackHost.cs et EmulationLegacyOptionHost.cs de osd-support.

Une DLL temporaire a vérifié le choix initial, le rejet d’une valeur absente du catalogue, la notification au cœur et la prise en compte de la nouvelle valeur à la trame suivante. La compilation Windows Release a réussi, la sonde a été supprimée et la DLL déployée vérifiée par empreinte.

### Commandes de disque dans la session native

Les opérations du contrôleur commun de osd-support sont accessibles à travers NativeCoreSession. Le choix de la politique et de la fabrique de contenu reste au module. La session vérifie que le cœur est chargé avant de relayer les appels et détache les rappels à la fermeture.

Une DLL native temporaire a vérifié le statut, la sélection, le rejet d’un index hors limites, l’éjection, l’insertion et l’ajout d’un emplacement. La compilation Windows Release a réussi. La sonde a été supprimée et la DLL déployée vérifiée par empreinte.

### Installation sans dossier de modules vide

CMake crée les répertoires des contenus effectivement installés. Le paquet actuel omet Modules. Host::discover accepte un répertoire absent, qui ne fournit aucune entrée. La compilation et le déploiement Windows Release ont confirmé que Modules reste absent. Les 595 PNG de src/App/Assets ont été comparés par empreinte à leur copie dans Assets, avec leurs chemins relatifs.

### Définition de dalle du papier électronique

Le profil C++ reprend la définition du contrat EmulationEPaperVideoConfiguration sur osd-support. Le rendu logiciel relie directement image source, définition de dalle et taille de sortie par les deux projections au plus proche de ResampleForLcd. Le dialogue et l’OSD reprennent les libellés des définitions LCD.

La sonde Windows Release a vérifié la valeur native par défaut, la sauvegarde et le rechargement de VGA et le changement des pixels échantillonnés sur une mire. Elle a été supprimée. Les DLL du thème et de la bibliothèque vidéo déployées ont été vérifiées par empreinte. Les sélecteurs ont été compilés.

### Définitions de grille des afficheurs matriciels

Les matrices LED, les afficheurs à points et les VFD partagent les neuf définitions fixes du catalogue EmulationVideoProcessingCatalog.LedMatrix.cs sur osd-support. Les profils anciens gardent la définition native par défaut. Le dialogue et l’OSD utilisent les dimensions du RESX anglais de référence, communes à toutes les langues. Les grilles fixes alimentent le moteur logiciel existant. Pour le VFD, la grille est visible en mode matriciel.

Le test Windows Release a vérifié les dimensions natives et fixes, la sauvegarde des trois réglages et l’effet d’une grille 32 × 16 sur les trois rendus logiciels. La sonde a été supprimée et les DLL vidéo et du thème déployées ont été vérifiées par empreinte. Les contrôles du dialogue et de l’OSD ont été compilés.

### Échantillonnage des cellules LED depuis la trame source

Le chemin logiciel reprend les cinq prises de FilterLedMatrixCellStructure.cs sur osd-support. Chaque cellule échantillonne le centre dans la trame source et, lorsque la grille réduit un axe, les quatre positions décalées avec une moitié du poids réservée au centre. La composition conserve les réglages C++ existants de taille en mode natif, de séparation, de luminosité et de halo.

Une sonde Windows Release a réduit une trame à bandes de 64 × 64 vers une grille et une sortie de 16 × 16. Elle a vérifié que les bandes visibles seulement dans les prises décalées contribuent à toutes les cellules. La sonde a été supprimée et VideoPresentation.dll déployée vérifiée par empreinte.

### Formes et couleurs LED

Le filtre C# `FilterLedMatrixCellStructure` combine le rouge et la moitié du vert source pour `RedGreen`. `RoundedSquare` utilise une distance signée avec des coins de rayon 0,10 en unités de cellule. Le portage expose et enregistre ces choix dans les profils. La fenêtre et l’OSD partagent les clés traduites de la branche `osd-support`.

Le contrôle Windows Release hors écran a vérifié l’enregistrement et la relecture du profil, les canaux rouge et vert et l’atténuation dans les coins. Les fichiers du contrôle ont été supprimés.

### Composition des matrices LED

Le rendu logiciel reprend la géométrie, le halo et la transition vers la couleur uniforme de FilterLedMatrixCellStructure.cs. EmulationLedMatrixVideoConfiguration.cs et EmulationLedMatrixSettingsBlock.cs limitent les commandes à la couleur, à la définition et à la forme. Le dialogue et l’OSD suivent ces trois choix. Les anciens profils restent lisibles et ces trois valeurs restent enregistrées.

La sonde Windows Release hors écran a vérifié le rechargement du profil, la couleur à un pixel par cellule, et la différence entre les coins carrés et arrondis avec agrandissement. La sonde a été supprimée. Le rendu matériel de ce filtre reste à porter.

### Filtre matériel de la matrice LED

Le shader de FilterLedMatrixCellStructure.cs et FilterLedMatrixColor.cs est commun à Direct3D 11, Vulkan et OpenGL. La passe reçoit la définition, la couleur et la forme du profil. Elle compose les cellules à la résolution d’affichage, avec l’échantillonnage de la trame source. Les combinaisons avec netteté, restauration, simulation de signal ou effets stylistiques utilisent encore le traitement logiciel.

Sous Windows Release, la sonde a confirmé le chemin matériel puis comparé une capture de chaque moteur au modèle logiciel. La scène combinait une réduction de la grille, un dégradé rouge et vert et des cellules à coins arrondis. La tolérance était de trois niveaux par canal. La sonde a été supprimée et la DLL installée vérifiée par empreinte.

### Cellules VFD

EmulationVfdVideoConfiguration.cs et EmulationVfdSettingsBlock.cs définissent les six couleurs, les trois formes et la définition de grille. Les profils C++ enregistrent ces trois paramètres. Les champs de couleur et de définition existants restent lisibles. Les profils sans forme utilisent les cellules rondes. Le dialogue et l’OSD reprennent le même ordre dans les 29 langues.

VfdVideoShaderParameters.cs fournit les teintes et FilterVfdCellStructure.cs définit l’échantillonnage, la couverture, le verre et le halo. Le rendu logiciel et le shader de Direct3D 11, Vulkan et OpenGL reprennent ces calculs. Les filtres matriciels LED et VFD partagent les prises de la trame source. Avec restauration, netteté, simulation de signal ou effets stylistiques, le chemin VFD reste logiciel.

La sonde Windows Release a vérifié l’enregistrement du profil, les cellules rectangulaires et carrées et les phosphores bleu-vert et blanc. Les trois moteurs matériels ont activé le shader et leurs captures sont restées à trois niveaux par canal au plus du logiciel. La sonde a été supprimée et les DLL installées vérifiées par empreinte.

### Optique des matrices de points

FilterDotMatrix.cs et DotMatrixVideoShaderParameters.cs définissent les écrans réfléchissants, rétroéclairés et émissifs. Le rendu logiciel et matériel partagent les mêmes fonds, teintes, coefficients de transmission et poids d’échantillonnage que la branche source. Le dialogue et l’OSD reprennent les quatre réglages de EmulationDotMatrixSettingsBlock.cs. Les anciens profils qui spécifiaient la palette blanche sont chargés en gris neutre. Sans technologie enregistrée, le profil utilise le mode réfléchissant du C#.

La sonde Windows Release a rechargé la configuration pour chaque technologie et vérifié que les trois rendus diffèrent. Les captures Direct3D 11, Vulkan et OpenGL sont restées à trois niveaux par canal au plus du rendu logiciel. Les 30 DLL de catalogue, ainsi que les DLL de thème et de présentation déployées, correspondent aux sorties de compilation. La sonde a été supprimée. Les combinaisons avec restauration, netteté, simulation de signal ou effets stylistiques passent encore par le rendu logiciel.

### Optique des afficheurs à segments

EmulationSegmentDisplaySettingsBlock.cs expose la disposition, la technologie et la couleur. Le dialogue et l’OSD reprennent les palettes propres au LCD, à la LED et au VFD. Une couleur incompatible adopte la première valeur de sa palette. Les profils antérieurs utilisent la technologie LED par défaut. Le rendu logiciel reprend les fonds, le contraste d’émission, le halo et les formes d’extrémité forcées par FilterSegmentDisplay.cs et FilterSegmentDisplayEndShape.cs.

La sonde Windows Release hors écran a validé les trois rendus distincts, la persistance du profil et le passage au noir pour un choix LCD incompatible. La sonde a été supprimée et les DLL installées vérifiées par empreinte. La transmission des niveaux de segments natifs, la passe matérielle et la parité temporelle restent ouvertes.

### Segments natifs des machines

VideoFrame.cs associe deux niveaux rouges à une trame XRGB8888. Dans le SDK natif, IMachineNativeSegments expose ces niveaux pour la trame demandée sans modifier la structure VideoFrame ni la table virtuelle d’IEmulatedMachine. L’onglet machine transmet les niveaux à VideoSurface avec la trame. Le rendu logiciel reprend l’interprétation de FilterSegmentDisplay.cs et remet à zéro la persistance des segments simulés lorsqu’il compose des segments natifs.

La sonde Windows Release a vérifié une trame avec remplissage de ligne, le fond, les trois niveaux de segment, le rejet de niveaux inversés et le retour à l’analyse raster sur la même trame. La sonde a été supprimée. Les DLL Emulation, VideoPresentation et le thème déployés correspondent aux sorties de compilation. Le passage par un module chargé, le transport interprocessus et les shaders de segments restent à vérifier ou à porter.

### Shader des segments natifs

La passe matérielle utilise les niveaux rouges de la source XRGB8888 progressive, comme FilterSegmentDisplay.cs. Elle gère les optiques LCD, LED et VFD. Le halo VFD reprend les quatre prises voisines du shader C#. Les trames entrelacées et les configurations avec effets stylistiques passent par le traitement logiciel. Le rendu des segments reconstruits à partir d’une image raster reste logiciel.

Sous Windows Release, les captures Direct3D 11, Vulkan et OpenGL ont vérifié le fond, les trois niveaux et le halo VFD ainsi que le retour au rendu logiciel pour des niveaux invalides. Les quatre prises de référence s’écartent d’au plus deux niveaux par canal du rendu logiciel. La sonde a été supprimée et la DLL déployée vérifiée par empreinte.

### Afficheurs à segments reconstruits

Le filtre matériel reprend FilterSegmentDisplay.cs de la branche osd-support. La géométrie des segments est partagée avec le filtre logiciel. Le shader trouve le segment le plus proche, prélève les niveaux dans le raster et applique le temps de réponse et la rémanence avant la composition LED, LCD ou VFD. L’historique utilise les deux dernières trames admissibles et les horodatages du flux. Le traitement logiciel prend le relais quand les effets combinés ne sont pas pris en charge par cette passe.

Vérification Windows en Release sur Direct3D 11, Vulkan et OpenGL. La sonde a vérifié la sélection matérielle, les transitions clair et sombre sur quatre trames, la stabilité des captures répétées, la disposition à seize segments avec points et deux-points, ainsi que la remise à zéro. La DLL construite et la DLL distribuée ont la même empreinte. Le dossier de la sonde a été supprimé.

### Afficheurs à papier électronique

FilterEPaper.cs et EPaperRefresh.cs définissent les six modes et leur cadence. Le C++ reprend les pigments, la grille de dalle, la trace résiduelle et son effacement, ainsi que l’inversion des pixels modifiés au rafraîchissement complet. Les lecteurs JSON conservent le mode et la définition des anciens profils. Le dialogue et l’OSD présentent les deux réglages de EmulationEPaperSettingsBlock.cs et reprennent les libellés des catalogues C#.

Une sonde Windows Release a vérifié les six modes, l’encre jaune, la disparition de la trace entre deux rafraîchissements, l’inversion complète et la stabilité des captures répétées sur Direct3D 11, Vulkan, OpenGL et le moteur logiciel. Le dossier de la sonde a été supprimé.

### Projection sur les moteurs matériels

ProjectionVideoShader.cs définit l’échantillonnage à la résolution de la dalle, l’ouverture des cellules, la diffusion et la convergence CRT. Les paramètres optiques de ProjectionVideoParameters.cs sont partagés avec le rendu logiciel. Le filtre de connexion et la séparation colorée du DLP mono-puce préparent l’image avant le shader. Une combinaison avec un traitement de signal global, la netteté ou un effet stylistique utilise le moteur logiciel.

La sonde Windows Release a vérifié la frange DLP sur un front en mouvement, sa disparition sur le front immobile, la réflexion de la toile grise avec lumière ambiante, la diffusion du CRT et la fermeture de la surface sur Direct3D 11, Vulkan et OpenGL. La DLL compilée correspond à celle du paquet Release. Le dossier temporaire a été supprimé.

### Plasma sur les moteurs matériels

Les filtres plasma de osd-support pilotent le phosphore, la limitation par luminance moyenne, le tramage par phase et le seuil de noir. Le shader reprend la persistance sur la trame précédente et les structures Stripe, Waffle, Delta et Monochrome. La grille d’échantillonnage et le calcul de couverture sont partagés avec la projection. Le filtre de connexion prépare la source que lit le shader.

La vérification Windows Release sur Direct3D 11, Vulkan et OpenGL a contrôlé les quatre structures, la teinte ambrée, la différence de luminance entre une petite zone et l’écran entier, ainsi que la trace de la trame précédente quand le numéro de trame progresse avec un horodatage identique. Le dossier temporaire a été supprimé.

### Afficheur vectoriel natif et raster

FilterVectorDisplay.cs définit les profils de phosphore, les points et segments gaussiens, l’excitation d’un raster et la décroissance selon les horodatages. Le SDK expose les segments normalisés et le RVB linéaire au moyen d’une interface facultative jointe à la trame. Une liste native vide laisse décroître le phosphore. Le dialogue et l’OSD reprennent la couleur, la largeur de faisceau de −20 à 20 et la courbure. L’ancien choix Gris est chargé en Blanc.

VectorRasterVideoShader.cs définit la synthèse matérielle du raster. Les trois moteurs utilisent les mêmes paramètres de faisceau que le traitement logiciel. La boucle de netteté garde une seule expression d’échantillonnage pour réduire la compilation du shader Direct3D 11. Le rendu des segments natifs utilise le traitement logiciel, comme dans la branche C#.

La sonde Windows Release a vérifié une trame sans raster avec un segment et un point fixe, trois états de décroissance, les transitions entre raster et segments, la courbure et la capture répétée, sur les moteurs logiciel, Direct3D 11, Vulkan et OpenGL. La sonde temporaire a été supprimée.

### LCD, LCD à rétroéclairage LED et OLED sur les moteurs matériels

Les filtres C# FilterLcdDisplay.cs, FilterLedBacklitLcdDisplay.cs, FilterFixedPixelResponse.cs, FilterOledDisplay.cs et FilterOledPixelStructure.cs définissent les ouvertures de sous-pixels, la fuite lumineuse et le noir des dalles, la réponse temporelle, le rétroéclairage local et la limitation de luminance. Les rendus C++ partagent les paramètres des dalles avec le moteur logiciel. Le signal et les zones de rétroéclairage local sont préparés avant le shader. Les effets stylistiques et la netteté générale restent traités par le moteur logiciel lorsqu’ils sont combinés avec ces dalles.

La sonde Windows Release a comparé la réponse TN et VA à 5 ms, le noir du rétroéclairage direct et Mini LED, le limiteur WOLED et les motifs de sous-pixels sur Direct3D 11, Vulkan et OpenGL. Lors du premier essai Direct3D 11, le compilateur dépassait 30 s. Le shader d’affichage chargé à partir du HLSL utilise désormais un niveau de compilation rapide et garde le cache de bytecode. L’initialisation à froid a pris environ 11 s lors du nouvel essai. La sonde temporaire a été supprimée.

### Décodage du signal de dalle dans le shader

PanelSignalVideoShader.cs applique le décodeur CRT au signal synchronisé sur les LCD, les LCD à rétroéclairage périphérique ou direct et les projecteurs hors DLP mono-puce. Le shader C++ décode les trames présente et précédente à la résolution source et arrondit chaque canal avant les réglages d’image, suivant la branche C#. La désynchronisation reste calculée par trame côté CPU. Les zones LED, la mesure de luminance OLED et la séparation DLP mono-puce utilisent toujours leur source préparée côté CPU.

La sonde Windows Release a vérifié sur Direct3D 11, Vulkan et OpenGL le rendu numérique, la réception monochrome, les trois décodeurs composite PAL, NTSC et SECAM, et la différence de rendu en désynchronisation. Le build Release passe et la DLL déployée correspond au produit compilé. Le dossier de la sonde a été supprimé.

### Bytecode du shader d’affichage Direct3D 11

Le build Windows extrait le HLSL 5.0 du paquet Qt et appelle fxc avec le même réglage de compilation que le secours à l’exécution. Le fichier DXBC est intégré aux ressources du binaire avec le paquet Qt qui fournit le SPIR-V et la table des liaisons HLSL. Les builds sans fxc gardent la compilation de secours à l’exécution. Le shader demande explicitement le niveau zéro de la texture, qui n’a pas de niveaux supplémentaires. Cela évite les avertissements de gradients implicites dans les boucles. [Manuel QSB de Qt](https://doc.qt.io/qt-6/qtshadertools-qsb.html)

Le paquet Windows Release contient exactement le bytecode produit par fxc. L’initialisation de Direct3D 11 a pris environ 250 ms sur cette machine. Les essais de décodage et de désynchronisation passent sur Direct3D 11, Vulkan et OpenGL. fxc émet encore des avertissements X4000 dans le HLSL généré. La sonde a été supprimée.

### Dossiers de session possédés par un module

EmulationSessionDirectoryFunctions.cs fournit au module deux opérations, à appeler après destruction du cœur. Le SDK C++ reprend le nettoyage ordinaire et celui des dossiers générés à nom GUID, avec remise en écriture des fichiers ordinaires en lecture seule sous Windows. Le module fournit en plus sa racine de session, ce qui permet de refuser un chemin hors de ce dossier. Les liens dans le dossier sont retirés sans parcourir leur cible. La fermeture du runtime et l’appel du nettoyage restent sous la responsabilité du propriétaire, comme dans le C#.

La sonde Windows Release a vérifié les GUID avec tirets et au format compact, le nettoyage d’un fichier en lecture seule, le refus de la racine et des chemins externes, et la conservation d’un fichier atteint par une jonction. La DLL déployée correspond au build. La sonde a été supprimée.

### Constructeurs de réglages matériels

HardwareSettings reprend les blocs CPU, mémoire et résolution de EmulationHardwareSettingsFunctions.cs et les formats MHz, kHz, B et KiB. Les champs utilisent les descripteurs existants du SDK. Le format numérique MHz expose la précision minimale et maximale au moyen de QLocale. Les choix audio numériques conservent des identifiants invariants et affichent les valeurs dans la langue fournie. Les cinq libellés proviennent de EmulationCpu.resx, EmulationMemory.resx et EmulationVideo.resx dans les 30 catalogues. RAM reste dans Constants.

La sonde Windows Release a contrôlé les blocs facultatifs, le modèle CPU localisable, les noms et les valeurs des choix et interrupteurs, et les formats français. Elle a été supprimée. Les aides détaillées, la présentation compacte et l’éditeur numérique dédié de la branche C# restent à raccorder aux descripteurs.

### Éditeur numérique des réglages de module

Le champ Number suit EmulationSettingsFieldControlFunctions.Inputs.cs et ConstrainedTextBox.cs. Il utilise la plage existante du SDK, filtre les caractères et le collage, conserve la dernière valeur complète pendant une saisie partielle et applique le pas avec les flèches haut et bas. Le dialogue utilise la langue active pour le séparateur et normalise la valeur transmise au module. L’OSD conserve les catégories d’éditeur de MachineController.OsdModuleSettings.cs.

La sonde Qt Windows Release a vérifié la saisie française, les bornes, le retour à la dernière valeur valide, les pas au clavier et le rejet des collages incompatibles. La compilation Release du SDK et du thème a réussi. Le répertoire de la sonde a été supprimé.

### Aide et cartes compactes dans les réglages du module

IModuleSettingsPresentation expose les blocs compacts et les clés d’aide détaillée par champ sans changement des descripteurs existants. RetroMediaLab reprend le seuil de 760 unités logiques de EmulationSettingsLayout.cs et la limite de huit champs par carte CPU de EmulationModuleHardwareSettingsSection.cs. Une action d’information présente l’aide courte au survol et ouvre un encart défilant pour l’aide détaillée, selon EmulationSettingsFieldLabel.cs. Le thème utilise un menu Qt pour cet encart.

La sonde Qt Windows Release a vérifié le passage d’une colonne à deux, le retrait d’une carte masquée, le libellé accessible, les textes de l’encart et le comportement avec l’aide courte seule. Le SDK et le thème ont compilé et le dossier temporaire a été supprimé.

### Fichiers de configuration des machines

ConfigurationFileAccessFunctions.cs définit le chemin GUID/machine.json, l’ordre des documents et la conversion des chemins par rapport à la base. Le SDK C++ expose ces opérations et laisse au module l’interprétation de son schéma JSON. Sous Windows, la lecture et l’écriture utilisent le nom de mutex calculé selon la convention de la branche C#. QSaveFile gère le fichier temporaire et la publication atomique. Un échec de publication renvoie IoFailure et conserve le document précédent.

La sonde Windows Release a vérifié la création et le remplacement d’un document, les chemins internes et externes, l’ordre de lecture, l’omission d’un document invalide et l’annulation d’une écriture pendant l’attente du mutex. Le dossier temporaire a été supprimé.

### Messages structurés

Le contrat C# `EmulationErrorService` fournit les catégories, codes, cibles et contextes. Le port C++ garde la cause d’origine et laisse passer les exceptions déjà structurées et les annulations. L’ouverture d’une session et le démarrage direct transmettent leurs erreurs à la vue, qui affiche le message localisé et les médias requis lorsqu’ils sont fournis. Les 30 catalogues reprennent les chaînes d’`EmulationError.resx`.

La compilation Windows Release et la sonde native ont vérifié le code d’erreur, le contexte, la cause, le texte natif et l’annulation. L’affichage utilise les surfaces d’erreur Qt existantes. Le dialogue détaillé du C# et la vérification avec un module réel restent à couvrir.

Le thème RetroMediaLab affiche maintenant le dialogue d’erreur du socle avec un titre localisé, le message, le nom de configuration de la machine et les icônes dédupliquées des médias requis. Les messages de statut utilisent l’indicateur de la vue et les messages silencieux restent dans le diagnostic Qt. Windows Release et une sonde Qt hors écran ont vérifié les trois cibles, les libellés français, les deux icônes et la fermeture du dialogue. La sonde injecte un message structuré côté application. La validation de bout en bout avec un module de machine reste à faire.

### Commandes de médias et sauvegardes

Les exceptions du démarrage, y compris après recréation du moteur, de l’insertion, de l’éjection, de l’enregistrement de cassette et des sauvegardes traversent le contrat de messages. Les commandes de sauvegarde directe et rapide passent par la même session. L’interface affiche les clés du C# selon la catégorie. Les exceptions déjà structurées et les annulations conservent leur identité.

La lecture vérifiée des fichiers d’état distingue la corruption, une identité différente et les erreurs d’accès. Windows Release a compilé les commandes et une sonde native a vérifié les quatre résultats de lecture, dont la conservation de la cause d’une erreur d’accès. La vérification des commandes avec un module de machine reste ouverte.

La validation Windows Release charge une DLL de test via son manifeste et son export SDK. Elle couvre les exceptions à la création, au démarrage et à la recréation de la machine, un montage refusé, une annulation d’éjection qui préserve le média monté, une sauvegarde rapide en échec et une incompatibilité d’état signalée par le module. Une exception de média requis émise par la DLL arrive au dialogue Qt hors écran avec son titre français, le nom et l’icône du média. Le manifeste, la DLL et la sonde ont été supprimés après exécution. Ce contrôle valide le raccordement du socle avec un module synthétique.

### Commandes de transport de cassette

TapeTransportPresentationFunctions.cs définit les règles d’activation et MachineView.cs définit la face compacte, les glyphes, les couleurs et le clignotement. Le thème Qt dessine les six commandes dans la barre des médias. En pause, la touche Pause indique l’état et la commande Lecture ou Enregistrement en cours alterne entre opacité pleine et 25 % toutes les 400 ms. Le modèle partagé préserve l’état par défaut du contrat IEmulationTapeTransport lorsque le SDK ne renseigne pas l’opération en cours.

Windows Release et une sonde Qt hors écran ont vérifié les états, les dimensions, les couleurs actives et le changement d’opacité. La sonde a été retirée. L’essai du transport d’une cassette avec une machine reste à faire.

### Préparation lors de l’insertion d’un fichier

MachineController.InsertMediaAsync appelle PrepareMediaAsync avant de monter le fichier. Les adaptateurs de la branche C# reçoivent le répertoire des médias convertis et peuvent adapter le chemin et les paramètres. Le contrat facultatif C++ IModuleMediaPreparation reçoit la configuration, les services d’exécution et le média à préparer. Session valide le résultat avant de toucher au montage courant et l’interface utilise ce parcours pour ouvrir un fichier. Les parcours de création de cassette et de configuration des cartes mémoire conservent leur appel d’insertion directe comme dans le C#.

Windows Release et un module de test chargé par manifeste ont vérifié le chemin converti, les paramètres, la protection en écriture, le rejet d’un changement d’emplacement et le maintien du montage après refus ou annulation. Le module de test et ses fichiers ont été supprimés.

### Dix emplacements de sauvegarde par configuration

Chaque configuration a dix emplacements indexés par l’identifiant du module et le GUID de la configuration. Le premier conserve le chemin historique utilisé par les raccourcis rapides. La barre de commandes et le menu plein écran affichent les dix choix d’écriture et de lecture. La date distingue les emplacements occupés et seuls ceux-ci sont activés à la lecture. L’écriture utilise un fichier de travail de même extension et publie la sauvegarde atomiquement après réussite du module.

Windows Release a compilé l’interface. Une DLL de test a vérifié les dix écritures et relectures après réouverture, l’isolation entre deux configurations, la compatibilité du fichier historique, la conservation après échec et annulation, et le retrait des fichiers temporaires. L’exercice visuel des menus avec une machine réelle reste à faire.

### Fermeture de session en erreur

MachineSession.DisposeAsync retire toujours la session et libère la machine en fin d’arrêt. Le port C++ retire la session de l’hôte avant de tenter l’arrêt, coupe sa sortie audio et limite l’arrêt de fermeture à une tentative même si la DLL lève une exception. La machine est détruite à la libération du dernier propriétaire et garde sa bibliothèque chargée jusque-là. Le thème retire l’onglet et signale l’échec dans la vue.

Windows Release et une DLL de test ont vérifié le retrait après exception, la réouverture de la configuration et l’ordre arrêt–destruction. L’arrêt à la destruction de l’hôte a aussi été vérifié avec une session encore référencée. La DLL et la sonde ont été supprimées.

La sonde Qt hors écran a chargé la DLL du thème Windows Release et ouvert une machine de test avec un état dans l’emplacement 10. Les menus de la barre de commandes et du plein écran présentent chacun les dix numéros pour les deux actions. Les dix choix sont actifs en sauvegarde et seul le dixième est actif en chargement. La date du fichier occupé a été vérifiée dans le menu de la barre. Le module et les ressources de la sonde ont été supprimés.

### Échec au démarrage d’une machine

Comme dans MachineSession.PowerOnAsync et DisposeCurrentMachineAsync, un démarrage en erreur provoque une tentative d’arrêt puis la destruction du moteur. Le C++ conserve les emplacements de médias dans la vue à l’arrêt et recrée le moteur au prochain démarrage. L’interface reçoit l’erreur du démarrage, y compris lorsque l’arrêt échoue aussi.

La Release Windows a compilé. Un module temporaire a confirmé la destruction après les deux exceptions, la conservation des emplacements de médias, puis le démarrage d’un moteur neuf dans la même session.

### Extinction d’une machine

PowerOffAsync détruit le moteur à l’extinction. Le C++ libère la sortie audio et le moteur, conserve les emplacements et les médias de la session, puis crée un moteur neuf au rallumage. Si l’arrêt lève une exception, la destruction a lieu et l’erreur remonte à l’interface.

Release Windows compilée. Un module temporaire a vérifié l’extinction normale, la destruction après une erreur d’arrêt, le remontage du média et la fermeture sans second appel au moteur détruit.

### Mécanique et pistes dans le SDK natif

Le vocabulaire du service MediaEngine pour les lecteurs est Floppy, HardDisk, Optical et Tape. Le SDK 2.3 expose la mécanique et permet de partager la même instance entre le contrôleur et son image, comme dans EmulationMediaProcessingService.Drives.cs et .Disks.cs. Le module pilote les signaux mécaniques et peut produire du PCM mono depuis son horloge. Le relais audio hôte est décrit ci-dessous.

Les types SDK couvrent également les métadonnées, la géométrie des supports à blocs, les diagnostics des secteurs et le formatage, la lecture et la réécriture partielle des pistes. Les signatures des méthodes 2.2 sont conservées. Un module utilisant ces nouveaux contrats doit déclarer une plage de versions qui inclut le SDK 2.3.

La Release Windows a compilé. Un module temporaire a vérifié la mécanique partagée, les neuf secteurs de la piste d’une image 720 Kio, la concordance entre la lecture simple et détaillée et la lecture d’une révolution MFM. La sonde et son image ont été supprimées.

### Sortie hôte des bruitages mécaniques

Le SDK expose createHostDriveMechanics pour le cas hostAudio de EmulationMediaProcessingService.Drives.cs. Les notifications de MediaDriveService réveillent la sortie Qt à la première activité. Les fragments de 20 ms sont mélangés hors du fil Qt avec la protection de MediaEngine. La sortie essaie le mono à 44,1 kHz, puis le format du périphérique en PCM 16 bits. Elle vide son tampon durant une seconde au maximum après la dernière activité. La mécanique reste possédée par le module, comme dans MediaDriveAudioController.cs.

La Release Windows a compilé et un module temporaire a émis deux événements de cassette depuis un fil secondaire. Avec la sortie Windows disponible, le bruitage hôte s’est vidé tandis que celui destiné au rendu PCM par le module est resté en attente. La sonde a aussi fermé la session avec la sortie encore en cours de vidange. Tous ses fichiers ont été supprimés.

### Préparation des médias dans le SDK

Les opérations de IEmulationMediaProcessing.cs sont exposées par le service média du SDK natif. Les modules peuvent préparer des CUE et leurs fichiers WAVE de session, interroger les tailles optiques et les extensions, lire et écrire les signaux de bandes, créer des disques durs et convertir les SCP. Les blocs de bande conservent position, données, intégrité et métadonnées. Les profils, le rendu et la protection du mélange des sons mécaniques utilisent le moteur existant.

Release Windows compilée. Un module temporaire a vérifié le changement de fichier CUE avec préservation du document initial et de son titre, les tailles optiques, les catalogues et la lecture après écriture d’un signal PCM WAV. La sonde et tous ses fichiers ont été supprimés.

### Recréation après changement de média

MachineSession.RecreateRunningMachineAsync détruit le moteur avant d’appeler la fabrique. Le C++ suit cet ordre lorsqu’un montage ou une réinitialisation exige une recréation. Si la fabrique, le démarrage ou l’enregistrement échoue, la session reste à l’arrêt avec sa précédente configuration de montage. Le moteur de remplacement est arrêté avant sa destruction.

La Release Windows a compilé. Une DLL temporaire a simulé une ressource exclusive, puis un échec de démarrage pendant une réinitialisation à froid. Elle a confirmé l’ordre de destruction, la conservation du montage et la reprise au démarrage suivant.

### Présentation des erreurs de session et de réglages

Les parcours d’ouverture, d’édition et de fermeture transmettent les erreurs structurées au dialogue du thème. Le nom donné par l’utilisateur reste prioritaire. Quand la configuration est anonyme, le nom de la machine est résolu dans la langue active, comme dans le sélecteur. Le diagnostic Qt utilise la gravité et consigne la cause native disponible, suivant les informations utilisées par ControlErrorPresenter.cs.

La Release Windows compile. Une sonde Qt hors écran avec un module chargé a contrôlé le nom traduit dans l’onglet et dans l’erreur de média requis déclenchée par le bouton de démarrage. La sonde a été supprimée.

### Clavier et manettes dans l’OSD sans pause

MachineInputController.SetOsdFocus laisse circuler les entrées de jeu pendant l’OSD sans pause. Le thème C++ maintient le clavier, les manettes et leur retour de force dans ce mode. Les commandes de navigation reconnues sont retirées du clavier et de la manette jusqu’au relâchement. Le filtre couvre boutons, axes normalisés et contrôles bruts comme dans OsdControllerReleaseFilter. Le routage de la souris hors des panneaux reste à rapprocher du C#.

Release Windows compilée. Une sonde hors écran a envoyé A et Flèche bas au thème avec un module temporaire. La machine a reçu A, la flèche de navigation a été filtrée et les relâchements ont vidé les entrées. Une manette synthétique a vérifié le masque d’un bouton et d’un axe brut. La sonde a été supprimée.

### Souris dans l’OSD sans pause

Comme dans le C#, les panneaux et les contrôles qui capturent la souris gardent la priorité. Hors des panneaux, l’émulation reçoit le déplacement relatif, les boutons et les deux axes de molette. Un bouton utilisé sur le menu reste masqué pour le jeu jusqu’à son relâchement. Qt fournit les déplacements depuis la position du pointeur pendant l’OSD.

La sonde du thème Windows Release a vérifié le passage des clics et de la molette hors des panneaux, le maintien de l’OSD ouvert, le filtrage à l’entrée du menu et la reprise après relâchement. Le dossier de la sonde a été supprimé.

### Informations machine dans l’OSD

Le panneau central reprend les rubriques de MachineController.OsdModuleSettings.cs dans osd-support. Il affiche les champs informatifs du module en lecture seule et masque le menu tant que le panneau est ouvert. Les rubriques sans information visible sont masquées. Haut et Bas font défiler les informations. Entrée, Échap ou Gauche ramènent au menu.

Windows Release a compilé. La sonde du thème a vérifié le centrage, la valeur fournie par le module, l’absence des champs masqués ou modifiables et le retour au menu. La sonde a été supprimée.

### Transparence de l’OSD

Le réglage global est accessible depuis le menu plein écran et les préférences d’émulation. La plage de 0 à 100 % et la valeur initiale de 5 % reprennent MachineController.cs et AppSettings.cs de la branche osd-support. Le profil utilisateur enregistre fullscreenOsdTransparency. L’aperçu agit sur le fond du menu, de l’éditeur, du panneau central, des statistiques et des indicateurs plein écran, en gardant les textes lisibles. Les libellés proviennent d’EmulationVideo.resx dans les trente catalogues source. Le popup de volume dédié du C# reste à porter.

Windows Release a compilé. La sonde du thème a vérifié la valeur initiale, l’aperçu du fond à 100 %, la sauvegarde et la relecture du JSON, puis l’enregistrement de 0 % en fermant le menu. La sonde a été supprimée.

### Volume de la machine en plein écran

Le menu Audio reprend le réglage de 0 à 100 % de MachineController.OsdModuleSettings.cs. Les raccourcis configurables Ctrl+Maj+Gauche et Ctrl+Maj+Droite changent le volume d’un point et affichent pendant deux secondes l’indicateur de FullscreenVolumeOsd.cs, centré en bas de l’écran. Le fond suit la transparence de l’OSD. Le volume s’applique à la sortie audio de la session et une valeur positive réactive le son s’il était coupé. Le mixeur des canaux et l’indicateur permanent de sourdine restent distincts.

Le build Windows Release et une sonde du thème hors écran ont vérifié le réglage dans le menu Audio, l’action du raccourci, la valeur et la transparence de l’indicateur puis sa disparition. La sonde a été supprimée.

### Sourdine en plein écran

FullscreenMuteOsd.cs place un pictogramme blanc ombré à 44 pixels logiques des bords inférieur et droit. Le portage utilise le pictogramme du thème et le libellé accessible du C#. L’indicateur persiste tant que la sourdine est active ou que le gain audio est nul, puis disparaît quand le son est rétabli. Un gain positif très faible peut s’arrondir à 0 % dans le réglage de volume tout en restant audible.

Windows Release a compilé. La sonde du thème a contrôlé la sourdine, le gain nul, le placement, la conservation du focus à l’affichage de la sourdine, la persistance après fermeture de l’indicateur de volume et la disparition en quittant le plein écran. Le backend hors écran a requis de réactiver la fenêtre avant le raccourci suivant après l’expiration de l’indicateur de volume. Le comportement du focus sur un bureau Windows reste à confirmer. La sonde a été supprimée.

### Mixeur audio de l’OSD

Le menu Audio ouvre un panneau central de curseurs verticaux comme dans FullscreenOsdWindow.CenterPanel.cs. Le volume général précède les réglages numériques du module, repérés par leur identifiant ou les groupes de mixage déclarés via IModuleAudioMixerBlocks. Gauche et Droite sélectionnent une voie, Haut et Bas ajustent sa valeur, et Entrée ou Échap reviennent au menu Audio. Le déplacement à la souris valide un réglage de module au relâchement pour limiter les écritures. Le volume général réagit pendant le déplacement. Un défilement horizontal garde accessibles les voies qui dépassent la largeur disponible.

Windows Release a compilé. Une sonde hors écran a vérifié le volume général, la valeur d’un canal fourni par un module temporaire, sa mise à jour par les flèches, la valeur enregistrée dans la configuration de session, l’exclusion d’un canal masqué et le retour au menu. La sonde a été supprimée.

### Statistiques en plein écran

FullscreenStatisticsOsd.cs et MachinePresentationFunctions.cs définissent cinq lignes colorées en haut à droite. Le portage affiche le moteur réellement actif, la taille de l’image, les images par seconde mesurées, la fréquence attendue et la vitesse d’exécution du processeur. La fréquence nominale du processeur sert de repli avant la mesure. Le panneau reprend le réglage de transparence de l’OSD et la préférence existante d’affichage des statistiques.

Windows Release a compilé. La sonde avec une machine temporaire a vérifié les cinq lignes en français, le format de l’image, la fréquence vidéo, la présence de la fréquence CPU, l’alignement et la désactivation du panneau. La sonde a été supprimée.

### Fermeture automatique et capture dans l’OSD

Le délai de 15 secondes suit FullscreenOsdWindow.cs. La capture d’un curseur ou d’une barre de défilement du panneau central maintient le menu ouvert. Les événements de souris des contrôles relancent le délai, y compris au relâchement. Un clic dans la zone vidéo laisse le menu ouvert. En mode sans pause, cette zone transmet toujours les événements de souris au jeu.

Windows Release a compilé. Les sondes du thème ont vérifié l’expiration du délai avec et sans capture dans l’éditeur et le mixeur, le clic hors menu et le routage du pointeur en mode sans pause. Les sondes ont été supprimées.

Les canaux à choix discrets reprennent OsdModuleEditor dans MachineController.OsdModuleSettings.cs. Les curseurs suivent l’ordre et affichent les libellés du module. La valeur sauvegardée est l’identifiant du choix. Les contrôles à choix hors du mixeur conservent leur présentation. La sonde Windows Release a vérifié les deux valeurs d’un canal à choix, la limite du curseur, le libellé et les identifiants dans la configuration de la session.

### Navigation dans les listes en lecture seule de l’OSD

FullscreenOsdWindow.Menu.cs déplace la sélection sur toutes les entrées visibles, puis vérifie séparément si l’entrée peut être activée. Le C++ reprend ce comportement, notamment pour parcourir et faire défiler les affectations de touches et les commandes indisponibles. Les séparateurs et les lignes masquées sont sautés. L’éditeur continue de naviguer uniquement entre les réglages modifiables.

Le build Windows Release et la sonde hors écran ont vérifié le défilement de trente lignes, le saut d’une entrée masquée et d’un séparateur ainsi que le blocage de l’activation sur une ligne indisponible. La sonde a été supprimée.

### Répétition des raccourcis et du mixeur plein écran

Le maintien du volume et des directions en édition reprend le délai initial de 300 ms et l’intervalle de 35 ms de `MachineInputController.cs`. Le mixeur est considéré comme un éditeur, comme dans `FullscreenOsdWindow.Menu.cs`. Le volume et la sourdine restent utilisables lorsque l’OSD a le focus.

La sonde Windows Release a exercé la touche Bas dans le mixeur et Ctrl+Maj+Gauche dans le menu, puis vérifié que le relâchement arrête les répétitions. La sonde a été supprimée. Le backend Qt hors écran désactive l’OSD lors de l’apparition de l’indicateur de volume, ce qui limite cette vérification au backend Windows.


### Portée des raccourcis du plein écran

Le menu applique la liste de `EmulationShortcutMap.cs` aux raccourcis Qt et aux raccourcis physiques. Quand le menu a le focus, seules ses commandes, la sourdine et le volume peuvent être déclenchés. Les raccourcis de session reprennent leur portée lorsque le menu est fermé.

La sonde Windows Release a vérifié que Ctrl+Maj+F11 laisse le menu et le plein écran ouverts, que Ctrl+Maj+Gauche ajuste le volume, puis que Ctrl+Maj+F11 quitte le plein écran après fermeture du menu avec Échap. La sonde a été supprimée.


### Retour à la machine après un OSD avec pause

Comme dans `MachineInputController.SetOsdFocus`, la sortie du menu avec pause efface les touches ordinaires suivies par les raccourcis. Une touche dont le relâchement ne parvient pas à la vue ne bloque plus le prochain raccourci clavier. Les boutons de souris encore enfoncés rejoignent le masque existant de reprise du pointeur. Le mode sans pause conserve les entrées en cours.

La sonde Windows Release a ouvert le menu avec pause, laissé une touche enfoncée et fermé le menu. La machine a repris et le volume a répondu au maintien de Ctrl+Maj+Gauche, puis s’est arrêté au relâchement. La partie souris a été compilée. La sonde a été supprimée.


### Priorité des raccourcis sur les entrées de la machine

Les touches du volume et des bascules du plein écran sont filtrées avant l’exécution de la commande, comme dans `MachineInputController.HandleKeyDown`. Les modificateurs déjà transmis sont retirés de l’état reçu par la machine tant que la combinaison est maintenue. Les raccourcis de bascule du menu utilisent également le filtre des sources de souris et de manette. Les entrées ordinaires restent utilisables dans le menu sans pause.

La sonde Windows Release a vérifié le réglage du volume hors du menu, l’ouverture sans pause depuis le pavé numérique, la remise d’un état clavier filtré à la machine et la transmission de la touche A dans les deux vues. Les sources physiques de souris et de manette ont été compilées. La sonde a été supprimée.


### Alias des commandes de manette

Le routage des raccourcis et le filtre des entrées du jeu reconnaissent les alias des boutons et gâchettes standard définis dans `EmulationInputMappingFunctions.cs`. L’éditeur conserve les noms canoniques pour les nouvelles affectations. La sonde a vérifié les quatorze paires de noms, les pressions analogiques de L2 et R2, les entrées capturées pour le filtrage, la sélection du périphérique et l’absence de doublons dans la capture. Le build Windows Release a réussi. La sonde a été supprimée.


### Capture des axes bruts

La capture reconnaît un déplacement de 0,35 depuis la position initiale, d’après `ControllerBindingCaptureFunctions.cs`. Le signe du déplacement choisit le sens. Le maintien et la reprise utilisent la zone neutre des axes bruts, comme `OsdControllerReleaseFilter.cs`. Les noms d’axes bruts numérotés sont reconnus indépendamment de la casse.

La compilation Windows Release et la sonde avec des états simulés ont vérifié les deux sens depuis des positions décentrées, les mouvements sous le seuil, le retour au repos et la zone neutre utilisée par le filtre. La sonde a été supprimée.


### Boutons système des manettes

Le backend Windows reprend la réception des états Guide et Share de `GameInputDeviceMonitor.cs`. Il publie les boutons pris en charge dans les contrôles nommés du périphérique, retire leurs états à la déconnexion et attend la fin des callbacks avant de libérer GameInput. La capture enregistre les noms XboxButton et Share. Guide est accepté comme alias de XboxButton. La voie RawGameController conserve son mappage XboxButton existant. [Microsoft GameInput v2](https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/input/gameinput-v2/interfaces/igameinput/methods/igameinput_registersystembuttoncallback-v2)

Windows Release a compilé. La sonde avec des états simulés a vérifié la capture en présence de contrôles bruts, le nom retenu par le filtre, l’alias, le relâchement et le ciblage de la manette. La réception physique des événements reste à vérifier sur une manette compatible. La sonde a été supprimée.


Le service InputProcessing lit aussi les boutons système dans les contrôles nommés fournis à la machine, avec les alias Guide et XboxButton. Les adaptateurs qui exposent ces boutons dans leur masque numérique continuent de fonctionner. La sonde liée à Emulation.dll en Windows Release a vérifié les commandes simples, les combinaisons, le ciblage du périphérique, le relâchement et la lecture du masque numérique. La sonde a été supprimée.


### Système de fichiers virtuel par défaut

L’environnement d’un cœur natif fournit un VFS à la demande, comme `ExternalCoreEnvironmentCallbackHost.cs`. Le fournisseur personnalisé conserve la priorité et reste détenu par l’appelant. La session libère son propre fournisseur après la désinitialisation du cœur.

La sonde avec un cœur natif dans la Release Windows a vérifié le refus d’une version inconnue, la lecture via la table par défaut, la disponibilité du fichier pendant la désinitialisation et la fermeture du fournisseur ensuite. Le fournisseur personnalisé reste utilisable après l’arrêt de la session. La sonde a été supprimée.

### Accès au système de fichiers virtuel natif

Le mode demandé par le cœur détermine les droits du fichier. Une écriture en mode mise à jour conserve les données existantes. Le partage de suppression Windows suit ExternalCoreVirtualFileSystem.cs du C#. La vérification Release sous Windows a couvert les accès en écriture seule, le maintien des octets non modifiés et le renommage puis la suppression avec un descripteur ouvert.

[CreateFileW et modes de partage](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) · [QFile et possession du descripteur](https://doc.qt.io/qt-6/qfile.html)

Le VFS ouvre aussi les chemins absolus longs sous Windows, en ajoutant le préfixe d’extension lors de l’appel Win32. Le chemin communiqué au cœur conserve sa forme d’origine. La sonde Windows Release a écrit puis relu un nom Unicode au-delà de MAX_PATH sur le disque local et a été supprimée. Les chemins UNC n’ont pas été testés sur un partage. [Limites des chemins Windows](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation)

### Fermeture de la préparation OpenGL

La surface possède la tâche de compilation du contexte partagé. À la fermeture, au changement de moteur et à la destruction du contexte, elle demande l’interruption et attend la fin de la tâche avant de libérer les ressources. Le pilote termine la compilation déjà engagée. Les réponses mises en file par un contexte détruit sont ignorées. [QThread](https://doc.qt.io/qt-6/qthread.html) · [QOffscreenSurface](https://doc.qt.io/qt-6/qoffscreensurface.html)

Windows Release a compilé. La vérification avec une surface Qt OpenGL a couvert la fin normale, le passage au moteur logiciel en cours de préparation et la destruction de la dernière surface pendant une préparation. La sonde a été supprimée.

### Aides dans les réglages de stockage

Les panneaux de configuration des lecteurs et cartouches reprennent la présentation de StorageConfigurationFieldFunctions.Present de la branche osd-support pour les aides courtes et détaillées. Les champs informatifs affichent le libellé du choix dans la langue active, selon le même traitement que les réglages de machine. Le build Windows Release a compilé les panneaux. La vérification visuelle avec un module déclarant ces réglages reste à faire.

### Saisie IPv4 dans les réglages

L’éditeur IPv4 reprend CreateIpv4 et ConstrainedTextBox sur osd-support. Il filtre les quatre octets décimaux, le collage et les zéros initiaux, en gardant la dernière adresse complète pendant la saisie. Qt peut ainsi finir une saisie partielle, et le dialogue applique la valeur complète. [QLineEdit](https://doc.qt.io/qt-6/qlineedit.html)

Windows Release a compilé. La sonde Qt hors écran a vérifié les collages valides et invalides, la saisie partielle, la restauration par Entrée et les bornes 0 et 255. Elle a été supprimée.

### Ouverture de dossier dans les réglages de machine

Le bouton Ouvrir le dossier reprend CreateOpenDirectory de la branche osd-support. Le SDK C++ reçoit le chemin absolu dans la description du module. Le thème crée le répertoire au clic et demande son ouverture à l’hôte. Le bouton ne fournit aucune valeur à la sauvegarde des réglages. Les 30 catalogues reprennent Common.OpenFolder de Actions.resx. Windows Release a compilé le socle et ses catalogues. L’ouverture depuis une machine réelle reste à vérifier lors du portage des modules.

### Indication de préparation lors des changements de vue

Le chargement reste lié au moteur, comme dans MachineController.VideoShaderLoadingChanged de la branche osd-support. Masquer la surface ou effacer une trame ne termine plus cet état. Le thème masque le badge avec la surface et le réaffiche à son retour si le moteur travaille encore. Windows Release a compilé. La sonde OpenGL a vérifié le masquage, l’effacement, la réapparition de la surface pendant la préparation et la fin normale. Elle a été supprimée.

### Réinitialisation des entrées après modification de session

La vue reprend MachineInputController.RefreshBindings de la branche osd-support. Après la sauvegarde des affectations ou le remplacement du moteur, elle remet à zéro la saisie, les raccourcis et l’état des manettes avant le prochain relevé des périphériques. La capture de la souris est relâchée si la politique du moteur ne la permet plus. Windows Release a compilé. La transition avec une machine réelle et des touches maintenues reste à vérifier.

### Restauration des médias au premier démarrage

MachineSession, dans osd-support, expose les médias déclarés insérés dès l’ouverture. Le socle C++ utilise la même liste pour la barre des médias, la synchronisation avant le premier démarrage et la recréation. La Release Windows a compilé. Une DLL temporaire a vérifié le montage déclaré, l’éjection d’un montage initial marqué éjecté et la répétition au redémarrage. La sonde a été supprimée.

MachineController.Open.cs passe runtime.MountedMedia à runtime.CreateMachine lors de l'ouverture. MachineSession conserve les médias marqués insérés, appelle Lifecycle.StartAsync à la mise sous tension et transmet les montages à la fabrique lorsqu'elle recrée la machine. Dans Acorn/Common/Services/Engine.cs, la fabrique transforme la liste reçue en médias de la configuration de la machine. Le socle C++ emploie createRuntimeWithMedia dès l’ouverture et lors de la recréation lorsque le module fournit la fabrique avec son contrat de stockage. Les modules qui fournissent seulement la configuration de stockage continuent à recevoir les montages avant Start. Windows Release et les deux variantes de la DLL temporaire ont vérifié les deux parcours, avec un lecteur déclaré éjecté. Les DLL temporaires ont été supprimées.

### API graphique de la configuration d’exécution

EmulationVideoConfigurationFunctions et MachineController.OsdVideo.cs sur osd-support transmettent le moteur du profil au module avant de créer le moteur d’émulation ou de calculer ses options à chaud. Le SDK C++ propose la même interface facultative et les valeurs Automatique, OpenGL, Vulkan et Direct3D 11. Le dialogue et l’OSD transmettent les changements de profil à la session. La configuration d’exécution dérivée ne remplace pas la configuration sauvegardée.

Windows Release a compilé. Une DLL temporaire a vérifié OpenGL à l’ouverture, le passage de l’option modifiable à chaud vers Vulkan, l’application différée de l’option de démarrage, la recréation avec la nouvelle API et la conservation de la configuration du profil. La DLL temporaire a été supprimée.

### Description des réglages selon le moteur vidéo

EmulationModuleSettingsSection.Layout.cs applique le moteur du profil à la configuration avant Describe. L’éditeur C++ utilise la même préparation que la fabrique pour obtenir les champs, leurs plages et leurs règles. L’OSD actualise ses descriptions au changement de moteur. Le stockage de la configuration continue de passer par la sauvegarde habituelle. La compilation et le déploiement Windows Release ont réussi.

Les modifications unitaires de l’OSD évaluent les dépendances avec la configuration préparée pour le moteur vidéo actif, comme lors de la description des champs. Seules les modifications de réglages résultantes sont transmises à la sauvegarde. La Release Windows a compilé et a été déployée localement.

Dans EmulationModuleSettingsSection.Layout.cs, choisir le moteur capture les valeurs de l’éditeur, enregistre le profil et reconstruit les réglages. L’onglet Vidéo du C++ permet maintenant cette sélection et reste sélectionné après reconstruction. La session active reçoit aussi le nouveau moteur. Les trois commandes vidéo partagent la liste des moteurs disponibles. La compilation et le déploiement Windows Release ont réussi.

Le sélecteur du moteur respecte la visibilité de Vidéo déclarée par le module, comme dans EmulationModuleSettingsSection.Layout.cs. Les aides courte et détaillée reprennent EmulationHelp.resx pour la base et les 29 langues. Les valeurs JSON ont été comparées aux ressources sources. La Release Windows a compilé et a été déployée localement.

L’éditeur de machine applique les critères Mémoire d’EmulationVideoConfigurationFunctions.cs pour présenter les réglages de mémoire d’affichage et de matériel vidéo dans l’onglet Vidéo. Le déplacement conserve les identifiants, les groupes d’origine et les dépendances des champs. Mémoire conserve ces réglages si Vidéo est masqué ou absent. La Release Windows a compilé et a été déployée localement.

EmulationVideoConfigurationFunctions.cs et EmulationVideoProcessingSettingsSection.Layout.cs regroupent les commandes de matériel vidéo à droite des réglages de rendu. Le thème C++ présente ce volet avec la mémoire vidéo, les contrôleurs et les jeux de puces identifiés par les mêmes critères. Les règles de visibilité restent calculées par groupe et par champ. Le titre reprend les ressources C# des 29 langues. La Release Windows a compilé et a été déployée localement.

Dans osd-support, BuildVideoSettingsTab rend le traitement vidéo accessible dans les réglages de la configuration. Le port C++ réutilise les contrôles du dialogue vidéo de session et les rend accessibles depuis Vidéo par un bouton dédié. Les changements sont enregistrés dans le profil de la configuration et transmis à la session si elle est ouverte. Annuler le dialogue de traitement abandonne ses modifications. La Release Windows a compilé et a été déployée localement.

Le dialogue de traitement C++ reprend les deux onglets communs Affichage et Image d’EmulationVideoProcessingSettingsSection.cs. Le choix de technologie est dans Affichage. Les ajustements, la restauration, le désentrelacement et les options propres à la technologie sont dans Image. Le dialogue conserve également les commandes de signal et les effets sous forme de groupes dans Image. Les libellés des onglets reprennent les catalogues C# des 29 langues. La Release Windows a compilé et a été déployée localement.

Dans osd-support, les changements de traitement mettent à jour le profil vidéo, tandis qu’un nouveau moteur reconstruit les réglages du module. Le dialogue C++ enregistre les changements de traitement sur place et conserve les éditions en cours dans la fenêtre de machine. Le changement de moteur reconstruit les réglages. La Release Windows a compilé et a été déployée localement.

MachineController.ApplyConfigurationAsync et MachineSession.UpdateMachineFactoryAsync actualisent les médias de session d’après la nouvelle configuration. Après une sauvegarde du stockage, le C++ actualise la liste des montages et écarte les emplacements qui ne sont plus configurés. La barre des médias et les descriptions de l’OSD sont rafraîchies après la sauvegarde. La prochaine mise sous tension recrée le moteur à partir de la configuration enregistrée. La Release Windows a compilé et a été déployée localement.

La sonde Windows Release a sauvegardé un nouvel emplacement pendant que la machine de test tournait. La liste de session reflète le nouvel emplacement, le moteur courant conserve son média jusqu’à l’arrêt et le moteur recréé monte uniquement le nouveau média au démarrage. La DLL, le programme de test et leur dossier temporaire ont été supprimés.

Dans osd-support, RebuildMediaDevices utilise les lecteurs exposés par la nouvelle configuration de la session. Le C++ présente les lecteurs déclarés par le module et activés dans la configuration. Les commandes d’insertion consultent la même liste. La sonde Windows Release a vérifié le passage de deux lecteurs à un, le refus d’insérer dans le lecteur retiré et le montage au redémarrage. La sonde a été supprimée. La Release Windows a compilé et a été déployée.

Sur osd-support, la sauvegarde d’un réglage de module et le changement de moteur vidéo passent par MachineController.ApplyConfigurationAsync, qui renouvelle les médias exposés. Le C++ recalcule les montages dans les deux cas et actualise la barre après le changement de moteur. Un module temporaire Windows Release a fait varier les lecteurs avec un réglage, puis avec le moteur vidéo. La vérification couvre la liste de session, le refus d’insertion dans un lecteur retiré et le remontage au redémarrage. Le dossier de la sonde a été supprimé.

EmulationMachineRuntimeService utilise la configuration vidéo préparée par le module pour décrire les lecteurs. Le port applique cette configuration à la description et à la validation des lecteurs en session et dans les réglages d’une machine arrêtée. La barre reconstruit aussi ses commandes si les propriétés du lecteur changent à identifiant égal. La Release Windows et la sonde SDK vérifient la description selon le moteur, les deux chemins de sauvegarde et le remontage. La sonde a été supprimée.

MachineController et TapeTransportPresentationFunctions limitent l’enregistrement à une machine sous tension, à un lecteur configuré et à un transport qui propose Record. Session::recordTape vérifie ces conditions à son point d’entrée avant de créer un fichier, ainsi que la présence du lecteur dans le moteur courant et l’annulation en cours. Le nettoyage d’un enregistrement déjà actif reste piloté par les commandes de transport. La Release Windows a compilé et a été déployée localement.

Le C# conserve les lecteurs décrits par la configuration hors de la boucle d’actualisation des voyants. La session C++ conserve maintenant la description des lecteurs pour les interrogations répétées. Le module est de nouveau interrogé après un changement de configuration, de moteur vidéo ou une recréation de la machine. La sonde Windows Release a vérifié les trois cas et la stabilité entre appels. Elle a été supprimée. La Release Windows a compilé et a été déployée.

La sonde Qt hors écran a chargé le thème Windows Release avec un module SDK temporaire. Elle a vérifié la présence des lecteurs déclarés, le retrait d’un lecteur après un changement de configuration, la section du matériel vidéo et les onglets Affichage et Image. Le dossier de la sonde a été supprimé.

Le relevé périodique des boutons de souris suit MachineInputController.Publish dans osd-support. Un panneau de l’OSD sans pause peut prendre la priorité sous un pointeur immobile. Un bouton absorbé reste masqué jusqu’à son relâchement et les événements de mouvement conservent leur propre delta. La Release Windows a compilé et a été déployée. Le scénario avec une souris physique reste à vérifier.

### Alignement sur les dernières modifications d’osd-support

Le commit C# 1c797743d retire le grain, le VHS, l’aberration chromatique, le halo et le sépia. Le C++ a retiré les filtres, leurs catalogues et leurs contrôles. Les groupes d’effets déclarés par les modules restent accessibles dans l’OSD. Un ancien profil est chargé normalement et perd sa section stylistique lorsqu’il est réécrit. Le contrat commun reprend aussi SegaSc3000Caret (262) et SegaSc3000Reset (263) du commit e5271d13f. Windows Release a compilé et a été déployée. Une sonde a vérifié les identifiants Qt, le rechargement d’un ancien profil et la conservation des autres paramètres. La sonde a été supprimée.

### Affectations clavier déclarées par une machine

Le contrat facultatif IMachineKeyboardBindings reprend la propriété KeyboardBindings de IEmulationInput. L’application traite les sources composites au moment de publier les entrées, comme MachineInputController.Publish. Une affectation explicite retire le caractère hôte de la touche cible. Les sources physiques déjà affectées à un raccourci global sont écartées et le masque du menu plein écran est appliqué à la souris et aux manettes. Windows Release a compilé et a été déployée. La sonde du code partagé a vérifié une combinaison clavier et souris, la molette, une manette nommée, l’exclusion d’une source réservée et la conservation du caractère d’une touche ordinaire. La sonde a été supprimée. La validation avec un module réel et ses périphériques reste ouverte.

La sonde Qt hors écran a ensuite exercé le thème Windows Release avec une machine temporaire qui enregistrait les instantanés reçus. Elle a vérifié que la combinaison clavier et molette transmet la commande à la machine, que le relevé suivant la relâche en gardant la touche ordinaire, et que la molette affectée au volume global ne déclenche pas la commande concurrente. Le défilement utilisé comme raccourci clavier hors capture ne devient pas un déplacement de la souris émulée. La sonde a été supprimée.

### Installation locale des cœurs

CoreReleases reprend les opérations de chemins, d’extraction et de sélection d’une installation active de EmulationCoreReleaseFunctions.cs. La destination est fixée par le module. Le nom de la bibliothèque ZIP est comparé sans tenir compte de la casse, avec conservation optionnelle de la date. Le remplacement exige un fichier préparé sur le même système de fichiers. La lecture des manifestes réutilise ConfigurationFiles, qui fournit aussi leur écriture atomique. La sonde Windows Release a vérifié l’extraction d’une entrée imbriquée, sa date, l’annulation, la conservation du fichier installé si la bibliothèque manque, le remplacement et le marqueur actif avec ou sans manifeste. Les fichiers temporaires ont été supprimés. Les métadonnées et le téléchargement HTTP du contrat restent à porter.

### Catalogue et téléchargement des cœurs

Le service commun reprend EmulationCoreReleaseFunctions.cs. La requête HEAD lit Last-Modified avec repli sur Date et conserve la longueur annoncée. Le SDK reprend la sélection par identifiant et par émulateur ainsi que le repli official-latest. Le téléchargement écrit par blocs avec la progression en octets et publie le fichier par remplacement atomique en fin de transfert. Le module appelle le service depuis le fil de son client Qt et conserve la responsabilité des messages, des étapes d’extraction et de la préparation avant remplacement. Les étapes suivantes écrivent le manifeste puis activent l’installation avant de signaler la fin. Une sonde Windows Release avec un serveur TCP sur la boucle locale a vérifié les dates, les versions avec et sans date, le refus HTTP, le téléchargement, l’annulation avec conservation du fichier, l’ordre des étapes et le refus avant remplacement. La version Windows Release a été déployée et les fichiers temporaires supprimés. Le raccordement aux catalogues réels appartient au portage des modules.

### Contexte OpenGL natif des cœurs

NativeCoreHardwareRender reprend EmulationOpenGlHardwareRenderContext de osd-support. Sous Windows, le contexte WGL commun gère les profils de compatibilité et Core, les callbacks natifs, le framebuffer avec couleur, profondeur et stencil, et la copie BGRA avec origine configurable. Le module pilote encore le cycle de vie du cœur, y compris ses rappels de réinitialisation et de destruction du contexte. La sonde Windows Release a vérifié les deux profils, l’accès aux symboles, les pixels et leur ordre après inversion, le redimensionnement, la restauration du contexte après un échec et la libération. La Release a été déployée et la sonde supprimée. Le backend de rendu matériel des cœurs est disponible sous Windows. [Contexte WGL du fil](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglmakecurrent) · [Résolution des symboles WGL](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglgetprocaddress).

### Tampons vidéo réutilisables

`EmulationSdk.CreateVideoBufferSet` expose la rotation de tampons de `EmulationVideoBufferSet.cs`. `EmulationVideoFunctions.cs` calcule le nombre d’octets à partir de la hauteur et du pas, puis copie les pixels en incluant le remplissage des lignes. Le SDK C++ reprend ces opérations dans `NativeCoreFrames`. Le callback utilise deux tampons et le partage implicite de QByteArray maintient les trames déjà publiées intactes lorsque le tampon revient en rotation. Le build Windows Release et la sonde temporaire ont validé la réutilisation, la conservation des trames, la duplication de la dernière trame et les limites de copie.

### Contrôleur de sortie audio du SDK

`EmulationAudioOutputController.cs` distingue la sortie empruntée de la sortie créée par sa fabrique et réessaie une écriture après un échec. Le contrôleur C++ reprend le démarrage, les changements de format, la latence, la coupure, la pause, les états du tampon et la récupération. Le contrôle Release utilise une sortie simulée et vérifie la reprise avec la fabrique, la conservation de la sortie empruntée, la destruction de la sortie possédée et l’arrondi du volume. Une valeur NaN est ramenée à zéro pour éviter de la convertir en entier.

### Géométrie et libellés vidéo

`EmulationVideoFunctions.ApplyGeometry` utilise un rapport d’image explicite s’il est fini et positif. Sinon, la hauteur de base permet de calculer le rapport, et si elle est nulle le rapport précédent est conservé. Le callback natif C++ applique cette règle à l’initialisation et à la réception de SetGeometry. Les libellés NTSC, PAL et SECAM ainsi que les fréquences à six décimales au plus sont invariants, comme dans le SDK C#. La sonde Windows Release a vérifié les deux parcours, un rapport invalide et les libellés avec les paramètres régionaux français. La sonde a été supprimée.

### Cache de pipelines Vulkan entre lancements

Le commit C# 1e3ac89c0 conserve les shaders compilés sur disque avec une empreinte SHA-256 et une écriture atomique. Vulkan utilise les mêmes opérations de stockage que Direct3D 11 pour les données de pipeline QRhi. La clé distingue le périphérique, la version Qt et Vulkan, la cible et les trois shaders compilés. Le fil de préparation importe le cache avant de créer ses pipelines. Si le fichier manque ou que son empreinte est invalide, le même parcours le régénère. La Release Windows a compilé et une sonde sur le moteur Vulkan a vérifié trois lancements distincts avec cache absent, présent puis corrompu. Les fichiers temporaires de la sonde ont été supprimés.

### Utilitaires de traitement audio du SDK

`EmulationAudioFunctions.cs` distingue la validation des choix de latence du chargement des paramètres déjà enregistrés. Le SDK C++ reprend ces deux règles, la copie bornée des échantillons natifs, la sélection des dernières trames et les deux règles de volume. Le contrôleur applique l’arrondi au pair, tandis que l’écriture directe tronque après bornage. Les deux chemins C++ du tampon audio partagent le calcul de capacité. La sonde Windows Release a vérifié les choix de latence, la sortie automatique, les dépassements de taille, le maintien des trames récentes et les deux conversions de volume. Elle a été supprimée.

### En-têtes des panneaux centraux de l’OSD

`FullscreenOsdWindow.CenterPanel.cs` affiche le glyphe du panneau avant le titre, puis un séparateur. Informations reprend les glyphes de `Icons.resx` et l’ordre défini dans `EmulationMachineTabConstants.cs`, avec la ROM avant la RAM. Windows Release a compilé et la sonde Qt hors écran a ouvert le panneau Informations et vérifié son titre, son icône, son séparateur et le glyphe de la section CPU. L’en-tête du Mixeur utilise le même chemin de création. La sonde a été supprimée.

### Commandes des panneaux centraux de l’OSD

`FullscreenOsdWindow.CenterPanel.cs` affiche les raccourcis de déplacement et de réglage du Mixeur, et une action Retour dans les deux panneaux. Le C++ reprend ces indications à partir des affectations actives et actualise un panneau ouvert après un changement de raccourci. La sonde Qt hors écran a vérifié le retour à la souris dans les deux panneaux, la mise à jour du raccourci et le réglage du volume au clavier. La Release Windows a compilé et la sonde a été supprimée.

### Écritures du mixeur en cours de réglage

`FullscreenOsdWindow.CenterPanel.cs` applique immédiatement l’aperçu et regroupe les écritures par réglage avant de les transmettre. Le C++ applique désormais la dernière valeur en attente de chaque voie au prochain passage de la boucle Qt et traite les écritures restantes au retour ou à la fermeture. La sonde hors écran a exercé deux voies, vérifié le regroupement et confirmé qu’un réglage pouvait être écrit avant le relâchement du curseur. La Release Windows a compilé. La sonde a été supprimée.

### Sélection des voies à la souris

`FullscreenOsdWindow.CenterPanel.cs` sélectionne la voie au clic sur sa carte et déplace directement le curseur au point cliqué. Le C++ reprend ces gestes en utilisant la géométrie du curseur du style Qt pour convertir le point en valeur. La sonde Release a sélectionné une voie par son libellé, l’a réglée au clavier, puis a vérifié le saut de valeur sur la piste d’une autre voie ainsi que son enregistrement. La sonde a été supprimée.
