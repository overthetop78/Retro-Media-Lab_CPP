# Validation du registre d’encodage — 15 novembre 2026

Huit sources C# originales sont compilées en lecture seule dans un exécutable de référence : registre, exceptions, interface, requête, secteur, valeurs par défaut, piste encodée et révolution. Les huit SHA-256 correspondent à l’inventaire ; empreinte du manifeste : `45f7429a3adce0640ff2a12df2276c6e4510db3e4fd24c9274b2f5b36ae4b44b`.

Le double d’encodeur du banc permet de comparer ordre, sélection, identité, cellules produites et révolution reçue. Le catalogue C# par défaut est remplacé uniquement pour satisfaire la liaison, et n’est jamais appelé dans ce banc : ses déclarations originales ont été confrontées au catalogue C++ dans le bloc 59. Le constructeur C++ par défaut est ici contrôlé avec ses 24 objets réels.

281 cas couvrent collections vides, entrées nulles, identifiants blancs Unicode, doublons et priorité des erreurs, casse, formes Unicode distinctes, caractères nuls internes, U+FEFF et U+200B, identifiants avec apostrophes/sauts de ligne, modification d’identifiant après construction, copie de collection et délégation de requêtes. Les diagnostics comparent clé et arguments. Les contrôles Unicode indépendants utilisent String.IsNullOrWhiteSpace comme référence et construisent le véritable registre de décodage C++.

Les contrôles C++ supplémentaires vérifient durée de vie après destruction du registre, copies/déplacements, libération après le dernier propriétaire, indépendance du catalogue, annulation et priorité de recherche avant annulation. Les textes chargés depuis les DLL sont comparés aux quatre entrées de chaque langue, soit 116 textes.

- Construction complète et déploiement Debug/Release réussis.
- Chaque configuration : **453 contrôles, 0 différence, 281 cas, 116 textes**.
- Audits : **541 fichiers C++**, **529 fichiers pour les frontières**, **0 violation** ; git diff --check réussi.
- Défaut de code trouvé et corrigé : perte de U+FEFF initial lors de la validation des identifiants, dans les deux registres.
- Le banc temporaire et ses journaux sont retirés après validation. Aucun test n’est ajouté au workflow GitHub.

Ces résultats valident le registre et la correction ciblée ; ils ne certifient pas le portage complet des bibliothèques médias.
