# Registre et sélection des décodeurs de flux — référence C#

Le bloc 33 reprend IFluxDecoder, FluxDecoderScoring, les opérations du registre explicite FluxDecoderRegistry, FluxDecoderRegistryExceptions et FluxDecodeSelection. Douze sources originales sont compilées dans le banc, avec des empreintes conformes à l’inventaire. Le C# reste en lecture seule.

## Contrat et catalogue explicite

Decoder fournit un identifiant stable, un nom descriptif et le décodage d’une révolution sans mutation de son flux. Les résultats produits par le registre sont possédés et partagés en lecture seule. Le constructeur copie le catalogue ordonné ; il valide toutes ses entrées avant de construire l’index ordinal, comme le C#. Une entrée nulle ou un identifiant vide/constitué d’espaces précède donc le contrôle des doublons, même si un doublon apparaît plus tôt dans la liste.

Les identifiants sont comparés exactement, sans insensibilité à la casse ni normalisation Unicode. Le nom descriptif ne sert pas d’identifiant. La participation à la détection automatique est déclarée par le codec : ce contrat permettra au codec Lisa de rester réservé au choix explicite, en reprenant l’exclusion par type du registre source sans dépendance du registre à sa classe concrète.

Le constructeur sans argument et le catalogue par défaut de 25 codecs ne sont pas portés dans ce bloc. Aucun catalogue réduit ne leur est substitué. Le banc utilise un double Lisa pour vérifier l’exclusion du registre C#, sans déclarer son algorithme GCR porté.

## Scores et stabilité

Le score automatique distingue secteurs à intégrité valide, secteurs non vérifiés, secteurs invalides, flux brut, flux structuré et simple confiance. Les poids source sont dans ScoringConstants.h. Aucune normalisation supplémentaire de confiance n’est ajoutée : les valeurs reçues restent celles du résultat.

Une sélection automatique compare score, confiance puis nombre de structures. Le choix explicite entre révolutions compare lexicographiquement nombre de secteurs valides, pénalité des invalides, présence de données (y compris vides), confiance et nombre de structures. Les valeurs NaN sont inférieures aux nombres, comme Double.CompareTo ; deux NaN sont égaux pour la poursuite des critères. Le premier codec ou la première révolution départage une égalité complète. Une liste de révolutions vide retourne une absence de sélection sans chercher le codec demandé.

## Cache par identité

Flux::SharedRevolution porte l’identité et la durée de vie de l’objet. Le registre ne confond pas deux révolutions de contenu identique. Sa clé faible combine propriété partagée et adresse d’objet ; deux objets distincts partageant un même propriétaire par alias restent séparés. Le cache ne prolonge pas la durée de vie d’une révolution.

Chaque couple révolution/codec partage une promesse et un résultat différé. Les appels simultanés exécutent le décodeur une seule fois, puis partagent son résultat ou son exception. Contrairement au cache fichier ISO, une exception de codec reste mémorisée pour cette même révolution ; une nouvelle instance autorise un nouveau décodage. Aucun verrou n’est conservé pendant l’exécution d’un codec ; des codecs différents peuvent travailler indépendamment.

Les entrées dont la clé a expiré sont purgées à l’accès suivant, et toutes les entrées sont libérées à la destruction du service. Les résultats encore détenus par un demandeur restent valides. Aucun thread détaché ou référence empruntée conservée n’est créé.

## Collecte ISO et ressources

La collecte crée une révolution possédée par fenêtre et la transmet à tous les codecs de cette fenêtre. Registry::operation fournit le contrat de service correspondant et garde le registre vivant tant que le callback est utilisé. Le résultat retourné à la collecte est une valeur possédée. Un codec répété dans la liste réutilise donc le résultat de la même fenêtre sans nouvelle exécution.

Le cache de capture ISO continue à partager ses résultats par empreinte de fichier. Une nouvelle capture produit de nouvelles identités de fenêtres ; un échec de capture suivi d’un réessai n’est pas bloqué par l’exception d’un ancien objet révolution. Les deux caches gardent leurs responsabilités distinctes dans MediaEngine.

Les sept diagnostics de MediaEngine/flux-registry.json sont générés avec Argos puis relus dans les 29 langues. Les corrections portent notamment sur les négations, identifiants/paramètres et le sens physique d’une révolution, qui désigne un tour enregistré et non une révolution politique. Les contrôles de fichier restent dans MediaFileSystems.
