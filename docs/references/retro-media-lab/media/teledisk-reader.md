# Lecture détaillée TeleDisk

Sources C# en lecture seule : `Images/Formats/Floppy/TeleDisk/Td0Reader.cs`, `Td0Section.cs`, `Td0Exceptions.cs` et `Images/Reading/Recognition/Td0SectorImageClassifier.cs`. Dépendances reprises : définitions TeleDisk, décompresseur avancé, codec sectoriel, détecteur BPB FAT, catalogue des géométries IBM et assemblage uniforme des secteurs.

`Reader.cpp` analyse le conteneur. Pour la signature avancée td, il décompresse seulement la partie suivant les douze octets d’en-tête, rétablit TD et recalcule le CRC d’en-tête comme le C#. Il contrôle ensuite CRC global, commentaire, pistes et données sectorielles. Les commentaires conservent leur date et leurs octets, sans conversion de texte. La marque terminale requiert un seul octet et les éventuels octets après celle-ci ne sont pas interprétés, comme dans la source.

Un secteur indisponible reste sans données dans les enregistrements détaillés et son CRC est calculé sur le contenu nul implicite. Avec l’initialisation CRC nulle, ce résultat est nul. Les drapeaux de CRC physique ne dispensent pas du contrôle du CRC de l’enregistrement TeleDisk : ce sont deux informations différentes.

`SectorImage.cpp` choisit la taille dominante par nombre de secteurs, avec priorité au premier groupe en cas d’égalité. Les autres tailles restent dans les enregistrements détaillés et ne deviennent pas des blocs logiques, conformément au C#. L’ordre logique est cylindre physique, face physique masquée et numéro de secteur ; une donnée manquante conserve son emplacement logique. Les adresses enregistrées peuvent différer des coordonnées physiques et sont préservées avec code de taille, ordre dans la piste, débit, drapeaux, marque supprimée et état de CRC.

La classification est IBM seulement pour des blocs de 512 octets avec BPB FAT reconnu ou saut de boot DOS. Une capacité et une géométrie IBM connues donnent le profil précis ; sinon ibm.scan. Sans ces indices, le repli est ucsd.ibm.mfm. L’assemblage uniforme commun est essayé seulement quand toutes les tailles correspondent ; sinon la capacité et le nombre de blocs suivent les secteurs de la taille dominante, y compris les emplacements absents.

L’adapter expose les profils IBM, IBM scan, UCSD, le préfixe IBM et les deux signatures. Sa sonde ne valide que taille minimale, format demandé et signature, sans imposer l’extension TD0 ; l’analyse détaillée s’effectue lors de la lecture. Les erreurs de conteneur et de décompression invalides sont explicitement incluses dans les rejets du registre ; l’annulation, les erreurs d’accès et les erreurs d’écriture ne sont pas absorbées.

Le writer natif de conteneur TeleDisk est désormais porté au bloc 119 ; la composition globale reste en cours. L’adapter commun retourne l’image sectorielle comme le C# ; l’API native readDetailed permet de conserver les enregistrements détaillés pour leur future réécriture.
