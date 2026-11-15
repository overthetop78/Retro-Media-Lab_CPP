# Validation ST/MSA

- Compilation Debug réussie : `build/st-msa-writing-build.log`. Un avertissement de dépréciation C++20 sur u8path a été corrigé ; la recompilation dans `build/st-msa-writing-final-build.log` réussit sans avertissement signalé.
- Contrôle natif ciblé : `build/st-msa-writing-smoke.log`, PASS. Transformation de 80×1×9 vers 40×2×9 à capacité constante ; vérification de la dernière adresse, conservation des données, tags, adresse enregistrée, états d’intégrité, marque supprimée et ordre enregistré. La source reste inchangée.
- Vérification du refus d’une cible avec perte de capacité et d’un secteur absent en mode strict. La complétion explicite produit des secteurs nuls et conserve les secteurs disponibles.
- Passage réel dans l’adapter : une source ST clairsemée échoue avant toute création de sortie ; la même image avec une source .ScP produit le MSA attendu. Relecture de ce fichier synthétique et vérification du secteur complété et du dernier secteur conservé. Les filtres ST/MSA et le refus IMG sont vérifiés.
- Le fichier synthétique est protégé par RAII dans le programme de contrôle, y compris sur exception. Programme, binaires et dossier temporaires supprimés après consignation. Aucun média utilisateur ni suite ancienne de codecs.
- Cinq diagnostics générés avec Argos dans 29 langues puis relus/corrigés : unités, pertes, blocs, secteurs, adresses et chinois traditionnel notamment. Catalogues compilés : `build/st-msa-writing-catalogs.log`, succès.
- Audit des littéraux : PASS, 942 fichiers C++, 29 langues. Projets Visual C++ régénérés.

Ces résultats couvrent l’écriture via l’adapter et la transformation ; le service global et la sélection des lecteurs source restent à terminer.
