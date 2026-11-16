# Préparation runtime : conversions SCP

## Responsabilités

`MediaPreparation.h` expose `convertScpAsync`. L'orchestration reste dans MediaEngine, séparée des décodeurs, des reconstructeurs et des écrivains ; App ne reçoit aucune logique matérielle ou de fichiers.

- ATR/ST : candidats FM/MFM collectés par le cache SCP existant ; sélection automatique dans le même ordre que le C# : Epson, ADFS, BBC DFS, IBM DOS, Atari 8 bits, puis ST.
- La sonde Epson retient les tailles dans le groupe de meilleure intégrité, puis la fréquence et la taille décroissante en cas d'égalité.
- ADFS utilise les géométries existantes, les deux sommes de contrôle de l'ancienne carte, sa capacité déclarée, puis le lecteur ADFS de MediaFileSystems.
- Les constructeurs réutilisent mesures, BPB, géométries et images sectorielles. Les capacités Atari/ST utilisent une multiplication vérifiée sur 64 bits ; le chemin IBM conserve la limite int32 de son catalogue C#.
- ADF : reconstruction Amiga pour les chemins SCP, sinon lecteur ADF existant ; géométrie DD/HD conservée. Deux pistes avec au moins 17 numéros distincts et un numéro supérieur à 10 sont nécessaires pour retenir HD.
- Validation de famille avant écriture. Les trois écrivains existants conservent leur écriture atomique et leur gestion d'annulation. Les fichiers source ne sont jamais ouverts en écriture ; une sortie désignant le même fichier, y compris un lien physique, est refusée avant lecture/décodage.
- Les registres et caches sont possédés par le service ; les résultats sont détenus par les opérations asynchrones. Aucun thread matériel, lecteur hôte ou sortie audio n'est créé.

## Vérification du bloc

Compilation Debug MediaEngine et catalogues Constants + 29 langues. Audit des littéraux : 1 442 fichiers C++, 29 langues.

Un seul exécutable temporaire couvre les branches automatiques Atari 90/130/180, BBC, IBM historique, ST, Epson et ADFS ; la sélection d'intégrité Amiga, le filtrage sur l'adresse physique et les positions logiques absentes ; l'inférence DD/HD et l'annulation.

Captures SCP synthétiques complètes produites avec l'encodeur existant pour Atari 90, ST 720 et Amiga DD : conversion publique ATR/ST/ADF, comparaison de chaque secteur avec la source, capture source inchangée. Refus ATR depuis ST, extension inconnue et annulation sans sortie. Une vérification complémentaire refuse la source comme destination.

Les médias et l'exécutable temporaires sont supprimés après utilisation. Aucun test ajouté au workflow.

## Périmètre restant

Ce bloc ne constitue pas le registre complet des politiques ISO : sélection explicite IBM/ibm.scan et raccordements des autres politiques restent dans l'inventaire. Le lecteur Amiga n'expose pas encore les notifications de progression du C#. Le service général de conversion et les transformations explicites ne sont pas déclarés terminés.

## Références locales C#

`Images/Reading/Reconstruction/Iso/AutomaticIsoScpSectorImagePolicy.cs`, `AutomaticIsoScpSelectionRules.cs`, `IbmPcIsoScpSectorImagePolicy.cs`, `Images/Reading/Reconstruction/Atari/Atari8BitIsoScpSectorImagePolicy.cs`, `AtariStIsoScpSectorImagePolicy.cs`, `Images/Reading/Reconstruction/AmigaScpSectorImageReader.cs`, `Images/Conversion/Atari/AtariScpRuntimeConversionService.cs`, `Images/Conversion/AmigaAdfConversionService.cs`. Sources consultées en lecture seule ; empreintes mises à jour dans l'inventaire.
