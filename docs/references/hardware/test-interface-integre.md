# Vérification matérielle intégrée au logiciel

## Fonctionnement

Le bouton « Tester l’interface » appelle DiskProbe dans Infrastructure.dll. Le service retrouve le périphérique par son identifiant PnP et vérifie son rattachement à l’interface sélectionnée ; aucune adresse physique fournie par le GUI n’est utilisée directement pour le processus élevé.

Le logiciel exécute les interrogations d’identification et une lecture brute limitée à un secteur. Chaque commande possède son propre état : non vérifiée, vérifiée, rejetée par la pile du périphérique, refus de droits ou échec. Une commande rejetée ne signifie pas qu’un système de fichiers étranger est illisible.

Si les droits sont insuffisants, le service demande UAC pour un processus auxiliaire du même exécutable. Le lanceur charge alors uniquement Infrastructure.dll et son point d’entrée de vérification, sans construire de GUI. La fenêtre principale conserve ses droits habituels. Le processus auxiliaire accepte uniquement la vérification intégrée en lecture seule, sans commande arbitraire ni chemin de fichier d’entrée/sortie.

Le résultat rejoint la fenêtre ouverte par un échange local éphémère, limité en taille et en durée. Le canal est réservé à l’utilisateur, possède un nom aléatoire et transmet les identifiants puis les métadonnées ; aucun contenu de secteur n’est transmis ni conservé. Le service vérifie les identifiants de retour et la cohérence des états. Les objets Qt, handles du processus et ressources COM sont détenus par RAII.

Une autorisation UAC refusée reste distincte d’un rejet de commande et d’un échec de communication ; les vérifications nécessitant ces droits ne sont pas déclarées réussies. Les textes du résultat, du refus et des erreurs sont dans les 29 catalogues de langues.

## Sources consultées le 14 novembre 2026

- [SHELLEXECUTEINFOW — Microsoft](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow) : lancement avec le verbe runas, récupération du handle de processus par SEE_MASK_NOCLOSEPROCESS, fermeture du handle par l’appelant.
- [QLocalServer — Qt](https://doc.qt.io/qt-6/qlocalserver.html) : restriction UserAccessOption et attente bloquante utilisable dans le thread de travail.
- [QLocalSocket — Qt](https://doc.qt.io/qt-6/qlocalsocket.html) : canal nommé Windows, échanges et attentes synchrones bornées.

Les protocoles matériels et leurs références restent dans `commandes-stockage-brut.md`. Le C# a été consulté en lecture seule : ses services Infrastructure sont la référence de séparation ; aucun parcours équivalent de test HDD/Zip avec UAC n’a été trouvé dans les fichiers concernés.
