# Images SCP issues de secteurs — 15 novembre 2026

Les deux services C# restent distincts dans le port : Visualization/SectorFluxVisualizer utilise l’encodage de visualisation, tandis que Formats/Scp/SectorConverter utilise l’encodage physique complet. Ils possèdent un SectorImageEncoder et acceptent une instance injectée. Aucun de ces composants ne dépend du GUI ou du matériel.

Le visualiseur conserve les intervalles sans conversion supplémentaire, renseigne leur nombre déclaré et reprend les valeurs ScpVisualizationDefaults : version et type zéro, une révolution, résolution zéro, index aligné et écriture autorisée. Les bornes de table viennent des pistes présentes, la sélection de face distingue face 0, face 1 et les deux, et FileSize conserve la capacité de l’image sectorielle.

Le constructeur ScpRevolution utilisé dans le C# conserve ici son origine par défaut Captured, même si l’image est produite pour visualisation. Cette métadonnée est reprise explicitement ; elle n’est pas remplacée implicitement par Synthetic. La conversion physique utilise au contraire ScpEncodedTrackFluxService, déjà porté dans syntheticRevolution, et conserve bien l’origine Synthetic et le compteur déclaré zéro du C#.

La conversion produit la version 0x19, les drapeaux d’index aligné/normalisation/logiciel tiers, le drapeau 96 TPI lorsque le cylindre maximal présent dépasse 41 et le drapeau 360 RPM lorsqu’une piste a la durée nominale correspondante. La capacité source ne remplace pas le FileSize initial zéro de cette sortie. Les compteurs et numéros de piste sont vérifiés avant réduction vers les champs SCP.

DiskTypes.cpp reprend le catalogue de classification par format, géométrie et capacité : familles Commodore/Amiga/Atari/Apple/IBM/Amstrad, puis repli sur la capacité tronquée au kibioctet. Les identifiants déjà partagés sont réutilisés ; les valeurs de type sont regroupées dans DiskTypeConstants.h.

SectorConverter expose la création de modèle, l’écriture par callback et l’écriture atomique par chemin. Ces méthodes C++ sont synchrones comme le writer existant ; elles conservent le jeton d’annulation et les erreurs, tandis que l’ordonnancement asynchrone du consommateur n’est pas ajouté à ce composant. L’écriture reste déléguée au writer SCP validé, sans seconde implémentation de sérialisation.

Les erreurs réutilisent les diagnostics traduits existants d’encodage d’image et de SCP. Aucun catalogue supplémentaire n’est nécessaire dans ce bloc.
