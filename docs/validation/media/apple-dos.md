# Apple DOS — 15 novembre 2026

Le banc compile 32 fichiers C# originaux, notamment toutes les classes du dossier Apple/Dos et les contrats de volumes/migration. Seul IFileSystemReader est remplacé par une interface de test contenant ses quatre membres sectoriels : les adaptateurs IMediaImageDocument du contrat général sont hors du périmètre de ce test. Le lecteur, writer, politiques et codecs de référence ne sont pas réécrits. Les images de test ne font qu’exposer les blocs produits par le writer aux deux interfaces.

Les 32 empreintes source correspondent à l’inventaire. Empreinte du manifeste : `b25daf760879553af5547f17560e454807d4ad1496ee743cc066b1384e26ad6c`.

165 cas couvrent les quatre identifiants DOS 3.2/3.3 et alias, le choix du type selon le système source, contenus vides et multi-listes, limites binaires, chaînes de catalogue/listes cycliques ou manquantes, pointeurs hors géométrie, secteurs de données absents, listes tronquées, tailles incohérentes, VTOC invalide, entrées supprimées ou inutilisées, politiques de noms et de volumes, noms hors ASCII, trop longs, entrées non racines, catalogue plein, types inconnus/verrouillés et suffixes numériques avec nuls terminaux.

Chaque volume créé compare les octets complets par SHA-256, les dimensions et l’identifiant ; les volumes relus comparent nom, capacité/espace libre, toutes les entrées (nom/type/taille/commentaire/attributs/référence/validité/contenu) et l’ordre des catégories d’avertissements. Les textes anglais C# sont normalisés en catégories ; les diagnostics traduits C++ ne prétendent pas avoir le même texte.

Trois divergences de portage ont été trouvées : remplacement ASCII par scalaire Unicode au lieu d’une unité UTF-16 (un caractère hors plan de base doit devenir deux points d’interrogation), rejet du zéro négatif admis par byte.TryParse dans DOS--00, puis refus des nuls terminaux permis par ce même parseur. Les trois sont corrigées dans le C++. Les cas déjà passés ne sont pas relancés sans raison ; la seconde passe couvre les cas restants et les politiques affectées. Huit cas supplémentaires ciblent exclusivement le placement des nuls et espaces dans le suffixe du numéro de volume : ils passent contre les bibliothèques finales, sans relancer les autres cas.

Les 143 premiers cas ont passé avant la découverte des divergences. Après correction, seuls les 40 cas restants ou affectés (incluant les politiques de noms déjà exécutées), trois nouveaux cas de type et les huit cas de suffixe sont exécutés : zéro différence. L’union couvre 165 cas distincts. Trois scénarios d’annulation déjà demandée (lecture, reconnaissance et écriture) vérifient le diagnostic d’annulation C++. Aucun fichier média réel n’est embarqué ; tous les buffers sont possédés par les modèles et RAII.

Les sources de production compilent dans la reconstruction Debug groupée par l’agent parent ; le banc natif compile et s’exécute contre ces DLL. Les 14 langues assignées à cet agent ont été relues/corrigées et leurs paramètres/invariants vérifiés ; les 15 autres sont revues par l’agent parent. Aucun parcours d’un bloc antérieur n’est relancé.

Le registre de lecteurs, son catalogue de formats et l’adaptateur document/volume restent hors de cette validation ; ils ne sont pas prétendus portés par ce banc sectoriel. Les anciens helpers d’exception non appelés ne sont pas exposés artificiellement comme nouvelle API C++.

L’inventaire a été intégré et vérifié par l’agent parent. Le banc temporaire et tous ses artefacts ont été supprimés après validation ; aucun test dans le workflow GitHub.
