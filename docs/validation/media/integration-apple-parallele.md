# Intégration Apple en parallèle — 15 novembre 2026

Les writers Apple bruts (67), 2IMG/NIB (68), DiskCopy (69) et Apple DOS (70) ont été intégrés aux cibles Engine/MFS existantes. Les trois derniers périmètres ont été exécutés par des agents distincts avec ownership de fichiers séparé ; les changements CMake, diagnostics et catalogues de base ont été centralisés.

La compilation groupée des cibles affectées en Debug réussit. Une reconstruction incrémentale publie ensuite les corrections Base64 DiskCopy puis ASCII UTF-16 et zéro négatif Apple DOS. Les 30 catalogues (Constants et 29 langues) sont compilés après relecture. Aucun avertissement ou erreur C++ dans le journal final.

Huit cas supplémentaires ont révélé la règle de NUL terminaux de Byte.TryParse : après correction ciblée de Codec.cpp et reconstruction, ils passent sans rejouer les 157 cas DOS précédents. L’union comporte 165 cas distincts. Les quatorze identifiants Apple/Macintosh du catalogue C# partagé sont maintenant centralisés dans MFS/FormatIds.h et réutilisés par les constantes Engine/DOS, à valeurs inchangées.

Le warning C4018 ancien dans Commodore/Dos/Sector.cpp est retiré par typage explicite de la borne int32 positive lors de sa comparaison avec les cylindres uint32. La comparaison conserve sa valeur ; aucun parcours de décodage n’est modifié.

L’audit de catalogues a été aligné sur le contrat de traduction déjà utilisé par le générateur : les invariants isolés restent dans Constants et sont absents des langues. Il vérifie que leurs clés existent, impose les autres clés exactes, protège paramètres et invariants dans les phrases et exige en-US identique à la base hors invariants isolés. Aucun contrôle de code ou de traduction n’est supprimé pour masquer une erreur. Audit de littéraux : **602 fichiers C++**, 29 langues, succès ; audit des frontières : **590 fichiers**, zéro violation ; git diff --check réussi avant la reprise des périmètres parallèles suivants.

L’audit reconnaît également les initialisateurs numériques de membres d’enums, qui sont eux-mêmes des constantes intégrales nommées ; il continue de refuser les autres littéraux hors constantes et ne dispense aucun fichier public. Après ajout des sources WOZ/ProDOS suivantes, le contrôle commun passe sur **618 fichiers C++** et **606 fichiers de frontières**, sans violation ; leur validation fonctionnelle reste suivie séparément dans les blocs 72/73.

Les validations de chaque bloc détaillent leurs cas ciblés ; les tests anciens non affectés ne sont pas relancés. Les corrections qui modifient réellement un comportement font l’objet de cas ciblés supplémentaires. Cette intégration ne certifie ni les conteneurs restants ni le registre complet de systèmes de fichiers.

## Intégration élargie 76–82

Les compilations Debug ciblées de HFS, HFE/Lisa, UCSD/correctif HFE, CP/M, MSA/ST, puis RT-11/constantes publiques CP/M ont terminé avec code 0 et sans avertissement C++. Les 30 DLL de catalogues Constants et langues ont également été compilées après stabilisation des JSON. Les bancs utilisent les véritables sources C# avec empreintes conservées dans leurs rapports ; les résultats de contenu ne sont pas déduits des seuls audits statiques.

Contrôle quiescent : `audit_literals.py` passe 692 fichiers C++, 56 clés d'erreurs du périmètre contrôlé et 29 langues ; `audit_media_architecture.py` passe 680 fichiers, zéro violation ; `git diff --check` passe. Les lecteurs et conversions encore manquants restent suivis dans l'inventaire. Les JSON relus n'ont pas été régénérés pour l'ajout ultérieur des invariants communs.
