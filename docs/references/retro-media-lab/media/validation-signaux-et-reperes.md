# Signaux et repères des bandes

Références C# : TapeSignalFunctions.cs, TapeBlockFunctions.cs et TapeImageFormatIds.cs ; empreintes à jour dans l’inventaire. Les fonctions natives restent dans MediaEngine, sans dépendance App et sans traitement matériel.

Lecture explicite des cinq conteneurs du C# : UEF, WAV, CSW, RAW et Atom TAP. L’écriture propose WAV, UEF, CSW et RAW. Les paramètres de sortie ne sont transmis qu’au RAW, comme dans la référence. Les writers existants conservent la responsabilité des formats, de la compression et de la publication atomique.

Le signal reste indépendant de l’index de fichiers. Avec une machine compatible, les blocs décodés lui sont associés par position PCM ou par chunk source et décalage série. Le repli UEF vers PCM est conservé. Le retour avant la porteuse BBC suit les mêmes seuils que le C#. Les marqueurs sont dédoublonnés et triés. withBlocks partage les échantillons possédés et ne recopie pas la bande.

L’index de présentation utilise les blocs dont l’intégrité est validée et qui déclarent une position d’échantillon. Il conserve les canaux, les sources et leurs décalages, les métadonnées, messages et volumes. Les plages sont découpées sans transformation PCM. Les indices, alignements, débordements et annulations sont contrôlés.

Validation Debug ciblée réussie : WAV/RAW exacts, polarités et durée UEF ; CSW conserve les durées et prolonge le dernier niveau sur les silences PCM conformément au writer du C# (CSW ne stocke pas une amplitude analogique zéro). Un contrôle initial comparait à tort ces zéros à du PCM inchangé ; cette attente a été corrigée sans modifier le codec. Lecture WAV/UEF BBC avec deux blocs réellement reconstruits et associés ; calcul des repères de porteuse et du décalage série ; index de deux blocs BBC sur une bande stéréo avec canal droit inversé, égalité de tous les échantillons et partage des sources ; machine inconnue, format refusé et annulation.

L’index n’ajoute pas artificiellement de position à un décodeur qui n’en fournit pas. Le premier fixture Spectrum ne déclarait pas samplePosition et devait donc rester inchangé, comme en C#. Le contrôle du découpage utilise des blocs BBC porteurs des positions attendues.

Compilation MediaEngine Debug sans avertissement ; trente catalogues compilés après génération Argos et relecture. Audit : 1357 fichiers C++, 56 clés App, 29 langues. Le contrôle et ses médias temporaires ont été supprimés. La composition générale de reconnaissance, les façades et l’intégration applicative restent dans l’objectif global.
