# Writers communs SCP et HFE

Référence C# : MediaWritingComposition.CreateDefault, ses deux entrées Flux et sa fonction Flux. Les identifiants writer sont repris de MediaImageWriterIds.

Les cibles comprennent toutes les définitions embarquées dont toutes les pistes sont bitcell, plus raw.scp ou raw.hfe selon le writer. La règle All conserve son comportement C# pour une liste vide. Il n'existe aucune liste réduite de noms recopiée dans ces adapters.

Le contrat flux commun filtre format, extension et représentation, puis conserve le document et ses métadonnées pendant l'opération différée. Le writer SCP utilise fromProtected et le backend SCP atomique existant. Comme en C#, son prédicat n'annonce pas une conversion préalable exhaustive : les contraintes de révolutions/timing sont contrôlées par la conversion et le writer.

Le writer HFE exige canRepresentExactly avant écriture, puis transmet les vraies métadonnées au writer natif. La sélection HFE uniforme/V3 reste celle du backend existant. Les erreurs et mécanismes de nettoyage existants sont réutilisés, sans duplication de catalogue.

Les factories natives sont prêtes pour la composition globale. La composition par défaut complète de tous les writers reste à porter.
