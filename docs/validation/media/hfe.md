# Vérification HFE

Le banc compile les douze sources HFE C# originales, les modèles de pistes/flux originaux et les trois helpers de conversion d’origine. Les façades de reconnaissance documentaire non appelées sont remplacées uniquement dans le banc ; aucun algorithme de lecture, écriture, conversion ou modèle de flux exercé n’est réécrit dans la référence.

La première comparaison donne **292 résultats identiques sur 294 cas** et **trois contrôles natifs atomiques réussis**. Les cas couvrent packing LSB, interlacement, bornes des pistes/table/données, troncature, révision/géométrie, métadonnées numériques, encodages, cellules et intervalles, opcodes v3 (y compris paramètres tronqués, skip, débit nul, random et index), timing variable, repli v1/v3 et limites des adaptateurs protégés. Les modèles comparent géométrie, métadonnées, bits, timing, caractéristiques et intervalles de révolution ; les conteneurs comparent leurs octets complets via SHA-256.

Deux écarts réels sont détectés pour une révolution neutre vide : le helper C++ omettait les contraintes de MediaFluxRevolutionData sur l’index et les intervalles positifs, et CanRepresentExactly absorbait ces erreurs d’argument. Ces contrôles sont ajoutés au helper et la propagation est alignée sur le C#. Après reconstruction, **huit cas ciblés réussissent sans différence**, couvrant les deux écarts et six nouveaux cas d’intervalles nuls ou sous-résolution. Les 292 parcours réussis ne sont pas rejoués : le cumul porte sur **300 cas distincts validés**.

Les contrôles natifs vérifient l’écriture par chemin, l’annulation et la conservation du fichier existant. Le répertoire et les handles sont libérés par RAII. Les erreurs .NET/C++ sont comparées en acceptation/refus, sans certifier une identité des types d’exceptions.

Les 46 sources C# compilées ont leurs empreintes vérifiées contre l’inventaire ; manifeste : `0e9ef887b9b30737c4066f641317339b9eb8ee0a797e30472fa5c2f70f8fc67c`. Les bibliothèques Debug et le banc natif final compilent sans avertissement C++. Aucun test ajouté au workflow GitHub.

Les 29 catalogues ont été relus/corrigés, avec contrôle des paramètres et invariants. Le banc et ses artefacts ont été supprimés après vérification de l’inventaire et autorisation de l’agent principal.
