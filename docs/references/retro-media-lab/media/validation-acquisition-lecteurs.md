# Profils et acquisition des lecteurs

## Défaut corrigé dans le port C++

DriveAccessScope et DriveOperationScope appelaient EndAccess/EndOperation même si leur constructeur échouait pendant Begin. Avec un accès préexistant et une position invalide, le constructeur annulait ainsi l’accès de l’autre propriétaire. Le C# appelle BeginRead/BeginWrite avant son try/finally dans MediaDriveRandomAccessData : l’accès refusé n’a pas à terminer un autre accès.

Le contrat C++ précise maintenant que Begin qui échoue libère uniquement son acquisition éventuelle. Les scopes terminent seulement les acquisitions dont le constructeur a réussi. RecordedDrive restaure son état mécanique si une erreur intervient avant l’acquisition ; si une notification lève après acquisition, il retire cet accès et préserve les autres utilisateurs. Sans autre utilisateur ou opération, il arrête le moteur. Les callbacks restent hors du verrou. Reset reste une commande du propriétaire de la mécanique ; ce contrôle ne lui attribue pas de propriété exclusive automatique sur des scopes externes concurrents.

## Profils et organisation

MediaDriveService expose les listes par MediaKind et les libellés : profils enregistrés avec Motor, profil par défaut en premier, puis anciens choix sans doublon dans l’ordre C#. HardDisk25 et OpticalSlot restent disponibles ; la banque de base assure leur rendu comme dans le C# actuel. Le libellé de HardDisk25 est l’invariant 2.5 pouces ; les profils enregistrés gardent le nom de leur modèle.

Les anciennes routines de synthèse MechanicalSoundFunctions.Floppy/HardDisk/Optical, le modèle MechanicalSoundProfile et Get ne sont pas reproduits. Preuve dans MechanicalSoundFunctions.cs : Render et Duration commencent par BaseProfile(device) ; pour Tape, Floppy, HardDisk et Optical, cette valeur existe et la fonction retourne immédiatement via RecordedMechanicalSoundFunctions. Le switch de synthèse situé après est donc inaccessible pour ces catégories. Les modèles ne servent qu’à ces anciennes branches et aux identifiants concaténés par ForDevice. Les identifiants et libellés utiles sont conservés par la nouvelle API, tandis que ProtectMix reste implémenté dans le mixage du lecteur. Cette suppression est explicite dans l’inventaire, sans la présenter comme un DSP porté.

## Vérification

Compilation Debug réussie. Régression ciblée : position invalide pendant un accès déjà actif, exception d’observateur avec un autre accès présent, exception d’observateur sur le premier accès, refus d’une opération avec une opération préexistante, fermeture unique lors d’une exception dans le corps d’un scope. Aucun accès préexistant n’est terminé par les constructions refusées ; les accès effectivement acquis sont libérés.

Contrôle des profils : quatre catégories, listes sans doublon, noms non vides, HDD de base en premier et HardDisk25 en dernier, liste optique et libellé invariant conformes au C#. Audit réussi : 1 395 fichiers C++, 56 clés App, 29 langues ; constantes conformes. Le banc et ses exécutables temporaires ont été supprimés.

Cette régression valide les cas d’acquisition cités et les listes de profils ; elle ne constitue pas une validation globale des trois bibliothèques ou de l’émulation.
