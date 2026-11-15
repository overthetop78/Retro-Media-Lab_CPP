# Adapters sectoriels ATR, IBM, MSX, COHERENT et Atom

Références C# en lecture seule : entrées correspondantes de `Images/Writing/MediaWritingComposition.cs`, prédicats CanCreate de `IbmRawConversionService.cs`, `MsxRawConversionService.cs`, `Atari/AtrConversionService.cs`, `Commodore/CoherentConversionService.cs` et identifiants de `Constants/MediaImageWriterIds.cs`.

Les adapters réutilisent les contrats sectoriels et backends existants ; ils ne reconstruisent ni ne réencodent les secteurs par une autre voie.

- ATR : trois profils Atari 8-bit et seule extension ATR. La géométrie/le profil source sont validés par Atr::canWrite ; le format cible est transmis à Atr::write.
- IBM : onze profils du catalogue et IBM scan, extensions IMA/IMG. Le prédicat natif vérifie géométrie et charge utile complète. IBM scan conserve la géométrie observée. Aucun identifiant IBM inconnu n’est accepté par simple préfixe dans les capacités d’écriture.
- MSX : quatre profils du catalogue, extension DSK, validation native du profil demandé et de la charge utile complète.
- COHERENT : identifiant Commodore 900, extensions BIN/IMG ; le backend impose son format et sa géométrie zonée. L’adapter ne réétiquette pas une autre image en COHERENT.
- Atom : seul profil Atom DOS et extension DSK. Le test de géométrie ignore le nom du profil source, comme la composition C#. L’écriture linéaire utilise la géométrie cible 40/1/10 × 256. Une réétiquette intermédiaire est inutile pour ce backend qui écrit seulement les octets et valide la cible avec requireFormatId=false.

Les identifiants des writers sont repris du C#, les extensions sont normalisées par l’adapter générique et chaque écriture retourne son unique chemin produit. Le document est gardé vivant par le contrat commun pendant l’opération différée.

La validation de géométrie seule n’affirme pas qu’une image clairsemée est inscriptible jusqu’au dernier octet : Atom et COHERENT conservent les prédicats source, leurs backends signalent ensuite toute donnée manquante. Les services de conversion et la composition globale de tous les writers restent en cours. Le writer ST/MSA et son transformateur dédié sont désormais portés au bloc 121, décrit dans st-msa-writing.md.
