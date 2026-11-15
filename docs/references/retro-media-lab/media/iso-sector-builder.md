# Construction sectorielle ISO

## Référence et découpage

Le bloc 29 porte les modèles de candidats et les aides de mesure, sélection, construction uniforme et normalisation. Seize sources C# originales, dont les modèles sectoriels et leurs validations, sont compilées inchangées pour la référence. [Validation](../../../validation/media/iso-sector-builder.md).

`Reconstruction/Iso/Models` possède les groupes ordonnés et les candidats avec révolution et piste source facultative. `Candidates` fournit l’indexation et la sélection ; `Measurement` mesure les géométries ; `Builder` produit une image uniforme ; `Normalizer` complète les données. Les fenêtres/décodeurs, la collecte parallèle avec cache et les politiques par machine sont des travaux distincts qui réutiliseront ce socle. Aucun accès matériel/fichier ou GUI n’est ajouté.

## Ordre et sélection

Les groupes conservent leur ordre d’insertion en plus d’un index de recherche. À chaque adresse, la priorité d’intégrité est vraie, absente, fausse ; le premier candidat est conservé à égalité. Les données retournées sont possédées. `bestData` ignore les candidats sans données ou avec données vides avant de sélectionner ; il retourne une copie vide si l’adresse ou des données utilisables sont absentes.

Un groupe sans candidat ne fournit pas de meilleur élément. Une donnée absente dans un candidat utilisé pour la mesure ou la construction produit un diagnostic structuré, plutôt que de laisser une erreur de pointeur/optionnel se propager. Le C# pouvait produire NullReferenceException ou ArgumentNullException pour cet état invalide selon l’opération ; les deux refus sont regroupés sous le diagnostic `IsoCandidateData` en C++.

## Mesure

La taille majoritaire est mesurée sur tous les candidats de tous les groupes, pas seulement les meilleurs. Les fréquences sont comptées complètement avant la sélection : à égalité finale, la première taille rencontrée reste prioritaire. Les dimensions sont le maximum des coordonnées plus un, avec la sémantique int32 source.

Le nombre de secteurs par piste est le nombre distinct d’adresses par couple cylindre/face, puis sa valeur majoritaire entre pistes. À égalité de fréquence, le plus grand nombre est retenu. La numérotation sectorielle distincte est triée et le mode zéro n’est activé que si le premier numéro est exactement zéro.

## Construction uniforme et métadonnées

L’ordre physique enregistré est calculé pour tous les groupes, y compris ceux qui seront rejetés ensuite. Dans chaque piste, le meilleur candidat détermine l’offset ; le tri est stable à égalité. Le filtrage respecte le C# : numéro supérieur à la géométrie sauf autorisation, coordonnées supérieures à la géométrie, puis résultat de la fonction d’index hors plage. Les coordonnées négatives ne sont pas filtrées artificiellement en amont ; les adresses logiques résultantes sont soumises aux validations sectorielles communes.

La fonction de normalisation est facultative. Sans elle, aucune taille n’est corrigée silencieusement. Des données devenues vides sont omises. Les fonctions d’index/normalisation conservent leur ordre d’appel et leurs exceptions sont propagées. L’indice logique utilise les opérations int32 source et l’image vérifie format, dimensions, capacité, doublons, débordement du nombre de blocs et adresses hors plage.

Les blocs conservent intégrité, révolution, intégrités séparées, marque supprimée, CHR enregistré, code N et rang physique. Les tags, codes de format/diagnostic et octets de CRC ne sont pas ajoutés lorsque le constructeur source les laisse vides. Le paramètre SourceTrack reste dans le candidat sans devenir une métadonnée de bloc inventée.

Le padding complète un secteur court avec des zéros ; un secteur trop long retourne des données vides, sans troncature. Une taille négative retourne aussi une séquence vide, conformément au test de longueur source.

## Géométrie déclarée et lecture

Le modèle sectoriel commun n’applique plus la limite d’une lecture à la taille nominale d’une image sparse : une géométrie peut déclarer un très grand bloc sans en allouer les données, comme en C#. Le garde d’un mébioctet est conservé dans les images adossées à une source pour les lectures effectives. La validation couvre les deux comportements, sans allouer de bloc géant.

## Ressources, traduction et portée

Les buffers et images sont possédés ; RAII couvre sorties et exceptions. Les aides et la construction vérifient l’annulation initiale et autour des fonctions fournies par le demandeur. Aucune dépendance aux contrôles d’App n’est ajoutée.

Les cinq diagnostics de `MediaEngine/iso-sector-builder.json` sont générés avec Argos depuis en-US, puis relus dans les 29 langues. Les corrections conservent le sens d’adressage cohérent, la négation des données/fonctions manquantes, les termes techniques ISO et les trois paramètres géométriques. Les erreurs de géométrie/format/bloc communes restent dans MediaFileSystems.

La collecte FM/MFM depuis les captures, le cache, les politiques par famille, registres/documents et conversions restent à porter.
