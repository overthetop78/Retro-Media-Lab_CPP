# Distribution Release — 16 novembre 2026

Les trois DLL conservent leurs responsabilités distinctes. Dépendances privées : MediaEngine vers MediaFileSystems, puis MediaFileSystems vers MediaAnalysis. Audit actuel : 1 595 fichiers, aucune violation. Audit des littéraux : 1 607 fichiers et 29 langues, aucun échec.

La compilation Release complète a réussi sans avertissement ni erreur de compilation. Le script officiel scripts/local-building/build.ps1 a ensuite produit build/Release/Retro Media Lab. Les trois DLL médias sont dans lib ; aucune DLL à la racine distribuée. Le dossier CMake bin reste intermédiaire.

Le helper isolé a chargé puis déchargé les trois DLL distribuées en ordre inverse et vérifié leur absence après déchargement. Nettoyage des handles et dossiers de recherche par RAII, y compris sur exception ; aucune fenêtre ouverte. Compilation du helper avec /W4 /WX réussie. Helper et artefacts supprimés. Aucun codec inchangé retesté.

L’inventaire couvre 1 492 fichiers C# : 1 083 Engine, 372 MFS et 37 Analysis. La comparaison des empreintes au relevé initial ne trouve aucun changement, fichier absent ou source ajoutée dans les trois projets. Les chemins de correspondance sont normalisés. Le statut ancien de MediaRecognitionContext est corrigé : fabrique concrète fournie par Composition/Recognition.cpp et Drives/Service.cpp, déjà validée dans validation-mecaniques-lecteurs.md.

Les primitives communes et les descriptions de flux sont détaillées dans contrats-base-primitives.md et descriptions-structures-flux.md. Les rapports ciblés antérieurs constituent les preuves de comportement ; compilation et inventaire seuls ne prouvent pas une équivalence exhaustive.

## Suivi encore ouvert

Les anciennes listes 87 à 99 contiennent des cases non actualisées au fil des blocs. Les batteries sector-registry, ImageDisk et CPC DSK ont été abandonnées sur instruction utilisateur : docs/validation/media/bancs-prepares-retires.md. Aucun succès de ces batteries n’est revendiqué. Des relectures linguistiques restent ouvertes dans ces suivis ; la présence automatique des clés et invariants ne les remplace pas. La clôture globale reste ouverte.
