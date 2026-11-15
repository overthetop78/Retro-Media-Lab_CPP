# Encodage des images sectorielles — 15 novembre 2026

Référence C# : SectorImageTrackEncoder, SectorImageVisualizationExceptions et EncodedTrackTiming. Le service C++ Encoding/SectorImageEncoder possède les registres d’encodeurs et de politiques, avec construction par défaut ou injection. Il reste dans MediaEngine ; il ne déclenche aucun accès mécanique et n’a aucune dépendance à App ou Infrastructure.

Les blocs disponibles sont projetés dans l’adressage de la politique, regroupés par cylindre/face et triés dans cet ordre. Chaque piste conserve un tri stable par RecordedTrackOrder lorsqu’il est présent, sinon par numéro d’adresse projetée. Les pistes dont la politique ne produit aucun secteur sont ignorées ; si aucune piste n’est produite, le service retourne le diagnostic traduit correspondant. Un encodeur absent pendant l’encodage d’une image est signalé comme une politique/prise en charge absente, conformément au C#. La recherche d’encodeur d’une piste ciblée conserve son diagnostic propre.

Les unités des API C# sont conservées explicitement :

- encode et encodePhysical retournent des DiskTrack avec les ticks communs de 25 ns.
- encodePhysicalTrack reçoit la durée d’index en nanosecondes et convertit la durée de cellule vers les nanosecondes avec contrôle de dépassement.
- encodeTrack utilise les durées fournies par l’appelant.

Les attributs de disposition physique ne sont ajoutés que pour ISO FM/MFM. Le mode image physique demande la conservation de la piste complète ; la piste physique ciblée n’ajoute pas cet attribut supplémentaire. Les attributs fournis sont copiés. Le nombre transmis à la politique reste celui des blocs avant transformation pour une piste physique ciblée, et celui des secteurs produits pour l’encodage d’une image, comme dans le C#.

Une différence de contrat découverte au raccordement est corrigée : AvailableBlocks expose dans le C# les données enregistrées sans le contrôle de taille nominale de GetBlock. Reconstruction/Sector/availableBlock utilise donc l’accès au bloc avec contrôle de présence, sans imposer ce contrôle de taille à l’encodage ou à la reconnaissance JFD. requireSectorBlock conserve son contrôle strict pour les lecteurs qui en ont besoin.

L’énumération des blocs peut lever l’annulation typée de MediaFileSystems ; le service la convertit au type d’annulation de MediaEngine. Les deux utilisent déjà la clé commune Media.Cancelled. L’extension C++ accepte aussi un jeton sur les opérations de piste ciblée ; le C# n’en propose que sur les opérations d’image complète. Les collections temporaires et résultats déjà construits sont possédés par des objets RAII.

Les validations restantes d’identifiants dans l’analyse des entrées et l’écriture FAT12 conservent U+FEFF, comme ThrowIfNullOrWhiteSpace. La vérification de nom récupérable Amiga conserve aussi ce caractère avant les contrôles de blanc/contrôle. Un helper privé à MediaFileSystems évite toute dépendance vers les détails de MediaEngine. Le filtre de reconstruction Commodore 900 compare désormais explicitement le numéro non négatif au nombre de secteurs non signé.

Les diagnostics du bloc sont traduits dans les 29 langues. L’unité technique ticks est protégée dans la liste d’invariants commune ; le diagnostic historique FluxIntervalOverflow est harmonisé dans les catalogues concernés.
