# Primitives Apple IWM GCR

Bloc 40 — référence : AppleIwmGcrCodec et constantes nécessaires de AppleIwmGcrFormat, communs au Macintosh et au Lisa. Huit sources C# originales inchangées sont compilées pour la comparaison.

## Transformation du bloc tagué

L’encodage reçoit exactement 524 octets : 12 tags, puis 512 octets sectoriels. Trois groupes de 175 positions accueillent les octets transformés par trois accumulateurs. Le dernier groupe ne contient que les deux premiers octets ; son troisième octet est nul. Les bits hauts sont regroupés en une valeur de six bits, suivie des six bits bas de chaque octet présent, soit 699 symboles de charge utile.

L’accumulateur commun effectue rotation du premier octet et report des retenues entre les trois sommes. Internal.h isole ces transitions sans modifier leur ordre. Encode.cpp applique les XOR avant la mise à jour des sommes ; Decode.cpp reconstruit les octets avant d’avancer le même état. Les masques et additions uint32 suivent le C#.

Quatre symboles de contrôle suivent la charge utile : bits hauts regroupés, troisième somme, deuxième somme, première somme. L’encodage convertit les 703 valeurs par la table 6-and-2 Apple II partagée. Le décodeur reçoit déjà des valeurs numériques et retourne séparément les quatre valeurs reconstruites dans l’ordre première/deuxième/troisième somme, puis bits hauts regroupés. Il ne compare pas lui-même le checksum enregistré ; cette vérification appartient au décodeur de piste.

## Limites et ressources

Une longueur différente de 524 octets à l’encodage produit IwmTaggedSize. Le décodage consomme les 699 premières valeurs et ignore les valeurs supplémentaires ; les bits hors six bits sont masqués aux mêmes endroits que dans le C#, sans validation supplémentaire des valeurs brutes. Une entrée trop courte déclenche IndexOutOfRangeException lors d’un accès ReadOnlySpan dans le C#. Le C++ la refuse avant tout accès avec IwmSymbolLength, taille reçue et minimum requis. Le comparateur associe ce diagnostic au refus source correspondant.

Les buffers, groupes et résultats sont possédés et libérés par RAII. Les deux fonctions contrôlent l’annulation à l’entrée et dans les boucles. Les constantes de format sont isolées dans Constants.h. Les deux erreurs sont dans MediaEngine/iwm-gcr-primitives.json et ses 29 catalogues, générés avec Argos puis relus ; les traductions précisent que les 524 octets incluent les tags. IWM rejoint la liste commune d’invariants.

AppleIwmGcrCodec est porté intégralement. AppleIwmGcrFormat reste partiel : champs d’adresse/données, paramètres et décodeur de piste commun, ainsi que spécialisations Macintosh/Lisa, seront repris ensuite. Aucun codec de piste IWM n’est encore annoncé au registre.
