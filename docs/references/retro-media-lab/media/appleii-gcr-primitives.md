# Primitives GCR Apple II

Bloc 36 — référence : sources C# locales en lecture seule, empreintes vérifiées dans inventaire-csharp.csv.

AppleIIGcrCodec.cs est porté entièrement : paires 4-and-4, adresse volume/piste/secteur et XOR, blocs de 256 octets en 6-and-2 et 5-and-3. Les tables de symboles suivent les tables originales ; les inverses sont construits à la compilation. Les chaînes XOR, ordre des groupes auxiliaires, bits bas et checksum final restent identiques à la source.

La lecture AppleBitLatch est commune aux futurs codecs Apple. Elle consomme huit bits puis décale jusqu’au bit haut de synchronisation. Une fin de flux insuffisante retourne une absence de résultat et conserve le curseur atteint ; moins de huit bits restants ne sont pas consommés. Compte et position sont validés dans l’ordre source. Les données retournées sont possédées ; annulation et sorties par exception libèrent les ressources par RAII.

Un symbole inconnu ou un bloc tronqué ne produit pas de secteur ; un checksum incorrect retourne les données avec intégrité fausse. La position finale reste observable dans les deux modes. Les trois diagnostics proviennent de MediaEngine/appleii-gcr-primitives.json et de ses 29 catalogues relus.

AppleIIGcrFormat.cs reste partiel : tables et constantes nécessaires aux primitives sont reprises. Prologues, paramètres de piste et associations adresse/données seront portés avec AppleIIGcrTrackEncoder et AppleIIGcrDecoder au bloc suivant. Aucun catalogue réduit par défaut n’est ajouté.
