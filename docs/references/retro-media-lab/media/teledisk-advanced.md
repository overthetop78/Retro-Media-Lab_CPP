# TeleDisk : décompression avancée

Référence source : `Images/Formats/Floppy/TeleDisk/Td0AdvancedDecompressor.cs`, consultée dans le dépôt C# en lecture seule.

Le décompresseur reprend le flux LZSS/Huffman adaptatif : fenêtre de 4096 octets, anticipation de 60 octets, seuil de deux, alphabet de 314 symboles, fréquences initiales à un, tables de positions et renormalisation à 0x8000. Le dictionnaire est initialisé aux espaces sur sa partie précédant l’anticipation ; sa fin reste nulle comme dans le tableau C#.

La lecture des bits est MSB-first. La fin du flux au milieu d’un caractère ou d’une distance arrête le décodage, comme les EndOfStreamException traitées dans le C#. Les références au dictionnaire autorisent le recouvrement et la position revient dans la fenêtre par masque. Atteindre exactement 3 000 000 octets déclenche le diagnostic de limite, conformément au test d’égalité du C#.

En C++, l’entrée est une vue utilisée pendant l’appel synchrone, les tableaux sont possédés sur la pile et la sortie est un vector ; aucune ressource à décharger manuellement. Un stop_token est vérifié avant le démarrage et à chaque symbole. La longueur du flux est contrôlée par indice d’octet plutôt que par multiplication de sa longueur par huit, évitant un débordement du compteur de bits.

La reconstruction déplace seulement les éléments entre la position d’insertion et la branche active avec move_backward. Le C# utilise Array.Copy avec une longueur multipliée par deux ; le port ne reprend pas la copie des éléments au-delà de la portion active. Le contrôle synthétique exerce la renormalisation. Aucun défaut reproductible du C# n’est revendiqué à partir de cette différence de copie.

Le lecteur de conteneur devra appeler cette fonction sur les données après l’en-tête et reconstruire la signature normale et son CRC avant de poursuivre l’analyse, comme Td0Reader.ReadDetailed.
