# Validation du classement des readers

- Compilation Debug finale : `build/reader-selection-final-build.log`, réussie sans avertissement signalé.
- Un programme ciblé avec readers synthétiques : `build/reader-selection-smoke.log`, PASS. Vérifie les cinq niveaux de confiance, la priorité du nombre d’indices, la stabilité des égalités, les fichiers associés avec casse différente, l’exclusion explicite avant sonde, la taille maximale des signatures, la durée de vie de la future après destruction du registre temporaire, le refus des readers nuls, la propagation d’une erreur de sonde et de l’annulation.
- Le garde-fou pour signatures dépassant la limite d’un entier signé 32 bits a ensuite été compilé ; aucun buffer de plusieurs gigaoctets n’a été créé pour le tester.
- Nouveau diagnostic généré avec Argos dans les 29 langues, relu et corrigé pour les contresens observés. Constants et les 29 DLL compilées : `build/reader-selection-catalogs.log`.
- Audit des littéraux : 873 fichiers, 29 langues, PASS. Frontières : 861 fichiers, aucune violation des règles vérifiées. Solution Visual C++ régénérée.
- Aucune ancienne batterie ni comparaison C# relancée. Banc temporaire supprimé. Ces résultats ne valident ni la reconnaissance complète encore à raccorder ni la totalité du portage.
