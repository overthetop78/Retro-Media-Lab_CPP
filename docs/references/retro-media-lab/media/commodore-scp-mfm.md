# Reconstruction SCP Commodore 1581

## Référence et organisation

Le bloc 25 porte la branche Read1581 de `CommodoreScpSectorImageReader`. Le banc compile 51 sources C# originales inchangées. [Validation](../../../validation/media/commodore-scp-mfm.md).

`Formats/CommodoreDisks/FluxReader` parcourt les pistes et fenêtres SCP, appelle le décodeur ISO MFM et publie la progression. `MfmReconstruction` accumule les candidats puis construit les blocs D81, indépendamment de l’acquisition et de l’affichage. Les géométries appartiennent à MediaFileSystems ; les fenêtres, le décodage, la sélection d’intégrité et les modèles sectoriels existants sont réutilisés.

## Candidats et coordonnées

Une donnée absente, différente de 512 octets ou numérotée hors de 1..10 est rejetée. La clé est le cylindre et la face physiques SCP, avec le numéro de secteur décodé. Les valeurs CH du champ MFM ne remplacent pas les coordonnées physiques : elles peuvent être incohérentes dans la capture, comme dans la source.

Le meilleur candidat suit la priorité intégrité vraie, absente, fausse ; le premier est conservé à égalité. L’ordre d’insertion initial des clés reste celui des blocs produits même lorsqu’un meilleur candidat remplace les données. La révolution source à base un suit le candidat retenu. Les autres métadonnées ne sont pas inventées ni copiées lorsque le constructeur C# les laisse à leur valeur par défaut.

Les candidats hors de 0..79 cylindres ou 0..1 faces sont filtrés lors de la construction. Ce filtrage tardif conserve la distinction entre absence de candidat décodé et absence de candidat utilisable. Les deux conditions ont des diagnostics distincts et traduisibles.

## Vue logique D81

Le helper de géométrie commun calcule le bloc initial avec l’inversion des faces source : cylindre × 40 + ((face XOR 1) × 10 + numéro − 1) × 2. Chaque secteur physique fournit ses deux moitiés de 256 octets. Les adresses logiques sont (bloc / 40, 0, bloc % 40), avec intégrité et révolution identiques sur les deux moitiés. La géométrie de sortie est 80 × 1 × 40 blocs, soit 819 200 octets, même lorsque la capture est partielle.

Les données sont possédées, les ressources temporaires libérées par RAII. L’annulation est contrôlée avant et pendant le parcours et la construction ; elle est également propagée après une notification demandant l’arrêt. Les exceptions du rappel de progression ne sont pas masquées.

## Traductions et limites

Deux clés sont dans `MediaEngine/commodore-scp-mfm.json`, générées avec `python scripts/translations/translate.py --all --files MediaEngine/commodore-scp-mfm.json`, puis relues dans les 29 langues. La liste commune d’invariants protège le nom technique « Commodore 1581 MFM ». Les corrections rétablissent notamment les négations perdues en japonais et turc, les libellés laissés anglais et la distinction décodage/reconstruction utilisable.

Le routage général par format, les entrées fichier/orchestration, registres, documents et conversions restent à porter. Ce bloc valide les données synthétiques, sans essai de matériel ou de média utilisateur.
