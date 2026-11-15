# Analyse du flux brut — référence C#

Le bloc 34 reprend RawFluxDecoder et RawFluxAnalysisDefinitions. Les quinze sources compilées par le banc, dont l’estimateur temporel et les modèles C#, ont des empreintes conformes à l’inventaire. Le dépôt C# reste en lecture seule.

## Estimation et anomalies

L’analyse utilise l’estimation non-FM commune, sans décoder d’octets ni de secteurs. La cellule estimée est exprimée dans les mêmes ticks que les intervalles. Les intervalles nuls restent dans la traversée, même si l’estimateur les exclut de ses échantillons.

Un intervalle est long uniquement s’il dépasse strictement dix cellules. Sinon, il est une impulsion courte uniquement après le premier intervalle et s’il est strictement inférieur à 0,55 cellule. Le premier intervalle commence à l’index et ne peut donc pas être une impulsion courte ; il peut être long. La priorité long/court et les égalités exactes des seuils restent celles du C#.

Le nombre de cellules provient d’un arrondi au pair, borné entre 1 et 64. Les cas non finis et extrêmes des aides sont comparés au C# .NET 10 ; le chemin normal du décodeur utilise une cellule finie d’au moins un tick. Les positions avancent aussi pour les intervalles sans anomalie. L’accumulation conserve la sémantique int32 source par arithmétique non signée et bit_cast, sans débordement signé indéfini.

Chaque anomalie produit une structure TimingAnomaly avec position et longueur, dans l’ordre du flux. Le résultat a une confiance fixe de 0,05, aucune donnée décodée et aucun secteur. L’annulation vérifie l’entrée, l’estimation commune et la traversée ; les buffers sont possédés et libérés par RAII sur toutes les sorties.

## Métadonnées et traduction

L’identifiant technique reste raw. Le nom source « Flux brut » est conservé dans la métadonnée descriptive pour compatibilité avec le résultat C#. La clé MediaEngine.RawFluxName fournit le nom à présenter dans la langue utilisateur ; RawFlux::Decoder expose cette clé. Les descriptions des anomalies sont des recettes localisées indépendantes de la langue choisie au moment du décodage.

Les trois textes de MediaEngine/raw-flux-analysis.json sont générés avec Argos puis relus dans les 29 langues. Les corrections conservent le sens de flux magnétique brut, anomalie temporelle non classée et intervalle long/impulsion courte. en-US conserve les descriptions source exactes nécessaires à la comparaison ; les autres langues utilisent une formulation naturelle.

## Registre explicite

Les classes Decoder d’ISO MFM, ISO FM, Commodore GCR et Commodore 900 GCR réutilisent leurs fonctions de décodage portées. Leurs identifiants et noms proviennent des constantes de chaque codec. Avec RawFlux::Decoder, elles peuvent être enregistrées explicitement sans adaptateur fourni par le consommateur.

Aucun catalogue par défaut réduit n’est introduit. Les vingt-cinq codecs du catalogue C# restent la cible ; Amiga et les autres codecs manquants, puis l’assemblage complet, sont encore à porter. App ne reçoit aucune logique d’analyse.
