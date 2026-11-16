# PrÃ©paration runtime : conversions SCP

## ResponsabilitÃ©s

`MediaPreparation.h` expose `convertScpAsync`. L'orchestration reste dans MediaEngine, sÃ©parÃ©e des dÃ©codeurs, des reconstructeurs et des Ã©crivains ; App ne reÃ§oit aucune logique matÃ©rielle ou de fichiers.

- ATR/ST : candidats FM/MFM collectÃ©s par le cache SCP existant ; sÃ©lection automatique dans le mÃªme ordre que le C# : Epson, ADFS, BBC DFS, IBM DOS, Atari 8 bits, puis ST.
- La sonde Epson retient les tailles dans le groupe de meilleure intÃ©gritÃ©, puis la frÃ©quence et la taille dÃ©croissante en cas d'Ã©galitÃ©.
- ADFS utilise les gÃ©omÃ©tries existantes, les deux sommes de contrÃ´le de l'ancienne carte, sa capacitÃ© dÃ©clarÃ©e, puis le lecteur ADFS de MediaFileSystems.
- Les constructeurs rÃ©utilisent mesures, BPB, gÃ©omÃ©tries et images sectorielles. Les capacitÃ©s Atari/ST utilisent une multiplication vÃ©rifiÃ©e sur 64 bits ; le chemin IBM conserve la limite int32 de son catalogue C#.
- ADF : reconstruction Amiga pour les chemins SCP, sinon lecteur ADF existant ; gÃ©omÃ©trie DD/HD conservÃ©e. Deux pistes avec au moins 17 numÃ©ros distincts et un numÃ©ro supÃ©rieur Ã  10 sont nÃ©cessaires pour retenir HD.
- Validation de famille avant Ã©criture. Les trois Ã©crivains existants conservent leur Ã©criture atomique et leur gestion d'annulation. Les fichiers source ne sont jamais ouverts en Ã©criture ; une sortie dÃ©signant le mÃªme fichier, y compris un lien physique, est refusÃ©e avant lecture/dÃ©codage.
- Les registres et caches sont possÃ©dÃ©s par le service ; les rÃ©sultats sont dÃ©tenus par les opÃ©rations asynchrones. Aucun thread matÃ©riel, lecteur hÃ´te ou sortie audio n'est crÃ©Ã©.

## VÃ©rification du bloc

Compilation Debug MediaEngine et catalogues Constants + 29 langues. Audit des littÃ©raux : 1 442 fichiers C++, 29 langues.

Un seul exÃ©cutable temporaire couvre les branches automatiques Atari 90/130/180, BBC, IBM historique, ST, Epson et ADFS ; la sÃ©lection d'intÃ©gritÃ© Amiga, le filtrage sur l'adresse physique et les positions logiques absentes ; l'infÃ©rence DD/HD et l'annulation.

Captures SCP synthÃ©tiques complÃ¨tes produites avec l'encodeur existant pour Atari 90, ST 720 et Amiga DD : conversion publique ATR/ST/ADF, comparaison de chaque secteur avec la source, capture source inchangÃ©e. Refus ATR depuis ST, extension inconnue et annulation sans sortie. Une vÃ©rification complÃ©mentaire refuse la source comme destination.

Les mÃ©dias et l'exÃ©cutable temporaires sont supprimÃ©s aprÃ¨s utilisation. Aucun test ajoutÃ© au workflow.

## PÃ©rimÃ¨tre restant

Ce bloc ne constitue pas le registre complet des politiques ISO : sÃ©lection explicite IBM/ibm.scan et raccordements des autres politiques restent dans l'inventaire. Le lecteur Amiga n'expose pas encore les notifications de progression du C#. Le service gÃ©nÃ©ral de conversion et les transformations explicites ne sont pas dÃ©clarÃ©s terminÃ©s.

## RÃ©fÃ©rences locales C#

`Images/Reading/Reconstruction/Iso/AutomaticIsoScpSectorImagePolicy.cs`, `AutomaticIsoScpSelectionRules.cs`, `IbmPcIsoScpSectorImagePolicy.cs`, `Images/Reading/Reconstruction/Atari/Atari8BitIsoScpSectorImagePolicy.cs`, `AtariStIsoScpSectorImagePolicy.cs`, `Images/Reading/Reconstruction/AmigaScpSectorImageReader.cs`, `Images/Conversion/Atari/AtariScpRuntimeConversionService.cs`, `Images/Conversion/AmigaAdfConversionService.cs`. Sources consultÃ©es en lecture seule ; empreintes mises Ã  jour dans l'inventaire.

Mise à jour bloc 205 : les politiques explicites ISO, leur registre/lecteur commun et la progression Amiga décrits comme restants ci-dessus sont maintenant portés ; voir `validation-politiques-iso.md`.
