# Reconstruction sectorielle 86F — bloc 231

Références C# conservées en lecture seule : `Images/Conversion/I86fBitCellFluxConverter.cs`, `Images/Reading/Decoding/I86f/I86fSectorImageReader.cs`, son fournisseur d’exceptions et `Constants/I86fDiskImageFormatIds.cs`.

Le lecteur de conteneur existant et sa projection en flux protégé restent réutilisés. Le service sectoriel manquant est ajouté à côté de ce backend, dans `Formats/I86f/SectorReader.cpp`. Il choisit explicitement `iso.mfm` lorsque les drapeaux l’indiquent et `iso.fm` sinon. Chaque conversion accumule les cellules jusqu’à une transition, sans ajouter de transition en fin de piste ; une piste sans transition est ignorée.

La collecte réutilise `IsoReconstruction::Candidates` et conserve la révolution zéro et l’index logique de la piste source. Les secteurs sans données ou à numéro négatif sont ignorés. La mesure et la construction uniforme réutilisent le constructeur ISO commun : ordre des secteurs à base zéro, géométrie IBM identifiée par capacité pour 512 octets, identifiant `86f.taille.cylindres.faces.secteurs` sinon. Les limites du constructeur commun restent appliquées ; le service ne rend pas artificiellement disponibles des secteurs qu’il écarte.

Le point d’entrée public `reconstructI86fSectorsAsync` retourne un document opaque utilisable avec les services publics de MediaEngine. Il n’expose pas SectorImage ou une autre classe MediaFileSystems à App et ne change pas la reconnaissance ordinaire de 86F en flux. Les erreurs de flux, de géométrie et d’absence de candidats réutilisent les diagnostics traduits existants. L’annulation est propagée pendant la lecture et le parcours.

La conversion refuse aussi les cellules qui ne valent pas zéro ou un et un temps total qui déborderait les ticks 32 bits, au lieu de laisser un calcul déborder.

Validation : compilation Debug réussie après correction des références aux identifiants des codecs ; aucun avertissement dans le build final. Un unique banc temporaire avec décodeurs injectés a confirmé le routage FM/MFM, les intervalles exacts sans transition finale ajoutée, les pistes vides, le filtrage des secteurs, les identifiants de géométrie IBM/86F, l’ordre à base zéro et l’annulation. Ce contrôle ne redécode aucun format média ni ne reteste les codecs. Le banc a été corrigé pour ne pas exiger la présence d’un secteur que le constructeur ISO commun exclut hors géométrie. Son dossier et ses artefacts ont été supprimés.
