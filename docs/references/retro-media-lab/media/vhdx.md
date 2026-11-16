# VHDX autonome et CRC32C

Référence en lecture seule : les trois sources `Images/Formats/HardDisk/Vhdx/`, `Functions/Crc32CFunctions.cs` et les constantes VHDX de `HardDiskFormatConstants.cs`. Les quatre empreintes sont vérifiées dans l'inventaire.

Le découpage sépare constantes et contrats, structures/GUID/CRC, métadonnées, lecteur de blocs et génération des structures et données. La primitive CRC32C partagée reste dans Primitives ; elle utilise le polynôme Castagnoli inversé et une table calculée à la compilation. BlockImage, les sources et la transaction atomique avec positionnement sont réutilisés. MediaEngine n'accède ni à App ni à MediaAnalysis.

Les GUID sont conservés dans l'ordre binaire little-endian des champs .NET/Windows et convertis explicitement pour leurs représentations textuelles. Les deux en-têtes sont qualifiés par signature, CRC32C et version ; la séquence la plus élevée choisit l'en-tête actif. Un journal actif nécessite sa relecture et reste refusé. Les deux tables de régions sont lues, puis la première copie valide est retenue, comme en C#. Les régions inconnues obligatoires sont refusées ; les régions facultatives sont ignorées.

Les cinq métadonnées obligatoires décrivent paramètres, taille virtuelle, identifiant Page83, taille logique et taille physique des secteurs. Les bornes de région, tailles, puissances de deux et alignements sont vérifiés ; les secteurs de 512 et 4096 octets sont pris en charge. Une région trop courte pour la table de métadonnées échoue avant de lire hors de cette région.

La BAT distingue données stockées, zéros et plages non allouées. Les emplacements intercalés des bitmaps de secteurs sont sautés pour calculer l'index des blocs. Les blocs partiellement présents demandent une résolution de bitmap absente du C# et restent refusés. La reconnaissance contrôle les structures ; les entrées BAT sont validées lors de la lecture du document, conformément à cette séparation dans le C#.

L'écriture Fixed/Dynamic conserve le choix de variante du document ou le déduit des plages nulles/non allouées. Elle génère les deux en-têtes, deux tables de régions, métadonnées, journal vide et BAT. Les blocs présents restent positionnés d'après leur index logique ; ils ne sont pas compactés. Les blocs nuls d'une sortie Dynamic sont marqués sans écrire de charge utile. Le dernier bloc physique est complété par des zéros. L'extension à la longueur minimale se fait par une écriture nulle terminale uniquement si nécessaire, sans dépendre d'un redimensionnement spécifique à une plateforme.

Les références, buffers et fichiers temporaires suivent RAII et la transaction commune sur succès, exception ou annulation. La fermeture ou suppression de médias utilisateur n'est pas impliquée.
