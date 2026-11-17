# Classification manuelle

Références C# : DiskClassificationSelector, DiskImageWorkspaceController.ApplyClassification/SelectExplorerRepresentationAsync, ScpMediaGeometryFunctions et les catalogues Classification.resx.

Les panneaux avancés des pages Visualisation et Explorateur partagent les contrôles, mais conservent des sélections indépendantes dans leurs sessions Infrastructure. Le catalogue public MediaEngine fournit les familles, formats et protections compatibles. Les choix détectés sont indiqués en gras ; les identifiants persistants restent distincts de leurs libellés traduits. La protection disparaît lorsqu’elle est incompatible avec le format choisi. L’Explorateur expose aussi la détection automatique et relance l’analyse sur le format explicite ; sa ressource source reste retenue pendant cette analyse.

Dans Visualisation, le choix règle le décodeur et la catégorie physique sans relancer l’Explorateur. Pour les flux, le rayon et le pointage reprennent le facteur C# : .43 pour une catégorie connue, .47 pour inconnu. Les choix et l’état déplié survivent à la reconstruction du thème. Les erreurs d’ouverture structurées de Visualisation sont résolues dans leur domaine traduit.

Vérification ciblée réussie : catalogue et protections, RWTS18 → décodeur RWTS18, retrait des protections incompatibles, rejet des identifiants de machine/protection invalides, conservation du choix manuel après détection, réactivation automatique, exploration effective d’une petite image FAT12 avec format explicite puis automatique. Le véritable thème restaure les contrôles de protection après changement fr-FR/en-US et reconstruction. Le sélecteur de protection reste lisible dans la fenêtre compacte de vérification. La capture panel.png montre ce dernier état.

Les programmes, exécutables et médias temporaires sont supprimés après vérification ; aucun matériel n’a été sollicité.
