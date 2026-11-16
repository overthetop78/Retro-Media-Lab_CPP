# Façade fichier sectoriel vers SCP

Référence C# : `SectorImageScpFileConversionService.cs`. Le convertisseur SCP et les encodeurs existaient en C++ ; la façade recevant un chemin source n'était pas exposée.

`canCreateSectorScp` reconnaît la paire raw.scp/.scp sans distinction de casse. `convertSectorFileToScpAsync` lit le document par le service commun, exige une représentation sectorielle et délègue à `Scp::SectorConverter`. Aucun explorateur de système de fichiers n'est lancé et aucun algorithme d'encodage n'est réimplémenté. Les gardes d'activité et les propriétaires locaux restent valables sur exception.

Compilation native réussie après ajout de l'inclusion directe du contrat de représentation sectorielle. Un seul contrôle de raccordement a converti un fichier IMG temporaire de 160 Kio, reconnu les 40 pistes et l'en-tête/checksum de la capture produite, puis confirmé qu'une annulation préalable ne crée aucune destination. Aucun comparatif de codec n'a été relancé. Le répertoire du contrôle et ses médias temporaires ont été supprimés.

Le contrat abstrait `GwFormatCapabilityReader` est également porté dans `GwFormatCapabilities.h` : chemin de l'exécutable, retour asynchrone et annulation. Il ne lance aucun processus ; une implémentation hôte appartient à Infrastructure. Son interprétation des résultats reste fournie par `parseGwReadHelp`.
