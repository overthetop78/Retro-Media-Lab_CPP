# Validation du catalogue des formats

- Export réel du constructeur C# : 98 formats, build/image-format-catalogue-export.log. Une qualification de namespace répétée dans les constantes temporaires générées a été corrigée avant l'export ; aucune modification du C# source.
- Compilation C++ Debug réussie sans avertissement signalé : build/image-format-catalogue-build.log.
- Contrôle natif ciblé PASS : build/image-format-catalogue-smoke.log. Compare les 98 entrées aux propriétés de l'export : ordre, identifiants, famille, clés disponibles, extensions/défauts, visibilité courante, compatibilités, tags, facteur physique, capacités physiques et densité.
- Toutes les extensions source présentes dans l'export sont vérifiées contre les sorties compatibles natives dans le même ordre. Normalisation IMG/.img et recherche IBM.1440 vérifiées.
- Les 98 identifiants réservés sont absents des 90 définitions brutes ; les deux factories brutes par défaut construisent leurs capacités à partir de ces 90 définitions.
- Argos a généré les descriptions et l'erreur pour 29 langues. Relecture et correction des sens incorrects : image disque/photo, ordre DOS/commande, nibble/morsure, brut/pétrole et formes traditionnelles chinoises. Les noms techniques restent intacts.
- L'extension du fichier commun d'invariants a révélé d'anciennes altérations ROM/ADM, Apple Macintosh et Atari : correction ciblée dans les catalogues signalés. Aucun autre catalogue régénéré.
- Audit des littéraux PASS : 971 fichiers C++, 29 langues, build/image-format-catalogue-audit.log. Projets Visual C++ régénérés.
- 30 bibliothèques de catalogues compilées : build/image-format-catalogue-catalogs.log ; les 28 catalogues de langues modifiés lors du dernier ajustement typographique ont ensuite été reconstruits : build/image-format-catalogue-catalogs-final.log.
- Exporteur C#, configuration et caches locaux, programme de contrôle et binaires temporaires supprimés. Aucun média utilisateur ou test intégré au workflow.

La comparaison couvre les données et la compatibilité du catalogue, pas la composition ou le fonctionnement global d'App.
