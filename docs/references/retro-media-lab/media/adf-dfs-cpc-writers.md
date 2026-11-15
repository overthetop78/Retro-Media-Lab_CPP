# Writers ADF/ADFS, BBC DFS et CPCEMU

Sources C# en lecture seule : les quatre entrées AcornAdf, AmigaAdf, BbcDfs et AmstradDsk de `Images/Writing/MediaWritingComposition.cs`, sa fonction WriteAmstradAsync et les méthodes CanCreate des services AcornAdf, BbcDfs, AmigaAdf et AmstradDsk.

L’adapter Acorn expose les six profils physiques natifs. DOS Plus 800 accepte seulement IMG/800 ; les autres acceptent ADF et, selon le profil, ADS pour 160, ADM pour 320 ou ADL/640 pour 640. Le profil de l’image doit correspondre à la cible, puis le prédicat natif vérifie géométrie et charge utile complète. L’adapter Amiga expose DD/HD avec ADF ; son prédicat conserve le test source d’égalité d’identifiant, l’écriture validant ensuite la géométrie et les secteurs.

Le writer BBC expose les profils natifs DFS et choisit SSD pour une face ou DSD pour deux faces. Il réutilise le prédicat et le writer BBC, sans nouvelle transformation.

Le writer CPCEMU expose Amstrad CPC, PCW et le format natif CPCEMU DSK, avec DSK/EDSK. Comme l’entrée commune C#, il construit le conteneur depuis l’image sectorielle détenue par le document. Le builder conserve les informations d’adresse, ordre, données, erreurs et tags natifs déjà portées.

EDSK sélectionne directement Extended. Pour DSK, le conteneur devient Extended si une piste manque, si la taille d’un secteur diffère de sa taille nominale ou si les tailles non rembourrées des pistes diffèrent. Le fichier conserve son extension demandée DSK : la signature du conteneur détermine son type réel. Les autres limites du writer natif restent appliquées ; ce passage n’introduit ni secteurs fictifs ni suppression de données.

Chaque adapter retourne un chemin unique et délègue l’écriture atomique au backend existant. Les services complets de conversion et leur sélection initiale des lecteurs restent en cours, ainsi que la composition globale de l’ensemble des writers.
