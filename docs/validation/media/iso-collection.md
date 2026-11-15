# Validation de la collecte ISO — 15 novembre 2026

Le banc compile 35 sources C# originales, dont le véritable IsoScpCandidateDecoder, les modèles, fenêtres et adaptateurs. Leurs empreintes sont vérifiées contre l’inventaire. Le registre et le lecteur sont des doubles de dépendance : le registre renvoie les résultats de contrôle et relève les appels ; le lecteur fichier n’est pas utilisé. Ce banc valide le collecteur, pas le registre général ni le cache fichier.

Les 591 cas comprennent 315 scores et 276 collectes : intégrités vraies/fausses/indéterminées, données absentes/vides/présentes, plusieurs quantités de secteurs et confiances finies/NaN/infinies ; 250 images déterministes à pistes mélangées, plusieurs révolutions et zéro à quatre décodeurs ; toutes les paires de cinq confiances adverses ; une jointure en débordement avant sélection. 552 cas retournent un résultat, 38 signalent l’absence de décodeur et un refuse le débordement.

Les comparaisons vérifient les groupes ordonnés, tous les secteurs et métadonnées, données et tags, CRC, intégrités séparées, source/révolution, la suite des appels à chaque fenêtre et leurs intervalles/index exacts. La progression compare identités et totaux ; les compteurs complétés sont comparés comme suites ordonnées de 1 au total par catégorie, sans imposer un ordre artificiel aux callbacks concurrents.

Huit contrôles supplémentaires couvrent annulation initiale et depuis la progression, fonction absente, exceptions du décodeur et de la progression, conservation des candidats après retour et deux intégrations réelles. Ces dernières encodent des données connues en FM puis MFM, adaptent les pistes et vérifient leur collecte via les décodeurs déjà portés. Les 58 textes des DLL des 29 langues sont comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **657 contrôles, aucun écart**, 591 cas et 58 textes.
- Audit : **406 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `1ab482bca8a1c6049a3bc42eabcf3896457f20fefcbf92b40dc5d13d75ded475`.

Le banc build/verification-iso-collection et ses journaux sont supprimés après vérification. Aucun média utilisateur ou ajout au workflow GitHub. La reprise du cache, le registre et les politiques par famille ne sont pas validés par ce bloc.
