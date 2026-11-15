# Codec Amiga MFM — référence C#

Le bloc 35 reprend AmigaMfmCodec, AmigaMfmFormat, AmigaMfmTrackEncoder et AmigaMfmDecoder. Les trente-cinq sources compilées par le banc, y compris primitives de bits, reconstruction temporelle, encodeur de base et modèles, ont des empreintes conformes à l’inventaire. Le dépôt C# reste en lecture seule.

## Primitives odd/even et parités

Les bits impairs et pairs de chaque paire d’octets sont séparés en deux moitiés puis réassemblés dans leur ordre logique. L’encodage exige une longueur paire. Le décodage conserve le comportement source pour une longueur impaire : la dernière case du résultat reste nulle.

Les deux calculs de parité conservent leurs parcours source, par groupes de quatre octets ou en réunissant les deux moitiés. La plage peut commencer à un offset non aligné ; sa longueur doit être positive ou nulle, multiple de quatre et entièrement disponible. La validation C++ compare les bornes sans addition signée susceptible de déborder. Les entrées C# en débordement qui finissent par une exception d’index sont rejetées avant accès, avec le diagnostic de plage explicite.

L’empaquetage cylindre/face conserve l’octet bas du calcul source même sur les entrées arbitraires de cette aide ; l’encodeur valide séparément ses coordonnées physiques.

## Encodage

Après les validations communes, la piste Amiga exige cylindre 0–127, face 0–1 et au plus 255 secteurs. Chaque charge utile doit contenir 512 octets avant validation de son numéro 0–255. L’ordre des refus suit le C#, notamment lorsque taille et numéro sont tous deux invalides.

Chaque secteur conserve l’ordre demandé, le nombre de secteurs restant, le champ de format FF, un label nul, les deux parités et les données odd/even. Il est précédé du gap de 100 bits et des deux mots bruts 4489, puis suivi du gap de 128 bits. Les octets passent par le codage MFM commun. La révolution utilise les durées demandées et les règles communes de construction d’intervalles.

Les attributs de marque supprimée, code de taille imposé, coordonnées enregistrées et CRC enregistrés ne sont pas pris en compte par cet encodeur, comme dans la source. Le bloc n’invente pas leur prise en charge. Les constantes de géométrie et de crédibilité haute densité sont conservées ; les politiques de reconnaissance de densité restent à porter.

## Décodage et troncature

La reconstruction adaptative MFM est partagée avec les autres codecs. Le décodeur recherche deux synchronisations successives et tente d’abord le secteur encodé complet, puis l’en-tête seul. Une synchronisation sans en-tête complet produit encore un secteur indéterminé à coordonnées nulles, comme le C#.

Les octets décodés rassemblent les quatre octets d’information et, si disponibles, les 512 octets de données. Le format et la parité d’en-tête sont contrôlés séparément de la parité des données. Une intégrité fausse domine ; les données absentes restent indéterminées lorsque l’en-tête est valide. Le type d’intégrité est Checksum et la taille logique reste 512 octets avec code 2.

Les structures conservent type, position, longueur et états d’intégrité distincts dans leur description. Le décodeur ne peuple pas artificiellement tag, format, CRC enregistrés ou champs d’intégrité séparés que le constructeur source laisse vides. L’avance après un secteur ou un en-tête tronqué et l’absence de rebouclage circulaire suivent la source. La confiance utilise les poids et le diviseur Amiga.

## Organisation, ressources et traduction

Codecs/Amiga sépare odd/even, parités, encodeur et décodeur. Les classes concrètes Encoder/Decoder utilisent les contrats communs et le registre explicite. La collecte ISO et son cache peuvent consommer le résultat sans dépendance supplémentaire d’App.

Buffers, résultats et pistes sont possédés ; RAII et les contrôles d’annulation couvrent aussi les sorties par exception. Les quatre diagnostics et la recette de description de MediaEngine/amiga-mfm.json sont générés avec Argos puis relus dans les 29 langues. Les corrections préservent Amiga/MFM, odd/even, les six paramètres de description, les négations et les termes de parité/somme de contrôle.

Le catalogue complet de codecs, les politiques Amiga de reconstruction et l’exploration restent distincts et non déclarés portés par ce bloc.
