# Lecture SCP et reconstruction Commodore 900

## Source et responsabilités

Le conteneur reste dans `MediaEngine/Formats/Scp/` : modèles possédés, calculs/temps, lecteur et fenêtres séparés. La reconstruction Commodore 900 est dans son dossier de format et appelle le codec GCR existant ; le choix d’intégrité stable est partagé dans `Decoding/Candidates.h`. Aucun accès matériel ou lien direct supplémentaire d’App vers MediaFileSystems/MediaAnalysis.

Le banc compile 47 sources C# originales en lecture seule, dont les algorithmes de lecture, fenêtres, sélection, décodage GCR et reconstruction Commodore. Toutes leurs empreintes correspondent à l’inventaire. Des contrats minimaux de banc remplacent les registres/documents, la valeur sectorielle de sortie et les branches des familles non exercées ; ils ne remplacent aucun algorithme comparé. Les branches hors Commodore 900 lèvent une erreur si elles sont appelées dans le banc. [Validation](../../../validation/media/scp-reader.md).

## Lecture bornée

Les octets ou une source d’accès aléatoire peuvent être lus. Les résultats possèdent leurs métadonnées et intervalles ; aucune vue n’échappe vers l’entrée. La source est empruntée uniquement pendant l’appel. Aucun cache global, arrière-plan ou instantané immuable de la source n’est introduit. Les reads d’une source sont exacts, bornés et limités à 65 536 octets ; le checksum couvre le conteneur par blocs, sans copie complète supplémentaire.

L’en-tête valide signature, nombre de révolutions un à 64, plage de pistes zéro à 167, largeur zéro ou seize et sélecteur de tête zéro à deux. Le support étendu est refusé après lecture de l’en-tête, comme dans le C#. Les autres bits sont préservés. La reconnaissance de signature exige au moins seize octets mais ne prétend pas valider le conteneur entier.

Les offsets de table sont little endian ; les mots de flux sont big endian sur seize bits. Un mot zéro cumule 65 536 ticks, un mot suivant non nul termine l’intervalle ; un cumul final sans transition reste un intervalle comme dans la source. Les plages, conversions et cumuls sont contrôlés avant accès/allocation. La limite de longueur/position et de taille de données est celle des entiers signés C# sur 32 bits ; les intervalles sont limités à uint32. La réservation initiale des intervalles est bornée au lieu d’être proportionnelle aux mots déclarés, qui peuvent être majoritairement des marqueurs de dépassement.

La valeur de checksum incorrecte est conservée comme un état faux et ne rend pas le fichier illisible. Un checksum déclaré nul est accepté pour une capture Writable. Des offsets se chevauchant ou pointant vers des octets structurels restent acceptés si les validations C# les acceptent ; aucun nouveau rejet de format arbitraire.

## Fenêtres et secteurs

Chaque révolution est raccordée à la suivante ; la dernière reste seule. Les numéros sont à base un et le drapeau de continuité est conservé. Les sommes d’intervalles et d’index sont contrôlées. Le visiteur traite une fenêtre à la fois pour ne pas conserver simultanément toutes les copies doubles. L’API de collection et la fenêtre primaire restent disponibles ; la primaire valide toutes les fenêtres comme la source, y compris un débordement ultérieur.

La reconstruction Commodore 900 ignore les pistes hors de sa géométrie, les données de taille différente de 512, les cylindres enregistrés incohérents et les secteurs hors de la zone. La face physique vient de la piste, puisque le décodeur expose une face logique zéro. Le candidat d’intégrité vraie prime sur l’inconnu, puis sur le faux ; les égalités gardent le premier candidat rencontré. Les blocs retiennent la révolution gagnante et les octets correspondants. La capacité se termine au plus grand bloc reconstruit, sans compléter les trous ni imposer la capacité maximale de la disquette.

La progression signale chaque piste, même ignorée, avec son adresse physique et les compteurs. Une exception de progression ou une annulation abandonne le résultat et libère les buffers par RAII. Les erreurs sont des codes/paramètres ; les dix-sept messages sont générés par Argos dans 29 langues puis relus, avec correction des paramètres perdus, des négations et du vocabulaire technique.

## Portée restante

Le writer SCP, le cache éventuel, les adaptateurs de documents, les registres/reconnaissance générale, les visualisations et les autres familles SCP restent à porter. Les modèles ne proposent pas encore la présentation symbolique de tous les drapeaux ; leurs octets sont préservés. La surcharge des fenêtres pour ProtectedTrack sera raccordée avec ce modèle. La reconstruction interne est validée sur fixtures, sans lecture ou écriture de matériel.
