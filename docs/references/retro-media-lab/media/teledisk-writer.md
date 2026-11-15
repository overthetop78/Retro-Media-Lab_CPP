# Écriture TeleDisk et cible UCSD

Références C# en lecture seule : `Images/Formats/Floppy/TeleDisk/Td0Writer.cs`, `Td0HeaderFactory.cs`, entrée UCSD de `Images/Writing/MediaWritingComposition.cs` et sélection de cible dans `Images/Conversion/UcsdImgConversionService.cs`.

Le writer natif accepte une image détaillée ou une image sectorielle. L’écriture détaillée conserve en-tête, commentaire et date, pistes vides, adresses enregistrées, codes de taille, données indisponibles et drapeaux. Les CRC sont recalculés et les données sont encodées en brut ou mot répété. Il produit TD, sans compresseur LZSS/Huffman global, comme le C#.

Pour une image sectorielle générique, les blocs sont groupés par coordonnées physiques et triés par numéro. Toutes les pistes de la géométrie sont créées, même vides. Les adresses enregistrées, lorsqu’elles existent, remplacent l’adresse physique dans l’en-tête sectoriel. Les drapeaux sont reconstruits depuis les états d’intégrité et de données supprimées ; l’absence d’un bloc ne fabrique pas de données.

La fabrique d’en-tête choisit version 21 et préserve le débit enregistré d’une image UCSD seulement si tous les blocs disponibles ont le même débit et les métadonnées nécessaires. BBC DFS et Atom utilisent le drapeau FM 0x80 ; ADFS 1600 utilise le débit 2. Le type de lecteur suit les seuils de cylindres, faces et capacité du C#. Les conversions d’adresses en octets sont contrôlées, y compris les valeurs négatives.

Correction explicite du comportement source : le C# accepte 255 secteurs dans une piste, puis écrit 0xFF comme compte ; le reader interprète cet octet comme la fin des pistes. Le writer C++ refuse donc tout compte supérieur ou égal à 255 avec l’erreur de carte de pistes non représentable. Le contrôle ciblé vérifie ce refus ; aucune modification du dépôt C#.

`Raw::ucsdWriterAdapter()` expose l’identifiant ucsd et les cibles UCSD IMG/TD0. TD0 utilise le prédicat de charges utiles représentables indépendamment du nom du profil source. IMG exige la géométrie UCSD 40/1/8 × 512, sans exiger le même identifiant source ; l’écriture linéaire suit cette géométrie. Les deux chemins passent par l’écriture atomique commune. Aucun changement du profil n’est imposé au chemin TD0 de l’adapter, conformément à WriteUcsdAsync dans la composition source.

Ce bloc ne porte pas encore le service de conversion complet et sa sélection de source SCP/TD0/IMG, ni la composition globale de tous les writers.
