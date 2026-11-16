# Lecteur SCP commun et cache partagé

Références C# : ScpReader.cs, ScpFileCache.cs, ScpMetadataFunctions.cs, ScpMetadataKeys.cs, ScpProtectedTrackImageAdapter.cs et IScpReader.cs. Le backend de conteneur existant est réutilisé.

Scp::Reader implémente ImageReader et le contrat injectable CaptureReader. Il reconnaît uniquement raw.scp, avec signature SCP et longueur minimale de l'en-tête, puis construit un document flux. La lecture de fichier et Remember sont disponibles séparément ; Remember est optionnel dans le contrat abstrait et implémenté par le lecteur concret. Comme dans le C#, la lecture commune mémorise l'image avec l'identité de son chemin physique ; ce chemin doit donc désigner un fichier.

Les neuf métadonnées source sont conservées : version brute/texte, type de disque, drapeaux, largeur, faces, résolution, résolution en nanosecondes et état de la somme de contrôle. Les booléens techniques restent True/False ; un état de checksum est une métadonnée, pas un nouveau rejet implicite.

L'identité chemin normalisé/taille/date est partagée avec le cache de reconstruction ISO dans Sources/FileIdentity. Les chemins suivent la casse insensible de Windows et la casse native sur les autres plateformes. Le cache SCP conserve une seule version courante par chemin. Les appels simultanés partagent un chargement ; un échec observé retire uniquement l'entrée concernée, sans effacer une entrée remplacée entre-temps.

Chaque attente utilise son propre jeton d'annulation. L'annulation de l'appelant conserve le chargement partagé. Les travailleurs reçoivent un jeton distinct appartenant au cache ; la destruction du dernier propriétaire les arrête et les rejoint. Les travailleurs terminés sont joints hors du verrou du cache, pour ne pas détruire les callbacks utilisateur sous ce verrou. Le cas d'un callback possédant lui-même une copie du cache libère le handle du travailleur déjà terminé sur son propre fil, évitant une auto-jonction.

TrackFluxRevolution possède désormais un pointeur partagé immuable, avec conservation du constructeur par valeur. Le document SCP utilise des pointeurs alias vers les révolutions de la capture possédée. Les buffers ne sont pas recopiés pour ce raccordement et leur propriétaire reste vivant tant que le document les utilise. La conversion par référence existante reste disponible pour les appelants ne fournissant pas de propriétaire partagé.

Les deux nouvelles erreurs sont traduites ; fichiers, captures, attentes et travailleurs ont un nettoyage RAII. Ce bloc termine le lecteur/cache et le raccordement SCP, sans constituer la composition complète de tous les lecteurs ou du service global de lecture.
