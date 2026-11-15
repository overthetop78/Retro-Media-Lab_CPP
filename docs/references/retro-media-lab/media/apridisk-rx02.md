# Apridisk et RX02 physique

Références C# consultées en lecture seule : les readers/writers sous `Floppy/Apridisk` et `Floppy/Rx02`, `DecRx02Geometry`, `DecRx02SectorOrder`, ainsi que `Raw/LinearSectorImageWriter` pour ses règles de validation.

Apridisk lit la signature ACT et parcourt les enregistrements avec leurs longueurs d’en-tête et de charge utile. Il ignore les types non sectoriels après vérification des bornes, accepte les secteurs bruts de 512 octets ou les runs `(compte uint16, valeur)`, rejette les expansions invalides et exige les 630 secteurs du profil ACT Apricot PC/Xi. Un doublon remplace le secteur précédent, comme le dictionnaire source. Les secteurs restent initialement valides au sens logique du conteneur, sans preuve de CRC physique.

Le writer Apridisk produit des enregistrements non compressés dans l’ordre logique et utilise l’écriture atomique commune. Il conserve la différence du C# entre `CanWrite` (géométrie sans imposer l’identifiant) et l’écriture effective (identifiant et géométrie stricts) ; aucune conversion implicite de profil n’est ajoutée.

RX02 lit 77 pistes physiques de 26 secteurs de 256 octets. L’entrelacement par deux, le décalage de la seconde moitié et le skew de six positions sont suivis de la rotation d’une piste avec retour de la dernière vers la première. Chaque paire de secteurs logiques forme un bloc RT-11 de 512 octets, numéroté à partir de un dans la piste. L’écriture effectue l’opération inverse. Le helper d’adressage est privé au backend et appelé uniquement avec les indices validés par les boucles de lecture/écriture.

Les adapters reprennent les capacités et sondes C# : Apridisk exige DSK, une taille minimale et la signature ; RX02 reconnaît la capacité exacte. La politique commune distingue les rejets de conteneur du débordement arithmétique. Les diagnostics utilisent des clés traduites ; les textes et noms techniques figurent dans leurs catalogues/constantes.

Ces writers natifs ne sont pas encore tous raccordés à la composition globale d’écriture. La composition globale de lecture reste également en cours.
