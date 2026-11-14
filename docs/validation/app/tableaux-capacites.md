# Validation des tableaux matériels

14 novembre 2026. Services réels d’Infrastructure et vues du thème RetroMediaLab, dans un banc temporaire.

## Vérification au démarrage

La configuration existante a été chargée en lecture seule puis passée au service de démarrage, sans cliquer sur Scanner. Le Greaseweazle enregistré est confirmé sur COM3, disponible, sans erreur. Le banc du point 21 couvre en complément les sept groupes, les deux backends de disquettes et les périphériques absents. Les réglages utilisateur ne sont pas réécrits par le banc.

## Tableaux optiques

ASUS BW-16D1HT : 41 fonctions MMC et 18 profils relevés, 33 lignes présentées. HL-DT-ST DVDRAM GH24NSD5 : 34 fonctions et 14 profils, 29 lignes. La déclaration BD diffère correctement entre les deux lecteurs. Les profils complets sont conservés dans l’état de session. Une réponse MMC tronquée est rejetée ; une réponse valide sans fonction n’invente aucune capacité.

## Tableaux des interfaces

Les boutons du logiciel ont exécuté le parcours de vérification avec UAC pour le pont USB JMicron et le Zip USB. Accès brut limité à un secteur vérifié pour les deux, passage ATA vérifié via JMicron et identification SCSI vérifiée. Les nouvelles métadonnées du contrôleur traversent l’échange du processus élevé sans perte : pilote, emplacement, version annoncée, limites de transfert/pages, alignement et adressage.

Le banc vérifie que les sélecteurs et les cellules n’affichent pas le modèle ni la capacité du disque raccordé. Les essais non effectués et les droits manquants restent neutres. Une erreur globale d’interrogation produit une ligne FAIL traduite. Les annonces du pilote ne sont pas assimilées aux essais réels.

## Rendu et état

PASS vert et FAIL rouge vérifiés dans les cellules, avec la couleur du thème ; sources et détails présents dans les quatre colonnes. Les capacités vérifiées figurent en tête du tableau de stockage. Contrôle visuel à 150 % et 1100 × 760 : tableaux visibles dès le haut de la zone défilante, texte lisible et hauteurs recalculées après mise en page. Le calcul anticipé de hauteur avant attribution de largeur a été corrigé.

Changer de lecteur recharge son résultat conservé sans relancer la vérification. Changement fr-FR/en-US et reconstruction de la présentation : les deux rapports de stockage et les deux rapports optiques sont conservés.

Les libellés sont générés avec Argos pour les 29 langues, puis relus et corrigés ; les invariants techniques restent communs. Les contrôles de clés, paramètres et marqueurs de traduction passent. Les captures, résultats, DLL du banc et catalogues intermédiaires sont supprimés après validation, sans ajout au workflow GitHub.

Compilations et déploiements Debug/Release réalisés dans leurs dossiers habituels. Audit final : 123 fichiers C++, 55 clés générales d’erreurs et 29 langues. Les clés/paramètres des catalogues matériels et `git diff --check` sont validés. Infrastructure reste indépendante d’App/Qt Widgets, App n’exécute pas d’appels natifs matériels.
