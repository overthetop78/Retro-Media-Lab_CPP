# Lecture Apple brute — 15 novembre 2026

Le banc compile 41 fichiers C# originaux, y compris les trois lecteurs, les sondes, le convertisseur d’ordre, les builders et modèles réels, les validations MFS, les primitives et dates utilisées par le véritable lecteur d’en-tête ProDOS. Aucun algorithme de référence n’est réécrit ; seul un espace de noms Functions vide satisfait un using inutilisé.

Les 41 empreintes correspondent à l’inventaire. Empreinte du manifeste : `4deec377b82943beaa1b13ca0342b9c5b2e347e6830ce551ab8f0ccfb37626cb`.

**195 comparaisons ciblées C#/C++, zéro différence**, plus **29 contrôles natifs ciblés**. Les parcours des blocs précédents ne sont pas relancés.

Les comparaisons couvrent les tailles tronquées des signatures/en-têtes, toutes les versions Lisa connues et leurs voisines, limites de noms et caractères, classes de stockage/longueur d’entrée ProDOS, limites de la fenêtre SOS, numéros sectoriels hors plage, longueurs de conversion alignées ou non, entrée vide, extensions en casse différente ou inconnues, capacités Apple reconnues ou non, conflits de signatures et priorités PO/SOS/ProDOS/DOS/Lisa/Macintosh, structure trouvée après réordonnancement. Pour chaque image acceptée : format, géométrie, capacité, compte de blocs, preuve de reconnaissance, adresse de chaque bloc et SHA-256 de la totalité des données sont comparés. Les exceptions sont normalisées par catégorie ; les diagnostics traduits ne sont pas comparés aux messages anglais C#.

Les cibles modifiées MediaFileSystems/MediaEngine et les 30 catalogues (Constants et 29 langues) compilent en Debug. Aucun avertissement ou erreur du compilateur C++ dans le journal final. La première invocation native omettait l’environnement MSVC ; elle a été corrigée avec vcvarsall, sans changement du code pour contourner l’erreur.

Audits : **583 fichiers C++**, **571 fichiers pour les frontières**, **zéro violation** ; git diff --check réussi. Catalogues relus et corrigés, paramètres et invariants préservés. Les tests des géométries du bloc 65 et les tests globaux antérieurs ne sont pas répétés, aucune modification de leur comportement n’étant introduite.

Le banc et les journaux sont supprimés après validation. Aucun ajout au workflow GitHub. Cette validation porte sur la reconnaissance et lecture brute Apple ; les writers et parseurs de systèmes de fichiers complets restent à porter.
