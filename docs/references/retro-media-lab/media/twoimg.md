# Conteneur 2IMG et dépendance NIB

Le conteneur reste dans MediaEngine, avec la signature `2IMG`, le créateur `GWGU` et la version 1 du C# conservés. `TwoImg/Constants.h` reprend les offsets de l’en-tête minimal de 64 octets et l’énumération distingue DOS, ProDOS et NIB. Le créateur historique est un identifiant de conteneur, pas un texte d’interface.

Le lecteur vérifie, dans l’ordre du C#, la taille disponible, la signature, la version, la représentabilité signée 32 bits de l’offset et de la longueur, puis la plage de données. Celle-ci doit commencer après l’en-tête déclaré et contenir au moins un octet. Le contrôle C++ soustrait seulement après vérification des tailles pour éviter un dépassement non signé. Les champs drapeaux et nombre de blocs ne déterminent pas le routage, comme dans la source. La charge DOS/ProDOS est confiée au lecteur brut Apple avec l’extension imposée par l’en-tête ; NIB utilise le lecteur de pistes réel.

Le writer choisit le format interne avant de construire la charge utile avec Apple::twoImgPayload. Il émet les champs, réserve le nombre de blocs aux charges ProDOS, laisse les champs non utilisés à zéro et publie le résultat avec writeAtomicImage. Il ne duplique pas la validation géométrique du writer Apple brut. Les buffers sont possédés et libérés automatiquement ; les callbacks d’écriture restent synchrones pendant la vie de leur charge utile.

`Nib/Reader.cpp` découpe les pistes fixes de 6 656 octets, dépaquette les bits MSB et appelle les décodeurs Apple II et RWTS18 déjà présents. Le filtrage conserve les coordonnées de piste, tailles et numéros attendus. Les doublons gardent le premier exemplaire sauf si un exemplaire suivant possède une intégrité explicitement valide. Au moins deux pistes crédibles RWTS18 sont nécessaires pour retenir cette famille. Une seule piste ne force pas le choix RWTS18.

La reconstruction reprend l’ordre de première rencontre des secteurs. Les pistes standards deviennent des blocs DOS 3.2, DOS 3.3 ou GCR ; les paires de secteurs physiques peuvent former les blocs ProDOS complets soumis à la sonde existante. Les blocs RWTS18 conservent leurs 768 octets et leur intégrité. Les images sans secteurs standards restent des images GCR vides de 35 pistes, comme dans le C#.

Les erreurs de conteneur possèdent six clés traduites. Les longueurs NIB et cibles d’écriture non prises en charge réutilisent les diagnostics génériques appropriés. Les 29 catalogues ont été relus et corrigés après Argos, notamment pour offset, troncature, entier signé, les langues restées en anglais et le chinois traditionnel.

Le portage couvre les six fichiers 2IMG et les trois fichiers du lecteur/disposition/erreurs NIB. Les helpers C# partagés de sélection et de reconstruction ne sont pas annoncés intégralement portés comme API autonome : leurs scores et usages hors de NIB restent à raccorder avec les autres conteneurs Apple. Aucun reader factice n’est utilisé pour le format NIB.
