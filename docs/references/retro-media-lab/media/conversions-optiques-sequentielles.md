# Conversions optiques et séquentielles

Référence : `Images/Conversion/Optical/OpticalImageConversionService.cs`, `Sequential/SequentialMediaConversionService.cs` et `MediaConversionComposition.cs` dans le dépôt C# en lecture seule.

Le service optique énumère uniquement les destinations acceptées directement par les writers, avec dédoublonnage et tri insensible à la casse. Il ne passe jamais par un convertisseur de représentation susceptible de perdre pistes, sessions ou sous-canaux. La lecture et l'écriture passent par les services communs ; la notification finale intervient après l'écriture réussie. Les requêtes invalides et destinations refusées utilisent les diagnostics déjà traduits `ConversionRequest` et `ConversionRepresentationMissing`.

Le service séquentiel utilisait encore directement le registre de reconnaissance et le writer. Il utilise désormais `MediaImageReadingService` et `MediaImageWritingService`, comme le C#, pour bénéficier aussi de leur gestion des lecteurs, des opérations et des résultats d'écriture. Son constructeur antérieur reste disponible par délégation. Les API publiques asynchrones exposent destinations, plan avec pertes avant acceptation et conversion. Elles conservent les clés, arguments et domaines de traduction des pertes et possèdent les services pendant toute l'opération.

La composition runtime partage les services de reconnaissance et d'écriture existants avec les nouveaux services optique et séquentiel. Aucun accès direct d'App aux représentations MFS n'est ajouté.

Validation du bloc : compilation Debug de MediaEngine réussie (22 étapes après reconfiguration) ; audit des constantes réussi sur 1 496 fichiers C++ et 29 langues. Aucun nouveau test de codec ni média temporaire. Le comportement optique a été confronté au service C# ; aucun nouveau test d'extraction optique réelle n'a été exécuté dans ce bloc.
