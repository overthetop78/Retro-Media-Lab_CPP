# VMDK monolithique autonome

Référence en lecture seule : `Images/Formats/HardDisk/Vmdk/VmdkFormat.cs`, `VmdkReader.cs`, `VmdkWriter.cs` et les constantes VMDK de `HardDiskFormatConstants.cs`. Les empreintes des trois sources correspondent à l'inventaire.

Le découpage C++ sépare constantes et contrats, analyse/génération du descripteur, lecture sparse, adaptateur lecteur, écriture sparse et sélection/publication de la variante. Les sources à accès aléatoire, modèles de blocs et transactions atomiques existants sont réutilisés. Aucun traitement matériel ni accès MediaAnalysis n'est ajouté à MediaEngine.

Un descripteur texte monolithicFlat décrit exactement une étendue RW FLAT, avec nom de fichier entre guillemets et décalage sectoriel facultatif. Les chaînes de parents et autres types sont refusés. Les chemins sont résolus relativement au descripteur, vérifiés et ajoutés sans doublon aux chemins associés. La capacité et le décalage sont bornés avant multiplication ; la longueur de l'étendue doit couvrir la plage déclarée. Les règles de lecture des affectations et lignes d'étendue reprennent celles du C#, notamment le nom immédiatement suivi du signe égal et la syntaxe de décalage décimal non signé.

Le sparse version 1 utilise le répertoire et les tables de grains. Une entrée nulle désigne une plage non allouée ; une entrée égale à un désigne une plage nulle uniquement si le drapeau correspondant est actif. Les grains stockés restent des plages de source possédées et bornées. Les versions inconnues, compression/streamOptimized, marque d'état non propre, chaînes de parents et structures hors fichier sont refusées. Le descripteur embarqué éventuel doit déclarer monolithicSparse.

La sortie FLAT publie ensemble le descripteur et le fichier `-flat.vmdk`. La sortie sparse réserve le descripteur et les tables, n'écrit que les grains non nuls, puis complète les tables par positionnement. Le dernier grain physique est complété par des zéros. Les descripteurs reprennent les paramètres C# et un CID aléatoire excluant les valeurs réservées. Les noms contenant des caractères Unicode ou des séquences ressemblant à des paramètres Qt sont insérés littéralement.

Le choix Fixed/Sparse respecte la métadonnée du document VMDK, y compris ses valeurs numériques C# ; sinon il dépend des plages nulles/non allouées. Les ressources et fichiers temporaires suivent RAII et les transactions communes sur sortie normale, annulation ou exception.
