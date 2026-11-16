# Validation des adapters flux

- Compilation Debug réussie sans avertissement signalé : build/flux-writer-adapters-build.log.
- Contrôle natif ciblé PASS : build/flux-writer-adapters-smoke.log.
- Cibles bitcell issues des ressources réellement embarquées acceptées par les deux writers ; raw.250 vérifié explicitement. Format/extension/représentation sélectionnés par WriterRegistry ; une extension croisée et une cible IBM sectorielle refusées.
- Sorties réellement produites par les adapters depuis un même document flux puis relues : SCP conserve les intervalles et la version brute 33 du document ; HFE conserve sa piste et la protection en écriture.
- Fichiers et handles possédés par QTemporaryDir et shared_ptr ; programme, binaires et dossier temporaires supprimés après contrôle.
- Aucun nouveau diagnostic ou catalogue. Audit des littéraux PASS : 967 fichiers C++, 29 langues. Projets Visual C++ régénérés.
- Aucun média utilisateur, aucune ancienne suite de codec répétée.

Ce contrôle valide les deux raccordements au registre commun, pas une composition globale de l'application.
