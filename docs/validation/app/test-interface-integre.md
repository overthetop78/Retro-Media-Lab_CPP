# Validation du bouton « Tester l’interface »

14 novembre 2026. Vérifications effectuées par les services du logiciel, déclenchées par le vrai bouton du GUI chargé dans un banc temporaire.

## Résultats remontés dans le GUI

| Interface sélectionnée | Lecture brute | Identification SCSI | Identification ATA |
| --- | --- | --- | --- |
| Pont USB JMicron 152D:2338 | Vérifiée, un secteur | Vérifiée | Vérifiée via JMicron, modèle ST3500630AS |
| Zip USB 059B:0033 | Vérifiée, un secteur | Vérifiée, IOMEGA ZIP 100, révision 90.J | Rejet de la requête SAT par la pile du périphérique |

Pour les deux interfaces, le parcours non élevé a demandé UAC, puis le processus auxiliaire a renvoyé le résultat au GUI. Aucun accès par un fichier du volume, aucune écriture de secteur, aucun formatage ou réglage matériel. Les données lues ne sont pas sauvegardées.

Le parcours intégré confirme désormais l’identification SCSI du Zip, qui restait non vérifiée dans la validation précédente. Le rejet ATA reste distinct de cette réussite et de celle de l’accès brut.

## Contrôles du logiciel

- Identifiants exacts, correspondance interface/périphérique, état de chaque commande et taille limitée de l’essai vérifiés.
- Bouton désactivé pendant l’opération ; résultat reçu dans la fenêtre, changement de langue et reconstruction du thème conservant les deux rapports.
- Lanceur livré vérifié avec une requête de cible inexistante : retour d’erreur, code de sortie d’échec, sans lancer App.
- Sérialisation des états et rejet des rapports incomplets/incohérents ou contenant des nombres invalides vérifiés.
- Affichage du refus UAC vérifié avec un état injecté dans le GUI. L’annulation manuelle de la boîte UAC n’a pas été exercée ; les acceptations UAC et les échanges réels l’ont été.
- Contrôle visuel à 150 % : résultats par capacité et modèle identifié lisibles dans les cartes défilantes. Le banc hors écran utilise explicitement le plugin et la police du kit installé ; sa première invocation avait omis le chemin du plugin hors écran, puis a été corrigée.
- Génération Argos des huit nouveaux libellés dans les 29 langues, relecture et correction des ambiguïtés (pilote, droits administrateur, états de vérification).

Les fichiers du banc, DLL auxiliaire de validation, scripts, captures, rapports JSON et catalogues intermédiaires sont temporaires et supprimés après validation. Aucun fichier de média n’est ajouté et aucun test n’est intégré au workflow GitHub.

Compilations et déploiements Debug/Release réussis dans leurs dossiers habituels. Audit : 119 fichiers C++, 55 clés générales d’erreurs et 29 langues ; clés et paramètres des catalogues matériels cohérents. Aucune dépendance App/Qt Widgets introduite dans Infrastructure, aucun appel matériel natif introduit dans App. `git diff --check` passe.
