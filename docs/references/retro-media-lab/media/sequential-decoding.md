# Décodage séquentiel : registre et adaptateur MFS

Référence C# consultée en lecture seule le 16 novembre 2026 : Contracts/SequentialDecodedBlock.cs, SequentialDecodeResult.cs, Interfaces/Decoding/ISequentialMediaDecoder.cs, Images/Reading/Decoding/Sequential/SequentialDecoderRegistry.cs et Images/Reading/SequentialContentDecoderAdapter.cs dans F:/GW GUI/src/RetroMediaLab.MediaEngine. SHA-256 des cinq sources vérifiés contre l’inventaire.

Les modèles moteur possèdent les données, métadonnées, positions sources et segments non décodés. Une position négative, un identifiant blanc ou une confiance non finie/hors 0–1 est refusé. Les vues exposées sont constantes. Les diagnostics textuels externes du C# restent transportables ; les nouveaux messages utilisent des recettes à clés traduisibles.

Le registre possède ses décodeurs, refuse un élément nul et conserve leur ordre. FindCompatible délègue la compatibilité au décodeur avec la sélection de machine ; aucun algorithme machine n’est ajouté au registre. GetRequired exige exactement un candidat : zéro et ambiguïté sont des rejets distincts. decodeAsync conserve document et décodeur via shared_ptr pendant la future différée ; le contrat direct du décodeur exige que son appelant conserve le document jusqu’à achèvement.

L’adaptateur MFS ne revendique que les documents moteur Tape à représentation séquentielle, avec origine ou identifiant sequential-content insensible à la casse. Il exécute tous les candidats dans l’ordre C#, sans avaler leurs exceptions. Le classement stable privilégie présence de blocs, confiance puis nombre de blocs. Seul le résultat choisi est transféré au contrat MFS ; ses diagnostics et tous ses segments non décodés sont conservés. Si plusieurs candidats existent, la sélection est expliquée par un message traduit.

Les données machine sont converties en blocs MFS déjà décodés. La construction des fichiers nommés reste dans SequentialContentReader ; le nom de volume vient uniquement de internalName trimé. Sans candidat, le volume vide et son avertissement de fichiers absents sont conservés. L’adaptateur se branche à MediaExplorer::createDefault via son point d’adaptation existant, puis les entrées suivent l’analyse MFS déjà reliée à MediaAnalysis. Aucune dépendance MFS vers les classes de décodeurs moteur n’est créée.

Les noms de types de segments dans les diagnostics sont des identifiants techniques invariants ; les phrases sont traduites. Les messages de rejet de sélection conservent le format demandé ; la machine continue d’être transmise au décodeur sans être transformée en texte de substitution.

Les décodeurs de protocoles réels et la composition automatique des services restent à porter. Le contrôle utilise des implémentations fournies au registre ; il ne constitue pas une validation d’une bande machine réelle ni d’un parcours depuis App.
