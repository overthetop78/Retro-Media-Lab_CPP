# Lecteur CP2

Référence C# en lecture seule : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/Cp2/`.

- `Cp2Reader.cs` : SHA256 `3d112378593a58f9cd8535ec441bb87638e5fd4e9dccb26dbcc578c3ecd85c36`.
- `Cp2Layout.cs` : SHA256 `7ee5bb0efc8f61fa9b87e6c729f091a3528b98828700477fb2eb5f67e8e0589a`.
- `Cp2Format.cs` : SHA256 `a200ed09bafbb81b011074e5418cb7f109b9215fbbeae89cca4591cbb71c3d33`.
- `Cp2Exceptions.cs` : SHA256 `62926ee745d0c83d29e10d70b3666a1d413a83760bfd75f0123c99f02c250331`.

## Découpage C++

`Constants.h` décrit les champs et dimensions binaires. `Layout.cpp` valide et décode les descripteurs de piste. `Reader.cpp` parcourt les groupes et reconstruit l'image IBM. `Adapter.cpp` expose les capacités et la reconnaissance dans le registre commun. Les accès aux fichiers et les images sectorielles réutilisent les abstractions existantes ; App n'intervient pas.

## Comportement conservé

La signature est `SOFTWARE PIRATES` et le premier groupe commence à l'octet 28. La longueur de métadonnées, diminuée d'un octet, doit être divisible par 387. Chaque descripteur peut contenir au plus 23 descriptions sectorielles de 16 octets. Les adresses cylindre/tête différentes de celles de la piste et les codes de taille supérieurs à sept sont ignorés : ces descriptions ne consomment aucune charge utile, comme les enregistrements synthétiques finaux du C#.

Les charges utiles sont lues dans l'ordre angulaire stable de chaque piste. Toutes les tailles décrites sont consommées, mais seuls les secteurs de 512 octets sont reconstruits ; le premier exemplaire d'une même adresse gagne. La position du groupe suivant reprend les deux derniers octets d'encadrement selon le calcul C#.

Les maxima observés déterminent cylindres, têtes et secteurs par piste ; plus de deux têtes est invalide. Le tampon linéaire est initialement nul, donc les adresses absentes deviennent des secteurs lisibles de zéros, comme dans le C#. L'identifiant IBM provient de la capacité connue ou de sa valeur en Kio ; les dimensions restent celles observées. La représentation source sectorielle existante évite de multiplier les copies de blocs lors de sa construction et possède son tampon immuable.

Le lecteur annonce les onze profils IBM du catalogue, accepte explicitement tout identifiant du préfixe `ibm.` et vérifie la signature indépendamment de l'extension. Il n'ajoute pas de writer CP2 inexistant dans le C#. Les sept erreurs traduites sont classées comme rejets de structure par le registre ; l'annulation reste distincte.
