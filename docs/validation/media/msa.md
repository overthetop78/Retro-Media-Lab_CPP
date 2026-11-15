# Vérification MSA et géométrie Atari ST

Le banc compile **30 sources C# originales**, dont les sept fichiers MSA, AtariStGeometry, les modèles sectoriels et le service d’écriture atomique. Le reader utilise son injection de lecture réelle et le writer son service de sortie réel avec capture mémoire. Les façades documentaires non exécutées sont seules remplacées dans le banc. Les empreintes sont vérifiées contre l’inventaire ; manifeste : `6f85a236f5a228ef81b9d22b9519f43252decc0cef74df5bd785704f9898da5d`.

**276 comparaisons C#/C++, zéro différence**, et **cinq contrôles natifs réussis** :

- RLE : marqueur isolé, séquences utiles de cinq octets, seuil de quatre octets, découpage à 65535, entrées vides, séquences tronquées, comptes nuls, consommation et production exactes.
- Reader : pistes brutes/compressées, une ou deux têtes, 1/9/18/36 secteurs, cylindres partiels 1 à 2 et dernier cylindre 255, troncatures et mutations de l’en-tête/longueurs/données, octets finaux ignorés et annulation.
- Writer : neuf profils ST, zéros/marqueurs/données littérales et contenu distinct par bloc, secteurs manquants ou courts au début/à la fin, géométrie incompatible, identifiants en casse différente, annulation.
- Géométrie ST : dimensions non positives, débordement du produit signé et capacités au voisinage de la limite.

Les sorties binaires intégrales sont comparées par SHA-256. Les images lues comparent format, géométrie, capacité, blocs logiques, adresses physiques, intégrité et contenu de tous les secteurs. Les erreurs sont normalisées en acceptation/refus, sans certifier une identité des classes d’exception .NET et C++.

Les cinq contrôles natifs supplémentaires portent sur l’écriture par chemin, l’annulation, la conservation de destination après annulation et après rejet d’un secteur absent. Le répertoire et les handles temporaires sont libérés par RAII. Aucun test n’est ajouté au workflow GitHub.

La compilation des bibliothèques Debug et du banc natif réussit sans avertissement. Les 29 catalogues MSA et les 29 catalogues de géométrie ST sont relus/corrigés, avec vérification des clés, paramètres et invariants techniques. Aucun défaut de production trouvé dans ce bloc ; les limites et comportements examinés correspondent au C#.

Le banc et ses artefacts sont supprimés après vérification de l’inventaire et autorisation de l’agent principal.
