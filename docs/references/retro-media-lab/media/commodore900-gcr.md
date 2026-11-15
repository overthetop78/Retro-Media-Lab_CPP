# Commodore 900 — codec de piste GCR

## Référence et organisation

Les sources C# originales de format, descriptions, décodeur et encodeur sont reprises dans `MediaEngine/Codecs/Commodore/`. Les modèles de décodage possédés et descriptions traduisibles sont séparés dans `Decoding/` ; les requêtes, validations et primitives d’encodage sont dans `Encoding/`. Vingt-deux empreintes de sources et contrats consultés correspondent à l’inventaire. Le dépôt C# reste inchangé.

Le banc compile 39 sources C# originales, sans adaptation de leurs algorithmes. Les identifiants, bits, intervalles, données, métadonnées et descriptions anglaises sont comparés au résultat C++. [Validation](../../../validation/media/commodore900-gcr.md).

## Encodage

L’identifiant est `commodore900.gcr`. Chaque secteur contient 512 octets ; le numéro de secteur et le cylindre peuvent utiliser toute la plage d’un octet, les faces sont zéro ou un. Ces limites du codec sont distinctes des zones du conteneur brut Commodore 900. Les secteurs répétés sont conservés. Les champs de requête que le C# ignore restent ignorés.

L’en-tête contient la marque 0x08, le cylindre, le secteur et leur XOR ; les données contiennent la marque 0x07, le contenu et son XOR. Chaque bloc est précédé de 40 cellules de synchronisation à un et suivi de 120 cellules alternées. Le GCR partagé produit 5 500 cellules par secteur. La requête commune emploie par défaut 40 ticks par cellule et 8 000 000 ticks d’index. La révolution conserve le comportement C# : aucune troncature ou complétion de la piste à la durée d’index.

## Décodage

La reconstruction NRZI emploie les primitives communes. Chaque suite d’au moins dix cellules à un devient une structure de synchronisation, même si le bloc suivant est invalide ou tronqué. Les blocs reconnus alimentent les octets décodés. Les structures sont triées de façon stable par position.

Un en-tête est associé au premier bloc de données situé strictement après sa fin et avant l’en-tête suivant. Les données immédiatement adjacentes à la fin de l’en-tête ne sont donc pas associées, conformément au C#. La recherche utilise une borne supérieure dans la liste ordonnée plutôt qu’une recherche répétée depuis son début. Les blocs non associés restent visibles dans les structures.

Le secteur décodé possède une face logique zéro, un code de taille deux et 512 octets, comme la source. L’intégrité combinée est fausse pour un en-tête ou des données invalides, vraie lorsque les deux sont valides, indéterminée pour un en-tête valide sans données. Les champs individuels d’intégrité d’en-tête et de données restent indéterminés comme dans le C#, sans enrichissement inventé. La confiance est le nombre de secteurs valides divisé par treize, plafonné à un.

## Traductions et ressources

Les descriptions possèdent des clés et des arguments imbriqués ; la résolution se fait à la demande. Un changement de langue ne nécessite pas un nouveau décodage. Les identifiants techniques, octets et données restent inchangés. Neuf diagnostics et sept descriptions/états sont dans `MediaEngine/commodore900-gcr.json`, avec les 29 catalogues de langues générés par Argos puis relus. Les placeholders et plusieurs contresens ont été corrigés après génération.

Les conteneurs sont possédés par valeur ; les chemins d’exception et d’annulation libèrent leurs allocations. Des contrôles d’annulation et de capacité précèdent les allocations/mutations concernées. Aucun fichier ou matériel n’est ouvert par le codec.

## Suite du portage

Le raccordement du flux décodé à l’image sectorielle, les registres, documents et conversions de fichiers restent à construire. Les aides ISO/attributs du socle commun seront validées avec leurs codecs consommateurs ; les descriptions génériques et erreurs propres aux autres formats restent partielles. Ce bloc ne constitue pas la validation d’une chaîne complète de lecture/écriture physique ni d’un support réel.
