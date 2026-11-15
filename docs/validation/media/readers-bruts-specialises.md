# Validation des readers bruts spécialisés

- Compilation Debug finale : `build/specialized-raw-final-build.log`, réussie sans avertissement signalé.
- Contrôle natif ciblé : `build/specialized-raw-smoke.log`, PASS. Vérifie les profils Epson 320 et Logo, adresses et tailles de leurs secteurs, pistes vides, état logique initial, refus de sonde sans choix explicite, géométrie/numérotation UCSD et refus d’un contenu COHERENT invalide.
- Le helper est vérifié sur filtre explicite insensible à la casse, validateur retournant faux sans exception (résultat de sonde vrai, comme en C#), rejet de format et propagation des erreurs E/S et de l’argument Epson inconnu.
- La vérification de l’argument inconnu a été ajoutée après consultation de son type C# exact. Elle a motivé le retrait de ce code de la politique de rejet ; le contrôle a été exécuté sur la DLL corrigée.
- Aucun média utilisateur ni ancienne batterie de codecs utilisé. La lecture positive exhaustive des profils COHERENT n’a pas été répétée : le raccordement utilise le backend existant. Banc temporaire supprimé après consignation.
- Deux messages générés avec Argos, relus et corrigés. Constants et les 29 DLL compilées : `build/specialized-raw-catalogs.log`. Audits des littéraux/frontières réussis et solution Visual C++ régénérée.
