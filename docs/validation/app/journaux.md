# Validation des journaux

Validation Windows du 14 novembre 2026, Qt 6.12.0, MSVC x64. Référence fonctionnelle : [journaux C#](../../references/retro-media-lab/journaux-csharp.md).

La page Journaux contient les 14 actions C#, leurs cases d'activation et d'archivage, une taille maximale en KiB et le dossier accessible depuis le pied de page. Les cases sont sauvegardées immédiatement ; la taille est validée à la perte de focus. Une valeur invalide restaure la valeur précédente. Zéro signifie aucune limite. Les langues et couleurs peuvent changer sans perdre ces réglages ni la session.

ConsoleLogSession écrit un fichier par action et consulte les réglages courants à chaque écriture. Les archives sont datées avec suffixe de collision. Sans archives, la réduction conserve les lignes UTF-8 récentes entières. Comme en C#, une entrée trop grande peut être omise lors de cette réduction ; dans un nouveau fichier après archivage, une entrée peut dépasser seule la limite.

DiagnosticLog fournit les fichiers quotidiens errors, warnings et information, indépendants des options par action. Les exceptions applicatives interceptées sont raccordées au journal d'erreurs. Les actions médias et les producteurs d'avertissements/informations seront raccordés lors du portage de leurs composants ; aucune opération non portée n'est simulée.

Dossier Windows : %APPDATA%/Retro Media Lab/Logs. Si portable.flag est un fichier à côté de l'exécutable : Data/Logs à côté de celui-ci. Le dossier est créé à la première écriture ou à son ouverture. Les échecs d'écriture sont non fatals ; l'échec d'ouverture produit un message traduit. Les ressources utilisent une destruction automatique, y compris en sortie par exception. L'adaptation Unix est préparée mais non validée sur Linux/macOS.

Contrôles réalisés :

- Compilation et déploiement Debug et Release réussis, sans avertissement C++ relevé dans les journaux finaux.
- Réglages par défaut et repli global, sauvegarde/rechargement, activation à chaud, validation des tailles, absence de sauvegardes parasites à la fermeture.
- Limites UTF-8, zéro sans limite, archives et collisions, entrée surdimensionnée, nettoyage des noms, séparation des actions et 200 écritures concurrentes.
- Trois diagnostics quotidiens avec métadonnées, chemins portables et échecs d'accès non fatals.
- Page Qt en mode offscreen, rendu sombre inspecté, 29 langues, unité KiB invariante et conservation des réglages au changement d'apparence.
- Ouverture du dossier contrôlée avec un gestionnaire d'URL de test : création et chemin vérifiés ; Explorer n'a pas été ouvert réellement par le test. Message d'erreur vérifié sur un chemin bloqué.
- Audit des constantes/traductions : 83 fichiers C++, 55 clés d'erreur et 29 langues. git diff --check réussi.

Le banc temporaire et ses artefacts ont été supprimés après utilisation. Aucun test ajouté au workflow GitHub.
