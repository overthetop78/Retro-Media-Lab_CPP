# Lecteur KryoFlux

Référence C# en lecture seule : `Images/Formats/Floppy/KryoFlux/KryoFluxReader.cs` et `KryoFluxFormat.cs`, dans MediaEngine. Les empreintes SHA256 correspondent à l'inventaire. Aucun writer KryoFlux n'est présent dans cette référence.

Le port sépare le décodage des impulsions, les métadonnées hors bande, la découverte des pistes et l'adaptateur de reconnaissance. Il utilise les modèles de pistes protégées, sources et registres existants. Aucune commande de matériel ne se trouve dans ce lecteur de fichiers.

La reconnaissance demande un nom NN.S.raw, l'extension raw et un bloc d'information KryoFlux complet dans les 512 premiers octets. Les fichiers du même préfixe sont classés par cylindre/tête, avec les limites 99/1. Les chemins associés excluent la source principale ; leur longueur totale est conservée. La protection du document vaut true, comme en C#.

Le premier parcours récupère index, métadonnées et horloge sck. Les clés sont sensibles à la casse et leur première valeur reste conservée ; chaque dernière valeur sck analysable, sans distinction de casse, remplace l'horloge de décodage. Une horloge nulle, négative ou non finie échoue. Les chaînes UTF-8, espaces et caractères nuls finaux sont traités avec Qt.

Le second parcours gère les impulsions courtes, directes et longues, NOP1/2/3, les débordements de 16 bits, les blocs hors bande, positions de flux et fin de fichier. Les positions d'index désignent les octets du flux sans les blocs hors bande. Les intervalles sont convertis en nanosecondes avec arrondi au plus proche, les demi-unités vers l'extérieur. Les valeurs non positives, hors plage et durées supérieures à uint32 sont refusées.

Avec plusieurs index, seuls les segments complets entre index sont retenus. Sinon les segments non vides disponibles servent de repli. La durée est la somme des intervalles, conformément au C# ; aucune correction du modèle d'index n'a été inventée pendant ce portage. Les lectures, décodages et parcours honorent l'annulation.

La composition globale des lecteurs et l'ouverture depuis App restent à terminer.
