# Conversion des représentations

Références C# : `Interfaces/Conversion/IMediaRepresentationConverter.cs`, `Images/Conversion/MediaRepresentationConverterRegistry.cs`, `MediaRepresentationConversionResult.cs`, `MediaConversionComposition.cs`, `Images/Formats/Floppy/Scp/Conversion/ScpFluxToSectorRepresentationConverter.cs`, `SectorToFluxRepresentationConverter.cs`, `Images/Conversion/Flux/FluxToHfeRepresentationConverter.cs` et le convertisseur de signal déjà porté.

Le registre possède les convertisseurs et résout le premier candidat dont les représentations et le format cible conviennent. Il refuse les inscriptions nulles et les identifiants dupliqués sans tenir compte de la casse. Les résultats possèdent document, diagnostics et pertes ; les messages d'erreur et pertes utilisent les catalogues traduits.

Flux vers secteurs : réutilisation de `ScpReconstruction::Reader` ; formats autorisés issus des lecteurs sectoriels, des définitions et du profil IBM scan. CPCEMU et UCSD restent des conteneurs : aucun indice de reformatage n'est transmis au reconstructeur. Progression initiale sur cylindres et têtes triés distincts, puis progression par piste de 5 à 90 pour cent. La source reste possédée par le résultat.

Secteurs vers flux : réutilisation de `Scp::SectorConverter` ; cibles SCP, HFE et HxC Stream ; représentation intermédiaire SCP et avertissement précisant qu'il ne s'agit pas de la capture physique originale.

Flux vers HFE : première révolution par piste, cellule de la définition bitcell ou estimation médiane arrondie au profil nominal le plus proche ; arrondi pair identique à Math.Round, alignement sur huit bits, égalisation des longueurs par cylindre ; le modèle commun normalise ensuite l'ordre des pistes par cylindre et tête. Préservation de la protection en écriture et des métadonnées, suppression de hfe.bitRate, encodage FM/MFM de la définition. Les temporisations et pertes sont explicitement déclarées.

L'adaptateur du convertisseur de signal réutilise son implémentation existante. La composition des trois convertisseurs Acorn et le service général de planification/écriture restent à porter avant de considérer la composition C# complète.

Validation : compilation Debug MediaEngine et des 30 catalogues réussie, sans avertissement de compilation. Contrôle ciblé réussi : refus des inscriptions nulles et identifiants dupliqués sans tenir compte de la casse, résolution, préparation HFE, alignement et égalisation par cylindre, suppression des anciennes métadonnées de débit, préservation des autres métadonnées et de la protection, filtres de conteneur CPCEMU/UCSD, progression, conservation de la source, génération de flux intermédiaire SCP et pertes déclarées. Annulation et refus des formats non autorisés vérifiés. Aucun fichier média externe, aucun test de codec relancé.

Une attente initiale du test sur l'ordre des pistes était incorrecte : `ProtectedTrackImage` normalise déjà cet ordre. Le test a été corrigé. Sa sortie normale est explicitement zéro ; les échecs lèvent une exception gérée pour éviter une boîte d'assertion Windows. Le test et son journal ont été supprimés après réussite. Les sources C# sont restées en lecture seule.
