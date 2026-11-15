# Cache des candidats ISO — référence C#

Le bloc 32 complète IsoScpCandidateDecoder avec l’entrée fichier, le partage des opérations et l’empreinte des captures. Le collecteur, la lecture SCP et les pistes protégées proviennent des blocs précédents. Le C# reste en lecture seule ; les trente-cinq sources du banc ont des empreintes conformes à l’inventaire.

## Identité et invalidation

La clé est la suite ordonnée des identifiants de codecs joints par un caractère nul, comme string.Join dans la source. Une entrée par liste de codecs conserve le chemin absolu normalisé, la présence du fichier, sa taille et sa date de dernière modification. Le chemin est comparé ordinalement, sans recherche d’équivalence de contenu ou résolution supplémentaire des liens. Une capture absente possède une empreinte sentinelle stable ; le lecteur réel conserve ses diagnostics communs si elle reste inaccessible.

Un changement de chemin, de taille ou de date remplace l’entrée de cette liste de codecs. Revenir à une ancienne capture provoque une nouvelle lecture. Une modification de contenu qui conserve exactement taille et date ne provoque pas de décodage, conformément au C#. Les listes de codecs différentes restent indépendantes, y compris leur ordre et les répétitions. La composition par séparateur nul reste celle de la source, y compris sa collision théorique entre un identifiant contenant ce séparateur et plusieurs identifiants.

## Concurrence et erreurs

Le premier appel réserve l’opération ; les demandeurs simultanés attendent le même résultat partagé immuable ou la même exception. Seuls le callback de progression et le token du premier appel pilotent ce travail. Le token d’un demandeur ultérieur n’annule pas une opération partagée ni un résultat déjà en cache.

Après un échec ou une annulation, l’entrée est retirée uniquement si elle est encore l’entrée courante. Une ancienne opération remplacée entre-temps ne peut pas évincer la nouvelle capture, qu’elle réussisse ou échoue. Aucun verrou du cache n’est conservé pendant la lecture, le décodage ou les callbacks.

La surcharge image protégée contourne le cache et n’affecte pas les entrées fichier. Les données renvoyées sont possédées et partagées en lecture seule. RAII libère promesses, résultats, sources et buffers à leur fin de vie ; aucun thread détaché n’est créé par ce service.

## Entrée fichier et API C++

Le lecteur injecté reprend le contrat du lecteur SCP source. Sans lecteur fourni, le service utilise fileSource puis Scp::read, adapte les pistes protégées et appelle la collecte ISO. Les chemins vides ou contenant un caractère nul sont refusés avant toute lecture ; l’erreur utilise le diagnostic OpenFailed déjà traduit de MediaFileSystems. Les autres erreurs de source restent celles du service commun.

Comme les lecteurs et collecteurs C++ déjà portés, les méthodes sont synchrones : le premier demandeur effectue le travail, les autres attendent son résultat différé. L’appel doit être planifié hors du thread graphique par le service consommateur. Les callbacks injectés doivent permettre les appels concurrents ; la durée de vie de l’objet doit couvrir ses appels en cours. Le passage de Task à cette API ne change pas la sélection, le cache ou la responsabilité des bibliothèques.

Le registre général des décodeurs, les politiques de reconstruction par famille, l’exploration complète et les documents/conversions restent à porter. App ne reçoit aucune logique de lecture ou de cache.
