# Écriture Apple brute

Bloc 67 : AppleRawImageWriter et MacintoshRawImageWriter. `Formats/Apple/RawWriter.cpp` construit les charges utiles puis délègue la publication au writer atomique existant ; aucune gestion de fichiers n’est ajoutée aux codecs.

Apple II reprend les combinaisons de cibles/extensions du C# : Apple DOS 113 Kio en D13 ; Apple DOS 140 Kio en DO/DSK ; ProDOS 140 Kio et SOS en PO ou DO/DSK avec réordonnancement ; ProDOS 800 Kio en PO. TwoImgPayload expose les mêmes charges utiles dans l’ordre requis par 2IMG, sans prétendre écrire son en-tête.

La validation Apple II porte sur BlockSize, BlockCount et Capacity, puis la présence et la taille de chaque bloc. Elle n’ajoute pas de restrictions sur Cylinder/Heads/SectorsPerTrack ou sur les adresses que le C# ne vérifie pas. Macintosh accepte les capacités 400/800 Kio GCR et 1,44 Mio MFM, vérifie la complétude et chaque adresse zonée ou linéaire, avant la taille de son contenu.

Le contrôle HasSupportedGeometry reste volontairement celui du C# : capacité, taille et compte des blocs ; il ne certifie pas les adresses ni la présence de tous les blocs. La construction de la charge utile effectue ces contrôles supplémentaires. L’ordre des refus (cible, géométrie, présence, adresse, taille selon le writer) est conservé.

Les diagnostics génériques existants servent aux formats/extensions non pris en charge, géométries, tailles et blocs absents. Leurs paramètres décrivent ici taille de bloc / compte / capacité. AppleWriteAddress est la seule nouvelle clé, traduite dans les 29 langues. Les buffers sont possédés par RAII ; l’écriture et l’annulation passent par writeAtomicImage. Les APIs C++ sont synchrones, conformément aux autres writers du port.
