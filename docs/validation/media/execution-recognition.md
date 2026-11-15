# Validation de l’exécution de reconnaissance

- Compilation Debug : `build/recognition-build.log`, réussie sans avertissement signalé.
- Un programme natif ciblé : `build/recognition-smoke.log`, PASS. Vérifie repli après rejet de format, résultat possédant le document et les échecs précédents, exception agrégée, absence de candidat, refus d’un document nul, arrêt sur E/S, annulation et équilibre des débuts/fins d’opération et d’accès.
- Le même programme vérifie qu’un lecteur fourni n’est pas instrumenté par la reconnaissance et qu’un échec pendant le début d’opération tente la fin d’opération. La première compilation du banc utilisait un nom de diagnostic inexistant ; seul le banc a été corrigé avant son exécution.
- Aucun média utilisateur, aucune ancienne batterie de codecs et aucune comparaison C# supplémentaire exécutés.
- Diagnostics générés avec Argos puis relus/corrigés. Constants et 29 DLL de langues compilées : `build/recognition-catalogs.log`.
- Audit des littéraux : 878 fichiers C++, 29 langues, PASS. Frontières : 866 fichiers, zéro violation des règles vérifiées. Solution Visual C++ régénérée. Ces contrôles ne prouvent pas l’achèvement de la composition et des services restants.
- Banc temporaire supprimé après consignation.
