# Politiques ISO et lecteurs SCP

## Organisation

Le registre `Reconstruction/Iso/Policy` rÃ©sout les mÃªmes familles dans le mÃªme ordre que le C#. Les politiques restent internes Ã  MediaEngine. Elles utilisent un type de sÃ©lection et des fonctions de construction ; aucune DLL supplÃ©mentaire n'est nÃ©cessaire pour sÃ©parer ces rÃ¨gles d'une mÃªme responsabilitÃ©.

`ExplicitFamilies` contient ADFS, gÃ©nÃ©rique/Amstrad et UCSD. `RuntimeFamilies` rÃ©utilise les constructeurs Atari, ST, BBC, IBM et Epson. La sonde IBM DOS est partagÃ©e avec la sÃ©lection automatique : aucun second catalogue de gÃ©omÃ©tries ou de BPB.

`Reader` possÃ¨de le collecteur/cache existant et accepte un chemin ou une reprÃ©sentation de flux protÃ©gÃ©. Le code de prÃ©paration runtime appelle ce service commun pour Atari. Le lecteur est synchrone sur un worker ; la faÃ§ade publique continue de fournir son opÃ©ration asynchrone. Les callbacks de progression ISO peuvent Ãªtre concurrents, comme ceux du collecteur existant ; le destinataire doit se synchroniser.

## RÃ¨gles conservÃ©es

- Atom utilise la politique gÃ©nÃ©rique avant la rÃ¨gle gÃ©nÃ©rale BBC DFS.
- La sÃ©lection Atari 90 du dÃ©codeur FM garde la comparaison exacte du C# ; un identifiant de casse diffÃ©rente reste traitÃ© en MFM. La rÃ©solution de la famille demeure insensible Ã  la casse.
- ADFS 256 octets utilise la face physique, mÃªme lorsque l'en-tÃªte enregistre H=0. Les profils 1 024 octets utilisent les adresses dÃ©codÃ©es cohÃ©rentes. Tous les candidats sont contrÃ´lÃ©s contre les dimensions et tailles explicitement demandÃ©es.
- Amstrad autorise les identifiants supÃ©rieurs au nombre de secteurs et les classe selon l'ordre mesurÃ©.
- UCSD classe les identifiants sÃ©parÃ©ment pour chaque piste physique, avec sa gÃ©omÃ©trie logique Ã  une face et huit secteurs.
- IBM explicite peut choisir sa gÃ©omÃ©trie sans OEM DOS. `ibm.scan` refuse un BPB sans identitÃ© DOS/IBM. Un format explicite connu prime sur le BPB. Mac1440 utilise la mÃªme rÃ¨gle IBM que le C#.
- BBC explicite conserve la gÃ©omÃ©trie demandÃ©e ; l'automatique utilise la gÃ©omÃ©trie observÃ©e.
- Une reconstruction sans candidat adressÃ© ni physique donne les deux comptes dans un diagnostic traduit.

## Amiga

Progression typÃ©e par rÃ©volution et par piste, totaux vÃ©rifiÃ©s, propagation de l'annulation et des exceptions du destinataire. Les deux Ã©checs de reconstruction ont dÃ©sormais leurs diagnostics propres : aucun secteur dÃ©codÃ©, ou aucun secteur utilisable avec les nombres dÃ©codÃ©s/conservÃ©s. Aucun message Amiga ne rÃ©utilise le diagnostic ISO gÃ©nÃ©rique.

## VÃ©rification

Un seul banc temporaire cible les nouvelles rÃ¨gles, sans rÃ©pÃ©ter les tests des codecs : rÃ©solution de toutes les familles, dÃ©codeurs choisis, gÃ©omÃ©tries explicites ST/BBC/IBM, numÃ©rotation Amstrad, faces physiques ADFS, refus des candidats hors gÃ©omÃ©trie, classement UCSD par piste, identitÃ© IBM scan, lecture par chemin et flux protÃ©gÃ©, rÃ©utilisation du cache, progression, erreurs du destinataire et annulation. Les deux diagnostics Amiga sont exercÃ©s sÃ©parÃ©ment, ainsi que la nouvelle tentative aprÃ¨s Ã©chec d'un rÃ©cepteur de progression.

Compilation Debug MediaEngine et catalogues Constants + 29 langues ; audit des littÃ©raux et des paramÃ¨tres de traduction. Argos puis relecture des 29 catalogues : corrections des nÃ©gations, faux termes techniques, fragments anglais et chinois traditionnel. Le banc, ses donnÃ©es et ses journaux temporaires sont supprimÃ©s aprÃ¨s utilisation.

RÃ©fÃ©rences : fichiers C# `Images/Reading/Reconstruction/Iso`, `Atari/AtariScpSectorImageReader.cs`, `EpsonQx10/EpsonQx10IsoScpSectorImagePolicy.cs`, `AmigaScpSectorImageReader.cs`, lus sans modification. L'inventaire conserve leurs empreintes actualisÃ©es.

Le registre des lecteurs gÃ©nÃ©raux et les autres familles de reconstruction restent suivis sÃ©parÃ©ment ; ce bloc ne termine pas le portage global des trois bibliothÃ¨ques.

Résultat final : MediaEngine et les 30 bibliothèques de catalogues compilés ; contrôles ciblés réussis, dont les deux erreurs Amiga et la reprise du cache après échec du destinataire ; audit des 1 447 fichiers C++ réussi et paramètres préservés dans les deux jeux de 30 catalogues. Banc temporaire supprimé.
