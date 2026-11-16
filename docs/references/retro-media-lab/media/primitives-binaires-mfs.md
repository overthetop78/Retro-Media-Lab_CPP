# Primitives binaires MediaFileSystems — bloc 233

Sources C# en lecture seule : `BigEndianInt32.cs`, `LittleEndianUInt16.cs`, `LittleEndianUInt24.cs` et `Constants/BitConstants.cs`.

Les fonctions little/big endian sont regroupées dans `src/Primitives/Binary.h`. La lecture et l’écriture little endian utilisent une représentation non signée de même largeur pour les opérations de décalage ; la valeur signée est récupérée par `std::bit_cast`. Les vérifications de bornes précèdent tout accès.

Une seule fonction `Binary::little24` lit les trois octets little endian avec un contrôle de plage sans addition susceptible de déborder. Elle remplace les assemblages locaux du codec ProDOS, des adresses de répertoire ADFS, de la capacité FileCore et des entrées de l’ancienne carte libre FileCore. Les fonctions propres aux formats, leurs unités et leurs calculs de capacité restent conservés ; aucun nouvel algorithme de système de fichiers n’est introduit.

Validation : compilation Debug réussie après ajout de l’inclusion explicite nécessaire dans OldMap ; build final sans avertissement. Le contrôle temporaire a vérifié les extrêmes signés 8/16/32/64 bits, les lectures et écritures aux deux ordres d’octets, les valeurs 24 bits nulles et maximales, les décalages hors plage et `size_t` maximal. Il n’a pas retesté les formats ProDOS ou Acorn. Son dossier et ses artefacts ont été supprimés.

Le rapprochement préalable du suivi avec le C# a aussi contrôlé les empreintes de ses 1 492 fichiers inventoriés : aucune différence ni source absente. Ce contrôle d’empreintes identifie la référence stable ; il ne prouve pas à lui seul la complétude du portage.
