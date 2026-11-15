# Validation des images SCP sectorielles — 15 novembre 2026

Le banc compile 92 fichiers C# originaux, dont visualiseur, conversion, catalogue de types, politiques, orchestrateur d’encodage, writer SCP et modèles réels. Les constantes de formats nécessaires sont extraites de leurs déclarations. Pour satisfaire la validation d’en-tête appelée par le writer sans embarquer les interfaces de reconnaissance de médias, ReadHeader est extrait sans modification du ScpReader original dans un fichier temporaire et utilise les véritables ScpDataValidator/ScpSignature. Aucun algorithme de validation d’en-tête n’est réécrit dans le banc.

Les 109 sources compilées ou consultées correspondent à l’inventaire. Empreinte du manifeste : `294e14750040619e539de461793821233a635408036b88273df19d2761c18128`.

1 097 cas comparent les types de disque, tous les champs d’en-tête produits, l’ordre et les numéros de piste, les révolutions, leurs compteurs déclarés, origines et intervalles, la capacité/taille déclarée, les erreurs et l’annulation. Pour les cas d’écriture, les octets issus du vrai ScpWriter C# en MemoryStream sont comparés aux octets du writer C++ par callback. Les encodeurs d’enregistrement isolent l’assemblage SCP tout en passant par les politiques et TrackEncoderBase originaux ; les requêtes qu’ils reçoivent sont également comparées.

Sont couverts : familles du catalogue, casse, identifiants proches ou inconnus, capacités autour des seuils et troncature au kibioctet, une ou deux faces, pistes éparses, limites 41/42 pour le drapeau TPI, 360 RPM, images logiques, absence de politique ou d’encodeur, images vides, annulation, géométries Apple autour de 80 pistes, limites de numéro de piste sur un octet et limites de table SCP à l’écriture.

Les contrôles natifs supplémentaires utilisent les composants réels : visualisation et conversion physique d’une image IBM, décodage de leurs données ISO MFM, sérialisation/relecture SCP avec checksum, écriture atomique par chemin identique à la sortie callback, conservation d’une destination préexistante après annulation et propagation d’une exception du callback. Les ressources de fichiers sont fermées avant suppression ; les autres buffers et modèles sont possédés par RAII.

- Construction complète et déploiement Debug/Release réussis.
- Chaque configuration : **1 110 contrôles, 0 différence, 1 097 cas**.
- Audits : **570 fichiers C++**, **558 fichiers pour les frontières**, **0 violation** ; git diff --check réussi.
- Aucun avertissement ou erreur C++ dans les journaux complets finaux.
- Aucun nouveau texte à traduire ; les erreurs existantes sont réutilisées.
- Banc et journaux temporaires retirés après validation ; aucun test dans le workflow GitHub.

La validation établit ces parcours d’assemblage et d’écriture SCP ; elle ne certifie pas encore tous les lecteurs de formats, les autres conversions ou l’intégration complète de l’application.
