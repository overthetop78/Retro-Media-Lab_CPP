# PrÃ©paration publique des disques

RÃ©fÃ©rences C# en lecture seule : CreateHardDiskAsync, CreateBlankDiskAsync et propriÃ©tÃ©s de catalogue/tailles de MediaRuntimePreparationService, contrat IMediaRuntimePreparation, constantes DiskImageFileExtensions.

Le nouvel en-tÃªte public MediaPreparation.h fournit les fonctions de crÃ©ation sans rÃ©vÃ©ler les types internes de MediaFileSystems. La crÃ©ation de disquette passe par BlankSectorImageFactory puis le service dâ€™Ã©criture existant. La crÃ©ation de HDD sÃ©lectionne exclusivement Acorn DAT/DSC ou MFM EMU, comme le C#. Les deux Ã©crivains utilisent leur chemin de crÃ©ation refusant lâ€™Ã©crasement. Lâ€™annulation et lâ€™option de prÃ©allocation sont transmises aux services dÃ©jÃ  portÃ©s.

Pour Acorn, la prÃ©paration ADFS exige des secteurs de 256 octets et le systÃ¨me de fichiers acorn-adfs. Le formateur existant crÃ©e uniquement le prÃ©fixe requis ; les plages restantes sont Ã  zÃ©ro sans construire un tampon Ã©gal Ã  la taille du disque. Une gÃ©omÃ©trie hors du contrat DSC, un systÃ¨me de fichiers ou format non pris en charge sont refusÃ©s. Pour EMU, la fonction de crÃ©ation existante conserve son comportement physique : sans systÃ¨me de fichiers le conteneur reste vierge, avec ADFS il est formatÃ© via Adaptec. Aucun nouveau code de format ou algorithme ADFS nâ€™est crÃ©Ã©.

La composition existante reconnaissance/Ã©criture est partagÃ©e dans Composition/Runtime.h et Runtime.cpp. La prÃ©paration ne dÃ©pend plus des Ã©tats internes RuntimeDisk. Les fonctions asynchrones capturent les paramÃ¨tres nÃ©cessaires par valeur et nâ€™empruntent aucun service local dont la durÃ©e de vie serait plus courte que la tÃ¢che.

Les 118 paires nom technique/extension sont extraites intÃ©gralement des constantes C#, avec assertion que chaque dÃ©claration est reconnue. Elles restent des donnÃ©es nommÃ©es, enregistrÃ©es dans lâ€™audit des constantes. Les tailles optiques publiques rÃ©utilisent les constantes dÃ©jÃ  prÃ©sentes : 2352 octets de secteur brut/audio et 96 octets de sous-canaux.

## VÃ©rification ciblÃ©e

La vÃ©rification utilise les en-tÃªtes publics pour crÃ©er une IBM 720 puis la lire par RuntimeDisk : 737280 octets, 1440 secteurs de 512 octets. Une DAT/DSC vide donne la capacitÃ© et les 33 octets de descripteur attendus ; le dernier secteur est Ã  zÃ©ro. Une DAT ADFS et une EMU ADFS sont relues secteur par secteur pour vÃ©rifier le prÃ©fixe du formateur partagÃ©. Une EMU sans systÃ¨me de fichiers reste sans secteurs dÃ©codÃ©s.

Les crÃ©ations DAT/DSC et EMU refusent des destinations dÃ©jÃ  existantes et prÃ©servent leurs octets. Les formats, gÃ©omÃ©tries et systÃ¨mes de fichiers non pris en charge ne crÃ©ent pas de sortie. Une annulation prÃ©alable ne crÃ©e ni DAT ni DSC. La crÃ©ation avec prÃ©allocation traverse le mÃªme Ã©crivain ; la vÃ©rification ne prÃ©tend pas mesurer lâ€™espace physique rÃ©servÃ© par le systÃ¨me.

Un diagnostic spÃ©cifique Ã  la prÃ©paration remplace les messages qui auraient parlÃ© dâ€™une image Ã  dÃ©coder ou du conteneur EMU dans le chemin DAT/DSC. Il est gÃ©nÃ©rÃ© avec Argos dans les 29 langues, relu et corrigÃ©. Les rÃ©sultats de compilation et lâ€™audit final sont consignÃ©s aprÃ¨s terminaison des commandes en cours.

## Suite

La conversion SCP Atari/Amiga et lâ€™exposition publique des services de signal de bande et de sons mÃ©caniques restent Ã  raccorder. Les fonctions CUE et les trois runtimes restent accessibles par leurs contrats publics propres ; cette organisation Ã©vite un service unique mÃ©langeant modÃ¨les et responsabilitÃ©s.
