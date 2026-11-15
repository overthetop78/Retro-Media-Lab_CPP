# BBC DFS : responsabilités et compatibilité

Référence : les six sources `FileSystems/Acorn/BbcDfs/` de MediaFileSystems et les sept sources `Images/Formats/Floppy/BbcDfs/` de MediaEngine du dépôt C# `F:/GW GUI`, consultées sans modification.

MediaFileSystems possède le schéma du catalogue DFS, son décodeur partagé, les noms, champs compactés, fichiers et volumes. MediaEngine possède les profils SSD/DSD, la reconnaissance des conteneurs, leurs sources et leur écriture. La validation structurelle du catalogue et son décodage ne sont pas dupliqués dans le moteur.

Les quatre profils sont SSD 40/80 pistes et DSD 40/80 pistes, avec secteurs de 256 octets et dix secteurs par piste. Les suffixes SSD/DSD acceptent les images tronquées comme le C#. Les alias IMG/IMA nécessitent une capacité exacte et des catalogues DFS structurés ; ADL exige le profil double face 80 pistes et deux catalogues DFS valides. Une extension ADL seule ne suffit pas à déclarer du DFS.

Les variantes connues Atom DOS et JFD conservent les règles C# : qualificatif racine, capacité de secours Atom seulement, répertoires de faces 0 et 2 et choix des profils. Les champs de longueur/adresses sur 18 bits et de secteur initial sur 10 bits sont décodés séparément. Les noms suivent le tri ordinal ASCII insensible à la casse du C#, y compris sa position relative aux signes de ponctuation. Les descriptions techniques des adresses sont des constantes et utilisent la locale classique.

La projection privée CatalogueView emprunte les secteurs physiques sans copier leurs octets ni modifier leurs adresses. Les secteurs BBC sont numérotés depuis zéro ; l'adresse enregistrée de face reste zéro sur les deux faces, conformément au modèle C#. Le marqueur de complétude des blocs entiers est conservé ; aucun indicateur de CRC d'en-tête ou de données n'est inventé. Les valeurs par défaut des images IBM/MSX restent inchangées.

Les secteurs de fichier absents sont remplacés par des zéros à leur position, avec avertissements. La capacité libre conserve le calcul C#. Les renforcements explicites sont la propagation des indicateurs d'intégrité invalides, la capacité libre inconnue dans ce cas et un diagnostic de bloc trop court. Une face illisible reçoit un diagnostic codé et un contexte de face séparé ; le commentaire d'entrée ne reçoit plus d'exception anglaise brute.

Les sources fichier sont lues à la demande par une interface en lecture seule : elles ne constituent pas une copie immuable des octets du fichier. Les écritures partagent le writer sectoriel linéaire et la publication atomique ; annulation et exception abandonnent le temporaire et préservent la destination existante.

Les diagnostics appartiennent aux domaines MediaFileSystems et MediaEngine ; leurs sept clés sont présentes dans les 29 langues. L'enregistrement général des lecteurs, les documents de reconnaissance et la façade applicative restent à porter. Aucun raccordement GUI supplémentaire n'est introduit ici.
