# Runtimes Flux et MFM

Références C# en lecture seule : MediaRuntimeFluxDisk, MediaRuntimeMfmDisk et son fichier Sectors, sélection OpenDiskAsync de MediaRuntimePreparationService. Le contrat public RuntimeDisk reste unique et ne révèle aucun type MediaFileSystems. La sélection après reconnaissance est MFM EMU, puis flux protégés, puis secteurs/blocs.

## Organisation

L’état interne commun définit les opérations du contrat et possède uniquement les services, le document, le verrou et les règles de fermeture/propriété. DecodedState reprend l’implémentation secteur/bloc existante ; FluxState et MfmState portent les deux autres comportements C#. Chaque état concret ferme ses ressources depuis son destructeur sans lever d’exception. La fermeture explicite tente la sauvegarde, réinitialise seulement une mécanique possédée même sur erreur, libère les caches puis signale l’erreur. Un échec de construction avant transfert rend la mécanique créée par le service ; une mécanique fournie par l’appelant reste empruntée. L’API asynchrone conserve son état jusqu’à la fin des tâches.

Le runtime Flux conserve les pistes et toutes leurs révolutions. Une piste absente produit des révolutions sans impulsions, avec la période demandée et le nombre de révolutions du conteneur. Aucun secteur n’est inventé. Chaque fenêtre remplace les impulsions de toutes les révolutions. Les protections du conteneur, du fichier et les régions faibles interdisent l’écriture. La publication réutilise les pistes protégées et l’écrivain existant ; elle ne marque l’état propre qu’après réussite et conserve les pistes non ciblées.

Le runtime MFM utilise la représentation EMU déjà portée et son unique lecteur sectoriel Adaptec chargé à la demande. Une écriture sectorielle crée une fenêtre physique, sans substituer une image sectorielle au conteneur. Le cache porte sur une piste et sa période ; une écriture l’invalide. Les paramètres Acorn sont validés dans les champs physiques initiaux, sans création de fichier DSC. PrepareBlockFormat vérifie seulement les limites de la géométrie future : il ne redimensionne ni n’efface le conteneur. La sauvegarde publie puis rouvre la représentation avant de retirer le drapeau de modification ; les échecs permettent une nouvelle tentative.

## Vérification ciblée

Compilation Debug réussie sans avertissement signalé. Le banc ouvre une SCP de deux révolutions avec une piste absente entre deux pistes présentes. Il vérifie l’absence de projection sectorielle, la piste sans impulsions, les modifications de toutes les révolutions, l’annulation, l’échec de publication et la nouvelle tentative, puis la relecture avec piste non ciblée identique. Une SCP protégée et une HFE v3 contenant une région faible restent en lecture seule. La fermeture conserve une mécanique empruntée.

L’écrivain HFE refuse volontairement de fabriquer des régions faibles : le banc construit donc une HFE v3 valide puis y place l’opcode de région faible lu par le lecteur existant. Cette correction concerne la fabrication du cas de vérification, pas le comportement de l’application.

Une EMU formatée de deux cylindres vérifie 66 secteurs de 256 octets, 33 secteurs par piste, recherche d’adresse et ordre physique. PrepareBlockFormat laisse les cellules et la capacité inchangées. Les paramètres physiques corrects sont acceptés, une différence de pas du contrôleur est rejetée et aucun DSC n’est créé. Un secteur modifié est relu après publication/réouverture ; l’autre piste reste identique. Une piste effacée puis restaurée passe par les fenêtres physiques.

Un échec de publication EMU conserve les changements ; après restauration du chemin source, ils restent lisibles et la nouvelle sauvegarde réussit. Le banc restaure le chemin en RAII, y compris si une vérification lève une exception. Les lectures ne sont pas exigées pendant que le fichier source est volontairement inaccessible : les en-têtes de piste restent lus à la demande, comme dans le conteneur existant. Une fermeture avec publication en erreur réinitialise la mécanique possédée. Une EMU vierge garde zéro secteur, et une IBM 720 vérifie que le runtime secteur/bloc existant reste sélectionné et modifiable.

Audit des constantes : 1 422 fichiers C++, 29 langues, aucune valeur applicative hors constantes nommées. Les refus réutilisent les diagnostics traduits existants. Le banc et toutes ses images/exécutables temporaires sont supprimés après utilisation.

## Suite

La sélection des trois runtimes est disponible dans RuntimeDisk::openAsync. Les autres opérations de la façade de préparation et les intégrations applicatives restent à compléter. Ce bloc ne certifie pas le portage global des trois bibliothèques.
