# Dialogue GreaseWeazle

## Réalisation

Options → séparateur → GreaseWeazle ouvre un dialogue unique, réutilisable, avec neuf onglets issus des anciens sous-menus Diagnostics et Matériel. Un sélecteur commun présente les lecteurs Greaseweazle configurés ; le lecteur initial reprend la sélection principale lorsqu’elle correspond. Les lecteurs débranchés restent visibles, mais une commande ne démarre pas sur un lecteur indisponible.

Les formulaires reprennent tous les champs C#, valeurs initiales et cases optionnelles. Ils affichent la commande, la sortie brute et un résumé traduit. Infrastructure valide les paramètres, construit les arguments, lance le processus, collecte ses sorties et écrit le journal propre à l’action. App assure uniquement la présentation et la transmission des choix.

Les titres des onglets réutilisent les clés Tool.Title déjà présentes dans les catalogues des journaux ; les traductions ne sont pas dupliquées. Les derniers arguments effectivement exécutés restent affichés avec leur résultat, même si la disponibilité du lecteur change ensuite.

Une seule commande de maintenance s’exécute à la fois. Le bouton devient Arrêter sur son onglet et demande confirmation. L’option bootloader demande une confirmation distincte, négative par défaut. La fermeture du dialogue annule l’opération en cours et attend la fin du nettoyage avant de masquer le dialogue. Un changement de thème conserve au contraire le service, les opérations en cours, les brouillons et résultats.

La sortie conservée pour affichage est bornée à 1 MiB ; les flux sont transmis au journal selon les limites configurées dans Journaux. Une erreur de journalisation est signalée. Les erreurs de validation, de sélection du lecteur et de démarrage utilisent des messages traduits. Les sorties techniques du programme externe restent brutes, comme en C#.

## Vérifications

Banc temporaire unique compilé avec un faux exécutable, sans accès au matériel réel :

- Construction et exécution des neuf commandes, chemins d’exécutable avec espaces, arguments device/drive conformes au C# et valeurs initiales.
- Validation des nombres, broches autorisées, pistes, PLL, vitesses et incompatibilités d’options.
- Sorties, journaux par action, résumé des informations, code de retour non nul et échec de démarrage.
- Décodage d’un caractère UTF-8 coupé entre deux blocs.
- Arrêt d’une commande avec processus enfant et destruction du service avec processus encore actifs : les descendants sont bien terminés.
- Action unique après le séparateur, neuf onglets, lecteur configuré, exécution depuis un bouton et restitution des sorties.
- Bootloader refusé puis accepté sur le faux exécutable : l’option n’est exécutée qu’après acceptation.
- Changement FR/EN, densité 75 %, reconstruction sombre/clair pendant une commande : dialogue, onglet, brouillons, résultats et opération active conservés.
- Fermeture pendant une commande : annulation, attente du nettoyage puis fermeture effective.
- Captures hors écran contrôlées en clair et sombre. Les paramètres longs de l’alignement défilent dans leur zone ; les onglets défilent horizontalement si nécessaire.

Les essais utilisent exclusivement le faux programme du banc. Aucune commande de broches, mouvement de tête ou micrologiciel n’a été envoyée au matériel de l’utilisateur. La branche Linux/macOS n’a pas été compilée ni exécutée ici.

Les traductions existantes viennent du C#. Trois messages complémentaires ont été traduits avec Argos puis relus, avec correction des sorties manifestement dégradées. Aucun test n’est ajouté aux workflows GitHub. Compilations et déploiements locaux Debug et Release réussis. Le banc et ses artefacts ont été supprimés. Audit réussi sur 140 fichiers C++, 56 clés d’erreur et 29 langues ; git diff --check passe.

## Navigation latérale

À la demande de l’utilisateur, les neuf onglets ont été remplacés par une liste latérale et neuf pages. Les contrôles, commandes et le service de maintenance restent ceux validés ci-dessus ; les libellés longs se replient sur plusieurs lignes. Les captures actuelles sont [ici](captures/README.md).
