# Pistes protégées et adaptateurs — référence C#

Le bloc 30 reprend les modèles ProtectedTrack/ProtectedTrackImage, TrackFeature/Kind, TrackTimingSegment, TrackFluxRevolution, EncodedDiskTrack/EncodedTrackImageFactory, ScpProtectedTrackImageAdapter et la surcharge protégée de ScpTrackDecodeWindowFactory du dépôt C# en lecture seule. Les trente sources réellement compilées par le banc ont une empreinte conforme à l’inventaire.

## Données et validation

Models/Flux possède les bits facultatifs, plages temporelles, structures, caractéristiques et révolutions accompagnées de leur résolution en nanosecondes. Les coordonnées doivent être positives ou nulles ; une piste exige des bits non vides ou des révolutions. Les métadonnées orientées bits exigent la présence du tableau de bits et des plages positives incluses dans sa longueur. La durée d’une cellule est finie et positive. Les résolutions sont strictement positives.

Une image exige des pistes, refuse les doublons puis trie par cylindre et face. Si plusieurs coordonnées sont dupliquées, la première coordonnée rencontrée dans l’ordre d’entrée est signalée, comme le GroupBy source. Les objets possèdent leurs données ; aucune référence pendante vers le conteneur ou les arguments n’est conservée.

## Adaptation

L’adaptateur SCP conserve tous les intervalles et index bruts, leur résolution `(resolution + 1) × 25 ns` et la protection d’écriture définie par le flag source. Il n’invente pas de bits ni de métadonnées. L’adaptateur d’encodage conserve bits et révolution, ajoute la plage temporelle couvrant les bits avec `cellTicks × resolutionNanoseconds`, et produit une image sans protection d’écriture.

Les fenêtres de décodage utilisent le même algorithme pour ScpTrack et ProtectedTrack : révolution courante jointe à la suivante, dernière révolution seule, indice à base un et indicateur de continuité. Les valeurs brutes ne sont pas rééchantillonnées, y compris pour des résolutions différentes dans une piste protégée, conformément à la source. Les débordements de somme des index et de nombre d’intervalles sont contrôlés. La collecte vectorielle valide toutes les jointures avant de rendre ses fenêtres.

## Ressources et traductions

RAII couvre buffers, objets et sorties par exception. Les adaptateurs et fenêtres vérifient l’annulation et propagent les exceptions des callbacks. Les neuf diagnostics de MediaEngine/protected-tracks.json ont été générés avec Argos, puis relus dans les 29 langues ; les corrections préservent paramètres, négations, unités et terminologie des cellules de bits.

La collecte des candidats ISO, son cache, les politiques par famille et les représentations/documents restent distincts et ne sont pas déclarés portés par ce bloc.
