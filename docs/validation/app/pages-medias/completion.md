# Clôture des cinq pages médias

Périmètre : Lecture, Écriture, Conversion, Visualisation et Explorateur, raccordés aux services existants à partir du projet C# en lecture seule. Les cinq images jointes à la demande servent de référence des commandes ; la disposition a été réorganisée conformément à l’autorisation utilisateur.

| Exigence | Sources actuelles | Preuve de vérification |
| --- | --- | --- |
| Lecture : lecteur, destination, nom, brut/format connu, profils, options, numérotation, exécution/arrêt, progression et résultat | CapturePage, CaptureSession, MediaCommands, GwProgressTracker, GreaseweazleTools | pages-app.md : progression GW et résumé SCP ; sortie fractionnée, réussite/échec, arrêt, nettoyage, format connu ; captures read et read-advanced |
| Écriture : source, interprétation explicite, profils/options, confirmation, exécution/arrêt et progression | CapturePage, CaptureSession et lanceur partagé | pages-app.md : écriture simulée, refus d’interprétation ambiguë, arrêt conservant la source ; capture write |
| Conversion : destinations multiples, profils, moteur strict, pertes, PCM, conflits et progression | ConversionPage, ConversionSession, ConversionDialogs, ConversionConflicts | pages-app.md : lot réel depuis le GUI, aller-retour RAW/WAV, conflits mixtes, protection des sources ; capture convert |
| Migration de fichiers depuis Conversion | MigrationDialog, MigrationSession, service MediaEngine Migration existant | migration/validation.md : chargement, pertes, sauvegarde réelle, protection et reconstruction du dialogue |
| Graphiques adaptés : secteurs, flux, optique, blocs/cartouches et bandes/signal | MediaGraph, FluxGraph, LinearGraph, SequentialGraph et modèles publics MediaEngine | pages-app.md : contrôles de géométrie/pointage/inspection et rendus conservés ; captures sous docs/references/retro-media-lab/media/screenshots |
| Zoom lié ou indépendant et pointage transformé | VisualizationSession et MediaGraph | zoom/validation.md : deux faces, molette, déplacement, sélection, liaison et reconstruction |
| Hexadécimal : cellules, offsets, 16 ou 8 octets et dernière colonne ASCII | HexGrid | pages-app.md : métriques du délégué, absence de troncature, 17/9 colonnes, dernière ligne partielle |
| Explorateur : arbre, dossiers/fichiers, volumes/sessions, détails/diagnostics, lecture physique | ExplorerPage, ExplorerSession, ExplorerReadSession | pages-app.md : sélection restaurée, résultat partagé, capture physique simulée et durée de vie de sa ressource temporaire ; capture explorer |
| Classification manuelle | ClassificationSelection, ClassificationPanel et sessions | classification/validation.md : catalogue, protections, décodeur, exploration explicite/automatique, langue et reconstruction |
| Langues, palettes, densité, conservation des données | WorkspaceState, sessions Infrastructure et coordinateur réel de thème | validation.md : parcours des cinq pages vides/chargées en 1080p et 800×600 ; langues fr-FR/en-US/de-DE ; clair/sombre et densité |
| Séparation GUI/services | CMake et contrats publics | audit_media_architecture.py : 1668 fichiers, aucune violation ; App n’accède pas à MediaFileSystems/MediaAnalysis |
| Constantes et traductions | catalogues et en-têtes nommés | audit_literals.py : 1680 sources, 56 clés d’erreur, 29 langues, aucune valeur applicative hors constantes nommées |

Compilation Release et installation locale terminées, sans erreur ni avertissement C++ dans le dernier bloc. Les programmes de vérification, copies d’entrée et sorties temporaires ont été supprimés. Les captures et comptes rendus restent dans la documentation.

Les essais d’opérations matérielles de ce portage utilisent un fournisseur GW simulé ; aucune écriture sur un support physique de l’utilisateur n’a été effectuée. Les conversions et migrations de petites images ont produit des fichiers réels. Un moteur non encore disponible reste signalé comme tel, sans remplacement silencieux : ce travail ne crée pas de nouveau moteur matériel.

Référence détaillée et historique des contrôles : ../../../references/retro-media-lab/media/pages-app.md. Les anciens paragraphes de cet historique décrivent leurs étapes respectives ; ce tableau et les validations de ce dossier décrivent l’état final des cinq pages.
