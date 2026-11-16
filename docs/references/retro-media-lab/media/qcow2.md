# QCOW2 autonome

Référence en lecture seule : `Images/Formats/HardDisk/Qcow2/Qcow2Format.cs`, `Qcow2Reader.cs`, `Qcow2Writer.cs` et les constantes QCOW2 de `HardDiskFormatConstants.cs`. Les trois sources du format correspondent aux empreintes de l'inventaire.

Le découpage C++ sépare les constantes, la validation de l'en-tête, les compteurs de références, la lecture des plages logiques, le calcul du placement et l'écriture. Il réutilise BlockImage, RecognitionContext, les sources à accès aléatoire et la publication atomique avec positionnement ; aucune dépendance à App ni MediaAnalysis n'est ajoutée.

Les versions 2 et 3 autonomes, non chiffrées et non compressées sont portées. Les fichiers parents demandant un résolveur absent, les snapshots, fonctionnalités incompatibles et autoclear non prises en charge, ainsi que les autres ordres de compteurs de références sont refusés comme dans le C#. La couverture L1, les bornes/alignements des clusters, la présence des blocs de références et les compteurs non nuls sont vérifiés lors de la reconnaissance. La lecture logique distingue les clusters stockés, explicitement nuls et non alloués.

Les tables L2 et blocs de références sont conservés dans les caches du lecteur pendant l'ouverture. Le document final possède les sources nécessaires à ses plages stockées ; tous les buffers et fichiers temporaires sont nettoyés par RAII et par la transaction commune.

L'écriture utilise les clusters de 64 KiB du C#, réserve les tables de métadonnées, n'écrit les données que pour les clusters non nuls, puis complète les tables et les références. Chaque cluster utilisé reçoit un compteur de références égal à un. La dernière portion logique est complétée par des zéros dans le conteneur physique. Le positionnement commun permet cette écriture sans conserver ni relire tout le disque.

La version 2 ou 3 est conservée uniquement lorsque le document source est QCOW2 et annonce une version prise en charge ; sinon la version 3 est choisie. Les tailles de buffers, produits et décalages de sortie sont bornés avant allocation ou écriture ; les décalages doivent rester représentables par le masque d'adressage QCOW2, sans troncature silencieuse.
