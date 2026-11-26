# Visualisation des pistes et dossiers des onglets

Le C# prépare les pistes SCP puis les présente dans l’ordre cylindre, face, à au moins 30 ms d’intervalle. Les images sectorielles utilisent un intervalle de 100 ms. Références C# locales : ScpVisualizationController.PrepareTracksAsync, MediaVisualizationController.PresentSectorImageAsync et VisualizerTabSection.xaml.

Sur le SCP CrazyCarsIII_Gen4_Tri_Format, les 164 pistes ont été présentées en 5,38 secondes de la première à la dernière. Les cellules persistent dans la vue et la barre d’état après chargement. Les captures scp-en-cours.png et scp-termine.png montrent le fond transparent et la disposition. La grille hexadécimale conserve les cellules, les offsets et la colonne ASCII.

Les sélecteurs enregistrent le dossier par onglet dans les paramètres utilisateur. L’annulation et l’indépendance entre Visualisation et Explorateur ont été vérifiées avec un sélecteur Qt à configuration isolée. La compilation et l’installation Release ont réussi. Le programme de vérification et ses paramètres temporaires ont été supprimés.
