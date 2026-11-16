# Portage CloneCD

Référence en lecture seule consultée le 16 novembre 2026 : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Optical/CloneCd/` (CloneCdFormat.cs, CloneCdDescriptorReader.cs, CloneCdReader.cs), `Constants/CloneCdConstants.cs`, `Contracts/Optical/CloneCdDescriptor.cs` et CloneCdTocEntry.cs. Les SHA-256 courants sont vérifiés lors de la mise à jour de l’inventaire. Aucune recherche Internet nécessaire pour ce portage du comportement présent.

## Organisation

`Formats/Optical/CloneCd/Descriptor.*` sépare la syntaxe INI et les contrats possédés du raccordement aux sources de secteurs dans `Reader.*`. Les déclarations de pistes réutilisent le contrat possédé BinCue::TrackDeclaration déjà partagé avec les descripteurs CUE ; aucun second modèle d’index ou de métadonnées n’est créé. Le descripteur expose des vues const sur ses collections ; il possède aussi les propriétés numériques des sessions et toutes les valeurs TOC comprises par le C#.

Le parseur accepte CloneCD 2 et 3, ignore lignes vides/commentaires `;` et `#`, compare sections et propriétés sans tenir compte de la casse et vérifie leurs doublons. Il exige CloneCD/Disc, des sessions et une TOC non vides et les comptes déclarés. Les propriétés numériques des sessions sont conservées ; les autres sections inconnues sont ignorées comme dans le C#. Les pistes et entrées sont triées numériquement. Les index et flags/ISRC réutilisent la validation du contrat CUE. Les alias de numéros de piste produisant un doublon sont refusés explicitement.

Les entiers suivent le C# : décimal sans signe plus, signe moins externe, préfixe 0x insensible à la casse, hexadécimal signé en complément à deux sur 32 bits, contrôle de la négation de INT_MIN. Par exemple `0xffffffff` vaut -1, `-0x96` vaut -150 ; `-0x80000000` déborde et est refusé. Les numéros indexés sont décimaux non négatifs.

## Sources optiques

Le CCD est le descripteur primaire, le IMG du même nom est obligatoire et non vide, aligné sur 2352 octets. Le SUB est optionnel et doit contenir exactement 96 octets par secteur IMG. Les sources de fichiers existantes sont réutilisées : aucun handle de fichier persistant. Les ouvertures du descripteur utilisent QFile local et QTextStream avec détection BOM ; fermeture sur réussite, exception et annulation.

Les INDEX 1 donnent les adresses IMG absolues et la fin de chaque piste est le prochain début ou la fin du IMG. Tous les intervalles doivent être non vides et bornés. Les INDEX 0 antérieurs donnent les pregaps stockés ; les index des pistes deviennent relatifs. La première entrée TOC dont Point correspond au numéro de piste fournit sa session, avec repli sur 1 si elle manque ou n’est pas positive. Ce n’est pas une détection basée sur un système de fichiers hôte.

Les modes 0/1/2 correspondent à AUDIO, MODE1/2352 et MODE2/2352. La fenêtre utilisateur est respectivement 0/2352, 16/2048 et 16/2336. Le SUB est exposé brut, 96 octets par secteur et avec le décalage absolu de la piste ; aucun décodage de sous-canaux supplémentaire n’est introduit. Catalogue et ISRC restent des invariants.

Le document conserve les chemins associés et publie version, sessionCount, trackCount et présence des sous-canaux. La reconnaissance suit le C# : descripteur valide et IMG existant ; elle ne prétend pas avoir vérifié l’intégralité des données. La lecture fait les contrôles de tailles et de pistes. Aucun writer CloneCD n’existe dans le C# de référence : aucun n’est ajouté ici. La composition globale des lecteurs demeure une tâche distincte.
