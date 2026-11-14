# Disponibilité au démarrage

14 novembre 2026. Référence C# consultée en lecture seule : `StartupHardwareMonitor.cs` et `GreaseweazleHardwareScanFunctions.cs`.

Après chargement des préférences, App lance le service HardwareAvailability d’Infrastructure, sans attendre l’ouverture des préférences ni un clic sur Scanner. Le service vérifie uniquement les catégories et backends configurés : Greaseweazle, KryoFlux, optique, audio, interfaces de disques, cartouches et bandes de données. Les détecteurs disponibles fournissent les états réels ; une catégorie sans adaptateur détectable n’est pas déclarée disponible arbitrairement.

Greaseweazle utilise la confirmation `gw info`, comme la détection existante, avec actualisation du port et du numéro de série. Les entrées audio désactivées ou débranchées restent indisponibles. Les configurations absentes sont conservées. Une erreur d’une catégorie n’interrompt pas les autres vérifications. Aucun test de secteurs, écriture, déplacement de tête ou demande UAC n’est lancé au démarrage.

L’inventaire tourne dans un thread de service ; son résultat est appliqué aux réglages actuels pour conserver les modifications intervenues pendant l’attente. WorkspaceState notifie la barre d’état. Le service appartient à la session, indépendamment des fenêtres du thème. À la fermeture, le travail est rejoint et les processus éventuels sont arrêtés et libérés.

Le banc vérifie les sept groupes, les deux backends de disquettes, les identifiants absents, les périphériques réels audio/optique et les interfaces présentes, la conservation des réglages et l’actualisation des ports. Un exécutable GW absent est signalé sans empêcher la vérification des autres catégories. Retour asynchrone et nettoyage vérifiés.

Compilations et déploiements Debug/Release réalisés dans leurs dossiers habituels. Audit final : 123 fichiers C++, 55 clés générales d’erreurs et 29 langues. Les clés/paramètres des catalogues matériels et `git diff --check` sont validés. Infrastructure reste indépendante d’App/Qt Widgets, App n’exécute pas d’appels natifs matériels.
