# Vérification ATR et ATX

**267 comparaisons C#/C++, zéro différence**, et **cinq contrôles natifs atomiques réussis**. Le banc compile **34 sources C# originales**, dont les douze sources ATR/ATX et les vrais modèles sectoriels. Seules les façades documentaires non exécutées sont remplacées ; le reader ATR utilise son injection de lecture originale et le writer son service de sortie original. Les empreintes sont vérifiées contre l’inventaire : manifeste `9936f0630b99e0021b4d0c8876f86a95303f02c266fef48eaf2bac7bea91a35b`.

Les comparaisons couvrent :

- ATR : dispositions 128/256/512 octets, secteurs d’amorçage, géométries physiques/logiques, profils, tailles/paragraphes, troncatures et récupération du boot, densité améliorée mal étiquetée avec VTOC DOS2/DOS3, longueur incluant l’en-tête, padding zéro/0x1a, padding mixte refusé, queue double densité et annulation.
- Writer ATR : trois profils, secteurs manquants/courts, capacités et nombres de blocs incompatibles, casse des identifiants, taille nominale ou géométrie différente mais contrat logique compatible, annulation.
- ATX : signatures et offsets, pistes/listes/données tronquées ou hors limites, types de piste ignorés, limites 40 pistes/26 secteurs/64 enregistrements, tous les bits de statut principaux, statut missing-data 0x10 avec offset volontairement invalide ignoré, choix statut valide prioritaire, positions et égalité stable, doublons inter-pistes et annulation.

Les images comparent format, géométrie, adressage, capacité, blocs logiques, adresses, intégrité, diagnostic et contenu intégral de chaque secteur. Les octets de conteneurs sont comparés par SHA-256. Les erreurs sont normalisées en acceptation/refus, sans prétendre identifier leurs types .NET/C++.

Les contrôles natifs vérifient l’écriture par chemin et la conservation de destination après annulation ou secteur manquant. Les temporaires et handles sont libérés par RAII. Deux avertissements C4267 du writer ATR ont été corrigés en séparant les valeurs de format uint32 des offsets size_t ; reconstruction ciblée sans avertissement, sans changement algorithmique ni rejeu des 267 parcours validés. Aucun défaut fonctionnel de production trouvé dans ce bloc. Aucun test ajouté au workflow GitHub.

Les 29 catalogues ATR/ATX sont relus et corrigés en parallèle ; paramètres et invariants techniques sont contrôlés. Le banc et ses artefacts sont supprimés après vérification finale de l’inventaire et autorisation de l’agent principal.
