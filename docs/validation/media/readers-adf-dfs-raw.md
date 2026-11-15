# Validation ADF / DFS / IBM / MSX

- Compilation Debug MediaEngine réussie, sans avertissement signalé : `build/adf-dfs-raw-final-build.log`.
- Un programme natif ciblé en mémoire : `build/adf-dfs-raw-smoke.log`, PASS. Profils annoncés IBM/MSX, rejet d’un préfixe incompatible, taille IBM invalide, sonde MSX positive/négative, raccordement ADF Amiga et DOS Plus, SSD, bases de numérotation zéro/un, tête enregistrée DFS et état logique initial des secteurs complets.
- Après réduction des lectures inutiles de la sonde SSD/DSD, le même programme a été exécuté sur la DLL finale. Aucune ancienne batterie de codecs ni comparaison C# relancée.
- Les données synthétiques ne couvrent pas toutes les géométries ni toutes les images ADF partielles ; la gestion des images partielles réutilise le backend existant et la recette de diagnostic a été contrôlée dans le code.
- Audit des littéraux : 871 fichiers C++, 29 langues, PASS. Audit des frontières : 859 fichiers, aucune violation des règles vérifiées. Solution Visual C++ régénérée.
- Aucun nouveau message à traduire. Banc temporaire supprimé après validation. Le portage global reste incomplet.
