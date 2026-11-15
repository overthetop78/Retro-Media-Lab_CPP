# Codec Micral N FM

Bloc 43 — portage de MicralNFmTrackEncoder, MicralNFmDecoder et MicralNFmFormat (checksum et modèle de bloc inclus), avec les fonctions FM et les contrats de piste communs. Trente-quatre sources C# originales sont compilées pour la comparaison.

## Encodage et checksum

Codecs/MicralN sépare constantes, contrats, checksum, encodeur et décodeur. Chaque secteur conserve l’ordre demandé et doit contenir 128 octets ; sa taille est vérifiée avant son numéro sur un octet. Le cylindre et la face passent par les validations communes. La face physique n’est pas enregistrée ; la face logique lue reste zéro.

Le préambule est composé de trois octets nuls et de FF encodés en FM, soit AAAAAAAAAAAAFFFF en cellules compactées. Suivent numéro de secteur, cylindre, 128 octets de données et checksum, encodés en FM, puis 128 cellules nulles de gap. Les attributs de marque supprimée, taille imposée et métadonnées enregistrées sont ignorés comme dans cet encodeur C#.

Le checksum démarre à zéro. Chaque mise à jour combine les XOR de donnée/somme/complément et la retenue du bit haut pour ajouter donnée et retenue, puis réduire le résultat sur un octet. La même primitive est partagée par lecture et écriture ; elle n’est pas remplacée par une somme additive ordinaire ou un CRC.

## Lecture et balayage

Le décodeur reconstruit le FM adaptatif commun et recherche le préambule complet. La lecture directe d’un bloc suppose la position fournie ; elle ne revalide pas la marque. Le bloc total occupe 2 160 bits. La lecture prend les 131 octets après les 64 bits de marque avec le lecteur commun des bits de données alternés. Les horloges ne sont pas vérifiées séparément, conformément au C#.

Le checksum ne couvre que les 128 octets de données, pas l’adresse. Des données lisibles à checksum faux sont conservées avec intégrité fausse. Une lecture tronquée ajoute une structure de marque seule sans secteur. Les données décodées ne comprennent ni adresse ni checksum. Le code de taille est zéro ; tag, format, CRC enregistrés et états d’intégrité séparés restent absents.

Après une marque, l’avance du balayage reste de 64 bits, même si un bloc complet a été lu. Des motifs de préambule à l’intérieur des données peuvent donc être examinés comme dans la source ; aucune heuristique nouvelle ne les filtre. Les structures restent dans l’ordre de découverte, sans suffixe circulaire ni tri ajouté.

## Diagnostics et ressources

Les deux erreurs et deux descriptions de MediaEngine/micraln-fm.json sont générées avec Argos puis relues dans les 29 langues. Les formulations de plage et taille déjà corrigées sont réutilisées. Micral N rejoint la liste commune d’invariants. Les résultats sont possédés et libérés par RAII ; tous les points d’entrée contrôlent l’annulation, y compris les opérations de checksum et lecture directe.

Le codec peut être fourni au registre explicite. Le catalogue complet, les autres codecs et les politiques de reconstruction restent des travaux distincts.
