# Bande de données SIMH

Références C# consultées en lecture seule le 16 novembre 2026 : Constants/SimhTapeConstants.cs et Images/Formats/Tape/Simh/SimhTapeFormat.cs, SimhTapeReader.cs, SimhTapeWriter.cs dans F:/GW GUI/src/RetroMediaLab.MediaEngine. SHA-256 des quatre sources vérifiés contre l’inventaire.

Le parseur commun sépare le framing du reader : il parcourt tous les mots d’objet avant reconnaissance, refuse les mots tronqués, bits de longueur réservés, records nuls, données/padding dépassant la source et mots de tête/fin différents. La présence de padding pair est requise ; sa valeur n’est pas interprétée, comme dans le C#. Les mots marqueurs sont bande (0), gap d’effacement (fffffffe) et fin de support (ffffffff).

Les records sont des segments DataBlock orientés Forward, possédant un range de la source commune et les métadonnées exactes simhObjectKind/simhRecordError. Les marqueurs conservent simhMarkerValue en hexadécimal sur huit chiffres. Le drapeau d’erreur et les octets physiques après une fin logique deviennent des avertissements traduisibles avec arguments. Un EOM arrête le parcours ; une erreur d’I/O ou une annulation ne devient pas une reconnaissance négative.

Le writer trie les segments par position avec un tri stable, valide les métadonnées et la longueur 24 bits, reprend le drapeau d’erreur et copie les ranges par le helper de copie existant dans le writer atomique. Il écrit padding nul puis mot de fin identique au mot de tête. Une amélioration de fiabilité refuse les objets après EOM au lieu de produire des records invisibles à la relecture. Les marqueurs avec données et objets inconnus sont refusés. Buffers et sources sont possédés/RAII ; un échec pendant la copie conserve la destination.

L’extension tap ne suffit pas à reconnaître SIMH ; la structure complète est vérifiée, conformément au C#. L’exploration MFS et la composition globale ne sont pas déclarées achevées par ce bloc de conteneur.
