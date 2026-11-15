# Validation 2IMG/NIB — 15 novembre 2026

Le banc compile **92 fichiers C# originaux**, dont les lecteurs/writers 2IMG, les lecteurs bruts Apple, le vrai lecteur NIB, les deux décodeurs, les helpers de sélection/reconstruction et les modèles. Les empreintes correspondent à l’inventaire : manifeste `933ac2343a0a72aa61ef0b4fabfb1ba92132278c2eebe23f6a629a5ba4604417`. Aucun algorithme de référence n’est recopié ou réécrit dans le banc.

Le writer C# écrit vers un transport de capture implémentant IAtomicImageFileWriter ; son en-tête et sa charge utile réels sont comparés au résultat C++. Les fixtures NIB sont générées avec les encodeurs C++ existants, puis leurs octets identiques sont lus par les pipelines C# et C++ réels. Cette méthode vérifie le raccordement, la sélection et la reconstruction sans répéter la campagne de validation des codecs déjà réalisée.

Résultat en Debug : **62 comparaisons ciblées, zéro différence**, plus **3 contrôles natifs de transport par chemin et annulation**.

Les cas couvrent les cinq cibles Apple acceptées (DOS 113/140, ProDOS 140/800 et SOS), leur casse et les cibles inconnues ; en-têtes courts, signatures invalides, versions non prises en charge, longueurs d’en-tête et plages de données aux limites, offsets/longueurs non représentables en entier signé et formats internes inconnus. Les octets écrits sont contrôlés par SHA-256. Pour les images lues : format, dimensions, capacité et nombre logique de blocs, ordre/adresses/intégrité de chaque bloc et SHA-256 de toutes les données sont comparés. Les exceptions C# sont comparées par catégorie, sans utiliser leurs messages comme traductions.

Les parcours NIB comprennent la piste vide, les pistes GCR 13/16 secteurs, une piste RWTS18 non retenue comme famille, et deux pistes RWTS18 retenues. L’écriture par chemin produit les mêmes octets que la construction en mémoire ; une annulation demandée conserve la destination déjà écrite. QTemporaryDir et les modèles possédés libèrent les fichiers/buffers même si un contrôle échoue.

La compilation de production et l’intégration des diagnostics/catalogues sont effectuées en construction groupée avec les autres agents. Aucun défaut de production n’a été révélé par cette comparaison. La construction de référence a nécessité d’ajouter les véritables dépendances et GlobalUsings.cs ; ces corrections concernaient le banc, pas l’application.

Les 29 catalogues ont été relus, les textes incorrects corrigés et les paramètres/invariants 2IMG vérifiés. Le banc, ses sources, objets et données temporaires sont supprimés après validation ; aucun test n’est ajouté au workflow GitHub. Les autres conteneurs et APIs autonomes de reconstruction Apple restent hors de cette validation.
