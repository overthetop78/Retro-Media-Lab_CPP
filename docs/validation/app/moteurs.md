# Préférences Moteurs

## Réalisation

La page Moteurs contient deux sous-onglets : Greaseweazle et KryoFlux. Greaseweazle propose les quatre opérations et les deux choix du C#. KryoFlux ne contient aucun contrôle.

`Infrastructure/Engines/EngineSettings` porte les identifiants et la sérialisation par backend/opération. App affiche les réglages et relaie leurs changements. Chaque sélection est sauvegardée immédiatement dans les préférences, indépendamment des autres opérations. Un ancien fichier sans section `engines` reçoit les valeurs initiales du C#.

Un identifiant de fournisseur absent reste conservé et s’affiche comme indisponible, sans remplacement par un autre choix. Cette extensibilité ne constitue pas un chargeur de DLL. Le moteur interne reste non implémenté. Aucun moteur d’exécution n’est ajouté ici.

## Vérifications du 14 novembre 2026

Banc temporaire unique exécuté avec succès : valeurs initiales C#, sélections indépendantes, sauvegarde/relecture, conservation des autres préférences, ancien fichier sans moteurs, rejet des données invalides, conservation d’identifiants inconnus, exactement deux sous-onglets et KryoFlux vide, changements FR/EN sans écriture parasite, conservation après reconstruction de la présentation.

Captures Qt hors écran vérifiées en clair et sombre, fenêtre de préférences 1100 × 700 avec facteur 1,5 : les quatre choix et les indications tiennent dans la fenêtre. Cela ne remplace pas une vérification interactive du matériel, qui n’est pas concerné par cette page.

Les libellés existants proviennent du C#. Les trois nouveaux messages ont été traduits avec Argos, puis relus ; catalogues de 29 langues cohérents. Les noms Greaseweazle et KryoFlux sont des invariants de Constants.

Compilations et déploiements locaux Debug et Release réussis avec `scripts/local-building/build.ps1`. L’audit des constantes/traductions passe : 128 fichiers C++, 56 clés d’erreur, 29 langues. `git diff --check` passe.

Le banc et ses fichiers temporaires ont été supprimés après validation. Aucun test n’est ajouté aux workflows GitHub.

À la demande de l’utilisateur, suppression du message sur la disponibilité du moteur interne et de son infobulle.
