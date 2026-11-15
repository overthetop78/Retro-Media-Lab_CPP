# Reconstruction SCP Commodore GCR 1541/1571

## Référence et découpage

Le bloc 22 reprend la branche privée `ReadGcr` de `CommodoreScpSectorImageReader.cs`, avec les fenêtres SCP, le décodeur Commodore et le sélecteur d’intégrité déjà portés. Les cinquante sources originales compilées par le banc sont inchangées. [Validation](../../../validation/media/commodore-scp-gcr.md).

`Formats/CommodoreDisks/FluxReader.cpp` parcourt les pistes et fenêtres, décode puis publie la progression. `GcrReconstruction.cpp` conserve le meilleur candidat de chaque adresse logique, les éléments nécessaires à l’inférence géométrique et l’ordre de première apparition. L’API `readGcrFlux` consomme une image SCP déjà analysée et retourne une image sectorielle possédée. Elle ne remplace pas encore le routage global vers les branches 1581 MFM et Commodore 900.

## Candidats et géométrie

Toutes les fenêtres de chaque piste physique sont décodées en Commodore GCR, sans filtrage préalable des coordonnées physiques. Un candidat doit avoir des données non nulles, une piste enregistrée dans 1..80 et un numéro de secteur non négatif. Le cylindre du secteur décodé désigne ici la piste enregistrée. Les coordonnées logiques et la présence de données sont filtrées comme dans le C# ; aucune taille supplémentaire n’est imposée à cette étape.

La priorité est valide, indéterminée, invalide. Le premier candidat reste retenu en cas d’égalité, ainsi que sa révolution d’origine. Le stockage C++ n’accumule pas les payloads perdants : il conserve uniquement le meilleur, tout en examinant chaque candidat pour la géométrie. L’ordre des blocs suit l’ordre de première apparition des clés, même lorsqu’un meilleur candidat les remplace.

Le résultat est 1571 lorsque le format demandé vaut exactement `commodore.1571`, qu’une piste enregistrée dépasse 40, ou qu’une piste physique quelconque possède la face un, même vide. La comparaison de cette demande est sensible à la casse, comme dans la branche source. Les autres chaînes ne forcent pas de transformation ; ce contrat est celui de la branche GCR, pas du routeur global.

En mode 1571, la géométrie est toujours deux faces de 35 pistes. Sinon, elle passe de 35 à 40 seulement si la piste maximale dépasse 35 et qu’au moins un candidat des pistes 36..40 contient un octet non nul. Un candidat perdant ou qui sera rejeté pour son numéro de secteur peut donc influencer cette décision : c’est le comportement source conservé. Les captures étendues entièrement nulles ne suffisent pas à étendre l’image.

Après cette inférence, les secteurs hors géométrie sont ignorés. Si aucun candidat initial n’existait, une erreur est levée. Si des candidats existaient mais sont tous éliminés ensuite, une image de géométrie complète sans bloc disponible est retournée, conformément au C#. Les secteurs manquants ne sont pas remplis.

## Résultat, progression et ressources

La capacité et le nombre logique de blocs décrivent la géométrie complète ; seuls les blocs réellement retenus sont présents. Chaque bloc reçoit ses coordonnées physiques, son payload, son intégrité et sa révolution. Aucun code de carte d’erreurs n’est déduit de l’intégrité, et aucun autre détail de protection n’est inventé.

Le rappel de progression est synchrone et reçoit la piste terminée, le nombre terminé et le total, y compris pour une piste sans révolution. Il ne doit pas conserver une référence empruntée à la piste. Une exception du rappel est propagée. Les boucles et la finalisation contrôlent l’annulation ; les buffers appartiennent aux objets locaux et sont libérés par RAII sur échec. Aucun fichier ou matériel n’est ouvert par cette API.

Le diagnostic d’absence de candidats est dans `MediaEngine/commodore-scp-gcr.json`, généré depuis en-US avec Argos puis relu/corrigé dans les 29 langues. Les valeurs de progression sont structurées, afin que l’affichage compose ses textes selon la langue active.

## Suite

La branche 1581 MFM, le routage général des formats, les conversions de fichiers et les registres/documents restent à porter. Les contrôles utilisent des fixtures synthétiques et ne déclarent aucune validation sur capture matérielle réelle.
