# Fabrique, flux et adaptateurs communs

Sources C# consultées en lecture seule. La représentation flux conserve le ProtectedTrackImage partagé, les pistes, révolutions, transitions et protection en écriture sans reconstruction sectorielle. Les surfaces sont triées/dédupliquées. Les compteurs reproduisent les sommes vérifiées du C# : int32 pour les révolutions, somme int32 par piste puis int64 total pour les transitions. La longueur logique reste absente.

La fabrique construit les documents de disquette secteur ou flux sans reconnaissance ni décodage supplémentaire et conserve leurs métadonnées. Les validations nulles et débordements utilisent des diagnostics typés et traduisibles.

Le mécanisme commun des writers conserve les capacités, normalise/déduplique les formats et extensions, extrait la représentation attendue, applique le prédicat de cible éventuel puis celui du document et valide la liste de fichiers produits. Le callback flux reçoit les métadonnées du document. Une future conserve l’état immuable de l’adaptateur et le document ; elle ne capture pas this. Les callbacks natifs sont synchrones et exécutés par la future différée, ce qui réutilise directement les writers C++ existants.

Les readers D64/D71/D81 fournissent maintenant ImageReader. Comme en C#, leurs sondes se fondent sur format demandé et longueur, indépendamment de l’extension. Leur lecture prend le snapshot borné du contexte puis réutilise le backend natif. Les deux adaptateurs de writers reprennent les entrées Commodore DOS et D81 de MediaWritingComposition : couples format/extension explicites, aucune transformation silencieuse, carte d’erreurs conservée si tous les blocs disponibles la fournissent. La validation explicite des pistes 35/40 du writer DOS est rétablie avant sortie.

La composition globale des readers/writers et les autres entrées de MediaWritingComposition restent à porter ; ces adaptateurs ne déclarent pas cette composition complète.

| Source | SHA-256 vérifié |
| --- | --- |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/D64/D64Reader.cs` | `0b4eee1e66804870dd67f3cef6cd635b18058b7b31486b710fad0047055ce5e8` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/D71/D71Reader.cs` | `ce3498a22262f64a915c7873c96a061b655dba6f8ed49b91ff53f4de4eedf1e0` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/D81/D81Reader.cs` | `809fc8d388936347b1836ef686e92341db9f060ba9f20445153805f339e69554` |
| `src/RetroMediaLab.MediaEngine/Images/Models/Flux/FluxMediaImageRepresentation.cs` | `319b895f669a5bf546c81d0dbb6f43e99824f32b7c2f5c2d23e7bb3af6575cd0` |
| `src/RetroMediaLab.MediaEngine/Images/Reading/MediaImageDocumentFactory.cs` | `3ad45defa2c22882f80e7cde622c0e2b2b95006106d49988a2b0fbc752cd26ea` |
| `src/RetroMediaLab.MediaEngine/Images/Writing/FluxMediaImageWriterAdapter.cs` | `83e712be948602edf1891e8049b86d25380a1187f2661b76aebae49876b84fd0` |
| `src/RetroMediaLab.MediaEngine/Images/Writing/MediaImageWriterAdapter.cs` | `0f74dada389b48c1bc9d8c6d20f08fd9b7ce0e479a592ea8eb4c86cbb92c6fb1` |
| `src/RetroMediaLab.MediaEngine/Images/Writing/MediaWritingComposition.cs` | `1157601f51fe21bde47fd91265b68ab9eb69f461e477127908ed78bc650c5f8d` |
| `src/RetroMediaLab.MediaEngine/Images/Writing/SectorMediaImageWriterAdapter.cs` | `3c88ba76818ddee562fefe4859ff6378ef3b18b1b8093ab22561208a1712e614` |
