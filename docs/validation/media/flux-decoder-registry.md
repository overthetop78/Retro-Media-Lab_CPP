# Validation du registre de décodeurs — 15 novembre 2026

Le banc compile douze sources C# originales : registre, erreurs, scoring, sélection, interface et modèles. Les empreintes SHA-256 correspondent à l’inventaire. Les codecs de contrôle fournissent des résultats prédéfinis ; le double Lisa vérifie l’exclusion par type du C#, et le catalogue par défaut n’est jamais appelé. Ce banc ne valide pas les algorithmes des codecs encore non portés.

Les 1 362 cas comprennent 840 scores, 500 sélections de meilleure révolution, 16 inscriptions et 6 décodages explicites. Ils croisent intégrités valides/invalides/indéterminées, présence de données vides, structures, flux brut, confiances finies/NaN/infinies, participation automatique, plusieurs codecs/révolutions, choix explicite et égalités. Les inscriptions couvrent entrées nulles, espaces Unicode, caractères non blancs, doublons après une entrée invalide, casse, identifiants Unicode et caractère nul. La recherche d’un identifiant absent précède bien le refus d’une révolution nulle. 1 222 cas retournent un résultat et 140 sont refusés.

Sept scénarios de cache C# comparent identités et nombres d’exécutions : même objet réutilisé, contenu identique dans un autre objet, répétition d’une révolution, exception mémorisée, nouvelle instance après échec, seize appels simultanés et copie du catalogue. Les résultats partagés sont comparés par identité, pas seulement par valeur.

Quinze contrôles C++ supplémentaires vérifient clés faibles et libération des résultats expirés à l’accès suivant, durée de vie du callback/service, deux objets aliasant un même propriétaire, indépendance de deux codecs, puis intégrations FM/MFM réelles. Pour chaque encodage, la collecte restitue les données et leur intégrité, exécute chaque codec une seule fois par fenêtre malgré une répétition, puis le cache fichier partage les résultats et les invalide à modification de date. Une exception de codec dans la première capture n’empêche pas le réessai sur de nouvelles identités. Les 203 textes des DLL des 29 langues sont comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 587 contrôles, aucun écart**, 1 362 cas, 7 scénarios et 203 textes.
- Audit : **414 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste des cas C# : `f1eae3880a2e82d744a4ea66e993f56568c32006c9309b327438d55191ca3cf1`.
- SHA-256 des scénarios C# : `ee582f814042431456e98ee55668cd45e1745e94e815ed103e912cc99ac2cb72`.

Le banc build/verification-flux-registry, ses captures synthétiques, binaires et journaux de comparaison sont supprimés après vérification. Aucun média utilisateur ou ajout au workflow GitHub. Le catalogue complet et les codecs manquants restent à porter.
