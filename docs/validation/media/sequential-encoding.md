# Encodage séquentiel et Atari CAS

Compilation Debug de MediaEngine réussie. Contrôle ciblé recompilé après le partage immuable des données : PASS. Audit : 1239 fichiers C++, 56 clés d’erreur, 29 langues ; PASS. Les 30 DLL de catalogues ont été compilées.

Le contrôle couvre les requêtes possédées, la sélection stricte du registre, la normalisation RAW/CSW vers WAV, la durée de vie asynchrone, les records/checksums Atari, les fichiers logiques vides, l’ordre des chunks conservés, les durées, les paramètres invalides, le refus des pertes et l’annulation. Aller-retour par le writer CAS, le reader CAS et le décodeur Atari réel réussi. Les artefacts temporaires sont supprimés.

Le portage reconstruit les records des blocs logiques CAS, dont le décodeur retire l’encadrement ; les octets série déjà encadrés restent directs. Il signale cette reconstruction dans les métadonnées et place le début des données après la pause. Les segments impossibles à conserver dans CAS sont refusés. Ces adaptations corrigent des incohérences du chemin inverse C# ; elles ne constituent pas une validation globale de tous les services de conversion.

Les services globaux et la composition automatique restent à porter.
