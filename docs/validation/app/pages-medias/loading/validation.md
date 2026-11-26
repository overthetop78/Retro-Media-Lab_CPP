# Progression des médias — 17 novembre 2026

Référence : RML C#, ApplicationStatusBar.xaml, TrackProgressStrip.xaml.cs, OperationProgressController.cs, ScpVisualizationController.cs, MediaVisualizationController.cs et TapeLoadingPresenter.cs. Sources C# conservées sans modification.

La barre réserve les colonnes matériel (245), profil (165), opération (125), puis les cellules extensibles, le chronomètre et la console. Aucun pourcentage générique ne remplace les pistes. Les cellules proviennent des pistes de la capture et des événements de décodage ou d’exécution ; les deux lignes ont leurs compteurs par face. Sans pistes connues, aucune cellule de piste n’est inventée.

Pour les cassettes, la séquence réelle est divisée en deux moitiés ordonnées, avec une seule ligne si elle ne comporte qu’un segment. Les cellules distinguent signal, silence, bloc décodé et inconnu. La présentation progressive reprend l’intervalle de 100 ms du C#. Les cinq segments UEF temporaires ont été affichés en groupes de trois et deux, avec conservation des silences.

Le cadre central et la barre sont raccordés aux pages Lecture, Écriture, Conversion, Explorateur et Visualisation. Les opérations matérielles ont été simulées par un outil temporaire : aucun accès matériel pendant cette vérification. Contrôles réussis : apparition immédiate, préparation asynchrone, progression, colonnes, profil, chronomètre, fin et annulation. Capture cassette vérifiée visuellement : cassette.png.

100 % utilise maintenant le facteur 1, sans réduction automatique cachée ; le DPI reste géré par Qt. Les sous-onglets gardent leur différence de taille prévue.

Compilation Release et installation locale réussies. Audits : 1 686 sources sans littéraux hors constantes, 29 langues ; 1 674 fichiers contrôlés, aucune violation des frontières. Les exécutables, médias et sources de vérification temporaires sont supprimés après utilisation. Ces contrôles ne mesurent pas un gain de vitesse de décodage.
